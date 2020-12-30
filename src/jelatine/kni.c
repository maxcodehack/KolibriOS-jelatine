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

/** \file kni.c
 * CLDC native interface implementation */

#include "wrappers.h"

#include "class.h"
#include "field.h"
#include "jstring.h"
#include "kni.h"
#include "loader.h"
#include "memory.h"
#include "vm.h"

#include "java_lang_String.h"
#include "java_lang_Throwable.h"

/******************************************************************************
 * Exceptions and errors                                                      *
 ******************************************************************************/

/** Instantiates an exception object with the message specified by message, and
 * causes that exception to be thrown. A thrown exception will be pending in
 * the current thread, but does not immediately disrupt native code execution.
 * The actual exception will be thrown when the program control returns from
 * the native function back to the virtual machine. Note that KNI_ThrowNew does
 * not run the constructor on the new exception object.
 * \param name The name of a java.lang.Throwable class as a UTF-8 string
 * \param message The message used to construct the java.lang.Throwable object;
 * represented as a UTF-8 string
 * \returns KNI_OK on success; otherwise KNI_ERR */

jint KNI_ThrowNew(const char *name, const char *message)
{
    class_t *cl;
    uintptr_t ref;
    java_lang_Throwable_t *ptr;
    java_lang_String_t *str;
    thread_t *thread = thread_self();

    cl = bcl_resolve_class(thread->fp->cl, name);
    ref = gc_new(cl);
    thread->exception = ref;

    if (message) {
        str = jstring_create_from_utf8(message);
        ptr = JAVA_LANG_THROWABLE_REF2PTR(thread->exception);
        ptr->detailMessage = JAVA_LANG_STRING_PTR2REF(str);
    }

    return KNI_OK;
} // KNI_ThrowNew()

/** Prints an error message to the system’s standard error output, and
 * terminates the execution of the virtual machine immediately
 * \param message The error message as a UTF-8 string */

void KNI_FatalError(const char *message)
{
    fprintf(stderr, "ERROR: %s", message);
    vm_fail();
} // KNI_FatalError()

/******************************************************************************
 * Instance field access                                                      *
 ******************************************************************************/

/** Returns the field ID of an instance field of the class represented by
 * classHandle. The field is specified by its name and signature. The
 * Get<Type>Field and Set<Type>Field families of accessor functions use field
 * IDs to retrieve the value of instance fields. The field must be accessible
 * from the class referred to by classHandle
 * \param classHandle A handle pointing to a class whose field ID will be
 * retrieved
 * \param name The field name as a UTF-8 string
 * \param signature The field descriptor as a UTF-8 string
 * \returns Returns a field ID or NULL if the lookup fails for any reason */

jfieldID KNI_GetFieldID(jclass classHandle, const char *name,
                        const char *signature)
{
    class_t *cl;
    field_t *field;

    cl = bcl_get_class_by_id(JAVA_LANG_CLASS_REF2PTR(*classHandle)->id);
    field = class_get_field(cl, name, signature, false);

    if (field && !field_is_static(field)) {
        return field->offset;
    } else {
        return (jfieldID) NULL;
    }
} // KNI_GetFieldID()

/******************************************************************************
 * Static field access                                                        *
 ******************************************************************************/

/** Returns the field ID of a static field of the class represented by
 * \a classHandle. The field is specified by its name and signature. The
 * GetStatic<Type>Field and SetStatic<Type>Field families of accessor functions
 * use field IDs to retrieve the value of static fields. The field must be
 * accessible from the class referred to by classHandle
 * \param classHandle A handle pointing to a class whose field ID will be
 * retrieved
 * \param name The field name as a UTF-8 string
 * \param signature The field descriptor as a UTF-8 string
 * \returns Returns a field ID or NULL if the lookup fails for any reason */

jfieldID KNI_GetStaticFieldID(jclass classHandle, const char *name,
                              const char *signature)
{
    class_t *cl;
    field_t *field;
    static_field_t *static_field;

    cl = bcl_get_class_by_id(JAVA_LANG_CLASS_REF2PTR(*classHandle)->id);
    field = class_get_field(cl, name, signature, true);

    if (field && field_is_static(field)) {
        static_field = cl->static_data + field->offset;

        return (jfieldID) static_field_data_ptr(static_field);
    } else {
        return (jfieldID) NULL;
    }
} // KNI_GetStaticFieldID()

/******************************************************************************
 * String operations                                                          *
 ******************************************************************************/

/** Copies a number of Unicode characters from a java.lang.String object
 * (denoted by \a stringHandle), beginning at \a offset, to the given buffer
 * \a jcharbuf. No range checking is performed. It is the responsibility of the
 * native function programmer to allocate the return buffer, and to make sure
 * that it is large enough
 * \param stringHandle A handle pointing to a java.lang.String object whose
 * contents are to be copied
 * \param offset The offset within the string object at which to start copying
 * (offset 0: first character in the Java string.)
 * \param n The number of 16-bit Unicode characters to copy
 * \param jcharbuf A pointer to a buffer to hold the Unicode characters */

void KNI_GetStringRegion(jstring stringHandle, jsize offset, jsize n,
                         jchar *jcharbuf)
{
    java_lang_String_t *string = JAVA_LANG_STRING_REF2PTR(*stringHandle);
    uint16_t *data = array_get_data(string->value);

    memcpy(jcharbuf, data + offset, n * sizeof(int16_t));
} // KNI_GetStringRegion()
