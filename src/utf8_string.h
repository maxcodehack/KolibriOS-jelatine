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

/** \file utf8_string.h
 * UTF8 string manager declaration and interface */

/** \def JELATINE_UTF8_STRING_H
 * utf8_string.h inclusion macro */

#ifndef JELATINE_UTF8_STRING_H
#   define JELATINE_UTF8_STRING_H (1)

#include "wrappers.h"

#include "util.h"

/******************************************************************************
 * String manager interface                                                   *
 ******************************************************************************/

extern void string_manager_init(size_t, size_t);
extern char *utf8_intern(const char *, size_t);

extern bool utf8_check(const char *);
extern size_t utf8_to_java_length(const char *);
extern void utf8_to_java(uint16_t *, const char *);
extern char *java_to_utf8(const uint16_t *, size_t);
extern char *utf8_slashify(char *);

#if JEL_PRINT
extern void string_manager_print( void );
extern size_t string_manager_size( void );
#endif // JEL_PRINT

#endif // !JELATINE_UTF8_STRING_H
