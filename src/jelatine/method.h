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

/** \file method.h
 * Method structure and interface */

/** \def JELATINE_METHOD_H
 * method.h inclusion macro */

#ifndef JELATINE_METHOD_H
#   define JELATINE_METHOD_H (1)

#include "wrappers.h"

#include "constantpool.h"
#include "util.h"

// Forward declarations

struct class_t;

/******************************************************************************
 * Method type declarations                                                   *
 ******************************************************************************/

/** Defines the number of bits of a method index used for encoding the number of
 * arguments the method accepts */
#define METHOD_ARGUMENTS_BITS (4)

/** Defines the maximum number of arguments accepted by a method */
#define METHOD_ARGUMENTS_MAX (1 << METHOD_ARGUMENTS_BITS)

/** Defines a bit mask to extract the number of arguments of a method */
#define METHOD_ARGUMENTS_MASK (METHOD_ARGUMENTS_MAX - 1)

/** Defines the number of bits of a method index used for encoding the actual
 * index in the dispatch table */
#define METHOD_INDEX_BITS (12)

/** Defines the maximum number of methods which can be declared by a class */
#define METHOD_INDEX_MAX (1 << METHOD_INDEX_BITS)

/** Defines the shift value used to extract the actual method index from the
 * index encoded in the bytecode */
#define METHOD_INDEX_SHIFT (METHOD_ARGUMENTS_BITS)

/** Represents the basic return type of a method, currently used only for
 * native methods */

enum return_value_t {
    RET_VOID   = 0, ///< No return value
    RET_INT    = 1, ///< boolean, byte, char, short or int return value
    RET_LONG   = 2, ///< long return value
    RET_OBJECT = 3, ///< Object reference return value
    RET_FLOAT  = 4, ///< Float return value
    RET_DOUBLE = 5  ///< Double return value
};

/** Typedef for the enum return_value_t type */
typedef enum return_value_t return_value_t;

/** Represents an exception handle for a Java method */

struct exception_handler_t {
    uint32_t start_pc; ///<  Starting PC (inclusive)
    uint32_t end_pc; ///<  Ending PC (exclusive)
    uint8_t *handler_pc; ///<  PC of the handler's code
    struct class_t *catch_type; ///<  Type of exception catched
};

/** Typedef for struct exception_handler_t */
typedef struct exception_handler_t exception_handler_t;

/** Native methods function pointer type */
typedef void (*native_proto_t)( void );

/** Represents a basic method structure */

struct method_t {
    const char *name; ///< Method name
    const char *descriptor; ///< Method descriptor
    const_pool_t *cp; ///< Method's constant pool
    uint16_t access_flags; ///< Access flags
    uint16_t args_size; ///< Size of the arguments passed
    uint16_t index; ///< Dispatch table index
    uint16_t max_stack; ///< Maximum number of operand stack slots
    uint16_t max_locals; ///< Maximum number of used locals
    uint16_t return_type; ///< Return type for native methods
    uint16_t code_length; ///< Length of the bytecode
    uint16_t exception_table_length; ///< Length of the exception table
    uint8_t *code; ///< Java bytecode of the method

    union {
        exception_handler_t *handlers; ///< Exception handlers
        long offset; ///< Offset in the class file of the method attributes
        native_proto_t function; ///< Native function if available
    } data; ///< Extra method data
};

/** Typedef for struct method_t */
typedef struct method_t method_t;

/** Represents the method manager of a class */

struct method_manager_t {
#ifndef NDEBUG
    uint32_t reserved; ///< Number of methods at creation time
#endif // !NDEBUG
    uint32_t entries; ///< Number of methods directly implemented
    method_t *methods; ///< Methods held by the manager
};

/** Typedef for struct method_manager_t */
typedef struct method_manager_t method_manager_t;

/******************************************************************************
 * Global variables                                                           *
 ******************************************************************************/

/** Dummy method used to halt the interpreter
 *
 * Basically the main() method of the first thread (or the run() method of
 * user-created threads) return to this method when they are done and this
 * method halts the interpreter execution. This method also contains a fake
 * exception handler which catches all possible exceptions and then deals with
 * them as uncatched exceptions */

extern method_t halt_method;

/******************************************************************************
 * Function prototypes                                                        *
 ******************************************************************************/

extern method_manager_t *mm_create(uint32_t);
extern method_t *mm_get(method_manager_t *, char *, char *);
extern uint32_t mm_get_count(method_manager_t *);
extern void mm_add(method_manager_t *, char *, char *, uint16_t,
                   const_pool_t *, method_attributes_t *);

extern void init_dummy_methods( void );

extern bool method_compare(const method_t *, const method_t *);
extern void method_link_native(method_t *, char *);
extern void method_purge(method_t *);
extern uint32_t method_get_code_length(const method_t *);
extern bool method_is_init(const method_t *);
extern uint16_t method_create_packed_index(const method_t *);

/** Gets the size in stack slots of the arguments packed into the translated
 * bytecode
 * \param m The packed method index as found in the translated bytecode
 * \returns The size in stack slots of the arguments */

static inline uint16_t method_unpack_arguments(uint16_t m)
{
    return m & METHOD_ARGUMENTS_MASK;
} // method_unpack_arguments()

/** Gets the index of a method packed into the translated bytecode
 * \param m The packed method index as found in the translated bytecode
 * \returns The actual method index */

static inline uint16_t method_unpack_index(uint16_t m)
{
    return m >> METHOD_INDEX_SHIFT;
} // method_unpack_index()

/** Checks if a method is public
 * \param m A pointer to a method
 * \returns true if the method is public, false otherwise */

static inline bool method_is_public(const method_t *m)
{
    return m->access_flags & ACC_PUBLIC;
} // method_is_public()

/** Checks if a method is protected
 * \param m A pointer to a method
 * \returns true if the method is protected, false otherwise */

static inline bool method_is_protected(const method_t *m)
{
    return m->access_flags & ACC_PROTECTED;
} // method_is_protected()

/** Checks if a method is private
 * \param m A pointer to a method
 * \returns true if the method is private, false otherwise */

static inline bool method_is_private(const method_t *m)
{
    return m->access_flags & ACC_PRIVATE;
} // method_is_private()

/** Checks if a method is native
 * \param m A pointer to a method
 * \returns true if the method is native, false otherwise */

static inline bool method_is_native(const method_t *m)
{
    return m->access_flags & ACC_NATIVE;
} // method_is_native()

/** Checks if a method is static
 * \param m A pointer to a method
 * \returns true if the method is static, false otherwise */

static inline bool method_is_static(const method_t *m)
{
    return m->access_flags & ACC_STATIC;
} // method_is_static()

/** Checks if a method is final
 * \param m A pointer to a method
 * \returns true if the method is final, false otherwise */

static inline bool method_is_final(const method_t *m)
{
    return m->access_flags & ACC_FINAL;
} // method_is_final()

/** Checks if a method is synchronized
 * \param m A pointer to a method
 * \returns true if the method is synchronized, false otherwise */

static inline bool method_is_synchronized(const method_t *m)
{
    return m->access_flags & ACC_SYNCHRONIZED;
} // method_is_synchronized()

/** Checks if a method is abstract
 * \param m A pointer to a method
 * \returns true if the method is abstract, false otherwise */

static inline bool method_is_abstract(const method_t *m)
{
    return m->access_flags & ACC_ABSTRACT;
} // method_is_abstract()

/** Sets the ACC_LINKED flag of a method
 * \param m A pointer to a method */

static inline void method_set_linked(method_t *m)
{
    m->access_flags |= ACC_LINKED;
} // method_set_linked()

/** Checks if a method has been linked
 * \param m A pointer to a method
 * \returns true if the method has been linked, false otherwise */

static inline bool method_is_linked(const method_t *m)
{
    return m->access_flags & ACC_LINKED;
} // method_is_linked()

/** Checks if a method is the main() method
 * \param m A pointer to a method
 * \returns true if the method is the main() method, false otherwise */

static inline bool method_is_main(const method_t *m)
{
    return m->access_flags & ACC_MAIN;
} // method_is_main()

/** Sets the index of a method
 * \param method A pointer to a method structure
 * \param index The method index */

static inline void method_set_index(method_t *method, uint16_t index)
{
    method->index = index;
} // method_set_index()

/** Gets the index of a method
 * \param method A pointer to a method structure */

static inline uint16_t method_get_index(const method_t *method)
{
    return method->index;
} // method_get_index()

/******************************************************************************
 * Method iterator                                                            *
 ******************************************************************************/

/** Method iterator, usually allocated on the stack when used */

struct method_iterator_t {
    method_t *methods; ///< Pointer to the methods of a class
    size_t entries; ///< Number of methods
    size_t index; ///< Current index in the fields array
};

/** Typedef for the     struct method_iterator_t type */
typedef struct method_iterator_t method_iterator_t;

/** Creates a method iterator from the method manager \a mm
 * \param mm A pointer to the method manager
 * \returns An initialized method iterator */

static inline method_iterator_t method_itr(method_manager_t *mm)
{
    method_iterator_t itr;

    itr.methods = mm->methods;
    itr.entries = mm->entries;
    itr.index = 0;

    return itr;
} // method_itr()

/** Returns true if another method is available
 * \param itr A method iterator
 * \returns true if there are more methods, false otherwise */

static inline bool method_itr_has_next(method_iterator_t itr)
{
    return itr.index < itr.entries;
} // method_itr_has_next()

/** Returns the next method, if the iterator has just been created it returns
 * the data of the first field (if it is present)
 * \param itr A pointer to the field iterator
 * \returns A pointer to an instance field */

static inline method_t *method_itr_get_next(method_iterator_t *itr)
{
    assert(method_itr_has_next(*itr));

    return itr->methods + itr->index++;
} // method_itr_get_next()

#endif // JELATINE_METHOD_H
