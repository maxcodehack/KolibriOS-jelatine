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

/** \file array.h
 * Array declaration and interface */

/** \def JELATINE_ARRAY_H
 * array.h inclusion macro */

#ifndef JELATINE_ARRAY_H
#   define JELATINE_ARRAY_H (1)

#include "wrappers.h"

#include "classfile.h"
#include "header.h"
#include "opcodes.h"

// Forward declarations

struct class_t;

/******************************************************************************
 * Array type definitions                                                     *
 ******************************************************************************/

/*
 * Non-reference array layout in memory
 *
 * +--------+
 * | header |
 * +--------+
 * | length |
 * +--------+
 * | elem 1 |
 * +--------+
 * | elem 2 |
 * +--------+
 * |  ...   |
 * +--------+
 *
 * Reference array layout in memory
 *
 * +--------+
 * |  ...   |
 * +--------+
 * | ref 2  |
 * +--------+
 * | ref 1  |
 * +--------+
 * | header |
 * +--------+
 * | length |
 * +--------+ */

/** Array header */

struct array_t {
    header_t header; ///< Object header
    uint32_t length; ///< Number of elements in the array
#if SIZEOF_JWORD_T == 8
    uint32_t padding; ///< Padding bytes to round the structure
#endif // SIZEOF_JWORD_T == 8
};

/** Typedef for struct array_t */
typedef struct array_t array_t;

/** Reference array header */

struct ref_array_t {
    header_t header; ///< Object header
    uint32_t length; ///< Number of elements in the array
#if JEL_POINTER_REVERSAL
    uint32_t count; ///< Garbage collection count
#elif SIZEOF_JWORD_T == 8
    uint32_t padding; ///< Padding bytes to round the structure's size
#endif // JEL_POINTER_REVERSAL
};

/** Typedef for struct ref_array_t */
typedef struct ref_array_t ref_array_t;

/******************************************************************************
 * Globals                                                                    *
 ******************************************************************************/

extern const char *array_names[];
extern uint8_t array_elem_sizes[];
extern uint8_t prim_to_array_types[];

/******************************************************************************
 * Array interface                                                            *
 ******************************************************************************/

extern void arraycopy_ref(array_t *, int32_t, array_t *, int32_t, int32_t);
extern size_t array_get_nref_size(array_t *);
extern size_t array_get_ref_n(array_t *);

/******************************************************************************
 * Array inlined functions                                                    *
 ******************************************************************************/

/** Gets a pointer to the actual array data
 * \param p A pointer to an array header */

static inline void *array_get_data(array_t *p)
{
    return ((char *) p) + sizeof(array_t);
} // array_get_data()

/** Gets a pointer to the actual array data for reference arrays
 * \param p A pointer to an array header */

static inline uintptr_t *array_ref_get_data(array_t *p)
{
    return ((uintptr_t *) p) - 1;
} // array_ref_get_data()

/** Returns the number of elements in an array
 * \param p A pointer to an array header */

static inline uint32_t array_length(array_t *p)
{
    return p->length;
} // array_length()

/** Returns the name of a basic array type
 * \param t The basic array type
 * \returns A string holding the name of the array class */

static inline const char *array_name(array_type_t t)
{
    return array_names[t - T_BOOLEAN];
} // array_name()

/** Returns the size of an element of the specified basic array type
 * \param t The basic array type
 * \returns The size in bytes of the basic array type */

static inline uint32_t array_elem_size(array_type_t t)
{
    return array_elem_sizes[t - T_BOOLEAN];
} // array_elem_size()

/** Returns the array type derived from a primitive type
 * \param t The primitive type
 * \returns The array type corresponding to the primitive type */

static inline array_type_t prim_to_array_type(primitive_type_t t)
{
    return prim_to_array_types[t];
} // prim_to_array_type()

#endif // !JELATINE_ARRAY_H
