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

/** \file kni.h
 * CLDC native interface */

/** \def JELATINE_KNI_H
 * kni.h inclusion macro */

#ifndef JELATINE_KNI_H
#   define JELATINE_KNI_H (1)

#include "wrappers.h"

#include "array.h"
#include "class.h"
#include "loader.h"
#include "thread.h"

#include "java_lang_String.h"

/******************************************************************************
 * KNI basic types                                                            *
 ******************************************************************************/

/** Java 'boolean' type */
typedef uint8_t jboolean;

/** Java 'byte' type */
typedef uint8_t jbyte;

/** Java 'char' type */
typedef uint16_t jchar;

/** Java 'short' type */
typedef int16_t jshort;

/** Java 'int' type */
typedef int32_t jint;

/** Java 'long' type */
typedef int64_t jlong;

/** Java 'float' type */
typedef float jfloat;

/** Java 'double' type */
typedef double jdouble;

/** Size type (string length, etc...) */
typedef jint jsize;

/******************************************************************************
 * KNI reference types                                                        *
 ******************************************************************************/

/** java.lang.Object reference */
typedef uintptr_t *jobject;

/** java.lang.Class reference */
typedef uintptr_t *jclass;

/** java.lang.Throwable reference */
typedef jobject jthrowable;

/** java.lang.String reference */
typedef jobject jstring;

/** Untyped java array reference */
typedef jobject jarray;

/** boolean array reference */
typedef jarray  jbooleanArray;

/** byte array reference */
typedef jarray  jbyteArray;

/** char array reference */
typedef jarray  jcharArray;

/** short array reference */
typedef jarray  jshortArray;

/** int array reference */
typedef jarray  jintArray;

/** long array reference */
typedef jarray  jlongArray;

/** float array reference */
typedef jarray  jfloatArray;

/** double array reference */
typedef jarray  jdoubleArray;

/** Array of object references */
typedef jarray  jobjectArray;

/** Reference to a field */
typedef uintptr_t jfieldID;

/******************************************************************************
 * KNI return types                                                           *
 ******************************************************************************/

/** KNI return type for functions returning void */
#define KNI_RETURNTYPE_VOID void

/** KNI return type for functions returning booleans */
#define KNI_RETURNTYPE_BOOLEAN int32_t

/** KNI return type for functions returning bytes */
#define KNI_RETURNTYPE_BYTE int32_t

/** KNI return type for functions returning chars */
#define KNI_RETURNTYPE_CHAR int32_t

/** KNI return type for functions returning shorts */
#define KNI_RETURNTYPE_SHORT int32_t

/** KNI return type for functions returning ints */
#define KNI_RETURNTYPE_INT int32_t

/** KNI return type for functions returning longs */
#define KNI_RETURNTYPE_LONG int64_t

/** KNI return type for functions returning floats */
#define KNI_RETURNTYPE_FLOAT float

/** KNI return type for functions returning doubles */
#define KNI_RETURNTYPE_DOUBLE double

/** KNI return type for functions returning object handles */
#define KNI_RETURNTYPE_OBJECT uintptr_t

/******************************************************************************
 * Various constants                                                          *
 ******************************************************************************/

/** \def KNI_FALSE
 * 'false' value of a java 'boolean' */

/** \def KNI_TRUE
 * 'true' value of a java ' boolean' */

#define KNI_FALSE (0)
#define KNI_TRUE  (1)

/** \def KNI_OK
 * Success return value for a KNI function */

/** \def KNI_ERR
 * Generic failure return value for a KNI function */

/** \def KNI_ENOMEM
 * Failure due to lack of available memory */

/** \def KNI_EINVAL
 * Failure due to incorrect arguments */

#define KNI_OK     (0)
#define KNI_ERR    (-1)
#define KNI_ENOMEM (-4)
#define KNI_EINVAL (-6)

/******************************************************************************
 * Version information                                                        *
 ******************************************************************************/

/** KNI version. Currently 1.0 */
#define KNI_VERSION (0x00010000)

/** Return the version information
 * \returns The native interface version */

static inline jint KNI_GetVersion( void )
{
    return KNI_VERSION;
} // KNI_GetVersion()

/******************************************************************************
 * Class and interface operations                                             *
 ******************************************************************************/

/** Initializes a handle with a reference to the named class or interface. The
 * function assumes that the class has already been loaded to the system and
 * properly initialized. If this is not the case, or if no class is found, the
 * handle will be initialized to NULL
 * \param name The descriptor of the class or interface to be returned. The
 * name is represented as a UTF-8 string
 * \param classHandle A handle in which the class pointer will be returned */

static inline void KNI_FindClass(const char *name, jclass classHandle)
{
    class_t *cl = bcl_find_class(name);

    if (cl != NULL) {
        *classHandle = class_get_object(cl);
    } else {
        *classHandle = JNULL;
    }
} // KNI_FindClass()

/** Initializes the \a parent handle to contain a pointer to the superclass of
 * the given class (represented by \a cl). If \a cl represents the class
 * java.lang.Object, then the function sets the \a parent handle to NULL
 * \param classHandle A handle initialized with a reference to the class object
 * whose superclass is to be determined
 * \param superClassHandle A handle which, upon return from this function, will
 * contain a reference to the superclass object */

static inline void KNI_GetSuperClass(jclass classHandle,
                                     jclass superClassHandle)
{
    class_t *cl, *parent;

    cl = bcl_get_class_by_id(JAVA_LANG_CLASS_REF2PTR(*classHandle)->id);
    parent = class_get_parent(cl);
    *superClassHandle = class_get_object(parent);
} // KNI_GetSuperClass()

/** Determines whether an object of class or interface \a cl1 can be safely cast
 * to a class or interface \a cl2
 * \param cl1 Handle initialized with a reference to the first class or
 * interface argument
 * \param cl2 Handle initialized with a reference to the second class or
 * interface argument
 * \returns KNI_TRUE if any of the following is true: the first and second
 * argument refer to the same class or interface, the first argument refers to
 * a subclass of the second argument, the first argument refers to a class that
 * has the second argument as one of its interfaces or the first and second
 * arguments both refer to array of classes with element types X and Y, and
 * KNI_IsAssignableFrom(X, Y) is KNI_TRUE. Otherwise KNI_FALSE */

static inline jboolean KNI_IsAssignableFrom(jclass cl1, jclass cl2)
{
    class_t *src = bcl_get_class_by_id(JAVA_LANG_CLASS_REF2PTR(*cl1)->id);
    class_t *dest = bcl_get_class_by_id(JAVA_LANG_CLASS_REF2PTR(*cl2)->id);

    return bcl_is_assignable(src, dest) ? KNI_TRUE : KNI_FALSE;
} // KNI_IsAssignableFrom()

/******************************************************************************
 * Exceptions and errors                                                      *
 ******************************************************************************/

extern jint KNI_ThrowNew(const char *, const char *);
extern void KNI_FatalError(const char *);

/******************************************************************************
 * Object operations                                                          *
 ******************************************************************************/

/** Sets the handle \a classHandle to point to the class of the object
 * represented by \a objectHandle
 * \param objectHandle A handle pointing to an object whose class is being
 * sought
 * \param classHandle A handle which, upon return of this function, will contain
 * a reference to the class of this object */

static inline void KNI_GetObjectClass(jobject objectHandle, jclass classHandle)
{
    class_t *cl;

    if (*objectHandle == JNULL) {
        *classHandle = JNULL;
    } else {
        cl = header_get_class((header_t *) *(objectHandle));
        *classHandle = class_get_object(cl);
    }
} // KNI_GetObjectClass()

/** Tests whether an object represented by objectHandle is an instance of a
 * class or interface represented by classHandle
 * \param objectHandle A handle pointing to an object
 * \param classHandle A handle pointing to a class or interface
 * \returns Returns KNI_TRUE if the given object is an instance of given class,
 * KNI_FALSE otherwise */

static inline jboolean KNI_IsInstanceOf(jobject objectHandle,
                                        jclass classHandle)
{
    class_t *cl;
    class_t *obj = header_get_class((header_t *) *objectHandle);

    cl = bcl_get_class_by_id(JAVA_LANG_CLASS_REF2PTR(*classHandle)->id);

    return bcl_is_assignable(obj, cl) ? KNI_TRUE : KNI_FALSE;
} // KNI_IsInstanceOf()

/******************************************************************************
 * Instance field access                                                      *
 ******************************************************************************/

extern jfieldID KNI_GetFieldID(jclass classHandle, const char *name,
                               const char *signature);

/** Returns the value of an instance field of type boolean. The field to access
 * is denoted by fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * accessed
 * \param fieldID The field ID of the given instance field
 * \returns The value of the specified instance field */

static inline jboolean KNI_GetBooleanField(jobject objectHandle,
                                           jfieldID fieldID)
{
    uint8_t nibble = *((uint8_t *) (*objectHandle + (fieldID >> 3)));

    return (nibble >> (fieldID & 0x7)) & 0x1;
} // KNI_GetBooleanField()

/** Returns the value of an instance field of type byte. The field to access is
 * denoted by fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * accessed
 * \param fieldID The field ID of the given instance field
 * \returns The value of the specified instance field */

static inline jbyte KNI_GetByteField(jobject objectHandle, jfieldID fieldID)
{
    return *((int8_t *) (*objectHandle + fieldID));
} // KNI_GetByteField()

/** Returns the value of an instance field of type char. The field to access is
 * denoted by fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * accessed
 * \param fieldID The field ID of the given instance field
 * \returns The value of the specified instance field */

static inline jchar KNI_GetCharField(jobject objectHandle, jfieldID fieldID)
{
    return *((uint16_t *) (*objectHandle + fieldID));
} // KNI_GetCharField()

/** Returns the value of an instance field of type short. The field to access is
 * denoted by fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * accessed
 * \param fieldID The field ID of the given instance field
 * \returns The value of the specified instance field */

static inline jshort KNI_GetShortField(jobject objectHandle, jfieldID fieldID)
{
    return *((int16_t *) (*objectHandle + fieldID));
} // KNI_GetShortField()

/** Returns the value of an instance field of type int. The field to access is
 * denoted by fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * accessed
 * \param fieldID The field ID of the given instance field
 * \returns The value of the specified instance field */

static inline jint KNI_GetIntField(jobject objectHandle, jfieldID fieldID)
{
    return *((int32_t *) (*objectHandle + fieldID));
} // KNI_GetIntField()

/** Returns the value of an instance field of type long. The field to access is
 * denoted by fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * accessed
 * \param fieldID The field ID of the given instance field
 * \returns The value of the specified instance field */

static inline jlong KNI_GetLongField(jobject objectHandle, jfieldID fieldID)
{
    return *((int64_t *) (*objectHandle + fieldID));
} // KNI_GetLongField()

#if JEL_FP_SUPPORT

/** Returns the value of an instance field of type float. The field to access is
 * denoted by fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * accessed
 * \param fieldID The field ID of the given instance field
 * \returns The value of the specified instance field */

static inline jfloat KNI_GetFloatField(jobject objectHandle, jfieldID fieldID)
{
    return *((float *) (*objectHandle + fieldID));
} // KNI_GetFloatField()

/** Returns the value of an instance field of type double. The field to access
 * is denoted by fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * accessed
 * \param fieldID The field ID of the given instance field
 * \returns The value of the specified instance field */

static inline jdouble KNI_GetDoubleField(jobject objectHandle, jfieldID fieldID)
{
    return *((double *) (*objectHandle + fieldID));
} // KNI_GetDoubleField()

#endif // JEL_FP_SUPPORT

/** Returns the value of a field of a reference type, and assigns it to the
 * handle toHandle. The field to access is denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose whose field is to
 * be accessed
 * \param fieldID The field ID of the given instance field
 * \param toHandle A handle to which the return value will be assigned */

static inline void KNI_GetObjectField(jobject objectHandle, jfieldID fieldID,
                                      jobject toHandle)
{
    *toHandle = *((uintptr_t *) (*objectHandle + fieldID));
} // KNI_GetObjectField()

/** Sets the value of an instance field of boolean type. The field to modify is
 * denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param value The value to set to the instance field */

static inline void KNI_SetBooleanField(jobject objectHandle, jfieldID fieldID,
                                       jboolean value)
{
    uint8_t nvalue = value ^ 0x1;
    uint8_t nibble = *((uint8_t *) (*objectHandle + (fieldID >> 3)));

    nibble |= value << (fieldID & 0x7);
    nibble &= ~(nvalue << (fieldID & 0x7));
    *((uint8_t *) (*objectHandle + (fieldID >> 3))) = nibble;
} // KNI_SetBooleanField()

/** Sets the value of an instance field of byte type. The field to modify is
 * denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param value The value to set to the instance field */

static inline void KNI_SetByteField(jobject objectHandle, jfieldID fieldID,
                                    jbyte value)
{
    *((int8_t *) (*objectHandle + fieldID)) = value;
} // KNI_SetByteField()

/** Sets the value of an instance field of char type. The field to modify is
 * denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param value The value to set to the instance field */

static inline void KNI_SetCharField(jobject objectHandle, jfieldID fieldID,
                                    jchar value)
{
    *((uint16_t *) (*objectHandle + fieldID)) = value;
} // KNI_SetCharField()

/** Sets the value of an instance field of short type. The field to modify is
 * denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param value The value to set to the instance field */

static inline void KNI_SetShortField(jobject objectHandle, jfieldID fieldID,
                                     jshort value)
{
    *((int16_t *) (*objectHandle + fieldID)) = value;
} // KNI_SetShortField()

/** Sets the value of an instance field of int type. The field to modify is
 * denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param value The value to set to the instance field */

static inline void KNI_SetIntField(jobject objectHandle, jfieldID fieldID,
                                   jint value)
{
    *((int32_t *) (*objectHandle + fieldID)) = value;
} // KNI_SetIntField()

/** Sets the value of an instance field of long type. The field to modify is
 * denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param value The value to set to the instance field */

static inline void KNI_SetLongField(jobject objectHandle, jfieldID fieldID,
                                    jlong value)
{
    *((int64_t *) (*objectHandle + fieldID)) = value;
} // KNI_SetLongField()

#if JEL_FP_SUPPORT

/** Sets the value of an instance field of float type. The field to modify is
 * denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param value The value to set to the instance field */

static inline void KNI_SetFloatField(jobject objectHandle, jfieldID fieldID,
                                     jfloat value)
{
    *((float *) (*objectHandle + fieldID)) = value;
} // KNI_SetFloatField()

/** Sets the value of an instance field of double type. The field to modify is
 * denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param value The value to set to the instance field */

static inline void KNI_SetDoubleField(jobject objectHandle, jfieldID fieldID,
                                      jdouble value)
{
    *((double *) (*objectHandle + fieldID)) = value;
} // KNI_SetDoubleField()

#endif // JEL_FP_SUPPORT

/** Sets the value of an instance field of a reference type. The field to modify
 * is denoted by \a fieldID
 * \param objectHandle A handle pointing to an object whose field is to be
 * modified
 * \param fieldID The field ID of the given instance field
 * \param fromHandle A handle pointing to an object that will be assigned to
 * the field */

static inline void KNI_SetObjectField(jobject objectHandle, jfieldID fieldID,
                                      jobject fromHandle)
{
    *((uintptr_t *) (*objectHandle + fieldID)) = *fromHandle;
} // KNI_SetObjectField()

/******************************************************************************
 * Static field access                                                        *
 ******************************************************************************/

extern jfieldID KNI_GetStaticFieldID(jclass, const char *, const char *);

/** Returns the value of a static field of boolean type. The field to access is
 * denoted by \a fieldID.
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \returns Returns the value of the specified static field */

static inline jboolean KNI_GetStaticBooleanField(
    jclass classHandle ATTRIBUTE_UNUSED, jfieldID fieldID)
{
    return *((uint8_t *) fieldID);
} // KNI_GetStaticBooleanField()

/** Returns the value of a static field of byte type. The field to access is
 * denoted by \a fieldID.
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \returns Returns the value of the specified static field */

static inline jbyte KNI_GetStaticByteField(jclass classHandle ATTRIBUTE_UNUSED,
                                           jfieldID fieldID)
{
    return *((int8_t *) fieldID);
} // KNI_GetStaticByteField()

/** Returns the value of a static field of char type. The field to access is
 * denoted by \a fieldID.
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \returns Returns the value of the specified static field */

static inline jchar KNI_GetStaticCharField(jclass classHandle ATTRIBUTE_UNUSED,
                                           jfieldID fieldID)
{
    return *((uint16_t *) fieldID);
} // KNI_GetStaticCharField()

/** Returns the value of a static field of short type. The field to access is
 * denoted by \a fieldID.
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \returns Returns the value of the specified static field */

static inline jshort KNI_GetStaticShortField(
    jclass classHandle ATTRIBUTE_UNUSED, jfieldID fieldID)
{
    return *((int16_t *) fieldID);
} // KNI_GetStaticShortField()

/** Returns the value of a static field of int type. The field to access is
 * denoted by \a fieldID.
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \returns Returns the value of the specified static field */

static inline jint KNI_GetStaticIntField(jclass classHandle ATTRIBUTE_UNUSED,
                                         jfieldID fieldID)
{
    return *((int32_t *) fieldID);
} // KNI_GetStaticIntField()

/** Returns the value of a static field of long type. The field to access is
 * denoted by \a fieldID.
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \returns Returns the value of the specified static field */

static inline jlong KNI_GetStaticLongField(jclass classHandle ATTRIBUTE_UNUSED,
                                           jfieldID fieldID)
{
    return *((int64_t *) fieldID);
} // KNI_GetStaticLongField()

#if JEL_FP_SUPPORT

/** Returns the value of a static field of float type. The field to access is
 * denoted by \a fieldID.
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \returns Returns the value of the specified static field */

static inline jfloat KNI_GetStaticFloatField(
    jclass classHandle ATTRIBUTE_UNUSED, jfieldID fieldID)
{
    return *((float *) fieldID);
} // KNI_GetStaticFloatField()

/** Returns the value of a static field of double type. The field to access is
 * denoted by \a fieldID.
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \returns Returns the value of the specified static field */

static inline jdouble KNI_GetStaticDoubleField(
    jclass classHandle ATTRIBUTE_UNUSED, jfieldID fieldID)
{
    return *((double *) fieldID);
} // KNI_GetStaticDoubleField()

#endif // JEL_FP_SUPPORT

/** Returns the value of a static field of a reference type, and assigns it to
 * the handle \a toHandle. The field to access is denoted by \a fieldID
 * \param classHandle A handle pointing to a class or interface whose static
 * field is to be accessed
 * \param fieldID A field ID of the given class
 * \param toHandle A handle to which the return value will be assigned */

static inline void KNI_GetStaticObjectField(jclass classHandle ATTRIBUTE_UNUSED,
                                            jfieldID fieldID, jobject toHandle)
{
    *toHandle = *((uintptr_t *) fieldID);
} // KNI_GetStaticObjectField()

/** Sets the value of a static field of boolean type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle  A handle pointing to the class or interface whose static
 * field is to be modified
 * \param fieldID A field ID of the given class
 * \param value The value to set to the static field */

static inline void KNI_SetStaticBooleanField(
    jclass classHandle ATTRIBUTE_UNUSED, jfieldID fieldID, jboolean value)
{
    *((uint8_t *) fieldID) = value;
} // KNI_SetStaticBooleanField()

/** Sets the value of a static field of byte type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle  A handle pointing to the class or interface whose static
 * field is to be modified
 * \param fieldID A field ID of the given class
 * \param value The value to set to the static field */

static inline void KNI_SetStaticByteField(jclass classHandle ATTRIBUTE_UNUSED,
                                          jfieldID fieldID, jbyte value)
{
    *((int8_t *) fieldID) = value;
} // KNI_SetStaticByteField()

/** Sets the value of a static field of char type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle  A handle pointing to the class or interface whose static
 * field is to be modified
 * \param fieldID A field ID of the given class
 * \param value The value to set to the static field */

static inline void KNI_SetStaticCharField(jclass classHandle ATTRIBUTE_UNUSED,
                                          jfieldID fieldID, jchar value)
{
    *((uint16_t *) fieldID) = value;
} // KNI_SetStaticCharField()

/** Sets the value of a static field of short type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle  A handle pointing to the class or interface whose static
 * field is to be modified
 * \param fieldID A field ID of the given class
 * \param value The value to set to the static field */

static inline void KNI_SetStaticShortField(jclass classHandle ATTRIBUTE_UNUSED,
                                           jfieldID fieldID, jshort value)
{
    *((int16_t *) fieldID) = value;
} // KNI_SetStaticShortField()

/** Sets the value of a static field of int type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle  A handle pointing to the class or interface whose static
 * field is to be modified
 * \param fieldID A field ID of the given class
 * \param value The value to set to the static field */

static inline void KNI_SetStaticIntField(jclass classHandle ATTRIBUTE_UNUSED,
                                         jfieldID fieldID, jint value)
{
    *((int32_t *) fieldID) = value;
} // KNI_SetStaticIntField()

/** Sets the value of a static field of long type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle  A handle pointing to the class or interface whose static
 * field is to be modified
 * \param fieldID A field ID of the given class
 * \param value The value to set to the static field */

static inline void KNI_SetStaticLongField(jclass classHandle ATTRIBUTE_UNUSED,
                                          jfieldID fieldID, jlong value)
{
    *((int64_t *) fieldID) = value;
} // KNI_SetStaticLongField()

#if JEL_FP_SUPPORT

/** Sets the value of a static field of float type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle  A handle pointing to the class or interface whose static
 * field is to be modified
 * \param fieldID A field ID of the given class
 * \param value The value to set to the static field */

static inline void KNI_SetStaticFloatField(jclass classHandle ATTRIBUTE_UNUSED,
                                           jfieldID fieldID, jfloat value)
{
    *((float *) fieldID) = value;
} // KNI_SetStaticFloatField()

/** Sets the value of a static field of double type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle  A handle pointing to the class or interface whose static
 * field is to be modified
 * \param fieldID A field ID of the given class
 * \param value The value to set to the static field */

static inline void KNI_SetStaticDoubleField(jclass classHandle ATTRIBUTE_UNUSED,
                                            jfieldID fieldID, jdouble value)
{
    *((double *) fieldID) = value;
} // KNI_SetStaticDoubleField()

#endif // JEL_FP_SUPPORT

/** Sets the value of a static field of a reference type. The field to modify is
 * denoted by \a fieldID
 * \param classHandle A handle pointing to the class or interface whose whose
 * static field is to be modified
 * \param fieldID A field ID of the given class
 * \param fromHandle A handle pointing to an object that will assigned to the
 * field */

static inline void KNI_SetStaticObjectField(jclass classHandle ATTRIBUTE_UNUSED,
                                            jfieldID fieldID,
                                            jobject fromHandle)
{
    *((uintptr_t *) fieldID) = *fromHandle;
} // KNI_SetStaticObjectField()

/******************************************************************************
 * String operations                                                          *
 ******************************************************************************/

/** Returns the number of Unicode characters in a java.lang.String object. If
 * the given string handle is NULL, the function returns -1
 * \param stringHandle A handle pointing to a java.lang.String string object
 * whose length is to be determined
 * \returns The length of the string, or -1 if the given string handle is
 * NULL */

static inline jsize KNI_GetStringLength(jstring stringHandle)
{
    java_lang_String_t *string;

    if (*stringHandle) {
        string = JAVA_LANG_STRING_REF2PTR(*stringHandle);
        return string->count;
    } else {
        return -1;
    }
} // KNI_GetStringLength()

extern void KNI_GetStringRegion(jstring, jsize, jsize, jchar *);

/** Creates a java.lang.String object from the given Unicode sequence. The
 * resulting object is returned to the caller via the stringHandle \a handle
 * \param uchars A Unicode sequence that will make up the contents of the new
 * string object
 * \param length The length of the string
 * \param stringHandle A handle to hold the reference to the new
 * java.lang.String object */

static inline void KNI_NewString(const jchar *uchars, jsize length,
                                 jobject stringHandle)
{
    java_lang_String_t *str;

    str = jstring_create_from_unicode(uchars, length);
    *stringHandle = JAVA_LANG_STRING_PTR2REF(str);
} // KNI_NewString()

/** Creates a java.lang.String object from the given UTF-8 string. The resulting
 * object is returned to the caller via the stringHandle \a handle
 * \param utf8chars A UTF-8 string that will make up the contents of the new
 * string object
 * \param stringHandle A handle to hold the reference to the new
 * java.lang.String object */

static inline void KNI_NewStringUTF(const char *utf8chars, jobject stringHandle)
{
    java_lang_String_t *str;

    str = jstring_create_from_utf8((utf8chars));
    *stringHandle = JAVA_LANG_STRING_PTR2REF(str);
} // KNI_NewStringUTF()

/******************************************************************************
 * Array operations                                                           *
 ******************************************************************************/

/** Returns the number of elements in a given Java array object. The \a array
 * argument may denote an array of any element type, including primitive types
 * or reference types. If the given array handle is NULL, the function returns
 * -1
 * \param array A handle pointing to an array object whose length is to be
 * determined
 * \returns Returns the number of elements in the array, or -1 if the given
 * array handle is NULL */

static inline jsize KNI_GetArrayLength(jarray array)
{
    if (array) {
        return ((array_t *) *array)->length;
    } else {
        return -1;
    }
} // KNI_GetArrayLength()

/** Returns an element of an array of booleans. The given array reference must
 * not be NULL. No array type checking or index range checking is performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \returns Returns the element at position index of an array of booleans */

static inline jboolean KNI_GetBooleanArrayElement(jbooleanArray arrayHandle,
                                                  jsize index)
{
    array_t *array = (array_t *) *arrayHandle;
    uint8_t *data = array_get_data(array);

    return (*(data + (index >> 3)) >> (index & 0x7)) & 0x1;
} // KNI_GetBooleanArrayElement()

/** Returns an element of an array of bytes. The given array reference must not
 * be NULL. No array type checking or index range checking is performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \returns Returns the element at position index of an array of bytes */

static inline jbyte KNI_GetByteArrayElement(jbyteArray arrayHandle, jsize index)
{
    array_t *array = (array_t *) *arrayHandle;
    int8_t *data = array_get_data(array);

    return *(data + index);
} // KNI_GetByteArrayElement()

/** Returns an element of an array of chars. The given array reference must not
 * be NULL. No array type checking or index range checking is performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \returns Returns the element at position index of an array of chars */

static inline jchar KNI_GetCharArrayElement(jcharArray arrayHandle, jsize index)
{
    array_t *array = (array_t *) *arrayHandle;
    uint16_t *data = array_get_data(array);

    return *(data + index);
} // KNI_GetCharArrayElement()

/** Returns an element of an array of shorts. The given array reference must
 * not be NULL. No array type checking or index range checking is performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \returns Returns the element at position index of an array of shorts */

static inline jshort KNI_GetShortArrayElement(jshortArray arrayHandle,
                                              jsize index)
{
    array_t *array = (array_t *) *arrayHandle;
    int16_t *data = array_get_data(array);

    return *(data + index);
} // KNI_GetShortArrayElement()

/** Returns an element of an array of ints. The given array reference must not
 * be NULL. No array type checking or index range checking is performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \returns Returns the element at position index of an array of ints */

static inline jint KNI_GetIntArrayElement(jintArray arrayHandle, jsize index)
{
    array_t *array = (array_t *) *arrayHandle;
    int32_t *data = array_get_data(array);

    return *(data + index);
} // KNI_GetIntArrayElement()

/** Returns an element of an array of longs. The given array reference must not
 * be NULL. No array type checking or index range checking is performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \returns Returns the element at position index of an array of longs */

static inline jlong KNI_GetLongArrayElement(jlongArray arrayHandle, jsize index)
{
    array_t *array = (array_t *) *arrayHandle;
    int64_t *data = array_get_data(array);

    return *(data + index);
} // KNI_GetLongArrayElement()

#if JEL_FP_SUPPORT

/** Returns an element of an array of floats. The given array reference must
 * not be NULL. No array type checking or index range checking is performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \returns Returns the element at position index of an array of floats */

static inline jfloat KNI_GetFloatArrayElement(jfloatArray arrayHandle,
                                              jsize index)
{
    array_t *array = (array_t *) *arrayHandle;
    float *data = array_get_data(array);

    return *(data + index);
} // KNI_GetFloatArrayElement()

/** Returns an element of an array of doubles. The given array reference must
 * not be NULL. No array type checking or index range checking is performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \returns Returns the element at position index of an array of doubles */

static inline jdouble KNI_GetDoubleArrayElement(jdoubleArray arrayHandle,
                                                jsize index)
{
    array_t *array = (array_t *) *arrayHandle;
    double *data = array_get_data(array);

    return *(data + index);
} // KNI_GetDoubleArrayElement()

#endif // JEL_FP_SUPPORT

/** Returns an element of an array of a reference type. The given array handle
 * must not be NULL. No array type checking or index range checking is
 * performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param toHandle A handle to which the return value will be assigned */

static inline void KNI_GetObjectArrayElement(jobjectArray arrayHandle,
                                             jint index, jobject toHandle)
{
    array_t *array = (array_t *) *arrayHandle;
    uintptr_t *data = array_ref_get_data(array);

    *toHandle = *(data - index);
} // KNI_GetObjectArrayElement()

/** Sets an element of an array of booleans. The given array handle must not be
 * NULL. No array type checking or index range checking is performed.
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param value The value that will be stored in the array */

static inline void  KNI_SetBooleanArrayElement(jbooleanArray arrayHandle,
                                               jsize index, jboolean value)
{
    array_t *array = (array_t *) *arrayHandle;
    uint8_t *data = array_get_data(array);
    uint8_t nvalue = value ^ 0x1;
    uint8_t nibble = *(data + (index >> 3));

    nibble |= (value << (index & 0x7));
    nibble &= ~(nvalue << (index & 0x7));
    *(data + (index >> 3)) = nibble;
} // KNI_SetBooleanArrayElement()

/** Sets an element of an array of bytes. The given array handle must not be
 * NULL. No array type checking or index range checking is performed.
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param value The value that will be stored in the array */

static inline void KNI_SetByteArrayElement(jbyteArray arrayHandle, jsize index,
                                           jbyte value)
{
    array_t *array = (array_t *) *arrayHandle;
    int8_t *data = array_get_data(array);

    *(data + index) = value;
} // KNI_SetByteArrayElement()

/** Sets an element of an array of chars. The given array handle must not be
 * NULL. No array type checking or index range checking is performed.
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param value The value that will be stored in the array */

static inline void KNI_SetCharArrayElement(jcharArray arrayHandle, jsize index,
                                           jchar value)
{
    array_t *array = (array_t *) *arrayHandle;
    uint16_t *data = array_get_data(array);

    *(data + index) = value;
} // KNI_SetCharArrayElement()

/** Sets an element of an array of shorts. The given array handle must not be
 * NULL. No array type checking or index range checking is performed.
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param value The value that will be stored in the array */

static inline void KNI_SetShortArrayElement(jshortArray arrayHandle,
                                            jsize index, jshort value)
{
    array_t *array = (array_t *) *arrayHandle;
    int16_t *data = array_get_data(array);

    *(data + index) = value;
} // KNI_SetShortArrayElement()

/** Sets an element of an array of ints. The given array handle must not be
 * NULL. No array type checking or index range checking is performed.
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param value The value that will be stored in the array */

static inline void KNI_SetIntArrayElement(jintArray arrayHandle, jsize index,
                                          jint value)
{
    array_t *array = (array_t *) *arrayHandle;
    int32_t *data = array_get_data(array);

    *(data + index) = value;
} // KNI_SetIntArrayElement()

/** Sets an element of an array of longs. The given array handle must not be
 * NULL. No array type checking or index range checking is performed.
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param value The value that will be stored in the array */

static inline void KNI_SetLongArrayElement(jlongArray arrayHandle, jsize index,
                                           jlong value)
{
    array_t *array = (array_t *) *arrayHandle;
    int64_t *data = array_get_data(array);

    *(data + index) = value;
} // KNI_SetLongArrayElement()

#if JEL_FP_SUPPORT

/** Sets an element of an array of floats. The given array handle must not be
 * NULL. No array type checking or index range checking is performed.
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param value The value that will be stored in the array */

static inline void KNI_SetFloatArrayElement(jfloatArray arrayHandle,
                                            jsize index, jfloat value)
{
    array_t *array = (array_t *) *arrayHandle;
    float *data = array_get_data(array);

    *(data + index) = value;
} // KNI_SetFloatArrayElement()

/** Sets an element of an array of doubles. The given array handle must not be
 * NULL. No array type checking or index range checking is performed.
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param value The value that will be stored in the array */

static inline void KNI_SetDoubleArrayElement(jdoubleArray arrayHandle,
                                             jsize index, jdouble value)
{
    array_t *array = (array_t *) *arrayHandle;
    double *data = array_get_data(array);

    *(data + index) = value;
} // KNI_SetDoubleArrayElement()

#endif // JEL_FP_SUPPORT

/** Sets an element of an array of a reference type. The given array handle
 * must not be NULL. No array type checking or index range checking is
 * performed
 * \param arrayHandle A handle pointing to an array object
 * \param index The index of the element in the array. Index 0 denotes the
 * first element in the array
 * \param fromHandle A handle from which value will be read */

static inline void KNI_SetObjectArrayElement(jobjectArray arrayHandle,
                                             jint index, jobject fromHandle)
{
    array_t *array = (array_t *) *arrayHandle;
    uintptr_t *data = array_ref_get_data(array);

    *(data - index) = *fromHandle;
} // KNI_SetObjectArrayElement()

/** Gets a region of \a n bytes of an array of a primitive type. The given
 * array handle must not be NULL. No array type checking or range checking is
 * performed
 * \param arrayHandle A handle initialized with the array reference
 * \param offset A byte offset within the array
 * \param n The number of bytes of raw data to get
 * \param dstBuffer The destination of the data as bytes */

static inline void KNI_GetRawArrayRegion(jarray arrayHandle, jsize offset,
                                         jsize n, jbyte *dstBuffer)
{
    // FIXME: If this function can be called on arrays of references
    array_t *array = (array_t *) *arrayHandle;
    char *data = array_get_data(array);

    memcpy(dstBuffer, data + offset, n);
} // KNI_GetRawArrayRegion()

/** Sets a region of n bytes of an array of a primitive type. The given array
 * handle must not be NULL. No array type checking or range checking is
 * performed
 * \param arrayHandle A handle initialized with the array reference
 * \param offset A byte offset within the array
 * \param n The number of bytes of raw data to change
 * \param srcBuffer The source of the data as bytes */

static inline void KNI_SetRawArrayRegion(jarray arrayHandle, jsize offset,
                                         jsize n, const jbyte *srcBuffer)
{
    // FIXME: If this function can be called on arrays of references
    array_t *array = (array_t *) *arrayHandle;
    char *data = array_get_data(array);

    memcpy(data + offset, srcBuffer, n);
} // KNI_SetRawArrayRegion()

/******************************************************************************
 * Parameter passing                                                          *
 ******************************************************************************/

/** Returns the value of a parameter of type boolean at physical location
 * specified by index. Parameter indices are mapped from left to right. Index
 * value 1 refers to the leftmost parameter that has been passed on to the
 * native method. Remember that parameters of type long or double take up two
 * entries on the operand stack. No type checking or index range checking is
 * performed.
 * \param index The index of a parameter of a primitive type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method.
 * \returns Returns the argument of a primitive type at position index in the
 * list of arguments to the native method. */

static inline jboolean KNI_GetParameterAsBoolean(jint index)
{
    return (jboolean) *((int32_t *) (thread_self()->fp->locals + index));
} // KNI_GetParameterAsBoolean()

/** Returns the value of a parameter of type byte at physical location
 * specified by index. Parameter indices are mapped from left to right. Index
 * value 1 refers to the leftmost parameter that has been passed on to the
 * native method. Remember that parameters of type long or double take up two
 * entries on the operand stack. No type checking or index range checking is
 * performed.
 * \param index The index of a parameter of a primitive type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method.
 * \returns Returns the argument of a primitive type at position index in the
 * list of arguments to the native method. */

static inline jbyte KNI_GetParameterAsByte(jint index)
{
    return (jbyte) *((int32_t *) (thread_self()->fp->locals + index));
} // KNI_GetParameterAsByte()

/** Returns the value of a parameter of type char at physical location
 * specified by index. Parameter indices are mapped from left to right. Index
 * value 1 refers to the leftmost parameter that has been passed on to the
 * native method. Remember that parameters of type long or double take up two
 * entries on the operand stack. No type checking or index range checking is
 * performed.
 * \param index The index of a parameter of a primitive type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method.
 * \returns Returns the argument of a primitive type at position index in the
 * list of arguments to the native method. */

static inline jchar KNI_GetParameterAsChar(jint index)
{
    return (jchar) *((int32_t *) (thread_self()->fp->locals + index));
} // KNI_GetParameterAsChar()

/** Returns the value of a parameter of type short at physical location
 * specified by index. Parameter indices are mapped from left to right. Index
 * value 1 refers to the leftmost parameter that has been passed on to the
 * native method. Remember that parameters of type long or double take up two
 * entries on the operand stack. No type checking or index range checking is
 * performed.
 * \param index The index of a parameter of a primitive type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method.
 * \returns Returns the argument of a primitive type at position index in the
 * list of arguments to the native method. */

static inline jshort   KNI_GetParameterAsShort(jint index)
{
    return (jshort) *((int32_t *) (thread_self()->fp->locals + index));
} // KNI_GetParamterAsShort()

/** Returns the value of a parameter of type long at physical location
 * specified by index. Parameter indices are mapped from left to right. Index
 * value 1 refers to the leftmost parameter that has been passed on to the
 * native method. Remember that parameters of type long or double take up two
 * entries on the operand stack. No type checking or index range checking is
 * performed.
 * \param index The index of a parameter of a primitive type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method.
 * \returns Returns the argument of a primitive type at position index in the
 * list of arguments to the native method. */

static inline jint KNI_GetParameterAsInt(jint index)
{
    return *((int32_t *) (thread_self()->fp->locals + index));
} // KNI_GetParameterAsInt()

/** Returns the value of a parameter of type long at physical location
 * specified by index. Parameter indices are mapped from left to right. Index
 * value 1 refers to the leftmost parameter that has been passed on to the
 * native method. Remember that parameters of type long or double take up two
 * entries on the operand stack. No type checking or index range checking is
 * performed.
 * \param index The index of a parameter of a primitive type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method.
 * \returns Returns the argument of a primitive type at position index in the
 * list of arguments to the native method. */

static inline jlong KNI_GetParameterAsLong(jint index)
{
    return *((int64_t *) (thread_self()->fp->locals + index));
} // KNI_GetParameterAsLong()

#if JEL_FP_SUPPORT

/** Returns the value of a parameter of type float at physical location
 * specified by index. Parameter indices are mapped from left to right. Index
 * value 1 refers to the leftmost parameter that has been passed on to the
 * native method. Remember that parameters of type long or double take up two
 * entries on the operand stack. No type checking or index range checking is
 * performed.
 * \param index The index of a parameter of a primitive type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method.
 * \returns Returns the argument of a primitive type at position index in the
 * list of arguments to the native method. */

static inline jfloat KNI_GetParameterAsFloat(jint index)
{
    return *((float *) (thread_self()->fp->locals + index));
} // KNI_GetParameterAsFloat()

/** Returns the value of a parameter of type double at physical location
 * specified by index. Parameter indices are mapped from left to right. Index
 * value 1 refers to the leftmost parameter that has been passed on to the
 * native method. Remember that parameters of type long or double take up two
 * entries on the operand stack. No type checking or index range checking is
 * performed.
 * \param index The index of a parameter of a primitive type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method.
 * \returns Returns the argument of a primitive type at position index in the
 * list of arguments to the native method. */

static inline jdouble KNI_GetParameterAsDouble(jint index)
{
    return *((double *) (thread_self()->fp->locals + index));
} // KNI_GetParameterAsDouble()

#endif // JEL_FP_SUPPORT

/** Reads the value (object reference) of the parameter specified by \a index,
 * and stores it in the handle \a toHandle. Parameter indices are mapped from
 * left to right. Index value 1 refers to the leftmost parameter that has been
 * passed on to the native method. No type checking or index range checking is
 * performed
 * \param index The index of a parameter of a reference type to the native
 * method. Index value 1 refers to the leftmost parameter in the Java method
 * \param toHandle A handle to to which the return value will be assigned */

static inline void KNI_GetParameterAsObject(jint index, jobject toHandle)
{
    *toHandle = *((uintptr_t *) (thread_self()->fp->locals + index));
} // KNI_GetParameterAsObject()

/** Reads the value of the 'this' pointer in the current stack frame of an
 * instance (non-static) native method, and stores the value in the handle
 * \a toHandle
 * \param toHandle A handle to contain the 'this' pointer */

static inline void KNI_GetThisPointer(jobject toHandle)
{
    *toHandle = *((uintptr_t *) (thread_self()->fp->locals));
} // KNI_GetThisPointer()

/** Reads the value of the class pointer in the current stack frame of a static
 * native method, and stores the value in the handle \a toHandle
 * \param toHandle A handle to contain the class pointer */

static inline void KNI_GetClassPointer(jclass toHandle)
{
    *toHandle = class_get_object(thread_self()->fp->cl);
} // KNI_GetClassPointer()

/** Returns a void value from a native function. Calling this function
 * terminates the execution of the native function immediately */

#define KNI_ReturnVoid() return

/** Returns a boolean value from a native function. Calling the function will
 * terminate the execution of the native function immediately
 * \param value The result value of the native method */

#define KNI_ReturnBoolean(value) return (value) ? 1 : 0

/** Returns a byte value from a native function. Calling the function will
 * terminate execution of the native function immediately
 * \param value The result value of the native method */

#define KNI_ReturnByte(value) return (jbyte) (value)

/** Returns a char value from a native function. Calling the function will
 * terminate execution of the native function immediately
 * \param value The result value of the native method */

#define KNI_ReturnChar(value) return (jchar) (value)

/** Returns a short value from a native function. Calling the function will
 * terminate execution of the native function immediately
 * \param value The result value of the native method */

#define KNI_ReturnShort(value) return (jshort) (value)

/** Returns an int value from a native function. Calling the function will
 * terminate execution of the native function immediately
 * \param value The result value of the native method */

#define KNI_ReturnInt(value) return (jint) (value)

/** Returns a long value from a native function. Calling the function will
 * terminate execution of the native function immediately
 * \param value The result value of the native method */

#define KNI_ReturnLong(value) return (jlong) (value)

#if JEL_FP_SUPPORT

/** Returns a float value from a native function. Calling the function will
 * terminate execution of the native function immediately
 * \param value The result value of the native method */

#define KNI_ReturnFloat(value) return (jfloat) (value)

/** Returns a double value from a native function. Calling the function will
 * terminate execution of the native function immediately
 * \param value The result value of the native method */

#define KNI_ReturnDouble(value) return (jdouble) (value)

#endif // JEL_FP_SUPPORT

/******************************************************************************
 * Handle operations                                                          *
 ******************************************************************************/

/** Internal function used for recording the new handles as temporary roots
 * while declaring them. This function also clears the passed handle so that
 * the garbage collector will not see a garbage reference
 * \param handle A KNI object handle
 * \returns The \a handle argument */

static inline jobject _KNI_push_handle(jobject handle)
{
    *handle = JNULL;
    thread_push_root(handle);
    return handle;
} // _KNI_push_handle()

/** Allocates enough space for n handles for the current native method
 * \param n A positive integer specifying the number of handles to allocate */

#define KNI_StartHandles(n) \
{ \
    size_t __declared_count__ = 0; \
    uintptr_t __handles__[n]

/** Declares a handle that can be used as an input, output, or in/out parameter
 * to an appropriate KNI function. The initial value of the handle is
 * unspecified. If the programmer has not allocated enough space for the handle
 * in the preceding call to KNI_StartHandles, the behavior of this function is
 * unspecified
 * \param handle The name by which the handle will be identified between the
 * calls to KNI_StartHandles and KNI_EndHandles / KNI_EndHandlesAndReturnObject
 */

#define KNI_DeclareHandle(handle) \
    jobject handle = _KNI_push_handle(&__handles__[__declared_count__++])

/** Checks if the given handle is NULL
 * \param handle The handle whose contents will be examined
 * \returns KNI_TRUE if the handle is NULL, KNI_FALSE otherwise */

static inline jboolean KNI_IsNullHandle(jobject handle)
{
    return *handle == JNULL;
} // KNI_IsNullHandle()

/** Checks if the given two handles refer to the same object. No parameter type
 * checking is performed
 * \param obj1 A handle pointing to an object
 * \param obj2 A handle pointing to an object
 * \returns KNI_TRUE if the handles refer to the same object, KNI_FALSE
 * otherwise */

static inline jboolean KNI_IsSameObject(jobject obj1, jobject obj2)
{
    return (*obj1 == *obj2);
} // KNI_IsSameObject()

/** Sets the value of the given handle to NULL
 * \param handle The handle whose value will be set to NULL */

static inline void KNI_ReleaseHandle(jobject handle)
{
    *handle = JNULL;
} // KNI_ReleaseHandle()

/** Undeclares and unallocates the handles defined after the preceding
 * KNI_StartHandles() call */

#define KNI_EndHandles() \
    while (__declared_count__--) { \
        thread_pop_root(); \
    } \
}

/** Undeclares and unallocates the handles defined after the preceding
 * KNI_StartHandles call. At the same time, this function will terminate the
 * execution of the native function immediately and return an object reference
 * back to the caller
 * \param objectHandle The handle from which the result value will be read */

#define KNI_EndHandlesAndReturnObject(objectHandle) \
    while (__declared_count__--) { \
        thread_pop_root(); \
    } \
    return (uintptr_t) *objectHandle; \
}

#endif // !JELATINE_KNI_H
