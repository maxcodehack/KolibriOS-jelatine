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

#ifndef PREVERIFIER_WRAPPERS_H
#   define PREVERIFIER_WRAPPERS_H (1)

#if HAVE_CONFIG_H
#   include <config.h>
#endif // HAVE_CONFIG_H

/* Adapt the NDEBUG macro to the convention used by the autotools of defining
 * all existing macros to 1 in order to simplify checking */

#ifdef NDEBUG
#   undef NDEBUG
#   define NDEBUG (1)
#endif // NDEBUG

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_INTTYPES_H
#   include <inttypes.h>
#endif // HAVE_INTTYPES_H

/* Boolean type wrapper */

/** \def bool
 * Boolean type wrapper used if the compiler doesn't provide it */

/** \def true
 * Boolean true value wrapper used if the compiler doesn't provide it */

/** \def false
 * Boolean false value wrapper used if the compiler doesn't provide it */

/** \def __bool_true_false_are_defined
 * True if bool, true and false are defined */

/** \typedef _Bool
 * C99 _Bool wrapper used if the compiler doesn't provide it */

#if HAVE_STDBOOL_H
#   include <stdbool.h>
#else
#   if !HAVE__BOOL
#       ifdef __cplusplus
typedef bool _Bool;
#       else
typedef unsigned int _Bool;
#       endif // __cplusplus
#   endif // !HAVE_BOOL
#   define bool _Bool
#   define false (0)
#   define true (1)
#   define __bool_true_false_are_defined (1)
#endif // HAVE_STDBOOL_H

#if !HAVE_PTRDIFF_T
/** ptrdiff_t wrapper, some compilers lack the ptrdiff_t type, in case we define
 * it ourselves */

typedef intptr_t ptrdiff_t;
#endif // !HAVE_PTRDIFF_T

// Time support

#if TIME_WITH_SYS_TIME
#   include <sys/time.h>
#   include <time.h>
#else
#   if HAVE_SYS_TIME_H
#       include <sys/time.h>
#   else
#       include <time.h>
#   endif
#endif

#if HAVE_DIRENT_H
#   include <dirent.h>
#endif // HAVE_DIRENT_H

#if HAVE_FCNTL_H
#   include <fcntl.h>
#endif // HAVE_FCNTL_H

#if HAVE_UNISTD_H
#   include <unistd.h>
#endif // HAVE_UNISTD_H

#if HAVE_SYS_TYPES_H
#   include <sys/types.h>
#endif // HAVE_SYS_TYPES_H

#if HAVE_SYS_STAT_H
#   include <sys/stat.h>
#endif // HAVE_SYS_STAT_H

#if HAVE_LANGINFO_H
#   include <langinfo.h>
#endif // HAVE_LANGINFO_H

#if HAVE_ICONV_H
#   include <iconv.h>
#endif // HAVE_ICONV_H

#if HAVE_ARPA_INET_H
#   include <arpa/inet.h>
#endif // HAVE_ARPA_INET_H

#endif // !PREVERIFIER_WRAPPERS_H
