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

/** \file loader.h
 * Bootstrap class-loader definition */

/** \def JELATINE_LOADER_H
 * loader.h inclusion macro */

#ifndef JELATINE_LOADER_H
#   define JELATINE_LOADER_H (1)

#include "wrappers.h"

#include "array.h"
#include "class.h"
#include "opcodes.h"
#include "util.h"

/******************************************************************************
 * Globals                                                                    *
 ******************************************************************************/

extern class_t *array_classes[];

/******************************************************************************
 * Loader interface                                                           *
 ******************************************************************************/

extern void bcl_init( void );
extern class_t *bcl_get_class_by_id(uint32_t);
extern void bcl_mark( void );
extern bool bcl_is_assignable(class_t *, class_t *);
extern void bcl_preload_bootstrap_classes( void );
extern class_t *bcl_resolve_class(class_t *, const char *);
extern class_t *bcl_find_class(const char *);
extern void bcl_link_method(class_t *, method_t *);
extern const uint8_t *bcl_link_opcode(const method_t *, const uint8_t *,
                                      internal_opcode_t);

/******************************************************************************
 * Inlined loader functions                                                   *
 ******************************************************************************/

/** Return the class of a basic array type.
 * The class must have already been resolved otherwise this function will
 * return a NULL pointer
 * \param type The basic array type
 * \returns The array class associated with this basic type */

static inline class_t *bcl_array_class(array_type_t type)
{
    assert((type >= T_BOOLEAN) && (type <= T_LONG)
           && array_classes[type - T_BOOLEAN]);

    return array_classes[type - T_BOOLEAN];
} // bcl_array_class()

/** Sets the class for the specified basic array type
 * \param type The basic array type
 * \param cl A pointer to the array class */

static inline void bcl_set_array_class(array_type_t type, class_t *cl)
{
    assert((type >= T_BOOLEAN) && (type <= T_LONG));

    array_classes[type - T_BOOLEAN] = cl;
} // bcl_set_array_class()

#endif // !JELATINE_LOADER_H
