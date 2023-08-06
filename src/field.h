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

/** \file field.h
 * Field structure and interface */

/** \def JELATINE_FIELD_H
 * class.h inclusion macro */

#ifndef JELATINE_FIELD_H
#   define JELATINE_FIELD_H (1)

#include "wrappers.h"

#include "classfile.h"

// Forward declarations

struct class_t;

/******************************************************************************
 * Field types definitions                                                    *
 ******************************************************************************/

/** Internal field representation */

struct field_t {
    const char *name; ///< Name pointer
    const char *descriptor; ///< Descriptor pointer
    uint16_t access_flags; ///< Access flags
    int16_t offset; ///< Offset in bytes
};

/** Typedef for struct field_t */
typedef struct field_t field_t;

/** Static field data */

struct static_field_t {
    union {
        int8_t jbyte;
        uint16_t jchar;
        int16_t jshort;
        int32_t jint;
        int64_t jlong;
#if JEL_FP_SUPPORT
        float jfloat;
        double jdouble;
#endif // JEL_FP_SUPPORT
        uintptr_t jref;
    } data; ///< Actual field data
    field_t *field; ///< Related field structure
};

/** Typedef for struct static_field_t */
typedef struct static_field_t static_field_t;

/******************************************************************************
 * Field interface                                                            *
 ******************************************************************************/

extern size_t field_size(const field_t *);
extern void field_parse_descriptor(const char *);
extern uintptr_t static_field_data_ptr(static_field_t *);

/******************************************************************************
 * Field inlined functions                                                    *
 ******************************************************************************/

/**  Checks if a field is static
 * \param field A pointer to a field_t structure
 * \returns true if the field is static, false otherwise */

static inline bool field_is_static(const field_t *field)
{
    return field->access_flags & ACC_STATIC;
} // field_is_static()

/**  Checks if a field is private
 * \param field A pointer to a field_t structure
 * \returns true if the field is private, false otherwise */

static inline bool field_is_private(const field_t *field)
{
    return field->access_flags & ACC_PRIVATE;
} // field_is_private()

/**  Checks if a field is protected
 * \param field A pointer to a field_t structure
 * \returns true if the field is protected, false otherwise */

static inline bool field_is_protected(const field_t *field)
{
    return field->access_flags & ACC_PROTECTED;
} // field_is_protected()

/**  Checks if a field is public
 * \param field A pointer to a field_t structure
 * \returns true if the field is public, false otherwise */

static inline bool field_is_public(const field_t *field)
{
    return field->access_flags & ACC_PUBLIC;
} // field_is_public()

/**  Checks if a field is a reference to an object
 * \param field A pointer to a field
 * \returns true if the field holds a reference to an object, false otherwise */

static inline bool field_is_reference(const field_t *field)
{
    return (field->descriptor[0] == '[') || (field->descriptor[0] == 'L');
} // field_is_public()

/******************************************************************************
 * Field iterators                                                            *
 ******************************************************************************/

/** An iterator for navigating the fields of a class */

struct field_iterator_t {
    field_t *next; ///< Next field
    field_t *end; ///< First pointer after the last field
    bool stat; ///< true when iterating over static fields, false otherwise
};

/** Typedef for the struct field_iterator_t */
typedef struct field_iterator_t field_iterator_t;

/******************************************************************************
 * Field iterators interface                                                  *
 ******************************************************************************/

extern field_iterator_t field_itr(struct class_t *, bool);
extern field_t *field_itr_get_next(field_iterator_t *);

/******************************************************************************
 * Field iterators inlined functions                                          *
 ******************************************************************************/

/** Returns an iterator for navigating the instance fields of a class
 * \param cl A pointer to a class
 * \returns An initialized iterator */

static inline field_iterator_t instance_field_itr(struct class_t *cl)
{
    return field_itr(cl, false);
} // instance_field_itr()

/** Returns an iterator for navigating the static fields of a class
 * \param cl A pointer to a class
 * \returns An initialized iterator */

static inline field_iterator_t static_field_itr(struct class_t *cl)
{
    return field_itr(cl, true);
} // static_field_itr()

/** Returns true if there are still fields which can be iterated over, false
 * otherwise
 * \param itr A field iterator */

static inline bool field_itr_has_next(field_iterator_t itr)
{
    return (itr.next != NULL);
} // field_itr_has_next()

#endif // !JELATINE_FIELD_H
