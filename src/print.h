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

/** \file print.h
 * Printing facilities interface */

/** \def JELATINE_PRINT_H
 * print.h inclusion macro */

#ifndef JELATINE_PRINT_H
#   define JELATINE_PRINT_H (1)

#include "wrappers.h"

#include "constantpool.h"
#include "method.h"
#include "thread.h"

#if JEL_PRINT
extern void print_bytecode(thread_t *, const uint8_t *, const_pool_t *);
extern void print_method_call(thread_t *, const method_t *);
extern void print_method_ret(thread_t *, const method_t *);
extern void print_method_unwind(thread_t *, method_t *);
#else
/** Dummy definition used when printing is disabled */
#define print_bytecode(thread, pc, cp)
/** Dummy definition used when printing is disabled */
#define print_method_call(thread, method);
/** Dummy definition used when printing is disabled */
#define print_method_ret(thread, method);
/** Dummy definition used when printing is disabled */
#define print_method_unwind(thread, method);
#endif // JEL_PRINT

#endif // JELATINE_PRINT_H
