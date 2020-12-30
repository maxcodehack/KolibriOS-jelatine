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

/** \file field.c
 * Field structure implementation */

#include "wrappers.h"

#include "class.h"
#include "field.h"
#include "util.h"

/******************************************************************************
 * Field implementation                                                       *
 ******************************************************************************/

/** Returns the size in bytes of a field
 * \param field A pointer to a field
 * \returns The size in bytes of the field */

size_t field_size(const field_t *field)
{
    switch (field->descriptor[0]) {
        case '[': // FALLTHROUGH
        case 'L':
            return SIZEOF_VOID_P;

        case 'B': // FALLTHROUGH
        case 'Z':
            return 1;

        case 'C': // FALLTHROUGH
        case 'S':
            return 2;

        case 'I': // FALLTHROUGH
#if JEL_FP_SUPPORT
        case 'F':
#endif // JEL_FP_SUPPORT
            return 4;

        case 'J': // FALLTHROUGH
#if JEL_FP_SUPPORT
        case 'D':
#endif // JEL_FP_SUPPORT
            return 8;

        default:
            dbg_unreachable();
            return -1;
    }
} // field_size()

/** Checks if a field descriptor is valid
 * \param desc A field descriptor */

void field_parse_descriptor(const char *desc)
{
    uint32_t i, j; // A couple of counters
    uint32_t dimensions; // Dimensions of an array

    i = 0;

    if (desc[0] == 0) {
        goto error;
    }

    switch (desc[i]) {
        case 'B':
        case 'C':
#if JEL_FP_SUPPORT
        case 'D':
        case 'F':
#endif // JEL_FP_SUPPORT
        case 'I':
        case 'J':
        case 'S':
        case 'Z':
            i++;
            goto basic;

        case 'L':
            j = i + 1;

            while (desc[j] != ';' && desc[j] != 0) {
                j++;
            }

            if (desc[j] == 0) {
                goto error;
            }

            return;

        case '[':
            dimensions = 0;

            while (desc[i] == '[') {
                i++;
                dimensions++;

                if ((desc[i] == 0) || dimensions > 255) {
                    goto error;
                }
            }

            switch (desc[i]) {
                case 'B':
                case 'C':
#if JEL_FP_SUPPORT
                case 'D':
                case 'F':
#endif // JEL_FP_SUPPORT
                case 'I':
                case 'J':
                case 'S':
                case 'Z':
                    if (desc[i + 1] != 0) {
                        goto error;
                    }

                    // Basic array class
                    return;

                case 'L':
                    while (desc[i] != ';' && desc[i] != 0) {
                        i++;
                    }

                    if (desc[i] == 0) {
                        goto error;
                    }

                    return;

                default:
                    goto error;
            }

        default:
            goto error;
    }

basic:
    if (desc[i] == 0) {
        return;
    }

error:
    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Invalid field descriptor");
} // field_parse_descriptor()

/** Returns a pointer to the actual data of a static field
 * \param static_field A pointer to a static field
 * \returns The address of the static field data */

uintptr_t static_field_data_ptr(static_field_t *static_field)
{
    switch (static_field->field->descriptor[0]) {
        case 'L': // FALLTHROUGH
        case '[': return (uintptr_t) &(static_field->data.jref);
        case 'B': // FALLTHROUGH
        case 'Z': return (uintptr_t) &(static_field->data.jbyte);
        case 'C': return (uintptr_t) &(static_field->data.jchar);
        case 'S': return (uintptr_t) &(static_field->data.jshort);
        case 'I': return (uintptr_t) &(static_field->data.jint);
        case 'J': return (uintptr_t) &(static_field->data.jlong);
#if JEL_FP_SUPPORT
        case 'F': return (uintptr_t) &(static_field->data.jfloat);
        case 'D': return (uintptr_t) &(static_field->data.jdouble);
#endif // JEL_FP_SUPPORT
        default:
            dbg_unreachable();
            return (uintptr_t) NULL;
    }
} // static_field_data_ptr()

/******************************************************************************
 * Field iterators implementation                                             *
 ******************************************************************************/

/** Find the next field to iterate on unless there are no more available
 * \param curr The current field
 * \param end A pointer past the last field
 * \param stat true if a static field is wanted, false otherwise
 * returns A pointer to the next field or NULL if no more are available */

static field_t *field_itr_find_next(field_t *curr, field_t *end, bool stat)
{
    while (curr < end) {
        if (field_is_static(curr) == stat) {
            return curr;
        }

        curr++;
    }

    return NULL;
} // field_itr_find_next()

/** Returns an iterator for navigating the instance or static fields of a class
 * depending on the value of stat
 * \param cl A pointer to a class
 * \param stat true if a static field iterator is needed, false otherwise
 * \returns An initialized iterator */

field_iterator_t field_itr(class_t *cl, bool stat)
{
    field_iterator_t itr;
    field_t *end = cl->fields + cl->fields_n;

    itr.next = field_itr_find_next(cl->fields, end, stat);
    itr.end = end;
    itr.stat = stat;

    return itr;
} // field_itr()

/** Returns the next field of this iteration
 * \param itr A field iterator
 * \returns The next field available */

field_t *field_itr_get_next(field_iterator_t *itr)
{
    field_t *field = itr->next;

    assert(field_itr_has_next(*itr));
    itr->next = field_itr_find_next(field + 1, itr->end, itr->stat);

    return field;
} // field_itr_get_next()
