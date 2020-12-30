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

/** \file classfile.h
 * Wrapper structure for class file access, this file also contains the
 * definitions which can be found in a class-file */

/** \def JELATINE_CLASSFILE_H
 * classfile.h inclusion macro */

#ifndef JELATINE_CLASSFILE_H
#   define JELATINE_CLASSFILE_H (1)

#include "wrappers.h"

#if JEL_JARFILE_SUPPORT
#   include <zzip/lib.h>
#endif // JEL_JARFILE_SUPPORT

#include "util.h"

/******************************************************************************
 * Classpath interface                                                        *
 ******************************************************************************/

extern void classpath_init( void );
extern void classpath_teardown( void );

/******************************************************************************
 * Class file type definitions                                                *
 ******************************************************************************/

/** Represents the basic type of a field or element of a Java class */

enum primitive_type_t {
    PT_BYTE, ///< Field of type byte
    PT_BOOL, ///< Field of type bool
    PT_CHAR, ///< Field of type char
    PT_SHORT, ///< Field of type short
    PT_INT, ///< Field of type int
    PT_FLOAT, ///< Field of type float
    PT_LONG, ///< Field of type long
    PT_DOUBLE, ///< Field of type double
    PT_REFERENCE, ///< Generic reference field
    PT_VOID ///< Used only for method descriptors!
};

/** Typedef for enum primitive_type_t */
typedef enum primitive_type_t primitive_type_t;

/** Defines the type of a constant pool element. Note that some of the entries
 * are not part of the Java VM spec but were specifically added by the Jelatine
 * JVM. Those are not found in regular class files but will be used by its
 * in-memory representation */

enum const_pool_type_t {
    CONSTANT_Utf8 = 1, ///< Modified UTF8 string
    CONSTANT_Integer = 3, ///< 32-bit Integer constant
    CONSTANT_Float = 4, ///< Single-precision floating point constant
    CONSTANT_Long = 5, ///< 64-bit Integer constant
    CONSTANT_Double = 6, ///< Double-precision floating point constant
    CONSTANT_Class = 7, ///< Represents a class or an interface
    CONSTANT_String = 8, ///< Java string constant
    CONSTANT_Fieldref = 9, ///< Represents a class' field
    CONSTANT_Methodref = 10, ///< Represents a class' method
    CONSTANT_InterfaceMethodref = 11, ///< Represents an interface's method
    CONSTANT_NameAndType = 12, ///< Name and type of a field or method
    // The following constants are internal to the VM and not seen in a file
    CONSTANT_Class_resolved = 13, ///< Represents a resolved class
    CONSTANT_Fieldref_resolved = 14, ///< Represents a resolved field
    CONSTANT_Methodref_resolved = 15, ///< Represents a resolved method
    CONSTANT_InterfaceMethodref_resolved = 2 ///< Represents a resolved interface method
};

/** Typedef for enum const_pool_type_t */
typedef enum const_pool_type_t const_pool_type_t;

/** Represents the different access flags of a class, field or method */

enum access_flags_t {
    ACC_PUBLIC = 0x0001, ///< Public access is allowed
    ACC_PRIVATE = 0x0002, ///< This field or method is private
    ACC_PROTECTED = 0x0004, ///< This field is protected
    ACC_STATIC = 0x0008, ///< This field or method is static
    ACC_FINAL = 0x0010, ///< This object or method cannot be modified
    ACC_SYNCHRONIZED = 0x0020, ///< This method is synchronized
    ACC_SUPER = 0x0020, ///< Alters the behaviour of INVOKESPECIAL
    ACC_VOLATILE = 0x0040, ///< This field cannot be cached
    ACC_TRANSIENT = 0x0080, ///< Not written or read by a POM
    ACC_NATIVE = 0x0100, ///< This method is native
    ACC_INTERFACE = 0x0200, ///< This class is an interface
    ACC_ABSTRACT = 0x0400, ///< This class is abstract
    ACC_STRICT = 0x0800, ///< This method is strict-fp
    // Internal flags for classes
    ACC_ARRAY = 0x1000, ///< This class is an array
    ACC_HAS_FINALIZER = 0x2000, ///< This class has a finalizer
    // Internal flags for methods
    ACC_LINKED = 0x1000, ///< This method has been linked
    ACC_MAIN = 0x2000 ///< This is the main() method
};

/** Typedef for enum access_flags_t */
typedef enum access_flags_t access_flags_t;

/**  Mask of defined flags of a class access_flags field */
#define CLASS_ACC_FLAGS_MASK \
    (ACC_PUBLIC | ACC_FINAL | ACC_SUPER | ACC_INTERFACE | ACC_ABSTRACT)
/**  Mask of defined flags of a field access_flags field */
#define FIELD_ACC_FLAGS_MASK \
    (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL | \
    ACC_VOLATILE | ACC_TRANSIENT)
/**  Mask of defined flags of a method access_flags field */
#define METHOD_ACC_FLAGS_MASK \
    (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL | \
    ACC_SYNCHRONIZED | ACC_NATIVE | ACC_ABSTRACT | ACC_STRICT)

/** Represents a class-file, this structure is used as a wrapper over the real
 * class file so that the class-loader code can ignore if the class-file is a
 * regular file or is embedded in a jar archive */

struct class_file_t {
    union {
        FILE *plain; ///< Plain file structure
#if JEL_JARFILE_SUPPORT
        ZZIP_FILE *compressed; ///< Compressed file
#endif // JEL_JARFILE_SUPPORT
    } file; ///< File handle

    bool jar; ///< True if the file is inside a JAR archive
};

/** Typedef for struct class_file_t */
typedef struct class_file_t class_file_t;

/******************************************************************************
 * Class file interface                                                       *
 ******************************************************************************/

extern class_file_t *cf_open(const char *);
extern void cf_close(class_file_t *);
extern uint8_t cf_load_u1(class_file_t *);
extern uint16_t cf_load_u2(class_file_t *);
extern uint32_t cf_load_u4(class_file_t *);
extern void cf_seek(class_file_t *, long, int);
extern long cf_tell(class_file_t *);
#if JEL_JARFILE_SUPPORT
extern ZZIP_FILE *jar_get_resource(const char *);
#endif // JEL_JARFILE_SUPPORT

/******************************************************************************
 * Class file components                                                      *
 ******************************************************************************/

/** Representation of a field description */

struct field_info_t {
    uint16_t access_flags; ///< Access flags
    const char *name; ///< Field name
    const char *descriptor; ///< Field descriptor
};

/** Typedef for struct field_info_t */
typedef struct field_info_t field_info_t;

/** Representation of a field attributes */

struct field_attributes_t {
    bool constant_value_found; ///< True when 'ConstantValue' attribute is found
    uint16_t constant_value_index; ///< Index of the constant value
};

/** Typedef for struct field_attributes_t */
typedef struct field_attributes_t field_attributes_t;

/** Represents the attributes of a method */

struct method_attributes_t {
    bool code_found; ///< true if the Code attribute was found
    uint16_t max_stack; ///< Holds the maximum number of stack slots used
    uint16_t max_locals; ///< Holds the maximum number of locals used
    uint16_t code_length; ///< Length of the code (in bytes)
    long code_offset; ///< Offset inside the class file of the Code attribute
    bool exceptions_found; ///< true Exceptions attribute was found
    uint16_t exception_table_length; ///< Number of exception handlers
};

/** Typedef for struct method_attributes_t */
typedef struct method_attributes_t method_attributes_t;

#endif // !JELATINE_CLASSFILE_H
