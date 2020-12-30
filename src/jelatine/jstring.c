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

/** \file jstring.c
 * Java String manager implementation */

#include "wrappers.h"

#include "class.h"
#include "jstring.h"
#include "memory.h"
#include "thread.h"
#include "utf8_string.h"
#include "util.h"

/******************************************************************************
 * Type definitions                                                           *
 ******************************************************************************/

/** Global manager for Java strings */

struct jstring_manager_t {
    struct class_t *str_cl; ///< java.lang.String class pointer
    struct class_t *char_array_cl; ///< [C character array class pointer

    uint32_t load; ///< Maximum load for Java strings
    uint32_t entries; ///< Used entries for Java strings
    uint32_t capacity; ///< Capacity for Java strings
    uint32_t init_capacity; ///< Initial capacity
    java_lang_String_t **buckets; ///< Buckets array for Java strings

    uint32_t lit_load; ///< Maximum load for Java literals
    uint32_t lit_entries; ///< Used entries for Java literals
    uint32_t lit_capacity; ///< Capacity for Java literals
    uint32_t lit_init_capacity; ///< Initial capacity
    java_lang_String_t **lit_buckets; ///< Buckets array for Java literals
};

/** Typedef for struct jstring_manager_t */
typedef struct jstring_manager_t jstring_manager_t;

/******************************************************************************
 * Local declarations                                                         *
 ******************************************************************************/

/** Global Java strings manager */
static jstring_manager_t jsm;

/******************************************************************************
 * Local function prototypes                                                  *
 ******************************************************************************/

static void jsm_rehash(uint32_t);
static void jsm_rehash_literals(uint32_t);

static uint32_t jstring_hash(const uint16_t *, uint32_t, uint32_t);
static bool jstring_equals(const java_lang_String_t *,
                           const java_lang_String_t *);

/******************************************************************************
 * Java string manager implementation                                         *
 ******************************************************************************/

/** Creates and initializes the Java string manager with an initial capacity
 * of 2 ^ \a log2cap capacity entries
 * \param log2cap The binary logarythm of the initial capacity
 * \param load Maximum load before a rehash is needed */

void jsm_init(uint32_t log2cap, uint32_t load)
{
    assert(log2cap != 0);
    assert(log2cap < 32);
    assert(load != 0);

    jsm.load = load;
    jsm.entries = 0;
    jsm.capacity = 1 << log2cap;
    jsm.init_capacity = 1 << log2cap;
    jsm.buckets = gc_malloc(jsm.capacity * sizeof(java_lang_String_t *));

    jsm.lit_load = load;
    jsm.lit_entries = 0;
    jsm.lit_capacity = 1 << log2cap;
    jsm.lit_init_capacity = 1 << log2cap;
    jsm.lit_buckets = gc_malloc(jsm.lit_capacity *
                                sizeof(java_lang_String_t *));
} // jsm_init()

/** Passes the pointers to the java.lang.String class and the [C array class to
 * the Java string manager, the manager is not really functional until this
 * second initialization step has been done
 * \param str_cl A pointer to the java.lang.String class
 * \param char_array_cl A pointer to the [C class */

void jsm_set_classes(class_t *str_cl, class_t *char_array_cl)
{
    jsm.str_cl = str_cl;
    jsm.char_array_cl = char_array_cl;
} // jsm_init_cl()

/** Marks the Java literals present in the manager. This function doesn't
 * acquire the VM lock as it can be called only when all the threads have
 * been stopped */

void jsm_mark( void )
{
    java_lang_String_t *str;
    uint32_t i;

    for (i = 0; i < jsm.lit_capacity; i++) {
        str = jsm.lit_buckets[i];

        while (str != NULL) {
            gc_mark_reference(JAVA_LANG_STRING_PTR2REF(str));
            str = str->next;
        }
    }
} // jsm_mark()

/** Purge the Java string manager from all the unused Java strings. This
 * function must be called after jsm_mark(). This function doesn't acquire the
 * VM lock as it can be called only when all the threads have been stopped */

void jsm_purge( void )
{
    java_lang_String_t *prev;
    java_lang_String_t *curr;
    java_lang_String_t list;
    jword_t used = 0;

    for (size_t i = 0; i < jsm.capacity; i++) {
        prev = &list;
        list.next = NULL;
        curr = (java_lang_String_t *) jsm.buckets[i];

        while (curr != NULL) {
            if (header_is_marked(&(curr->header))) {
                prev = curr;
                used++;
            } else {
                prev->next = curr->next;
            }

            curr = curr->next;
        }

        jsm.buckets[i] = list.next;
    }

    jsm.entries = used;
} // jsm_purge()

/** Rehashes the Java string manager. This function doesn't acquire the
 * VM lock as it can be called only when all the threads have been stopped
 * \param capacity The new capacity */

static void jsm_rehash(uint32_t capacity)
{
    java_lang_String_t **buckets;
    java_lang_String_t *str;
    uint32_t hash;

    buckets = gc_malloc(capacity * sizeof(java_lang_String_t *));

    for (size_t i = 0; i < jsm.capacity; i++) {
        str = jsm.buckets[i];

        while (str != NULL) {
            java_lang_String_t *tmp;

            // Interned stings always have their cachedHashCode field set
            hash = str->cachedHashCode;
            hash &= capacity - 1;

            tmp = str->next;
            str->next = buckets[hash];
            buckets[hash] = str;

            str = tmp;
        }
    }

    gc_free(jsm.buckets);
    jsm.buckets = buckets;
    jsm.capacity = capacity;
} // jsm_rehash()

/** Rehashes the Java string manager literals hash-table. This function doesn't
 * acquire the VM lock as it can be called only when all the threads have been
 * stopped
 * \param capacity The new capacity */

static void jsm_rehash_literals(uint32_t capacity)
{
    java_lang_String_t **buckets;
    java_lang_String_t *str;
    uint32_t hash;
    uint32_t i;

    buckets = gc_malloc(capacity * sizeof(java_lang_String_t *));

    for (i = 0; i < jsm.lit_capacity; i++) {
        str = jsm.lit_buckets[i];

        while (str != NULL) {
            java_lang_String_t *tmp;

            // Interned stings always have their cachedHashCode field set
            hash = str->cachedHashCode;
            hash &= capacity - 1;

            tmp = str->next;
            str->next = buckets[hash];
            buckets[hash] = str;

            str = tmp;
        }
    }

    gc_free(jsm.lit_buckets);
    jsm.lit_buckets = buckets;
    jsm.lit_capacity = capacity;
} // jsm_rehash_literals()

/** Interns a java.lang.String object, helper function used for implementing
 * java.lang.String.intern()
 * \param jstr A pointer to a java.lang.String object
 * \returns A pointer to the interned string */

java_lang_String_t *jstring_intern(java_lang_String_t *jstr)
{
    java_lang_String_t *curr;
    uint16_t *data = array_get_data(jstr->value);
    uint32_t count = jstr->count;
    uint32_t offset = jstr->offset;
    uint32_t hash;

    if (jstr->cachedHashCode == 0) {
        jstr->cachedHashCode = jstring_hash(data, offset, count);
    }

    tm_lock();

    // Check in the literal table
    hash = jstr->cachedHashCode;
    hash &= jsm.lit_capacity - 1;
    curr = jsm.lit_buckets[hash];

    while (curr != NULL) {
        if (jstring_equals(jstr, curr)) {
            tm_unlock();
            return curr;
        } else {
            curr = curr->next;
        }
    }

    // Check in the intern table
    hash = jstr->cachedHashCode;
    hash &= jsm.capacity - 1;
    curr = jsm.buckets[hash];

    while (curr != NULL) {
        if (jstring_equals(jstr, curr)) {
            tm_unlock();
            return curr;
        } else {
            curr = curr->next;
        }
    }

    // We haven't found a matching string, add the current one to the manager
    hash = jstr->cachedHashCode;
    hash &= jsm.capacity - 1;

    jstr->next = jsm.buckets[hash];
    jsm.buckets[hash] = jstr;
    jsm.entries++;

    if (jsm.entries > jsm.capacity * jsm.load) {
        jsm_rehash(jsm.capacity * 2);
    } else if ((jsm.entries < (jsm.capacity / 2) * jsm.load)
               && (jsm.capacity > jsm.init_capacity))
    {
        /* Though we are adding an entry to the manager a collection might
         * have reclaimed some entries before so we also check if we must
         * shrink the manager, we don't do this in jsm_purge() because that
         * function is called during a garbage collection */
        jsm_rehash(jsm.capacity / 2);
    }

    tm_unlock();
    return jstr;
} // jstring_intern()

/** Creates a new Java string literal from an utf8 string and interns it
 * in the literal hash-table
 * \param src A pointer to the source string
 * \returns A Java reference to the newly created string */

java_lang_String_t *jstring_create_literal(const char *src)
{
    java_lang_String_t *str;
    uintptr_t value, ref;
    uint32_t len, hash, cached_hash_code;
    uint16_t *data, *str_data;

    if (!utf8_check(src)) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "UTF8 string contains invalid characters");
    }

    len = utf8_to_java_length(src);
    value = gc_new_array_nonref(T_CHAR, len);
    data = array_get_data((array_t *) value);
    utf8_to_java(data, src);
    cached_hash_code = jstring_hash(data, 0, len);

    tm_lock();
    hash = cached_hash_code & (jsm.lit_capacity - 1);

    // Check if we can find this literal in the hash-table of interned strings
    str = (java_lang_String_t *) jsm.lit_buckets[hash];

    while (str != NULL) {
        // Note that the interned string must be a literal, thus permanenent
        if (str->count == len) {
            str_data = array_get_data(str->value);

            if (memcmp(data, str_data, len * sizeof(uint16_t)) == 0) {
                // This literal is already present, just return it
                tm_unlock();
                return str;
            }
        }

        str = (java_lang_String_t *) str->next;
    }

    // Prevent the value array from being accidentally collected
    thread_push_root(&value);
    ref = gc_new(jsm.str_cl);
    str = JAVA_LANG_STRING_REF2PTR(ref);
    thread_pop_root();

    str->value = (array_t *) value;
    str->count = len;
    str->offset = 0;
    str->cachedHashCode = cached_hash_code;

    // Add the string to the literal hash-table
    str->next = jsm.lit_buckets[hash];
    jsm.lit_buckets[hash] = str;
    jsm.lit_entries++;

    if (jsm.lit_entries > (jsm.lit_capacity * jsm.lit_init_capacity)) {
        thread_push_root(&ref);
        jsm_rehash_literals(jsm.lit_capacity * 2);
        thread_pop_root();
    }

    tm_unlock();
    return str;
} // jstring_create_literal()

/** Creates a new Java string from an utf8 string
 * \param src A pointer to the source string
 * \returns A Java reference to the newly created string */

java_lang_String_t *jstring_create_from_utf8(const char *src)
{
    uintptr_t value;
    uint32_t len;

    java_lang_String_t *str;

    if (!utf8_check(src)) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "UTF8 string contains invalid characters");
    }

    len = utf8_to_java_length(src);

    if (len == 0) {
        value = JNULL;
    } else {
        value = gc_new_array_nonref(T_CHAR, len);
        thread_push_root(&value);
    }

    utf8_to_java(array_get_data((array_t *) value), src);
    str = JAVA_LANG_STRING_REF2PTR(gc_new(jsm.str_cl));

    if (len != 0) {
        thread_pop_root();
    }

    str->value = (array_t *) value;
    str->count = len;
    str->offset = 0;
    str->cachedHashCode = 0;
    str->next = NULL;

    return str;
} // jstring_create_from_utf8()

/** Creates a new Java string from an Unicode array
 * \param uchar A pointer to the source array
 * \param length The string length
 * \returns A Java reference to the newly created string */

java_lang_String_t *jstring_create_from_unicode(const uint16_t *uchar,
                                                uint32_t length)
{
    java_lang_String_t *str;
    uintptr_t value;

    if (length == 0) {
        value = JNULL;
    } else {
        value = gc_new_array_nonref(T_CHAR, length);
        thread_push_root(&value);
    }

    memcpy(array_get_data((array_t *) value), uchar, length * sizeof(uint16_t));
    str = JAVA_LANG_STRING_REF2PTR(gc_new(jsm.str_cl));

    if (length != 0) {
        thread_pop_root();
    }

    str->value = (array_t *) value;
    str->count = length;
    str->offset = 0;
    str->cachedHashCode = 0;
    str->next = NULL;

    return str;
} // jstring_create_from_unicode()

/** Calculates the hash of a Java string
 * \param data The source array with the string character's
 * \param offset The offset
 * \param count The length
 * \returns The computed hash for this string */

static uint32_t jstring_hash(const uint16_t *data, uint32_t offset,
                             uint32_t count)
{
    uint32_t hash = 0;
    uint32_t limit = count + offset;

    for (size_t i = offset; i < limit; i++) {
        hash = (hash * 31) + data[i];
    }

    return hash;
} // jstring_hash()

/** Compares two java strings
 * \param str1 A pointer to a Java string
 * \param str2 A pointer to a Java string */

static bool jstring_equals(const java_lang_String_t *str1,
                           const java_lang_String_t *str2)
{
    const uint16_t *data1, *data2;

    if (str1 == str2) {
        return true;
    }

    if (str1->count == str2->count) {
        data1 = array_get_data(str1->value);
        data2 = array_get_data(str2->value);

        if (memcmp(data1 + str1->offset, data2 + str2->offset,
                    str1->count * sizeof(uint16_t)) == 0)
        {
            return true;
        }
    }

    return false;
} // jstring_equals()

#if JEL_PRINT

/** Prints a Java string to the standard output
 * \param str A pointer to a Java string */

void jstring_print(java_lang_String_t *str)
{
    uint16_t *data = array_get_data(str->value);
    char *tmp = java_to_utf8(data + str->offset, str->count);

    printf(tmp);
    gc_free(tmp);
} // jstring_print()

#endif // JEL_PRINT
