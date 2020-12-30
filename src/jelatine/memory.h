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

/** \file memory.h
 * Garbage-collector and allocator type definitions and interface */

/** \def JELATINE_MEMORY_H
 * memory.h inclusion macro */

#ifndef JELATINE_MEMORY_H
#   define JELATINE_MEMORY_H (1)

#include "wrappers.h"

#include "opcodes.h"
#include "header.h"
#include "java_lang_ref_WeakReference.h"


// Forward declarations
struct class_t;

/******************************************************************************
 * Function prototypes                                                        *
 ******************************************************************************/

// Gargbage collected heap management
extern void gc_init(size_t);
extern void gc_teardown( void );
extern void gc_enable(bool);
extern void gc_collect(size_t);
extern size_t gc_free_memory( void );
extern size_t gc_total_memory( void );

// Managed allocations
extern void gc_mark_potential(uintptr_t);
extern void gc_mark_reference(uintptr_t);
extern uintptr_t gc_new(struct class_t *);
extern uintptr_t gc_new_array_nonref(array_type_t, int32_t);
extern uintptr_t gc_new_array_ref(struct class_t *, int32_t);
extern uintptr_t gc_new_multiarray(struct class_t *, uint8_t, jword_t *);

// Finalization
#if JEL_FINALIZER
extern void gc_register_finalizer(uintptr_t);
extern void gc_register_finalizable(uintptr_t);
extern uintptr_t gc_get_finalizable( void );
#endif // JEL_FINALIZER

// Special references
extern void gc_register_weak_ref(java_lang_ref_WeakReference_t *);

// Unmanaged allocations
extern void *gc_malloc(size_t);
extern void *gc_palloc(size_t);
extern void gc_free(void *);

#endif // !JELATINE_MEMORY_H
