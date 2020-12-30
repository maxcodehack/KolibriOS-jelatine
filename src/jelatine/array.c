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

/** \file array.c
 * Array methods' implementation */

#include "wrappers.h"

#include "array.h"
#include "class.h"
#include "kni.h"
#include "loader.h"
#include "memory.h"
#include "thread.h"
#include "util.h"

/******************************************************************************
 * Globals                                                                    *
 ******************************************************************************/

/** This array holds the names of the primitive-type arrays */

const char *array_names[] = {
    "[Z",
    "[C",
    "[F",
    "[D",
    "[B",
    "[S",
    "[I",
    "[J"
};

/** This array holds the size in bytes of the basic array element types.
 * T_BOOLEAN is a special case as it set as having an element size of 1 byte
 * per element when in fact it uses 1 bit per element */

uint8_t array_elem_sizes[] = {
    1, // T_BOOLEAN, special
    2, // T_CHAR
    4, // T_FLOAT
    8, // T_DOUBLE
    1, // T_BYTE
    2, // T_SHORT
    4, // T_INT
    8  // T_LONG
};

/** This array holds an ::array_type_t value for each ::primitive_type_t
 * element which can be used in a non-reference array */

uint8_t prim_to_array_types[] = {
    T_BYTE,    // PT_BYTE
    T_BOOLEAN, // PT_BOOL
    T_CHAR,    // PT_CHAR
    T_SHORT,   // PT_SHORT
    T_INT,     // PT_INT
    T_FLOAT,   // PT_FLOAT
    T_LONG,    // PT_LONG
    T_DOUBLE   // PT_DOUBLE
};

/******************************************************************************
 * Array implementation                                                       *
 ******************************************************************************/

/** Gets the size (in bytes) of the non-reference area of an array
 * \param array A pointer to the array header
 * \returns The size (in bytes) of the array's non-reference area */

size_t array_get_nref_size(array_t *array)
{
    class_t *cl = header_get_class(&(array->header));
    size_t size;

    if (cl->elem_type == PT_REFERENCE) {
        size = sizeof(ref_array_t) - sizeof(header_t);
    } else {
        size = sizeof(array_t) - sizeof(header_t);

        if (cl->elem_type == PT_BOOL) {
            size += size_div_inf(array->length, 8);
        } else {
            size += array->length
                    * array_elem_size(prim_to_array_type(cl->elem_type));
        }
    }

    return size;
} // array_get_nref_size()

/** Gets the number of references in an array
 * \param array A pointer to an array header
 * \returns The number of references in the array */

size_t array_get_ref_n(array_t *array)
{
    class_t *cl = header_get_class(&(array->header));

    assert(class_is_array(cl));

    if (cl->elem_type == PT_REFERENCE) {
        return array->length;
    } else {
        return 0;
    }
} // array_get_ref_n()

/** C implementation of java.lang.System.arraycopy() for reference arrays.
 * If an element is found not to be compatible with the destination array an
 * ArrayStoreException is thrown
 * \param src A pointer to the source array
 * \param src_offset The offset in the source array
 * \param dest A pointer to the destination array
 * \param dest_offset The offset in the destination array
 * \param length The number of elements to copy */

void arraycopy_ref(array_t *src, int32_t src_offset, array_t *dest,
                   int32_t dest_offset, int32_t length)
{
    class_t *src_cl;
    class_t *dest_cl = header_get_class(&(dest->header))->elem_class;
    uintptr_t *src_data = array_ref_get_data(src) - src_offset;
    uintptr_t *dest_data = array_ref_get_data(dest) - dest_offset;
    uintptr_t ref;
    int32_t i;

    if ((src == dest) && (src_offset < dest_offset)) {
        /* An intermediary copy is needed to ensure that the specification of
         * java.lang.System.arraycopy() is perfectly implemented */
        array_t *temp;
        uintptr_t *temp_data;

        temp = (array_t *) gc_new_array_ref(dest_cl, length);
        temp_data = array_ref_get_data(temp);

        for (i = 0; i < length; i++) {
            temp_data[-i] = src_data[-i];
        }

        src_data = temp_data;
    }

    for (i = 0; i < length; i++) {
        ref = src_data[-i];
        src_cl = header_get_class((header_t *) ref);

        if ((src_cl == dest_cl) || bcl_is_assignable(src_cl, dest_cl)) {
            dest_data[-i] = ref;
        } else {
            KNI_ThrowNew("java/lang/ArrayStoreException", NULL);
            return;
        }
    }
} // arraycopy_ref()
