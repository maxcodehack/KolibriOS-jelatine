#
# SYNOPSIS
#
#   ACX_FUNC_VA_COPY
#
# DESCRIPTION
#
#   Checks for va_copy and defines HAVE_VA_COPY if it is found, then checks
#   for __va_copy and HAVE___VA_COPY if it is found
#
# LAST MODIFICATION
#
#   2011-04-06
#
# COPYLEFT
#
#   Copyright © 2007-2011 Gabriele Svelto <gabriele.svelto@gmail.com>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([ACX_FUNC_VA_COPY],
         [AS_VAR_PUSHDEF([ac_var], [acx_cv_have_va_copy])
          AC_CACHE_CHECK([for va_copy], [ac_var],
                         [AC_RUN_IFELSE([AC_LANG_SOURCE([
#if HAVE_STDLIB_H
#   include <stdlib.h>
#endif

#if HAVE_STDARG_H
#   include <stdarg.h>
#endif

void f(void *last, ...)
{
    va_list ap, aq;
    va_start(ap, last);
    va_copy(aq, ap);
    va_end(ap);
    exit(va_arg(aq, int));
}

int main()
{
    f(NULL, 0);
    return -1;
}
                                                        ])],
                                        [AS_VAR_SET([ac_var], [yes])],
                                        [AS_VAR_SET([ac_var], [no])],
                                        [AS_VAR_SET([ac_var], [yes])])])
          AS_IF([test yes = AS_VAR_GET([ac_var])],
                [AC_DEFINE([HAVE_VA_COPY], [1],
                           [Define to 1 if the system has the `va_copy' \
                            function])], [])
          AS_VAR_POPDEF([ac_var])
          AS_VAR_PUSHDEF([ac_var], [acx_cv_have___va_copy])
          AC_CACHE_CHECK([for __va_copy], [ac_var],
                         [AC_RUN_IFELSE([AC_LANG_SOURCE([
#if HAVE_STDLIB_H
#   include <stdlib.h>
#endif

#if HAVE_STDARG_H
#   include <stdarg.h>
#endif

void f(void *last, ...)
{
    va_list ap, aq;
    va_start(ap, last);
    __va_copy(aq, ap);
    va_end(ap);
    exit(va_arg(aq, int));
}

int main()
{
    f(NULL, 0);
    return -1;
}
                                                        ])],
                                        [AS_VAR_SET([ac_var], [yes])],
                                        [AS_VAR_SET([ac_var], [no])],
                                        [AS_VAR_SET([ac_var], [yes])])])
          AS_IF([test yes = AS_VAR_GET([ac_var])],
                [AC_DEFINE([HAVE___VA_COPY], [1],
                           [Define to 1 if the system has the `__va_copy' \
                            function])], [])
          AS_VAR_POPDEF([ac_var])])
