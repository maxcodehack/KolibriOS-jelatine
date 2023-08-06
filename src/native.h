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

/** \file native.h
 * Native methods declaration */

/** \def JELATINE_NATIVE_H
 * native.h inclusion macro */

#ifndef JELATINE_NATIVE_H
#   define JELATINE_NATIVE_H (1)

#include "wrappers.h"

#include "header.h"
#include "method.h"

/******************************************************************************
 * Function prototypes                                                        *
 ******************************************************************************/

extern native_proto_t native_method_lookup(const char *, const char *,
                                           const char *);

#endif // !JELATINE_NATIVE_H
