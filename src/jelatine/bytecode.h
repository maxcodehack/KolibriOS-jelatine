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

/** \file bytecode.h
 * Bytecode manipulation functions interface */

/** \def JELATINE_BYTECODE_H
 * bytecode.h inclusion macro */

#ifndef JELATINE_BYTECODE_H
#   define JELATINE_BYTECODE_H (1)

#include "class.h"
#include "method.h"

extern void translate_bytecode(class_t *, method_t *, uint8_t *,
                               exception_handler_t *);

#endif // JELATINE_BYTECODE_H
