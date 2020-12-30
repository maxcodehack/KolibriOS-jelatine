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

/** \file wrappers.h
 * Portability wrappers */

/** \def JELATINE_WRAPPERS_H
 * wrappers.h inclusion macro */

#ifndef JELATINE_WRAPPERS_H
#   define JELATINE_WRAPPERS_H (1)

#if HAVE_CONFIG_H
#   include <config.h>
#endif // HAVE_CONFIG_H

// Thread-related includes go first

#if JEL_THREAD_POSIX
#   include <pthread.h>
#elif JEL_THREAD_PTH
#   include <pth.h>
#endif

// More thread-related definitions
#if !defined(HAVE_PTHREAD_YIELD) && !defined(HAVE_PTHREAD_YIELD_NP)
#   if HAVE_SCHED_H
#       define NEED_SCHED_H (1)
#   endif // HAVE_SCHED_H
#endif // !defined(HAVE_PTHREAD_YIELD) && !defined(HAVE_PTHREAD_YIELD_NP)

/* Adapt the NDEBUG macro to the convention used by the autotools of defining
 * all existing macros to 1 in order to simplify checking */

#ifdef NDEBUG
#   undef NDEBUG
#   define NDEBUG (1)
#endif // NDEBUG

#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_INTTYPES_H
#   include <inttypes.h>
#endif // HAVE_INTTYPES_H

// Boolean type wrapper

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

/** \typedef jword_t
 * Represents the basic unit in which the Java heap is accessed */

#if SIZEOF_LONG == 8
typedef uint64_t jword_t;
#else
typedef uint32_t jword_t;
#endif // SIZEOF_LONG == 8

/** \cond */

// Adapt WORDS_BIGENDIAN and FLOAT_WORDS_BIGENDIAN

#ifndef WORDS_BIGENDIAN
#   define WORDS_BIGENDIAN (0)
#endif // !WORDS_BIGENDIAN

#ifndef FLOAT_WORDS_BIGENDIAN
#   define FLOAT_WORDS_BIGENDIAN (0)
#endif // !FLOAT_WORDS_BIGENDIAN

/** Java nil reference */
#define JNULL ((uintptr_t) NULL)

// Floating point support

#if JEL_FP_SUPPORT

#if HAVE_MATH_H
#   include <math.h>
#endif // HAVE_MATH_H

#ifndef NAN
#   if HAVE___BUILTIN_NAN
#       define NAN __builtin_nan("")
#   else
static inline float __nan( void )
{
    uint32_t nan_bits = 0x7fc00000;
    float nan;

    memcpy(&nan, &nan_bits, sizeof(float));
    return nan;
}
#       define NAN (__nan())
#   endif
#endif // NAN

#endif // JEL_FP_SUPPORT

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

#if !HAVE_VA_COPY
#   if HAVE___VA_COPY
#       define va_copy(x, y) __va_copy(x, y)
#   else // HAVE___VA_COPY
#       define va_copy(x, y) memcpy(&x, &x, sizeof(va_list))
#   endif // HAVE___VA_COPY
#endif // !HAVE_VA_COPY

/******************************************************************************
 * Compiler features                                                          *
 ******************************************************************************/

#if HAVE_FUNC_ATTRIBUTE_CONST
#   define ATTRIBUTE_CONST __attribute__((const))
#else
#   define ATTRIBUTE_CONST
#endif // HAVE_FUNC_ATTRIBUTE_CONST

#if HAVE_FUNC_ATTRIBUTE_MALLOC
#   define ATTRIBUTE_MALLOC __attribute__((malloc))
#else
#   define ATTRIBUTE_MALLOC
#endif // HAVE_FUNC_ATTRIBUTE_MALLOC

#if HAVE_FUNC_ATTRIBUTE_NORETURN
#   define ATTRIBUTE_NORETURN __attribute__((noreturn))
#else
#   define ATTRIBUTE_NORETURN
#endif // HAVE_FUNC_ATTRIBUTE_NORETURN

#if HAVE_FUNC_ATTRIBUTE_PURE
#   define ATTRIBUTE_PURE __attribute__((pure))
#else
#   define ATTRIBUTE_PURE
#endif // HAVE_FUNC_ATTRIBUTE_PURE

#if HAVE_VAR_ATTRIBUTE_UNUSED
#   define ATTRIBUTE_UNUSED __attribute__((unused))
#else
#   define ATTRIBUTE_UNUSED
#endif // HAVE_VAR_ATTRIBUTE_UNUSED

/** \endcond */

#endif // !JELATINE_WRAPPERS_H
