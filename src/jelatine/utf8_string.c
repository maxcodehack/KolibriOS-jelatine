/***************************************************************************
 *   Copyright © 2005-2011 by Gabriele Svelto                              *
 *   gabriele.svelto@gmail.com                                             *
 *                                                                         *
 *   This file is part of Jelatine.                                        *
 *                                                                         *
 *   Jelatine is free software: you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Jelatine is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with Jelatine.  If not, see <http://www.gnu.org/licenses/>.     *
 ***************************************************************************/

/** \file utf8_string.c
 * UTF8 string manager implementation */

#include "wrappers.h"

#include "memory.h"
#include "thread.h"
#include "utf8_string.h"
#include "util.h"
#include "vm.h"

/******************************************************************************
 * Type definitions                                                           *
 ******************************************************************************/

/** String in the (Java class file modified) UTF8 format
 *
 * The string data can be both in the plain UTF8 format or in its Java class
 * file variation, the string manager doesn't care about it, only the conversion
 * functions do. The ones going from the Java class file modified format start
 * with the jutf8_ prefix, the ones working on plain UTF8 strings begin with
 * the utf8_ prefix. Note that the structure holds only a pointer to the next
 * object in the hash table, and the data array is used as a C89-compatible
 * variable size array. */

struct utf8_string_t {
    struct utf8_string_t *next; ///< Next string in the bucket
    char data[]; ///< Variable size array holding the string data
};

/** Typedef for struct utf8_string_t */
typedef struct utf8_string_t utf8_string_t;

/** The string manager holds all the UTF8 strings used by the VM */

struct string_manager_t {
    size_t load; ///< Maximum load
    size_t threshold; ///< Rehash threshold value
    size_t entries; ///< Used entries
    size_t capacity; ///< Capacity
    utf8_string_t **buckets; ///< Buckets array
};

/** Typedef for the struct string_manager_t */
typedef struct string_manager_t string_manager_t;

/******************************************************************************
 * Local declarations                                                         *
 ******************************************************************************/

/** Global string manager */
static string_manager_t sm;

/******************************************************************************
 * Local functions prototypes                                                 *
 ******************************************************************************/

static void string_manager_rehash(size_t);
static uint32_t utf8_hash(const char *, size_t);

/******************************************************************************
 * String manager implementation                                              *
 ******************************************************************************/

/** Creates and initializes the string manager with an initial capacity of
 * 2^(\a log2cap) entries and \a load load
 * \param log2cap The binary logarythm of the initial capacity
 * \param load The allowed load on the manager */

void string_manager_init(size_t log2cap, size_t load)
{
    assert(log2cap != 0);
    assert(log2cap < 32);
    assert(load != 0);

    sm.capacity = 1 << log2cap;
    sm.threshold = 4;
    sm.load = load;
    sm.entries = 0;

    sm.buckets = gc_malloc(sm.capacity * sizeof(utf8_string_t *));
} // string_manager_init()

/** Rehashes the string manager pointed by \a sm
 * \param capacity The new capacity */

static void string_manager_rehash(size_t capacity)
{
    utf8_string_t **buckets;
    utf8_string_t *str, *next;
    char *data;
    size_t len;
    uint32_t hash;

    buckets = gc_malloc(capacity * sizeof(utf8_string_t *));

    for (size_t i = 0; i < sm.capacity; i++) {
        str = sm.buckets[i];

        while (str != NULL) {
            data = str->data;
            len = strlen(data);

            /* Mask the hash to extract the bits that fit the current table
             * capacity */
            hash = utf8_hash(data, len) & (capacity - 1);
            next = str->next;
            str->next = buckets[hash];
            buckets[hash] = str;
            str = next;
        }
    }

    gc_free(sm.buckets);
    sm.buckets = buckets;
    sm.capacity = capacity;
} // string_manager_rehash()

/** Checks if a string in the Java class file modified UTF8 format is valid,
 * i.e. checks if the length of the string matches the given one and that the
 * string doesn't contain invalid characters
 * \param src A pointer to the string to be checked
 * \returns True if the string is valid, false otherwise */

bool utf8_check(const char *src)
{
    size_t i = 0;

    while (src[i] != '\0') {
        switch ((unsigned char) src[i] >> 4) {
            case 0x8:
            case 0x9:
            case 0xA:
            case 0xB:
                return false;

            case 0xC:
            case 0xD: // 110xxxxx 10xxxxxx
                if ((src[i + 1] & 0xC0) != 0x80) {
                    return false;
                }

                i += 2;
                break;

            case 0xE: // 1110xxxx 10xxxxxx 10xxxxxx
                if ((src[i + 1] & 0xC0) != 0x80) {
                    return false;
                }

                if ((src[i + 2] & 0xC0) != 0x80) {
                    return false;
                }

                i += 3;
                break;

            case 0xF:
                return false;

            default:
                i++;
                break;
        }
    }

    return true;
} // utf8_check()

/** Interns a string in the UTF8 format in the string manager and returns a
 * pointer to the interned string.
 *
 * Empty strings may be interned by specifying a NULL pointer for \a src and a
 * 0 value for \a len. Strings in the Java class file modified format are
 * treated as plain UTF8 strings.
 *
 * \param src A pointer to an array of bytes, may be NULL
 * \param len The string length
 * \returns A pointer to the newly created string */

char *utf8_intern(const char *src, size_t len)
{
    utf8_string_t *str;
    char *empty = "";
    uint32_t hash;

    if (len == 0) {
        src = empty;
    }

    tm_lock(); // Only one thread at a time can intern an UTF-8 string

    // Mask the hash to save the bits that fit the current table capacity
    hash = utf8_hash(src, len) & (sm.capacity - 1);
    str = sm.buckets[hash];

    while (str != NULL) {
        if (strcmp(str->data, src) == 0) {
            break;
        } else {
            str = str->next;
        }
    }

    if (str == NULL) {
        str = gc_palloc(sizeof(utf8_string_t) + len + 1);
        memcpy(str->data, src, len);
        // No zero termination as the memory has already been cleared
        str->next = sm.buckets[hash];
        sm.buckets[hash] = str;
        sm.entries++;

        /* Since interned UTF8 strings usually belong to a class' constant pool
         * they are permanent objects, the manager's hash-table is always grown
         * when rehashed and never shrinks */
        if (sm.entries > (sm.capacity * sm.load)) {
            string_manager_rehash(sm.capacity << 1);
        }
    }

    tm_unlock();
    return str->data;
} // utf8_intern()

/** Calculates the length of a string in the Java class file modified UTF8
 * format when converted to its Java representation. The functions excepts the
 * source string to be valid.
 * \param str A pointer to a string in the Java class file modified UTF8 format
 * \returns The length of the string once converted */

size_t utf8_to_java_length(const char *str)
{
    size_t i = 0;
    size_t len = 0;

    while (str[i] != 0) {
        len++;

        if (str[i] & 0x80) {
            if (str[i] & 0x20) { // 1110xxxx 10xxxxxx 10xxxxxx
                i += 3;
            } else { // 110xxxxx 10xxxxxx
                i += 2;
            }
        } else {
            i++;
        }
    }

    return len;
} // utf8_to_java_length()

/** Converts a string in the Java class file modified UTF8 format into a Java
 * string. The java chars of the resulting string are written in \a dst.
 * \param dst A pointer to an array of Java chars
 * \param src A pointer to a string in the Java class file modified UTF8 format
 */

void utf8_to_java(uint16_t *dst, const char *src)
{
    size_t i = 0;
    size_t j = 0;

    while (src[i] != '\0') {
        switch ((unsigned char) src[i] >> 4) {
            case 0xc:
            case 0xd: // 110xxxxx 10xxxxxx
                dst[j] = ((src[i] & 0x1f) << 6) | (src[i + 1] & 0x3f);
                i += 2;
                break;

            case 0xe: // 1110xxxx 10xxxxxx 10xxxxxx
                dst[j] = (((src[i]) & 0xf) << 12)
                         | ((src[i + 1] & 0x3f) << 6) | (src[i + 2] & 0x3f);
                i += 3;
                break;

            default:
                dst[j] = src[i];
                i++;
                break;
        }

        j++;
    }
} // utf8_to_java()

/** Converts a Java string into an UTF-8 string.
 * \param data The string's data
 * \param length The string length
 * \returns A newly created UTF-8 string allocated from the memory heap */

char *java_to_utf8(const uint16_t *data, size_t length)
{
    size_t utf8_length = 1; // At least the nul character is needed
    uint16_t jc;
    char *str;

    for (size_t i = 0; i < length; i++) {
        jc = data[i];

        // FIXME: Add support for surrogates
        if ((jc >= 0x0001) && (jc <= 0x007f)) {
            utf8_length++;
        } else if ((jc == 0) || ((jc >= 0x0080) && (jc <= 0x07ff))) {
            utf8_length += 2;
        } else if (jc >= 0x0800) {
            utf8_length += 3;
        }
    }

    str = gc_malloc(utf8_length);
    str[utf8_length - 1] = 0; // Zero termination

    for (size_t i = 0, j = 0; i < length; i++) {
        jc = data[i];

        // FIXME: Add support for surrogates
        if ((jc >= 0x0001) && (jc <= 0x007f)) {
            str[j++] = jc;
        } else if ((jc == 0) || ((jc >= 0x0080) && (jc <= 0x07ff))) {
            str[j++] = 0xc0 | ((jc >> 6) & 0x1f);
            str[j++] = 0x80 | (jc & 0x3f);
        } else if (jc >= 0x0800) {
            str[j++] = 0xe0 | (jc >> 12);
            str[j++] = 0x80 | ((jc >> 6) & 0x3f);
            str[j++] = 0x80 | (jc & 0x3f);
        }
    }

    return str;
} // java_to_utf8()

/** Turns a class or package name in the classfile internal format
 * \param src The source string
 * \returns The pointer passed to this function */

char *utf8_slashify(char *src)
{
    size_t len = strlen(src);

    for (size_t i = 0; i < len; i++) {
        if (src[i] == '.') {
            src[i] = '/';
        }
    }

    return src;
} // utf8_slashify()

/** Calculates the hash value of an UTF-8 string
 * \param data The string's data
 * \param len The number of characters in the string */

static uint32_t utf8_hash(const char *data, size_t len)
{
    uint32_t hash = 5381;

    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) ^ data[i];
    }

    return hash;
} // utf8_hash()

#ifndef NDEBUG

/** Prints the total memory consumption of the UTF8 string manager */

size_t string_manager_size( void )
{
    utf8_string_t *str;
    size_t size;

    size = sizeof(string_manager_t) + sizeof(jword_t);
    size += (sizeof(utf8_string_t *) * sm.capacity) + sizeof(jword_t);

    for (size_t i = 0; i < sm.capacity; i++) {
        str = sm.buckets[i];

        while (str != NULL) {
            size += offsetof(utf8_string_t, data)
                    + size_ceil(strlen(str->data) + 1, sizeof(jword_t))
                    + sizeof(jword_t);
            str = str->next;
        }
    }

    return size;
} // string_manager_size()

/** Prints out the contents of the UTF8 string manager */

void string_manager_print( void )
{
    utf8_string_t *str;

    printf("sm = \n"
           "    load = %zu\n"
           "    threshold = %zu\n"
           "    entries = %zu\n"
           "    capacity = %zu\n"
           "    buckets = %p\n",
           sm.load, sm.threshold, sm.entries, sm.capacity, (void *) sm.buckets);

    for (size_t i = 0; i < sm.capacity; i++) {
        str = sm.buckets[i];

        printf("    buckets[%zu] = \n", i);

        while (str != NULL) {
            printf("\t%s\n", str->data);
            str = str->next;
        }
    }
} // print_string_manager()

#endif // !NDEBUG
