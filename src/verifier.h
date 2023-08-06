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

/** \file verifier.h
 * Bytecode and class file verification interface */

/** \def JELATINE_VERIFIER_H
 * verifier.h inclusion macro */

#ifndef JELATINE_VERIFIER_H
#   define JELATINE_VERIFIER_H (1)

#include "wrappers.h"

#include "class.h"
#include "classfile.h"

/******************************************************************************
 * Verifier interface                                                         *
 ******************************************************************************/

extern bool same_package(const class_t *, const class_t *);

extern void verify_field(const class_t *, const field_info_t *,
                         const field_attributes_t *);

extern void verify_field_access(const class_t *, const class_t *,
                                const field_t *);

#endif // JELATINE_VERIFIER_H
