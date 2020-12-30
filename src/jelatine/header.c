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

/** \file header.c
 * Implementation of header-related functions */

#include "wrappers.h"

#include "class.h"
#include "header.h"

#if JEL_POINTER_REVERSAL

/** Turns a header in its compacted form including the garbage-collection
 * counter, also marks the header
 * \param header A pointer to a header */

void header_create_gc_counter(header_t *header)
{
    class_t *cl = header_get_class(header);

    *header = (header_get_class(header)->id << 16)
              | (1 << HEADER_JAVA_OBJECT_SHIFT)
              | (1 << HEADER_MARK_SHIFT);

    // Reference arrays have an external counter
    if (class_is_array(cl) && cl->elem_type == PT_REFERENCE) {
        ((ref_array_t *) header)->count = 0;
    }
} // header_create_gc_counter()

/** Restores a header to its runtime form, removing the gc counter, clearing
 * its mark bit and restoring the class pointer
 * \param header A pointer to a header
 * \param ct A pointer to the header class */

void header_restore(header_t *header, class_t *cl)
{
    *header = (uintptr_t) cl | (1 << HEADER_JAVA_OBJECT_SHIFT);

    if (class_is_array(cl) && cl->elem_type == PT_REFERENCE) {
        ((ref_array_t *) header)->count = 0;
    }
} // header_restore()

/** Returns the garbage-collection count of a header
 * \param header A pointer to a header
 * \param array True if the header belongs to a reference array
 * \returns The garbage-collection counter */

uint32_t header_get_count(const header_t *header, bool array)
{
    const header_t mask = (header_t) 0xffff >> HEADER_RESERVED;

    if (array) {
        return ((ref_array_t *) header)->count;
    } else {
        return (*header >> HEADER_RESERVED) & mask;
    }
} // header_get_count()

/** Sets the garbage-collection counter of a header
 * \param header A pointer to a header
 * \param count The new gc counter value
 * \param array True if the header belongs to a reference array */

void header_set_count(header_t *header, uint32_t count, bool array)
{
    const header_t mask = ((1 << HEADER_RESERVED) - 1) | 0xffff0000;

    if (array) {
        ((ref_array_t *) header)->count = count;
    } else {
        assert(count < (1 << (16 - HEADER_RESERVED)));
        *header = (*header & mask) | (count << HEADER_RESERVED);
    }
} // header_set_count()

/** Gets the class index held by a compacted header
 * \param header A pointer to a header
 * \returns The index in the class-table of the header's class */

uint32_t header_get_class_index(const header_t *header)
{
    return *header >> 16;
} // header_get_class_index()

#endif // JEL_POINTER_REVERSAL
