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

/** \file vm.h
 * Virtual machine definition and interface */

/** \def JELATINE_VM_H
 * vm.h inclusion macro */

#ifndef JELATINE_VM_H
#   define JELATINE_VM_H (1)

#include "wrappers.h"
#include "class.h"
#include "memory.h"
#include "thread.h"

/******************************************************************************
 * Virtual machine interface                                                  *
 ******************************************************************************/

extern void vm_run( void );

#if JEL_TRACE
extern bool vm_opt_trace_bytecodes( void );
extern bool vm_opt_trace_methods( void );
#endif // JEL_TRACE

#if JEL_PRINT
extern bool vm_opt_print_bytecodes( void );
extern bool vm_opt_print_methods( void );
#endif // JEL_PRINT

/******************************************************************************
 * Inlined virtual machine functions                                          *
 ******************************************************************************/

/** Terminates abruptly the VM */

static inline void vm_fail( void )
{
    // TODO: This should be improved
    exit(1);
} // vm_fail()

/******************************************************************************
 * Options type definitions                                                   *
 ******************************************************************************/

/** Virtual machine options structure, holds the global options of the virtual
 * machine obtained by parsing the command-line */

struct options_t {
    char *classpath; ///< Default classpath
    char *boot_classpath; ///< Default classpath for system classes
    size_t heap_size; ///< Heap size
    size_t stack_size; ///< Size of a thread's stack
    char *main_class; ///< Name of the class holding the main() method
    char **jargs; ///< Arguments of the Java application
    int jargs_n; ///< Number of arguments

#if JEL_TRACE
    bool trace_methods; ///< True if method tracing is enabled
    bool trace_opcodes; ///< True if opcode tracing is enabled
#endif // JEL_TRACE

#if JEL_PRINT
    bool print_methods; ///< True if method printing is enabled
    bool print_opcodes; ///< True if opcode printing is enabled
    bool print_memory; ///< True if memory operations printing is enabled
#endif // JEL_PRINT

    bool version; ///< True if the machine should print its version number
    bool help; ///< True if the machines should print the help notice
};

/** Typedef for the struct options_t */
typedef struct options_t options_t;

extern options_t options;

/******************************************************************************
 * Virtual machine options interface                                          *
 ******************************************************************************/

/** Sets the global option 'classpath'
 * \param classpath A pointer to a string holding the new value of the
 * classpath */

static inline void opts_set_classpath(char *classpath)
{
    options.classpath = classpath;
} // opts_set_classpath()

/** Gets the global option 'classpath'
 * \returns The classpath string */

static inline char *opts_get_classpath( void )
{
    return options.classpath;
} // opts_get_classpath()

/** Sets the global option 'default classpath'
 * \param boot_classpath A pointer to a string holding the new value of the
 * boot classpath */

static inline void opts_set_boot_classpath(char *boot_classpath)
{
    options.boot_classpath = boot_classpath;
} // opts_set_boot_classpath()

/** Gets the global option 'boot classpath'
 * \returns The boot classpath string */

static inline char *opts_get_boot_classpath( void )
{
    return options.boot_classpath;
} // opts_get_boot_classpath()

/** Sets the global option 'heap size'
 * \param size The maximum size of the heap */

static inline void opts_set_heap_size(size_t size)
{
    options.heap_size = size;
} // opts_set_heap_size()

/** Gets the global option 'heap size'
 * \returns The maximum size of the heap */

static inline size_t opts_get_heap_size( void )
{
    return options.heap_size;
} // opts_get_heap_size()

/** Sets the global option 'stack size'
 * \param size The default size of a thread's stack */

static inline void opts_set_stack_size(size_t size)
{
    options.stack_size = size;
} // opts_set_stack_size()

/** Gets the global option 'stack size'
 * \returns size The default size of a thread's stack */

static inline size_t opts_get_stack_size( void )
{
    return options.stack_size;
} // opts_get_stack_size()

/** Sets the global option 'main class'
 * \param class_name A string holding the name of the class which main method
 * should be called by the virtual machine */

static inline void opts_set_main_class(char *class_name)
{
    options.main_class = class_name;
} // opts_set_main_class()

/** Gets the global option 'main class'
 * \returns A string holding the name of the main class */

static inline char *opts_get_main_class( void )
{
    return options.main_class;
} // opts_get_main_class()

/** Sets the global option 'jargs'
 * \param jargs A pointer to an array of strings holding the parameters passed
 * to the Java application */

static inline void opts_set_jargs(char **jargs)
{
    options.jargs = jargs;
} // opts_set_jargs()

/** Gets the global option 'jargs'
 * \returns An array of strings holding the arguments passed to the Java
 * application */

static inline char **opts_get_jargs( void )
{
    return options.jargs;
} // opts_get_jargs()

/** Sets the global option 'jargs_n'
 * \param n The number of arguments passed to the Java application */

static inline void opts_set_jargs_n(int n)
{
    options.jargs_n = n;
} // opts_set_jargs_n()

/** Gets the global option 'jargs_n'
 * \returns The number of arguments passed to the Java application */

static inline int opts_get_jargs_n( void )
{
    return options.jargs_n;
} // opts_set_jargs_n()

#if JEL_TRACE

/** Sets the global option 'trace methods'
 * \param enable true if method tracing must be enabled, false otherwise */

static inline void opts_set_trace_methods(bool enable)
{
    options.trace_methods = enable;
} // opts_set_trace_methods()

/** Gets the global option 'trace methods'
  * \returns true if method tracing must be enabled, false otherwise */

static inline bool opts_get_trace_methods( void )
{
    return options.trace_methods;
} // opts_get_trace_methods()

/** Sets the global option 'trace opcodes'
 * \param enable true if opcode tracing must be enabled, false otherwise */

static inline void opts_set_trace_opcodes(bool enable)
{
    options.trace_opcodes = enable;
} // opts_set_trace_opcodes()

/** Gets the global option 'trace opcodes'
 * \returns true if opcode tracing must be enabled, false otherwise */

static inline bool opts_get_trace_opcodes( void )
{
    return options.trace_opcodes;
} // opts_get_trace_opcodes()

#endif // JEL_TRACE

#if JEL_PRINT
/** Sets the global option 'print methods'
 * \param enable true if method printing must be enabled, false otherwise */

static inline void opts_set_print_methods(bool enable)
{
    options.print_methods = enable;
} // opts_set_print_methods()

/** Gets the global option 'print methods'
 * \returns true if method printing must be enabled, false otherwise */

static inline bool opts_get_print_methods( void )
{
    return options.print_methods;
} // opts_get_print_methods()

/** Sets the global option 'print opcodes'
 * \param enable true if opcode printing must be enabled, false otherwise */

static inline void opts_set_print_opcodes(bool enable)
{
    options.print_opcodes = enable;
} // opts_set_print_opcodes()

/** Gets the global option 'print opcodes'
 * \returns true if method tracing must be enabled, false otherwise */

static inline bool opts_get_print_opcodes( void )
{
    return options.print_opcodes;
} // opts_get_print_opcodes()

/** Sets the global option 'print memory'
 * \param enable true if memory printing must be enabled, false otherwise */

static inline void opts_set_print_memory(bool enable)
{
    options.print_memory = enable;
} // opts_set_print_memory()

/** Gets the global option 'print memory'
 * \returns true if memory printing must be enabled, false otherwise */

static inline bool opts_get_print_memory( void )
{
    return options.print_memory;
} // opts_get_memory()

#endif // JEL_PRINT

/** Sets the global option 'version'
 * \param enable true if version information must be displayed, false
 * otherwise */

static inline void opts_set_version(bool enable)
{
    options.version = enable;
} // opts_set_version()

/** Gets the global option 'version'
 * \returns true if version information must be displayed, false otherwise */

static inline bool opts_get_version( void )
{
    return options.version;
} // opts_get_version

/** Sets the global option 'help'
 * \param enable true if help information must be displayed, false otherwise */

static inline void opts_set_help(bool enable)
{
    options.help = enable;
} // opts_set_help()

/** Gets the global option 'help'
 * \returns true if help information must be displayed, false otherwise */

static inline bool opts_get_help( void )
{
    return options.help;
} // opts_get_help()

#endif // !JELATINE_VM_H
