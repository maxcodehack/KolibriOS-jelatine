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

/** \file interpreter.h
 * Bytecode interpreter interface */

/** \def JELATINE_INTERPRETER_H
 * interpreter.h inclusion macro */

#ifndef JELATINE_INTERPRETER_H
#   define JELATINE_INTERPRETER_H (1)

#include "wrappers.h"

#include "method.h"

/******************************************************************************
 * Macros                                                                     *
 ******************************************************************************/

/** \def PRINT_OPCODE
 * Macro used in the OPCODE macro to print out an opcode when it is executed */

#if JEL_PRINT
#   define PRINT_OPCODE() print_bytecode(thread, pc, fp->method->cp)
#else
#   define PRINT_OPCODE()
#endif // JEL_PRINT

/** \def INTERPRETER_PROLOG
 * Macro used for creating the prolog of the interpreter, expands into the head
 * of a switch statement if the labels-as-values extension is not available,
 * into a direct jump otherwise */

#if JEL_THREADED_INTERPRETER
#   define INTERPRETER_PROLOG \
goto *dispatch[*pc];
#else
#   define INTERPRETER_PROLOG \
dispatch:\
    switch (*pc) {
#endif // JEL_THREADED_INTERPRETER

/** \def INTERPRETER_EPILOG
 * As for the prolog, closes the switch statement or does nothing */

#if JEL_THREADED_INTERPRETER
#   define INTERPRETER_EPILOG \
return;
#else
#   define INTERPRETER_EPILOG \
    }\
\
return;
#endif // JEL_THREADED_INTERPRETER

/** \def OPCODE
 * Declares an opcode, expands into a case statement or into a label */

#if JEL_THREADED_INTERPRETER
#   define OPCODE(x) x##_label: \
    PRINT_OPCODE();
#else
#   define OPCODE(x) case x: \
    PRINT_OPCODE();
#endif // JEL_THREADED_INTERPRETER

/** \def DISPATCH
 * Dispatches the next interpreter, jumps back to the interpreter prolog or
 * straight to the next opcode */

#if JEL_THREADED_INTERPRETER
#   define DISPATCH goto *dispatch[*pc]
#else
#   define DISPATCH goto dispatch
#endif // JEL_THREADED_INTERPRETER

/******************************************************************************
 * Function prototypes                                                        *
 ******************************************************************************/

extern void interpreter(method_t *);

#endif // !JELATINE_INTERPRETER_H
