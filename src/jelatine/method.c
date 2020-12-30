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

/** \file method.c
 * Method handling implementation */

#include "wrappers.h"

#include "class.h"
#include "classfile.h"
#include "constantpool.h"
#include "loader.h"
#include "method.h"
#include "memory.h"
#include "native.h"
#include "opcodes.h"
#include "util.h"

/******************************************************************************
 * Global declarations                                                        *
 ******************************************************************************/

/** Dummy variable used to hold the halt method's bytecode */
uint8_t halt_method_code[] = { WIDE, HALT, WIDE, HALT };

/** Dummy exception handler used by the halt method */

exception_handler_t halt_exception_handler = {
    0, // start_pc
    4, // end_pc
    halt_method_code, // handler_pc
    NULL // catch_type
};

/** Halt method structure */

method_t halt_method = {
    "halt_method", // name
    NULL, // descriptor
    NULL, // cp
    0, // access_flags
    0, // args_size
    0, // index
    0, // max_stack
    0, // max_locals
    RET_VOID, // return_type
    4, // code_length
    1, // exception_table_length
    halt_method_code, // code
    { NULL } // handlers
};

/** Holds the dummy code of an abstract method */
static uint8_t abstract_method_code[] = { WIDE, METHOD_ABSTRACT };

/** Holds the dummy code of a not-yet-linked method */
static uint8_t load_method_code[] = { WIDE, METHOD_LOAD };

/** Holds the dummy code of a native method */
static uint8_t native_method_code[] = { WIDE, INVOKE_NATIVE };

/******************************************************************************
 * Method implementation                                                      *
 ******************************************************************************/

/** Initializes the dummy methods used by the interpreter */

void init_dummy_methods( void )
{
    class_t *object = bcl_resolve_class(NULL, "java/lang/Object");

    halt_exception_handler.catch_type = object;
    halt_method.cp = cp_create_dummy();
    halt_method.data.handlers = &halt_exception_handler;
} // init_dummy_methods()

/** Creates a method manager
 * \param count The number of methods the manager will hold
 * \returns A pointer to the newly created method manager */

method_manager_t *mm_create(uint32_t count)
{
    method_manager_t *mm = gc_palloc(sizeof(method_manager_t));

#ifndef NDEBUG
    mm->reserved = count;
#endif // !NDEBUG
    mm->methods = gc_palloc(count * sizeof(method_t));

    return mm;
} // mm_create()

/** Parses a method's descriptor, throws an exception upon failure
 * \param method A pointer to the relevant method */

static void parse_method_descriptor(method_t *method)
{
    const char *desc = method->descriptor;
    size_t i = 0, j = 0; // A couple of counters
    uint32_t args_size = 0; // Size (in stack slots) of the arguments
    uint32_t dimensions = 0; // Array dimensions

    switch (desc[i]) {
        case '(':
            i++;
            goto arguments_pass;

        default:
            goto malformed_descriptor_exception;
    }

arguments_pass:

    switch (desc[i]) {
        case ')':
            if ((dimensions != 0) || (desc[i + 1] == 0)) {
                goto malformed_descriptor_exception;
            }

            i++;
            goto return_type_pass;

        case 'B':
        case 'C':
        case 'S':
        case 'Z':
        case 'I':
        case 'F':
            dimensions = 0;
            args_size++;
            i++;
            goto arguments_pass;

        case 'J':
        case 'D':
            if (dimensions != 0) {
                dimensions = 0;
                args_size++;
            } else {
                args_size += 2;
            }

            i++;
            goto arguments_pass;

        case 'L':
            dimensions = 0;
            j = i + 1;

            while (desc[j] != ';' && desc[j] != 0) {
                j++;
            }

            if ((desc[j] == 0) || (j == i + 1)) {
                goto malformed_descriptor_exception;
            }

            args_size++;
            i = j + 1;
            goto arguments_pass;

        case '[':
            dimensions++;
            i++;
            goto arguments_pass;

        default:
            goto malformed_descriptor_exception;
    }

return_type_pass:

    switch (desc[i]) {
        case 'V':
            break;

        case 'B':
        case 'C':
        case 'F':
        case 'I':
        case 'S':
        case 'Z':
            break;

        case 'J':
        case 'D':
            break;

        case 'L':
            j = i + 1;

            while (desc[j] != ';' && desc[j] != 0) {
                j++;
            }

            if ((desc[j] == 0) || (j == i + 1)) {
                goto malformed_descriptor_exception;
            }

            i = j;
            break;

        case '[':
             j = i;

            while (desc[j] == '[')
                j++;

            if (desc[j] == 'L') {
                while (desc[j] != ';' && desc[j] != 0) {
                    j++;
                }

                if ((desc[j] == 0) || (j == i + 1)) {
                    goto malformed_descriptor_exception;
                }
            } else {
                switch (desc[j]) {
                    case 'B':
                    case 'C':
                    case 'D':
                    case 'F':
                    case 'I':
                    case 'J':
                    case 'S':
                    case 'Z':
                        break;

                    default:
                        goto malformed_descriptor_exception;
                }
            }

            i = j;
            break;

        default:
            goto malformed_descriptor_exception;
    }

    if (desc[i + 1] != 0) {
        goto malformed_descriptor_exception;
    }

    if (!method_is_static(method)) {
        args_size++; // Include the 'this' pointer as an argument
    }

    if (args_size >= METHOD_ARGUMENTS_MAX) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Number of arguments in a method exceed the VM limits");
    }

    method->args_size = args_size;
    return;

malformed_descriptor_exception:
    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Malformed method descriptor");
} // parse_method_descriptor()

/** Checks the access flags of a method
 * \param method The method to be checked */

static void method_check_access_flags(method_t *method)
{
    uint16_t access_flags;

    access_flags = method->access_flags;

    if (((access_flags & ACC_PRIVATE) && (access_flags & ACC_PROTECTED))
        || ((access_flags & ACC_PRIVATE) && (access_flags & ACC_PUBLIC))
        || ((access_flags & ACC_PROTECTED) && (access_flags & ACC_PUBLIC)))
    {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Wrong combination of method access flags");
    }

    if ((access_flags & ACC_ABSTRACT)
        && ((access_flags & ACC_FINAL) || (access_flags & ACC_SYNCHRONIZED)
            || (access_flags & ACC_PRIVATE) || (access_flags & ACC_STATIC)
            || (access_flags & ACC_STRICT) || (access_flags & ACC_NATIVE)))
    {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Wrong combination of method access flags");
    }

    if (method_is_init(method)) {
        if ((access_flags & ACC_STATIC) || (access_flags & ACC_FINAL)
            || (access_flags & ACC_SYNCHRONIZED) || (access_flags & ACC_NATIVE)
            || (access_flags & ACC_ABSTRACT))
        {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Constructor is either static, final, synchronized, native "
                    "or abstract");
        }
    }
} // method_check_access_flags()

/** Adds a new method to the method manager
 * \param mm The method manager
 * \param name The method name
 * \param descriptor The method descriptor
 * \param access_flags Access flags of the method
 * \param cp The method's constant pool
 * \param attr The method attributes */

void mm_add(method_manager_t *mm, char *name, char *descriptor,
            uint16_t access_flags, const_pool_t *cp, method_attributes_t *attr)
{
    uint32_t i;
    method_t *method;

    if (mm_get(mm, name, descriptor) != NULL) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Duplicated methods with same name and descriptor");
    }

    assert(mm->entries < mm->reserved);

    i = mm->entries;
    method = mm->methods + i;

    method->name = name;
    method->descriptor = descriptor;
    method->access_flags = access_flags;
    method->cp = cp;
    method_check_access_flags(method);

    if (access_flags & ACC_NATIVE) {
        if (attr->code_found == true) {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Code attribute found for a method declared as native");
        }

        // A native method doesn't specify any of these
        method->max_stack = 0;
        method->max_locals = 0;
        method->code_length = 0;
        method->exception_table_length = 0;
        method->code = NULL;
    } else if (access_flags & ACC_ABSTRACT) {
        if (attr->code_found == true) {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Code attribute found for a method declared as abstract");
        }
         // Make the method code point to the dummy abstract method code
        method->max_stack = 0;
        method->max_locals = 0;
        method->code_length = 1;
        method->exception_table_length = 0;
        method->code = abstract_method_code;
        method->data.handlers = NULL;
   } else {
        if (attr->code_found == false) {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Java method lacks the Code attribute");
        }

        method->max_stack = attr->max_stack;
        method->max_locals = attr->max_locals;
        method->code_length = attr->code_length;
        method->exception_table_length = attr->exception_table_length;
        method->code = load_method_code;
        method->data.offset = attr->code_offset;
    }

    // Finally parse and validate the method descriptor
    parse_method_descriptor(method);

    mm->entries++;
} // mm_add()

/** Finds a method by its name and descriptor, returns NULL if the method is not
 * found
 * \param mm A pointer to the method manager
 * \param name The method name
 * \param descriptor The method descriptor
 * \returns A pointer to the method if it was found, NULL otherwise */

method_t *mm_get(method_manager_t *mm, char *name, char *descriptor)
{
    for (size_t i = 0; i < mm->entries; i++) {
        if (strcmp(mm->methods[i].name, name) == 0) {
            if (strcmp(mm->methods[i].descriptor, descriptor) == 0) {
                return mm->methods + i;
            }
        }
    }

    return NULL;
} // mm_get()

/** Returns the number of methods in the manager
 * \param mm A pointer to the method manager
 * \returns The number of methods in the manager */

uint32_t mm_get_count(method_manager_t *mm)
{
    return mm->entries;
} // mm_get_count()

/** Creates the packed index of a method
 * \param method A pointer to a method
 * \returns The packed index */

uint16_t method_create_packed_index(const method_t *method)
{
    return (method->index << METHOD_ARGUMENTS_BITS) | method->args_size;
} // method_create_packed_index()

/** Returns true if the method is an initialization method
 * \param method A pointer to a method
 * \returns true if an initialization method was passed to the function, false
 * otherwise */

bool method_is_init(const method_t *method)
{
    if (strcmp(method->name, "<init>") == 0) {
        return true;
    } else {
      return false;
    }
} // method_is_init()

/** Gets the length of a method code, the length is automatically
 * adjusted if extra opcodes were added by the translator
 * \param method A pointer to a method structure
 * \returns The length of the bytecode in bytes */

uint32_t method_get_code_length(const method_t *method)
{
    if (method_is_synchronized(method)) {
        return method->code_length + 1;
    } else {
        return method->code_length;
    }
} // method_get_code_length()

/** Compares two methods signatures
 * \param method1 A pointer to a method structure
 * \param method2 A pointer to a method structure
 * \returns true if the method signatures match, false otherwise */

bool method_compare(const method_t *method1, const method_t *method2)
{
    if ((strcmp(method1->name, method2->name) == 0)
        && (strcmp(method1->descriptor, method2->descriptor) == 0))
    {
        return true;
    } else {
        return false;
    }
} // method_compare()

/** Link a native method
 * \param method A pointer to the method
 * \param class_name The name of the class the method belongs to */

void method_link_native(method_t *method, char *class_name)
{
    const char *desc = method->descriptor;

    method->access_flags |= ACC_LINKED;
    method->code = native_method_code;
    method->data.function = native_method_lookup(class_name, method->name,
                                                 method->descriptor);

   if (method->data.function == NULL) {
        fprintf(stderr,"\nError!Native method implementation not found."
			"\nClassname:%s,method name:%s\n",class_name,method->name);
			
		c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Native method implementation not found");
    }

    // Examine and set the return type

    while (*desc++ != ')')
        ;

    switch (*desc) {
        case 'V': method->return_type = RET_VOID;   break;
        case 'B': // FALLTHROUGH
        case 'C': // FALLTHROUGH
        case 'S': // FALLTHROUGH
        case 'Z': // FALLTHROUGH
        case 'I': method->return_type = RET_INT;    break;
        case 'J': method->return_type = RET_LONG;   break;
#if JEL_FP_SUPPORT
        case 'F': method->return_type = RET_FLOAT;  break;
        case 'D': method->return_type = RET_DOUBLE; break;
#endif // JEL_FP_SUPPORT
        case 'L': // FALLTHROUGH
        case '[': method->return_type = RET_OBJECT; break;
        default:  dbg_unreachable();
    }
 } // method_link_native()

/** Frees the code and exception handlers of a method
 * \param method A pointer to the method */

void method_purge(method_t *method)
{
    gc_free(method->code);
    gc_free(method->data.handlers);
} // method_purge()


