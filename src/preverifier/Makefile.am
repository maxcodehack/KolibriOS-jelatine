## Process this file with automake to produce Makefile.in

###############################################################################
##   Copyright © 2005-2011 by Gabriele Svelto                                ##
##   gabriele.svelto@gmail.com                                               ##
##                                                                           ##
##   This file is part of Jelatine.                                          ##
##                                                                           ##
##   Jelatine is free software: you can redistribute it and/or modify        ##
##   it under the terms of the GNU General Public License as published by    ##
##   the Free Software Foundation, either version 3 of the License, or       ##
##   (at your option) any later version.                                     ##
##                                                                           ##
##   Jelatine is distributed in the hope that it will be useful,             ##
##   but WITHOUT ANY WARRANTY; without even the implied warranty of          ##
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           ##
##   GNU General Public License for more details.                            ##
##                                                                           ##
##   You should have received a copy of the GNU General Public License       ##
##   along with Jelatine.  If not, see <http://www.gnu.org/licenses/>.       ##
###############################################################################

bin_PROGRAMS = preverifier

preverifier_SOURCES = \
    check_class.c \
    check_code.c check_code.h \
    classloader.c \
    classresolver.c \
    convert_md.c convert_md.h \
    file.c \
    inlinejsr.c \
    jar.c jar.h \
    jarint.h \
    jartables.h \
    jar_support.c \
    locale_md.h \
    main.c \
    oobj.h \
    opcodes.h \
    path.h \
    path_md.h \
    signature.h \
    stubs.c \
    sysmacros_md.h \
    sys_api.h \
    sys_support.c \
    tree.h \
    typecodes.h \
    typedefs.h \
    typedefs_md.h \
    utf.c utf.h \
    util.c \
    wrappers.h

preverifier_LDFLAGS = $(preverifier_LIBS)

EXTRA_DIST = opcodes.init opcodes.in_out opcodes.length README.TXT
