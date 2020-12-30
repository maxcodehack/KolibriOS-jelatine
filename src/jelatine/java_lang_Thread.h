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

/** \file java_lang_Thread.h
 * Java Class 'java.lang.Thread' declaration */

/** \def JELATINE_JAVA_LANG_THREAD_H
 * java_lang_Thread.h inclusion macro */

#ifndef JELATINE_JAVA_LANG_THREAD_H
#   define JELATINE_JAVA_LANG_THREAD_H (1)

#include "wrappers.h"

#include "header.h"

/** C structure mirroring java.lang.Thread */

struct java_lang_Thread_t {
    uintptr_t name; ///< Reference to the thread's name
    uintptr_t runnable; ///< Reference to the thread's Runnable object
    header_t header; ///< Header of the object
    uintptr_t vmThread; ///< Reference to an internal representation
#if (SIZEOF_VOID_P == 2)
    uint16_t padding; ///< Padding needed on machines with 16-bit pointers
#endif
    int32_t priority; ///< Priority of this thread
};

/** Typedef for struct java_lang_Thread_t */
typedef struct java_lang_Thread_t java_lang_Thread_t;

/** Number of references in a java.lang.Thread instance */
#define JAVA_LANG_THREAD_REF_N (2)

/** Size in bytes of the non-reference area of a java.lang.Thread instance */
#define JAVA_LANG_THREAD_NREF_SIZE \
        (sizeof(java_lang_Thread_t) - offsetof(java_lang_Thread_t, vmThread))

/** Turns a reference to a Java thread into a C pointer */
#define JAVA_LANG_THREAD_REF2PTR(r) \
        ((java_lang_Thread_t *) ((r) - offsetof(java_lang_Thread_t, header)))

/** Turns a C pointer to a Java thread into a Java reference */
#define JAVA_LANG_THREAD_PTR2REF(p) \
        ((uintptr_t) (p) + offsetof(java_lang_Thread_t, header))

#endif // !JELATINE_JAVA_LANG_THREAD_H
