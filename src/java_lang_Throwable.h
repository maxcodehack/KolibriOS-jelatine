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

/** \file java_lang_Throwable.h
 * Java Class 'java.lang.Throwable' declaration */

/** \def JELATINE_JAVA_LANG_THROWABLE_H
 * java_lang_Throwable.h inclusion macro */

#ifndef JELATINE_JAVA_LANG_THROWABLE_H
#   define JELATINE_JAVA_LANG_THROWABLE_H (1)

#include "wrappers.h"

/** Structure mirroring an instance of the java.lang.Throwable class */
struct java_lang_Throwable_t {
    uintptr_t detailMessage; ///< Detailed message
    header_t header; ///< Object header
};

/** Typedef for struct java_lang_Throwable_t */
typedef struct java_lang_Throwable_t java_lang_Throwable_t;

/** Turns a reference to a Java Throwable object into a C pointer */
#define JAVA_LANG_THROWABLE_REF2PTR(r) \
        ((java_lang_Throwable_t *) ((r) \
                                    - offsetof(java_lang_Throwable_t, header)))

/** Turns a C pointer into a reference to a Java Throwable object */
#define JAVA_LANG_THROWABLE_PTR2REF(p) \
        ((uintptr_t) (p) + offsetof(java_lang_Throwable_t, header))

#endif // !JELATINE_JAVA_LANG_THROWABLE_H
