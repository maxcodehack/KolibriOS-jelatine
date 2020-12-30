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

AC_INIT([Jelatine], [0.9.4], [gabriele.svelto@gmail.com], [jelatine],
        [http://jelatine.sourceforge.net])
AM_INIT_AUTOMAKE

# Prerequisite autoconf version
AC_PREREQ([2.63])

# Inform autoconf to output a config.h file
AC_CONFIG_HEADERS([config.h])

# Check if main.c is present in the 'src' directory
AC_CONFIG_SRCDIR([src/jelatine/main.c])

# Set to 'lib' the directory where AC_LIBOBJ replacements are present
AC_CONFIG_LIBOBJ_DIR([lib])

# Set to 'm4' the directory where aclocal should look for extra macros
AC_CONFIG_MACRO_DIR([m4])

# Jelatine definitions templates
AH_TEMPLATE([JEL_JARFILE_SUPPORT], [Enabled if JAR file support is needed])
AH_TEMPLATE([JEL_SOCKET_SUPPORT], [Enabled if socket support is used])
AH_TEMPLATE([JEL_THREAD_POSIX], [Enabled when POSIX thread support is used])
AH_TEMPLATE([JEL_THREAD_PTH], [Enabled when GNU Pth thread support is used])
AH_TEMPLATE([JEL_THREAD_NONE], [Enabled when thread support is disabled])
AH_TEMPLATE([JEL_THREADED_INTERPRETER],
    [Enabled if the threaded, optimized interpreter is needed])
AH_TEMPLATE([JEL_FINALIZER], [Enabled if object finalization is needed])
AH_TEMPLATE([JEL_FP_SUPPORT], [Enabled if floating-point support is needed])
AH_TEMPLATE([JEL_POINTER_REVERSAL],
    [Enabled if the pointer reversal based garbage collector is needed])
AH_TEMPLATE([JEL_TRACE], [Enabled if bytecode/method tracing is needed])
AH_TEMPLATE([JEL_PRINT], [Enabled if bytecode/method printing is needed])
AH_TEMPLATE([JEL_CLASSPATH_DIR], [Holds the default classpath directory])
AH_TEMPLATE([NDEBUG], [Enabled if debugging code must be removed])

################################################################################
# Define the extra arguments the user can pass to the configure script         #
################################################################################

AC_ARG_ENABLE([preverifier],
              [AS_HELP_STRING([--disable-preverifier],
                              [Do not build the preverifier])],
              [want_preverifier="$enableval"], [want_preverifier=yes])

AC_ARG_ENABLE([jar],
              [AS_HELP_STRING([--disable-jar],
                              [Disables JAR file support in the VM])],
              [jar_support="$enableval"], [jar_support=yes])

AC_ARG_ENABLE([socket],
              [AS_HELP_STRING([--disable-socket],
                              [Disables socket support in the VM])],
              [socket_support="$enableval"], [socket_support=yes])

AC_ARG_ENABLE([threaded],
              [AS_HELP_STRING([--disable-threaded],
                              [Disables optimized threaded interpreter])],
              [threaded="$enableval"], [threaded=auto])

AC_ARG_WITH([thread-model],
            [AS_HELP_STRING([--with-thread-model=<model>],
                            [Selects the thread model used by the VM (pthread, pth, none) (default: auto)])],
            [thread_model="$withval"], [thread_model=auto])

AC_ARG_ENABLE([finalizer],
              [AS_HELP_STRING([--disable-finalizer],
                              [Disables object finalization support])],
              [finalizer="$enableval"], [finalizer=yes])

AC_ARG_ENABLE([fp-support],
              [AS_HELP_STRING([--disable-fp-support],
                              [Disables fp support in the VM])],
              [fp_support="$enableval"], [fp_support=yes])

AC_ARG_ENABLE([prgc],
              [AS_HELP_STRING([--disable-prgc],
                              [Disables the pointer reversal algorythm in the garbage collector])],
              [prgc="$enableval"], [prgc=yes])

AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug], [Enables debugging code])],
              [debug="$enableval"], [debug=no])

AC_ARG_ENABLE([tracing],
              [AS_HELP_STRING([--enable-tracing],
                              [Enables bytecode/method tracing facilities])],
              [tracing="$enableval"], [tracing=no])

AC_ARG_ENABLE([printing],
              [AS_HELP_STRING([--enable-printing],
                              [Enables bytecode/method printing facilites])],
              [printing="$enableval"], [printing=no])

################################################################################
# Check for programs                                                           #
################################################################################

# Check for a working C compiler

AC_PROG_CC_C99

AS_IF([test no = "$ac_cv_prog_cc_c99"],
      [AC_MSG_WARN([You do not seem to have a C99 compiler, compilation may fail])])

AM_PROG_CC_C_O

# Enable POSIX extensions for targets which disables them by default
AC_USE_SYSTEM_EXTENSIONS

# Check for a usable 'javac' compiler
AX_PROG_JAVAC

# Check for a usable 'jar' program
AX_PROG_JAR

################################################################################
# Check for libraries                                                          #
################################################################################

# JAR support
PKG_CHECK_MODULES([zziplib], [zziplib >= 0.13.50], [have_zzip=yes],
                  [have_zzip=no])

# POSIX threads
AX_PTHREAD([have_pthread=yes])

# libpth
AC_CHECK_LIB([pth], [pth_init], [pth_LIBS="-lpth"], [pth_LIBS=""])

# Floating point support
LT_LIB_M

AS_IF([test yes = "$fp_support"], [jelatine_LIBS="$jelatine_LIBS $LIBM"])

# Needed on some hosts for the preverifier
AC_CHECK_LIB([iconv], [iconv],
             [preverifier_LIBS="-liconv"], [preverifier_LIBS=""])

# Time measurements on Linux
AC_CHECK_LIB([rt], [clock_gettime], [jelatine_LIBS="$jelatine_LIBS -lrt"])

################################################################################
# Check for header files                                                       #
################################################################################

# Check for the standard C includes
AC_HEADER_STDC

AS_IF([test "yes" != "$ac_cv_header_stdc"],
      [AC_MSG_WARN([You don't seem to have standard C headers])])

AC_HEADER_STDBOOL
AC_HEADER_TIME

AC_CHECK_HEADERS([arpa/inet.h dirent.h fcntl.h iconv.h inttypes.h langinfo.h \
                  math.h sched.h stdarg.h sys/stat.h sys/types.h unistd.h])

# Socket support
AC_CHECK_HEADERS([sys/socket.h netinet/in.h netdb.h], [], [socket_support=no])

# libpth
AC_CHECK_HEADER([pth.h])

################################################################################
# Check for types                                                              #
################################################################################

# Check for the presence of size_t
AC_TYPE_SIZE_T

# Check the size of pointer and long integer types
AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])

# Check for intX_t types
AC_TYPE_INT8_T
AC_TYPE_UINT8_T
AC_TYPE_INT16_T
AC_TYPE_UINT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT64_T
AC_TYPE_INTPTR_T
AC_TYPE_UINTPTR_T

# Check for the presence of ptrdiff_t
AC_CHECK_TYPES([ptrdiff_t])

################################################################################
# Check for structures                                                         #
################################################################################

# Unused

################################################################################
# Check for compiler features                                                  #
################################################################################

AC_C_CONST
AC_C_INLINE
AC_C_BIGENDIAN
AX_C_FLOAT_WORDS_BIGENDIAN

# Check for built-in functions
AX_GCC_BUILTIN([__builtin_nan])

# Check for the __attribute__((unused)) variable attribute
AX_GCC_VAR_ATTRIBUTE([unused])

# Check for function attributes
AX_GCC_FUNC_ATTRIBUTE([const])
AX_GCC_FUNC_ATTRIBUTE([malloc])
AX_GCC_FUNC_ATTRIBUTE([noreturn])
AX_GCC_FUNC_ATTRIBUTE([pure])

# Check for label-as-values gcc extension

AC_MSG_CHECKING([for gcc's labels-as-values extension])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
                                   int main()
                                   {
                                       void *happy_label;

                                       happy_label = &&happy;
                                       goto *happy_label;

                                       happy:
                                           return 0;
                                   }
                  ])],
                  [AC_MSG_RESULT([yes])
                   have_labels_as_values=yes],
                  [AC_MSG_RESULT([no])])

AS_IF([test auto = "$threaded" && test yes = "$have_labels_as_values"],
      [threaded=yes
       AC_DEFINE([JEL_THREADED_INTERPRETER], [1])])

# Check for thread local storage support
AX_TLS

################################################################################
# Check for functions                                                          #
################################################################################

AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_VPRINTF
AC_CHECK_FUNC([vsnprintf], , [AC_MSG_ERROR([snprintf() is required])])
AC_REPLACE_FUNCS([memcpy memmove])
AC_CHECK_FUNCS([gettimeofday clock_gettime])
ACX_FUNC_VA_COPY

# Tests for fp support
old_LIBS="$LIBS"
LIBS="$LIBS $LIBM"

AC_CHECK_FUNCS([fmod \
                sin \
                cos \
                tan \
                sqrt \
                ceil \
                floor], [],
               [AS_IF([test yes = "$fp_support"],
                      [AC_MSG_ERROR([required function $ac_func() is missing])])])

LIBS="$old_LIBS"

# Check for thread-related functions
old_LIBS="$LIBS"
old_CFLAGS="$CFLAGS"
old_CC="$CC"
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"

AC_CHECK_FUNCS([pthread_yield \
                pthread_yield_np \
                sched_yield])

LIBS="$old_LIBS"
CFLAGS="$old_CFLAGS"
CC="$old_CC"

################################################################################
# Check for system services                                                    #
################################################################################

# Unused

################################################################################
# Do automatic tests                                                           #
################################################################################

# Check for floating point support

AS_IF([test yes = "$fp_support"],
      [AC_DEFINE([JEL_FP_SUPPORT], [1])])

# Check for jar support

AS_IF([test no = "$JAR"],
      [AS_IF([test yes = "$jar_support"],
             [AC_MSG_WARN(['jar' not found, JAR file support disabled])])
       jar_support=no])

AS_IF([test yes = "$jar_support"],
      [AS_IF([test yes = "$have_zzip"],
             [LIBS="$LIBS $zziplib_LIBS"
              CFLAGS="$CFLAGS $zziplib_CFLAGS"
              AC_SUBST([LIBS])
              AC_SUBST([CFLAGS])],
             [jar_support=no
              AC_MSG_WARN([JAR file support disabled, zziplib may not be installed or you may have an outdated version])])])

AS_IF([test yes = "$jar_support"],
      [classpath="classpath.jar"
       AC_DEFINE([JEL_JARFILE_SUPPORT], [1])],
      [classpath="classpath/"])

# Check for socket support

AS_IF([test yes = "$socket_support"],
      [AC_DEFINE([JEL_SOCKET_SUPPORT], [1])])

# Select the thread model

AS_IF([test auto = "$thread_model"],
      [AS_IF([test yes = "$have_pthread"],
             [thread_model=pthread],
             [AS_IF([test yes = "$ac_cv_lib_pth_init" && \
                     test yes = "$ac_cv_header_pth_h"],
                    [thread_model=pth],
                    [thread_model=none])])])

AS_CASE([$thread_model],
        [pthread],
        [AS_IF([test yes = "$have_pthread"],
               [AC_DEFINE([JEL_THREAD_POSIX], [1])
                CC="$PTHREAD_CC"
                AC_SUBST([CC])
                jelatine_CFLAGS="$jelatine_LIBS $PTHREAD_CFLAGS"
                jelatine_LIBS="$jelatine_LIBS $PTHREAD_LIBS"],
        [AC_MSG_ERROR([POSIX threads headers and libraries were not found])])],
        [pth],
        [AS_IF([test no = "$ac_cv_lib_pth_init" || \
                test no = "$ac_cv_header_pth_h"],
               [AC_MSG_ERROR([GNU Pth was not found])],
               [AC_DEFINE([JEL_THREAD_PTH], [1])
                jelatine_LIBS="$jelatine_LIBS $pth_LIBS"])],
        [none],
        [AC_DEFINE([JEL_THREAD_NONE], [1])],
        [AC_MSG_ERROR([Unknown thread model: $thread_model])])

# Deal with finalization support

AS_IF([test yes = "$finalizer"],
      [AS_IF([test none = "$thread_model"],
             [finalizer=no
              AC_MSG_WARN([Finalization disabled, thread support is required])],
             [AC_DEFINE([JEL_FINALIZER], [1])])])

# Set preverifier path

AS_IF([test yes = "$want_preverifier"],
      [preverifier="../preverifier/preverifier"],
      [preverifier="preverifier"])

AS_IF([test yes = "$cross_compiling"],
      [preverifier="preverifier"])

# Define other variables

AS_IF([test yes = "$prgc"], [AC_DEFINE([JEL_POINTER_REVERSAL], [1])])
AS_IF([test yes = "$debug"],
      [JAVACFLAGS="$JAVACFLAGS -g"
       AC_SUBST([JAVACFLAGS])],
      [AC_DEFINE([NDEBUG], [1])
       JAVACFLAGS="$JAVACFLAGS -g:none"
       AC_SUBST([JAVACFLAGS])])
AS_IF([test yes = "$tracing"], [AC_DEFINE([JEL_TRACE], [1])])
AS_IF([test yes = "$printing"], [AC_DEFINE([JEL_PRINT], [1])])

# Set output variables

# Jelatine output variables
AC_SUBST([jelatine_LIBS])
AC_SUBST([jelatine_CFLAGS])
AC_SUBST([classpath])
AC_SUBST([jar_support])

# Preverifier output variables
AM_CONDITIONAL([COND_PREVERIFIER], [test yes = "$want_preverifier"])
AC_SUBST([preverifier])
AC_SUBST([preverifier_LIBS])

AC_CONFIG_FILES([Makefile
                 Doxyfile
                 docs/Makefile
                 src/Makefile
                 src/classpath/Makefile
                 src/preverifier/Makefile
                 src/jelatine/Makefile])

AC_OUTPUT

#####################
# Print configuration
#####################
cat <<EOF | tee -a config.log

## -------------- ##
## Configuration. ##
## -------------- ##

Configuration for $PACKAGE_STRING
    Build preverifier: $want_preverifier
    Floating-point support: $fp_support
    JAR support: $jar_support
    Socket support: $socket_support
    Threaded interpreter: $threaded
    Thread model: $thread_model
    Finalization support: $finalizer
    Debugging: $debug

Used CFLAGS:
    CPPFLAGS: $CPPFLAGS
    CFLAGS: $CFLAGS

Installation:
    Prefix: $prefix

EOF
