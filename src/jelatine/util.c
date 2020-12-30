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

/** \file util.c
 * Utility functions implementation */

#include "wrappers.h"

#include "class.h"
#include "memory.h"
#include "util.h"
#include "vm.h"

/******************************************************************************
 * Debugging facilities                                                       *
 ******************************************************************************/

#ifndef NDEBUG

/** Prints a formatted string to the stderr stream, warning the user about
 * abnormal situations which do not prevent the program from continuing
 * execution
 * \param filename The name of the source file in which this function was called
 * \param lineno The line where this function was called
 * \param format A format string using the printf() guidelines */

void dbg_warning_internal(const char *filename, int lineno,
                          const char *format, ... )
{
    va_list args;

    fprintf(stderr, "%s:%d: WARNING: ", filename, lineno);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
} // dbg_warning_internal()

/** Prints a formatted string to the stderr stream, notifying the user of an
 * error condition and aborts
 * \param filename The name of the source file in which this function was called
 * \param lineno The line where this function was called
 * \param format A format string using the printf() guidelines */

void dbg_error_internal(const char *filename, int lineno,
                        const char *format, ... )
{
    va_list args;

    fprintf(stderr, "%s:%d: ERROR: ", filename, lineno);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
} // dbg_error_internal()

/** Prints a formatted string to the stderr stream, the function prepends the
 * filename and line where the function was called and appends a newline
 * \param filename The name of the file from which the function was called
 * \param lineno The number of the line in which the function was called
 * \param format A format string using the printf() guidelines */

void dbg_printf_internal(const char *filename, int lineno,
                         const char *format, ... )
{
    va_list args;

    fprintf(stderr, "%s:%d: ", filename, lineno);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
} // dbg_printf_internal()

/** Aborts execution and emits a warning that the code has reached a point which
 * shouldn't be reachable under normal conditions
 * \param filename The name of the file from which the function was called
 * \param lineno The number of the line in which the function was called */

void dbg_unreachable_internal(const char *filename, int lineno)
{
    fprintf(stderr, "%s:%d: ERROR: Impossible control flow\n", filename,
            lineno);
    abort();
} // dbg_unreachable_internal()

#endif // !NDEBUG

/******************************************************************************
 * String functions                                                           *
 ******************************************************************************/

/** Replacement function for strchrnul() which is a GNU extension
 * \param s String to search for the character \a c
 * \param c The character to look for
 * \returns A pointer to the first instance of \a c or a pointer to the nul
 * terminator if \a c is not found in the string */

char *cstrchrnul(const char *s, int c)
{
    while ((*s != '\0') && (*s != c)) {
        s++;
    }

    return (char *) s;
} // cstrchrnul()

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/** Sorts an uint16 array and a pointer array using the integer array as the
 * reference. This function is used to sort the interface table
 * \param array A pointer to an uint16 array
 * \param ptrs A pointer to an array of pointers
 * \param len The length of the arrays */

void sort_asc_uint16_ptrs(uint16_t *array, void **ptrs, uint32_t len)
{
    uint32_t i = 0;
    uint32_t k = len - 1;
    uint32_t min, max, j, temp;
    void *temp_ptr;

    while (i < k) {
        min = i;
        max = i;

        for (j = i + 1; j <= k; j++) {
            if (array[j] < array[min]) {
                min = j;
            }

            if (array[j] > array[max]) {
                max = j;
            }
        }

        temp = array[min];
        temp_ptr = ptrs[min];
        array[min] = array[i];
        ptrs[min] = ptrs[i];
        array[i] = temp;
        ptrs[i] = temp_ptr;

        if (max == i) {
            temp = array[min];
            temp_ptr = ptrs[min];
            array[min] = array[k];
            ptrs[min] = ptrs[k];
            array[k] = temp;
            ptrs[k] = temp_ptr;
        } else {
            temp = array[max];
            temp_ptr = ptrs[max];
            array[max] = array[k];
            ptrs[max] = ptrs[k];
            array[k] = temp;
            ptrs[k] = temp_ptr;
        }

        i++;
        k--;
    }
} // sort_asc_uint16_ptrs()

/******************************************************************************
 * Exceptions                                                                 *
 ******************************************************************************/

#if !NDEBUG

/** Throw a C pseudo-exception in debug mode
 * \param file The file were the exception was thrown
 * \param line The line were the exception was thrown
 * \param exc The exception number / type
 * \param desc A printf()-like formatted string holding a description of the
 * thrown exception */

void c_throw_dbg(const char *file, int line, int exc, const char *desc, ...)
{
    thread_t *self = thread_self();
    va_list args, copy;
    char dummy[1] = { 0 };
    size_t len;

    // Set the exception information
    self->c_exception.file = file;
    self->c_exception.line = line;

    if (desc != NULL) {
        // Calculate the length of the exception message ...
        va_start(args, desc);
        va_copy(copy, args);
        len = vsnprintf(dummy, 1, desc, copy);
        va_end(copy);

        // ... and print it into the exception description
        self->c_exception.description = gc_malloc(len + 1);
        vsnprintf(self->c_exception.description, len + 1, desc, args);
        va_end(args);
    } else {
        desc = NULL;
    }

    longjmp(*(self->c_exception.buf), exc); // Jump to the closest catcher
} // c_throw_dbg()

#else

/** Throw a C pseudo-exception
 * \param exc The exception number / type
 * \param desc A printf()-like formatted string holding a description of the
 * thrown exception */

void c_throw(int exc, const char *desc, ...)
{
    thread_t *self = thread_self();
    va_list args, copy;
    char dummy[1] = { 0 };
    size_t len;

    if (desc != NULL) {
        // Calculate the length of the exception message ...
        va_start(args, desc);
        va_copy(copy, args);
        len = vsnprintf(dummy, 1, desc, copy);
        va_end(copy);

        // ... and print it into the exception description
        self->c_exception.description = gc_malloc(len + 1);
        vsnprintf(self->c_exception.description, len + 1, desc, args);
        va_end(args);
    } else {
        desc = NULL;
    }

    longjmp(*(self->c_exception.buf), exc); // Jump to the closest catcher
} // c_throw()

#endif // !NDEBUG

/** Clear the current exception. This frees the exception description */

void c_clear_exception( void )
{
    thread_t *self = thread_self();

    gc_free(self->c_exception.description);
    self->c_exception.description = NULL;
} // c_clear_exception()

/** Print the exception of type \a exc currently held by the calling thread
 * \param exc An exception type */

void c_print_exception(int exc)
{
    static const char *names[] = {
        "",
        "java.lang.VirtualMachineError",
        "java.lang.NoClassDefFoundError"
    };

    thread_t *self = thread_self();
    const char *desc = self->c_exception.description;

#ifndef NDEBUG
    printf("Thrown exception:\n"
           "   type:\t\t%s\n"
           "   description:\t\t%s\n"
           "   file:\t\t%s\n"
           "   line:\t\t%d\n",
           names[exc], desc ? desc : "", self->c_exception.file,
           self->c_exception.line);
#else
    printf("Thrown exception:\n"
           "    type:\t\t%s\n"
           "    description:\t\t%s\n",
           names[exc], desc ? desc : "");
#endif // !NDEBUG
} // c_print_exception()

/******************************************************************************
 * Time functionality                                                         *
 ******************************************************************************/

/** Returns a timespec structure filled with the current time plus the offset
 * specified by \a ms and \a nanos
 * \param ms An offset in microseconds
 * \param nanos An offset in nanoseconds
 * \returns The current time plus the specified offset */

struct timespec get_time_with_offset(uint64_t ms, uint32_t nanos)
{
    struct timespec res;

#if HAVE_CLOCK_GETTIME
    clock_gettime(CLOCK_REALTIME, &res);
#elif HAVE_GETTIMEOFDAY
    struct timeval now;

    gettimeofday(&now, NULL);
    res.tv_sec = now.tv_sec;
    res.tv_nsec = now.tv_usec * 1000;
#else
#   error "get_time_with_offset() doesn't have an appropriate fallback."
#endif

    res.tv_sec += (ms / 1000);
    res.tv_nsec += (ms % 1000) * 1000000 + nanos;

    if (res.tv_nsec > 1000000000) {
        res.tv_nsec -= 1000000000;
        res.tv_sec++;
    }

    return res;
} // get_time_with_offset()

