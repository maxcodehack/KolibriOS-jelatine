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

/** \file java_lang_ref_Reference.h
 * Java Class 'java.lang.ref.Reference' declaration */

/** \def JELATINE_JAVA_LANG_REF_REFERENCE_H
 * java_lang_ref_Reference.h inclusion macro */

#ifndef JELATINE_JAVA_LANG_REF_REFERENCE_H
#   define JELATINE_JAVA_LANG_REF_REFERENCE_H (1)

#include "wrappers.h"

/** java.lang.ref.Reference instance layout */

struct java_lang_ref_Reference_t {
    header_t header; ///< Parent class instance
    uintptr_t referent; ///< Referent of this reference
};

/** Typedef for struct java_lang_ref_Reference_t */
typedef struct java_lang_ref_Reference_t java_lang_ref_Reference_t;

/** Turns a reference to a Java Reference object into a C pointer */
#define JAVA_LANG_REF_REFERENCE_REF2PTR(r) \
        ((java_lang_ref_Reference_t *) \
         ((r) - offsetof(java_lang_ref_Reference_t, header)))

/** Turns a C pointer into a reference to a Java WeakReference object */
#define JAVA_LANG_REF_REFERENCE_PTR2REF(p) \
        ((uintptr_t) (p) + offsetof(java_lang_ref_Reference_t, header))

#endif // !JELATINE_JAVA_LANG_REF_REFERENCE_H
