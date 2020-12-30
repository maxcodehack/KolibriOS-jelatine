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

/** \file jstring.h
 * Java Strings manager definition and interface */

/** \def JELATINE_JSTRING_H
 * jstring.h inclusion macro */

#ifndef JELATINE_JSTRING_H
#   define JELATINE_JSTRING_H (1)

#include "wrappers.h"

#include "array.h"
#include "header.h"
#include "util.h"

#include "java_lang_String.h"

struct class_t;

/******************************************************************************
 * Java string manager interface                                              *
 ******************************************************************************/

extern void jsm_init(uint32_t, uint32_t);
extern void jsm_set_classes(struct class_t *, struct class_t *);
extern void jsm_mark( void );
extern void jsm_purge( void );

extern java_lang_String_t *jstring_intern(java_lang_String_t *);
extern java_lang_String_t *jstring_create_literal(const char *);
extern java_lang_String_t *jstring_create_from_utf8(const char *);
extern java_lang_String_t *jstring_create_from_unicode(const uint16_t *,
                                                       uint32_t);
#if JEL_PRINT
extern void jstring_print(java_lang_String_t *);
#endif // JEL_PRINT

#endif // !JELATINE_JSTRING_H
