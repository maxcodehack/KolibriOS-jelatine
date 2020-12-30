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

/** \file java_lang_Class.h
 * Java Class 'java.lang.Class' declaration and interface */

/** \def JELATINE_JAVA_LANG_CLASS_H
 * java_lang_Class.h inclusion macro */

#ifndef JELATINE_JAVA_LANG_CLASS_H
#   define JELATINE_JAVA_LANG_CLASS_H (1)

#include "wrappers.h"

#include "header.h"
#include "jstring.h"

/** java.lang.Class instance layout */

struct java_lang_Class_t {
    uintptr_t name; ///< Java name
    header_t header; ///< Object header
    int32_t id; ///< Id in the class table corresponding to this class
    int8_t is_array; ///< true if this class is an array
    int8_t is_interface; ///< true if this class is an interface
};

/** Typedef for struct java_lang_Class_t */
typedef struct java_lang_Class_t java_lang_Class_t;

/** Number of references in a java.lang.Class instance */
#define JAVA_LANG_CLASS_REF_N (1)

/** Size in bytes of the non-reference area of a java.lang.Class instance */
#define JAVA_LANG_CLASS_NREF_SIZE \
        (sizeof(java_lang_Class_t) - offsetof(java_lang_Class_t, id))

/** Turns a reference to a Java Class object into a C pointer */
#define JAVA_LANG_CLASS_REF2PTR(r) \
        ((java_lang_Class_t *) ((r) - offsetof(java_lang_Class_t, header)))

/** Turns a C pointer into a reference to a Java Class object */
#define JAVA_LANG_CLASS_PTR2REF(p) \
        ((uintptr_t) (p) + offsetof(java_lang_Class_t, header))

#endif // !JELATINE_JAVA_LANG_CLASS_H
