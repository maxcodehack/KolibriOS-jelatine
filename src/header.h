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

/** \file header.h
 * Object header related definitions and type declarations */

/** \def JELATINE_HEADER_H
 * header.h inclusion macro */

#ifndef JELATINE_HEADER_H
#   define JELATINE_HEADER_H (1)

#include "wrappers.h"

// Forward declarations

struct class_t;

/* Every object allocated on the heap has an header describing it, essentialy
 * the header contains information necessary to the garbage-collector to handle
 * the object plus meta-data needed by the virtual machine to handle Java
 * objects and data used to describe the VM internal structures. Here is the
 * layout of an object's description field using the default values:
 *
 *  0                                                             31
 * +------------------------------------------------------------+-+-+
 * |                   packed class pointer                     |1|M|
 * +------------------------------------------------------------+-+-+
 *
 * A Java header has bit 30 set to 1 to enable the garbage collector to locate
 * it during the sweep phase and bit 31 used as the mark bit. The header used
 * by C objects in the Java heap is somewhat different: it has bit 30 always
 * set to 0 to denote that this is a C object and mark bit always set:
 *
 *  0                                                             31
 * +------------------------------------------------------------+-+-+
 * |                           size                             |0|1|
 * +------------------------------------------------------------+-+-+
 *
 * Note that the header format is the same for 64-bit architectures, the field
 * is just larger and bits 62 and 63 are used instead of 30 and 31. */

/** Represents the meta-data added to an object including the object
 * description and the extra fields used by the gc */
typedef uintptr_t header_t;

/** Reserved bits in the header a Java object or C allocation
 *
 * At the moment 2 bits are reserved, bit 31 is set if the object is marked and
 * bit 30 is set if this is a Java object. The mark bit is always set for C
 * objects */
#define HEADER_RESERVED (2)

// Define the shift values and bit masks to handle the object header

/** Shift value used to obtain the mark bit */
#define HEADER_MARK_SHIFT (0)

/** Shift value used to obtain the C object bit */
#define HEADER_JAVA_OBJECT_SHIFT (1)

/** Mask used to extract the packed class pointer from the header */
#define HEADER_PCP_MASK (~(((uintptr_t) 1 << HEADER_RESERVED) - 1))

/** Mask used to extract the size of an allocation from the header */
#define HEADER_SIZE_MASK HEADER_PCP_MASK

/******************************************************************************
 * Header interface                                                           *
 ******************************************************************************/

#if JEL_POINTER_REVERSAL
extern void header_create_gc_counter(header_t *);
extern void header_restore(header_t *, struct class_t *);
extern uint32_t header_get_count(const header_t *, bool);
extern void header_set_count(header_t *, uint32_t, bool);
extern uint32_t header_get_class_index(const header_t *);
#endif // JEL_POINTER_REVERSAL

/******************************************************************************
 * Header inlined functions                                                   *
 ******************************************************************************/

/** Checks if the header's 'Java object' bit is set
 * \param header A pointer to an object header
 * \returns True if the header belongs to a Java object, false otherwise */

static inline bool header_is_object(header_t *header)
{
    return (*header >> HEADER_JAVA_OBJECT_SHIFT) & 1;
} // header_is_object()

/** Creates a Java object header from the pointer to the class the object
 * belongs to
 * \param cl A pointer to a class_t structuer
 * \returns A Java object header */

static inline header_t header_create_object(struct class_t *cl)
{
    assert(((uintptr_t) cl & ((1 << HEADER_RESERVED) - 1)) == 0);
    return (uintptr_t) cl | (1 << HEADER_JAVA_OBJECT_SHIFT);
} // header_create_object()

/** Creates a C object header of the specified size
 * \param size The size of the C object
 * \returns A Java object header */

static inline header_t header_create_c(size_t size)
{
    assert((size & ((1 << HEADER_RESERVED) - 1)) == 0);
    return size | (1 << HEADER_MARK_SHIFT);
} // header_create_c()

/** Extracts the class pointer from an object header
 * \param header pointer to an object header
 * \returns The class pointer embedded in the header */

static inline struct class_t *header_get_class(header_t *header)
{
    assert(header_is_object(header));
    return (struct class_t *) (*((uintptr_t *) header) & HEADER_PCP_MASK);
} // header_get_class()

/** Extracts the size of a C object from an object header
 * \param header A pointer to an object header
 * \returns The size of the C object */

static inline size_t header_get_size(header_t *header)
{
    assert(!header_is_object(header));
    return *header & HEADER_SIZE_MASK;
} // header_get_size()

/** Checks if the header's 'marked' bit is set
 * \param header A pointer to an object header
 * \returns True if the header is marked, false otherwise */

static inline bool header_is_marked(header_t *header)
{
    return (*header >> HEADER_MARK_SHIFT) & 1;
} // header_is_marked()

/** Sets the mark bit of an object's header
 * \param header A pointer to an object header */

static inline void header_set_mark(header_t *header)
{
    *header |= 1 << HEADER_MARK_SHIFT;
} // header_set_mark()

/** Clears the mark bit of an object's header
 * \param header A pointer to an object header */

static inline void header_clear_mark(header_t *header)
{
    *header &= ~((uintptr_t) 1 << HEADER_MARK_SHIFT);
} // header_clear_mark()

#endif // !JELATINE_HEADER_H
