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

/** \file constantpool.c
 * Constant pool functions implementation */

#include "wrappers.h"

#include "class.h"
#include "constantpool.h"
#include "jstring.h"
#include "memory.h"
#include "utf8_string.h"
#include "util.h"

/******************************************************************************
 * Local declarations                                                         *
 ******************************************************************************/

/** Dummy constant pool used by array classes */
static const_pool_t dummy_cp = { 0, NULL, { 0 } };

/******************************************************************************
 * Constant pool implementation                                               *
 ******************************************************************************/

/** Reads a tag from the tag array
 * \param tags A pointer to the tag array
 * \param entry The entry to be read
 * \returns The requested entry */

static const_pool_type_t tag_read(uint8_t *tags, uint32_t entry)
{
    unsigned int offset = (entry % 2) * 4;

    return (tags[entry / 2] >> offset) & 0x0f;
} // tag_read()

/** Writes a tag in the tag array
 * \param tags A pointer to the tag array
 * \param entry The entry to be writtend to
 * \param data The new tag data */

static void tag_write(uint8_t *tags, uint32_t entry, const_pool_type_t data)
{
    unsigned int offset = (entry % 2) * 4;
    unsigned int mask = 0xf0 >> offset;

    tags[entry / 2] = (tags[entry / 2] & mask) | (data << offset);
} // tag_write()

/** Creates a new constant-pool from the specified class-file
 * \param cl A pointer to the class owning the constant pool
 * \param cf A pointer to an open class-file
 * \returns A pointer to the newly created constant-pool */

const_pool_t *cp_create(class_t *cl, class_file_t *cf)
{
    const_pool_t *cp;
    uint8_t *tags;
    jword_t *data;
    uint8_t tag;
    uint16_t entries, u2_data;
#if JEL_FP_SUPPORT
    uint32_t u4_data;
#endif // JEL_FP_SUPPORT
    uint64_t u8_data;
    size_t len;
#if JEL_FP_SUPPORT
    float f_data;
    double d_data;
#endif // JEL_FP_SUPPORT
    char *str;
    char *utf8_str;
    uintptr_t jstring;

    entries = cf_load_u2(cf);
    cp = gc_palloc(sizeof(const_pool_t) + sizeof(jword_t) * entries);
    cp->entries = entries;
    cp->tags = gc_palloc(size_div_inf(entries, 2));

    tags = cp->tags;
    data = cp->data;

    /* The first constant pool element is empty, we fill it with the class
     * pointer */
    tag_write(tags, 0, 0);
    data[0] = (uintptr_t) cl;

    for (size_t i = 1; i < entries; i++) {
        tag = cf_load_u1(cf);
        tag_write(tags, i, tag);

        switch (tag) {
            case CONSTANT_Utf8:
                len =  cf_load_u2(cf);
                str = (char *) gc_malloc(len + 1);

                for (size_t j = 0; j < len; j++) {
                    str[j] = cf_load_u1(cf);
                }

                utf8_str = utf8_intern(str, len);
                cp_data_set_ptr(data, i, utf8_str);
                gc_free(str);

                break;

            case CONSTANT_Integer:
                cp_data_set_int32(data, i, (int32_t) cf_load_u4(cf));
                break;

#if JEL_FP_SUPPORT
            case CONSTANT_Float:
                // We do the memcpy() to respect strict aliasing constraints
                u4_data = cf_load_u4(cf);
                memcpy(&f_data, &u4_data, sizeof(float));
                cp_data_set_float(data, i, f_data);
                break;
#endif // JEL_FP_SUPPORT

            case CONSTANT_Long:
                u8_data = (uint64_t) cf_load_u4(cf) << 32;
                u8_data |= cf_load_u4(cf);
                cp_data_set_int64(data, i, (int64_t) u8_data);
                /* 8-byte objects take two constant pool entries, make the next
                 * constant pool entry valid but unusable */
                i++;
                tag_write(tags, i, 0);

                break;

#if JEL_FP_SUPPORT
            case CONSTANT_Double:
#if WORDS_BIGENDIAN != FLOAT_WORDS_BIGENDIAN
                // We're on a weird ARM target with middle-endian doubles
                u8_data = cf_load_u4(cf);
                u8_data = (uint64_t) cf_load_u4(cf) << 32;
#else
                u8_data = (uint64_t) cf_load_u4(cf) << 32;
                u8_data |= cf_load_u4(cf);
#endif // WORDS_BIGENDIAN != FLOAT_WORDS_BIGENDIAN
                memcpy(&d_data, &u8_data, sizeof(double));
                cp_data_set_double(data, i, d_data);
                /* 8-byte objects take two constant pool entries, make the next
                 * constant pool entry valid but unusable */
                i++;
                tag_write(tags, i, 0);

                break;
#endif // JEL_FP_SUPPORT

            case CONSTANT_Class:
            case CONSTANT_String:
                u2_data = cf_load_u2(cf);
                cp_data_set_uint16(data, i, u2_data);
                break;

            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
                u2_data = cf_load_u2(cf);
                cp_data_set_fieldref_class(data, i, u2_data);
                u2_data = cf_load_u2(cf);
                cp_data_set_fieldref_name_and_type(data, i, u2_data);
                break;

            case CONSTANT_NameAndType:
                u2_data = cf_load_u2(cf);
                cp_data_set_name_and_type_name(data, i, u2_data);
                u2_data = cf_load_u2(cf);
                cp_data_set_name_and_type_descriptor(data, i, u2_data);
                break;

            default:
                c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                        "Unknown constant pool tag value");
        }
    }

    // Check and process the various string entries

    for (size_t i = 0; i < (size_t)entries - 1; i++) {
        tag = tag_read(tags, i);

        switch (tag) {
            case CONSTANT_Utf8:
                if (cp_data_get_ptr(data, i) == NULL) {
                    break;
                }

                if (!utf8_check(cp_data_get_ptr(data, i))) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "Malformed CONSTANT_Utf8 entry");
                }

                break;

            case CONSTANT_String:
                u2_data = cp_data_get_uint16(data, i);

                if ((u2_data >= entries) || (u2_data == 0)) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "Malformed CONSTANT_String entry, string_index is "
                            "out of bounds");
                }

                if (tag_read(tags, u2_data) != CONSTANT_Utf8) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "Malformed CONSTANT_String entry, string_index "
                            "doesn't point to a CONSTANT_Utf8 entry");
                }

                jstring = JAVA_LANG_STRING_PTR2REF(
                    jstring_create_literal(*((char **) (data + u2_data))));

                cp_data_set_uintptr(data, i, (uintptr_t) jstring);
                break;

            default:
                continue;
        }
    }

    return cp;
} // cp_create()

/** Creates a dummy, empty constant pool used by array classes
 * \returns A pointer to the dummy constant pool */

const_pool_t *cp_create_dummy( void )
{
    return &dummy_cp;
} // cp_create_dummy()

/** Gets the tag of an element of the constant pool
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The tag of the specified constant pool entry */

const_pool_type_t cp_get_tag(const_pool_t *cp, uint16_t entry)
{
    if (entry >= cp->entries) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Constant pool index out of bounds");
    } else {
        return tag_read(cp->tags, entry);
    }
} // cp_get_tag()

/** Sets the tag and data of an element of the constant pool, the data is a
 * generic pointer, this function is used for replacing index-based reference
 * with direct pointers after resolution
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \param tag The new tag
 * \param data The new data */

void cp_set_tag_and_data(const_pool_t *cp, uint16_t entry, uint8_t tag,
                         void *data)
{
    assert(entry < cp->entries);

    tag_write(cp->tags, entry, tag);
    cp_data_set_ptr(cp->data, entry, data);
} // cp_set_tag_and_data()

/** Gets the the name of a class, fails if the \a entry parameter is out
 * of range or if the relevant entry has a tag different from CONSTANT_Class
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The index of the class name contained in the specified entry */

char *cp_get_class_name(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t class_index;

    if (tag == CONSTANT_Class_resolved) {
        return ((class_t *) cp_data_get_ptr(cp->data, entry))->name;
    }

    if (tag != CONSTANT_Class) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Not a CONSTANT_Class entry");
    }

    class_index = cp_data_get_uint16(cp->data, entry);
    return cp_get_string(cp, class_index);
} // cp_get_class_name()

/** Gets a string stored in the specified entry of the constant pool,
 * fails if the \a entry parameter is out of range or if the relevant entry has
 * a tag different from CONSTANT_Utf8
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns A pointer to the string contained in the specified entry */

char *cp_get_string(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if (tag != CONSTANT_Utf8) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Not a CONSTANT_Utf8 entry");
    }

    return (char *) cp_data_get_ptr(cp->data, entry);
} // cp_get_string()

/** Gets a CONSTANT_String or CONSTANT_Class value in the specified entry of
 * the constant pool fails if the \a entry parameter is out of range or if the
 * relevant entry has a tag different from CONSTANT_String or CONSTANT_Class
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns A Java reference to the string */

uintptr_t cp_get_ref(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if ((tag != CONSTANT_String) && (tag != CONSTANT_Class)) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_String or CONSTANT_Class entry");
    }

    return cp_data_get_uintptr(cp->data, entry);
} // cp_get_ref()

/** Gets a CONSTANT_Integer value in the specified entry of the constant
 * pool fails if the \a entry parameter is out of range or if the relevant entry
 * has a tag different from CONSTANT_Integer
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The value of the integer contained in the specified entry */

int32_t cp_get_integer(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if (tag != CONSTANT_Integer) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Not a CONSTANT_Integer entry");
    }

    return cp_data_get_int32(cp->data, entry);
} // cp_get_integer()

#if JEL_FP_SUPPORT

/** Gets a constant float value in the specified entry of the constant
 * pool fails if the \a entry parameter is out of range or if the relevant entry
 * has a tag different from CONSTANT_Float
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The value of the float contained in the specified entry */

float cp_get_float(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if (tag != CONSTANT_Float) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Not a CONSTANT_Float entry");
    }

    return cp_data_get_float(cp->data, entry);
} // cp_get_float()

#endif // JEL_FP_SUPPORT

/** Gets a constant long value in the specified entry of the constant
 * pool fails if the \a entry parameter is out of range or if the relevant entry
 * has a tag different from CONSTANT_Long
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The value of the long contained in the specified entry */

int64_t cp_get_long(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if (tag != CONSTANT_Long) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Not a CONSTANT_Long entry");
    }

    return cp_data_get_int64(cp->data, entry);
} // cp_get_long()

#if JEL_FP_SUPPORT

/** Gets a constant double value in the specified entry of the constant
 * pool fails if the \a entry parameter is out of range or if the relevant entry
 * has a tag different from CONSTANT_Double
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The value of the double contained in the specified entry */

double cp_get_double(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if (tag != CONSTANT_Double) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Not a CONSTANT_Double entry");
    }

    return cp_data_get_double(cp->data, entry);
} // cp_get_double()

#endif // JEL_FP_SUPPORT

/** Gets the name from a CONSTANT_NameAndType entry
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The name of the class held by the specified entry */

char *cp_get_name_and_type_name(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t string_index;

    if (tag != CONSTANT_NameAndType) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_NameAndType entry");
    }

    string_index = cp_data_get_name_and_type_name(cp->data, entry);
    return cp_get_string(cp, string_index);
} // cp_get_name_and_type_name()

/** Gets the descriptor from a CONSTANT_NameAndType entry
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The name of the class held by the specified entry */

char *cp_get_name_and_type_type(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t string_index;

    if (tag != CONSTANT_NameAndType) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_NameAndType entry");
    }

    string_index = cp_data_get_name_and_type_descriptor(cp->data, entry);
    return cp_get_string(cp, string_index);
} // cp_get_name_and_type_name()

/** Gets the class index from a CONSTANT_Fieldref
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The index of the class held by the specified entry */

uint16_t cp_get_fieldref_class(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if (tag != CONSTANT_Fieldref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_Fieldref entry");
    }

    return cp_data_get_fieldref_class(cp->data, entry);
} // cp_get_fieldref_class()

/** Gets the field name from a CONSTANT_Fieldref
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The name of the field held by the specified entry */

char *cp_get_fieldref_name(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t name_and_type_index;

    if (tag != CONSTANT_Fieldref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_Fieldref entry");
    }

    name_and_type_index = cp_data_get_fieldref_name_and_type(cp->data, entry);
    return cp_get_name_and_type_name(cp, name_and_type_index);
} // cp_get_fieldref_name()

/** Gets the field descriptor from a CONSTANT_Fieldref
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The descriptor of the field held by the specified entry */

char *cp_get_fieldref_type(const_pool_t *cp, uint16_t entry) {
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t name_and_type_index;

    if (tag != CONSTANT_Fieldref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_Fieldref entry");
    }

    name_and_type_index = cp_data_get_fieldref_name_and_type(cp->data, entry);
    return cp_get_name_and_type_type(cp, name_and_type_index);
} // cp_get_fieldref_type()

/** Gets the method name from a CONSTANT_Methodref
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The name of the method held by the specified entry */

char *cp_get_methodref_name(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t name_and_type_index;

    if (tag != CONSTANT_Methodref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_Methodref entry");
    }

    name_and_type_index = cp_data_get_fieldref_name_and_type(cp->data, entry);
    return cp_get_name_and_type_name(cp, name_and_type_index);
} // cp_get_methodref_name()

/** Gets the method descriptor from a CONSTANT_Methodref structure
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The descriptor of the method held by the specified entry */

char *cp_get_methodref_descriptor(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t name_and_type_index;

    if (tag != CONSTANT_Methodref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_Methodref entry");
    }

    name_and_type_index = cp_data_get_fieldref_name_and_type(cp->data, entry);
    return cp_get_name_and_type_type(cp, name_and_type_index);
} // cp_get_methodref_descriptor()

/** Gets the class index from a CONSTANT_Methodref
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The class index held by the specified entry */

uint16_t cp_get_methodref_class(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if (tag != CONSTANT_Methodref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_Methodref entry");
    }

    return cp_data_get_fieldref_class(cp->data, entry);
} // cp_get_methodref_class()

/** Gets the method name from a CONSTANT_InterfaceMethodref
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The name of the method held by the specified entry */

char *cp_get_interfacemethodref_name(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t name_and_type_index;

    if (tag != CONSTANT_InterfaceMethodref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_InterfaceMethodref entry");
    }

    name_and_type_index = cp_data_get_fieldref_name_and_type(cp->data, entry);
    return cp_get_name_and_type_name(cp, name_and_type_index);
} // cp_get_interfacemethodref_name()

/** Gets the method descriptor from a CONSTANT_InterfaceMethodref structure
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The descriptor of the method held by the specified entry */

char *cp_get_interfacemethodref_descriptor(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);
    uint16_t name_and_type_index;

    if (tag != CONSTANT_InterfaceMethodref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_InterfaceMethodref entry");
    }

    name_and_type_index = cp_data_get_fieldref_name_and_type(cp->data, entry);
    return cp_get_name_and_type_type(cp, name_and_type_index);
} // cp_get_interfacemethodref_descriptor()

/** Gets the class index from a CONSTANT_InterfaceMethodref
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns The entry corresponding to the method's class */

uint16_t cp_get_interfacemethodref_class(const_pool_t *cp, uint16_t entry)
{
    const_pool_type_t tag = cp_get_tag(cp, entry);

    if (tag != CONSTANT_InterfaceMethodref) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Not a CONSTANT_InterfaceMethodref entry");
    }

    return cp_data_get_fieldref_class(cp->data, entry);
} // cp_get_interfacemethodref_class()

#ifdef JEL_PRINT

/** Print the constant pool contents to the specified file
 * \param cp A pointer to a valid constant pool object */

void cp_print(const_pool_t *cp)
{
    for (size_t i = 0; i < cp->entries; i++) {
        switch (tag_read(cp->tags, i)) {
            case 0:
                printf("#%zu: Empty entry\n", i);
                break;

            case CONSTANT_Utf8:
                printf("#%zu: CONSTANT_Utf8: %s\n",
                       i, (char *) cp_data_get_ptr(cp->data, i));
                break;

            case CONSTANT_Integer:
                printf("#%zu: CONSTANT_Integer: %d\n",
                       i, cp_data_get_int32(cp->data, i));
                break;

#if JEL_FP_SUPPORT
            case CONSTANT_Float:
                printf("#%zu: CONSTANT_Float: %f\n",
                       i, cp_data_get_float(cp->data, i));
                break;
#endif // JEL_FP_SUPPORT

            case CONSTANT_Long:
#if SIZEOF_LONG == 8
                printf("#%zu: CONSTANT_Long: %ld\n",
                       i, cp_data_get_int64(cp->data, i));
#else
                printf("#%zu: CONSTANT_Long: %lld\n",
                       i, cp_data_get_int64(cp->data, i));
#endif // SIZEOF_LONG == 8
                break;

#if JEL_FP_SUPPORT
            case CONSTANT_Double:
                printf("#%zu: CONSTANT_Double: %f\n",
                       i, cp_data_get_double(cp->data, i));
                break;
#endif // JEL_FP_SUPPORT

            case CONSTANT_Class:
                printf("#%zu: CONSTANT_Class: %d\n",
                       i, cp_data_get_uint16(cp->data, i));
                break;

            case CONSTANT_String:
                printf("#%zu: CONSTANT_String: %p\n",
                       i, cp_data_get_ptr(cp->data, i));
                break;

            case CONSTANT_Fieldref:
                printf("#%zu: CONSTANT_Fieldref: %d, %d\n",
                       i, cp_data_get_fieldref_class(cp->data, i),
                       cp_data_get_fieldref_name_and_type(cp->data, i));
                break;

            case CONSTANT_Methodref:
                printf("#%zu: CONSTANT_Methodref: %d, %d\n",
                       i, cp_data_get_fieldref_class(cp->data, i),
                       cp_data_get_fieldref_name_and_type(cp->data, i));
                break;

            case CONSTANT_InterfaceMethodref:
                printf("#%zu: CONSTANT_InterfaceMethodref: %d, %d\n",
                       i, cp_data_get_fieldref_class(cp->data, i),
                       cp_data_get_fieldref_name_and_type(cp->data, i));
                break;

            case CONSTANT_NameAndType:
                printf("#%zu: CONSTANT_InterfaceMethodref: %d, %d\n",
                       i, cp_data_get_name_and_type_name(cp->data, i),
                       cp_data_get_name_and_type_descriptor(cp->data, i));
                break;

            default:
                printf("#%zu: Invalid entry!!!\n", i);
        }
    }
} // cp_print()

#endif // JEL_PRINT
