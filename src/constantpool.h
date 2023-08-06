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

/** \file constantpool.h
 * Constant pool definition and interface */

/** \def JELATINE_CONSTANTPOOL_H
 * constantpool.h inclusion macro */

#ifndef JELATINE_CONSTANTPOOL_H
#   define JELATINE_CONSTANTPOOL_H (1)

#include "wrappers.h"

#include "classfile.h"
#include "field.h"
#include "util.h"

// Forward declarations

struct class_t;
struct field_t;
struct static_field_t;
struct method_t;

/******************************************************************************
 * Constant pool type definitions                                             *
 ******************************************************************************/

/** Represents a CONSTANT_Fieldref_info structure, but also a
 * CONSTANT_Methodref_info structure or a CONSTANT_InterfaceMethodref structure
 */

struct fieldref_t {
    uint16_t class_index; ///< Index of the class
    uint16_t name_and_type_index; ///< Name-and-type index of the ref
};

/** Typedef for struct fieldref_t */
typedef struct fieldref_t fieldref_t;

/** Represents a CONSTANT_NameAndType_info structure */

struct name_and_type_t {
    uint16_t name_index; ///< Name index of the field/method
    uint16_t descriptor_index; ///< Descriptor index
};

/** Typedef for struct name_and_type_t */
typedef struct name_and_type_t name_and_type_t;

/** Structure representing a class' constant pool. Note that the first entry of
 * the constant pool is usually unaccessible, yet the cp_set_tag_and_data() is
 * allowed to access it. The entry there contained is a pointer to the class to
 * which the constant pool belongs too */

struct const_pool_t {
    uint16_t entries; ///< Number of entries in the costant-pool
    uint8_t *tags; ///< Tags array
    jword_t data[]; ///< Data array
};

/** Typedef for :struct const_pool_t */
typedef struct const_pool_t const_pool_t;

/******************************************************************************
 * Constant pool interface                                                    *
 ******************************************************************************/

extern const_pool_t *cp_create(struct class_t *, class_file_t *);
extern const_pool_t *cp_create_dummy( void );
extern const_pool_type_t cp_get_tag(const_pool_t *, uint16_t);
extern void cp_set_tag_and_data(const_pool_t *, uint16_t, uint8_t, void *);
extern char *cp_get_class_name(const_pool_t *, uint16_t);
extern char *cp_get_string(const_pool_t *, uint16_t);
extern uintptr_t cp_get_ref(const_pool_t *, uint16_t);
extern int32_t cp_get_integer(const_pool_t *, uint16_t);
extern int64_t cp_get_long(const_pool_t *, uint16_t);
extern char *cp_get_name_and_type_name(const_pool_t *, uint16_t);
extern char *cp_get_name_and_type_type(const_pool_t *, uint16_t);
extern uint16_t cp_get_fieldref_class(const_pool_t *, uint16_t);
extern char *cp_get_fieldref_name(const_pool_t *, uint16_t);
extern char *cp_get_fieldref_type(const_pool_t *, uint16_t);
extern uint16_t cp_get_methodref_class(const_pool_t *, uint16_t);
extern char *cp_get_methodref_name(const_pool_t *, uint16_t);
extern char *cp_get_methodref_descriptor(const_pool_t *, uint16_t);
extern uint16_t cp_get_interfacemethodref_class(const_pool_t *, uint16_t);
extern char *cp_get_interfacemethodref_name(const_pool_t *, uint16_t);
extern char *cp_get_interfacemethodref_descriptor(const_pool_t *, uint16_t);
#if JEL_FP_SUPPORT
extern float cp_get_float(const_pool_t *, uint16_t);
extern double cp_get_double(const_pool_t *, uint16_t);
#endif // JEL_FP_SUPPORT
#ifdef JEL_PRINT
extern void cp_print(const_pool_t *);
#endif // JEL_PRINT

/******************************************************************************
 * Inlined functions for accessing the constant pool data section             *
 ******************************************************************************/

/** Writes a pointer value in the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param ptr A generic pointer */

static inline void cp_data_set_ptr(jword_t *data, size_t i, void *ptr)
{
    *((void **) (data + i)) = ptr;
} // cp_data_set_ptr()

/** Reads a pointer value from the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns A generic pointer */

static inline void *cp_data_get_ptr(const jword_t *data, size_t i)
{
    return *((void **) (data + i));
} // cp_data_get_ptr()

/** Writes an uintptr_t value in the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param ptr An uintptr_t value */

static inline void cp_data_set_uintptr(jword_t *data, size_t i, uintptr_t ptr)
{
    *((uintptr_t *) (data + i)) = ptr;
} // cp_data_set_uintptr()

/** Reads an uintptr_t value from the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns An uintptr_t value */

static inline uintptr_t cp_data_get_uintptr(const jword_t *data, size_t i)
{
    return *((const uintptr_t *) (data + i));
} // cp_data_get_uintptr()

/** Writes an uint16_t value in the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index
 * \param v An uint16_t value */

static inline void cp_data_set_uint16(jword_t *data, size_t i, uint16_t v)
{
    *((uint16_t *) (data + i)) = v;
} // cp_data_set_uint16()

/** Reads an uint16_t value from the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns An uint16_t value */

static inline uint16_t cp_data_get_uint16(const jword_t *data, size_t i)
{
    return *((const uint16_t *) (data + i));
} // cp_data_get_uint16()

/** Writes an int32_t value in the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param v An int32_t value */

static inline void cp_data_set_int32(jword_t *data, size_t i, int32_t v)
{
    *((int32_t *) (data + i)) = v;
} // cp_data_set_int32()

/** Reads an int32_t value from the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns An int32_t value */

static inline int32_t cp_data_get_int32(const jword_t *data, size_t i)
{
    return *((const int32_t *) (data + i));
} // cp_data_get_int32()

/** Writes a float value in the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param v A float value */

static inline void cp_data_set_float(jword_t *data, size_t i, float v)
{
    *((float *) (data + i)) = v;
} // cp_data_set_float()

/** Reads a float value from the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns A float value */

static inline float cp_data_get_float(const jword_t *data, size_t i)
{
    return *((const float *) (data + i));
} // cp_data_get_float()

/** Writes an int64_t value in the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param v An int64_t value */

static inline void cp_data_set_int64(jword_t *data, size_t i, int64_t v)
{
    *((int64_t *) (data + i)) = v;
} // cp_data_set_int64()

/** Reads an int64_t value from the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns An int64_t value */

static inline int64_t cp_data_get_int64(const jword_t *data, size_t i)
{
    return *((const int64_t *) (data + i));
} // cp_data_get_int64()

/** Writes a double value in the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param v A double value */

static inline void cp_data_set_double(jword_t *data, size_t i, double v)
{
    *((double *) (data + i)) = v;
} // cp_data_set_double()

/** Reads a double value from the constant pool
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns A double value */

static inline double cp_data_get_double(const jword_t *data, size_t i)
{
    return *((const double *) (data + i));
} // cp_data_get_double()

/** Sets the class index of a Fieldref entry
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param idx The new class index */

static inline void cp_data_set_fieldref_class(jword_t *data, size_t i,
                                              uint16_t idx)
{
    ((fieldref_t *) (data + i))->class_index = idx;
} // cp_data_set_fieldref_class()

/** Reads the class index of a Fieldref entry
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns The class index of the Fieldref structure */

static inline uint16_t cp_data_get_fieldref_class(const jword_t *data, size_t i)
{
    return ((const fieldref_t *) (data + i))->class_index;
} // cp_data_get_fieldref_class()

/** Sets the NameAndType index of a Fieldref entry
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param idx The new name and type index */

static inline void cp_data_set_fieldref_name_and_type(jword_t *data, size_t i,
                                                      uint16_t idx)
{
    ((fieldref_t *) (data + i))->name_and_type_index = idx;
} // cp_data_set_fieldref_name_and_type()

/** Reads the NameAndType index of a Fieldref entry
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns The name and type index */

static inline uint16_t cp_data_get_fieldref_name_and_type(const jword_t *data,
                                                          size_t i)
{
    return ((const fieldref_t *) (data + i))->name_and_type_index;
} // cp_data_get_fieldref_name_and_type()

/** Sets the name field of a NameAndType entry
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param idx The new name index */

static inline void cp_data_set_name_and_type_name(jword_t *data, size_t i,
                                                  uint16_t idx)
{
    ((name_and_type_t *) (data + i))->name_index = idx;
} // cp_data_set_name_and_type_name()

/** Reads the name field of a NameAndType entry
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns The name index */

static inline uint16_t cp_data_get_name_and_type_name(const jword_t *data,
                                                      size_t i)
{
    return ((const name_and_type_t *) (data + i))->name_index;
} // cp_data_get_name_and_type_name()

/** Sets the descriptor field of a NameAndType entry
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \param idx The new descriptor index */

static inline void cp_data_set_name_and_type_descriptor(jword_t *data, size_t i,
                                                        uint16_t idx)
{
    ((name_and_type_t *) (data + i))->descriptor_index = idx;
} // cp_data_set_name_and_type_descriptor()

/** Reads the descriptor field of a NameAndType entry
 * \param data A pointer to the constant pool data array
 * \param i An index in the constant pool
 * \returns The descriptor index */

static inline uint16_t cp_data_get_name_and_type_descriptor(const jword_t *data,
                                                            size_t i)
{
    return ((const name_and_type_t *) (data + i))->descriptor_index;
} // cp_data_get_name_and_type_descriptor()

/******************************************************************************
 * Inlined constant pool functions                                            *
 ******************************************************************************/

/** Gets the class to which this constant pool belongs to
 * \param cp A pointer to the constant pool
 * \returns The class to which this constant pool belongs to */

static inline struct class_t *cp_get_class(const_pool_t *cp)
{
    return (struct class_t *) cp_data_get_ptr(cp->data, 0);
} // cp_get_class()

/** Gets the pointer of a resolved reference to a class
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns A pointer to the requested class */

static inline struct class_t *cp_get_resolved_class(const_pool_t *cp,
                                                    uint16_t entry)
{
    assert(entry < cp->entries);
    assert(cp_get_tag(cp, entry) == CONSTANT_Class_resolved);

    return (struct class_t *) cp_data_get_ptr(cp->data, entry);
} // cp_get_resolved_class()

/** Gets the pointer of a resolved reference to an instance field
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns A pointer to the requested field */

static inline struct field_t *cp_get_resolved_instance_field(const_pool_t *cp,
                                                             uint16_t entry)
{
    assert(entry < cp->entries);
    assert(cp_get_tag(cp, entry) == CONSTANT_Fieldref_resolved);

    return (struct field_t *) cp_data_get_ptr(cp->data, entry);
} // cp_get_resolved_instance_field()

/** Gets the pointer of a resolved reference to a static field
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns A pointer to the requested field */

static inline struct field_t *cp_get_resolved_static_field(const_pool_t *cp,
                                                           uint16_t entry)
{
    static_field_t *static_field;

    assert(entry < cp->entries);
    assert(cp_get_tag(cp, entry) == CONSTANT_Fieldref_resolved);

    static_field = (static_field_t *) cp_data_get_ptr(cp->data, entry);

    return static_field->field;
} // cp_get_resolved_static_field()

/** Gets the pointer of a resolved reference to a method
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns A pointer to the requested method */

static inline struct method_t *cp_get_resolved_method(const_pool_t *cp,
                                                      uint16_t entry)
{
    assert(entry < cp->entries);
    assert(cp_get_tag(cp, entry) == CONSTANT_Methodref_resolved);

    return (struct method_t *) cp_data_get_ptr(cp->data, entry);
} // cp_get_resovled_method()

/** Gets the pointer of a resolved reference to an interface method
 * \param cp A pointer to a valid constant pool object
 * \param entry The constant pool entry number
 * \returns A pointer to the requested method */

static inline struct method_t *cp_get_resolved_interfacemethod(const_pool_t *cp,
                                                               uint16_t entry)
{
    assert(entry < cp->entries);
    assert(cp_get_tag(cp, entry) == CONSTANT_InterfaceMethodref_resolved);

    return (struct method_t *) cp_data_get_ptr(cp->data, entry);
} // cp_get_resovled_interfacemethod()

#endif // !JELATINE_CONSTANTPOOL_H
