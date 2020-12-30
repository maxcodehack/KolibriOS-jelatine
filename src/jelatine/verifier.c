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

/** \file verifier.c
 * Bytecode and classfile verification utilities */

#include "wrappers.h"

#include "class.h"
#include "classfile.h"
#include "field.h"
#include "verifier.h"

/******************************************************************************
 * Verifier implementation                                                    *
 ******************************************************************************/

/** Checks if two classes belong to the same package
 * \param cl1 A class pointer
 * \param cl2 A class pointer
 * \returns True if both classes belong to the same package */

bool same_package(const class_t *cl1, const class_t *cl2)
{
    uint32_t i = 0;
    uint32_t sl1 = 0;
    uint32_t sl2 = 0;
    const char *str1 = cl1->name;
    const char *str2 = cl2->name;

    while (str1[i] != 0) {
        if (str1[i] == '/') {
            sl1 = i;
        }

        i++;
    }

    i = 0;

    while (str2[i] != 0) {
        if (str2[i] == '/') {
            sl2 = i;
        }

        i++;
    }

    if (sl1 != sl2) {
        return false;
    }

    if ((sl1 == 0) && (sl2 == 0)) {
        return true;
    } else if (memcmp(str1, str2, sl1 + 1) == 0) {
        return true;
    }

    return false;
} // same_package()

/** Verify the validity of a field
 *
 * If the verification of the field fails this function will throw a
 * verification exception
 * \param cl A pointer to the class to which the field belongs
 * \param info A pointer to the field_info structure representing the field
 * \param attr A pointer to the field_attributes structure holding the field
 * attributes */

void verify_field(const class_t *cl, const field_info_t *info,
                  const field_attributes_t *attr)
{
    uint16_t access_flags = info->access_flags;
    const char *name = info->name;
    const char *descriptor = info->descriptor;
    field_t *field;

    // Check for duplicates
    for (size_t i = 0; i < cl->fields_n; i++) {
        field = cl->fields + i;

        if ((strcmp(field->name, name) == 0)
            && (strcmp(field->descriptor, descriptor) == 0))
        {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Class contains a duplicate field");
        }
    }

    // Check if the field descriptor is sound
    field_parse_descriptor(descriptor);

    // Check if the constant value corresponds to the correct field
    if (attr->constant_value_found)
    {
        if (!(access_flags & ACC_STATIC)) {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Non-static field has a ConstantValue attribute");
        }

        // A reference constant must be a string
        if (descriptor[0] == 'L') {
            if (strcmp(descriptor, "Ljava/lang/String;") != 0) {
                c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                        "A constant static field holds a non-String reference");
            }
        }
    }

    // Check if access flags are sound
    if (((access_flags & ACC_PUBLIC) && (access_flags & ACC_PROTECTED))
        || ((access_flags & ACC_PUBLIC) && (access_flags & ACC_PRIVATE))
        || ((access_flags & ACC_PROTECTED) && (access_flags & ACC_PRIVATE))
        || ((access_flags & ACC_FINAL) && (access_flags & ACC_VOLATILE)))
    {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Illegal field's access flags");
    }

    // Check if access flags are compatible with the class holding this field
    if (class_is_interface(cl)) {
        if (!((access_flags & ACC_PUBLIC) && (access_flags & ACC_STATIC)
              && (access_flags & ACC_FINAL)))
        {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Interface has non-static, public, final field");
        }

        if (access_flags & ACC_TRANSIENT) {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Interface field has ACC_TRANSIENT access flag set");
        }
    }
} // verify_field()

/** Check if the field \a field of class \a dst is accessible from class \a src
 *
 * This function throws a verification exception if the field is not accessible
 * \param acl The class trying to access the field
 * \param cl The class to which the field belongs
 * \param field The field to be accessed */

void verify_field_access(const class_t *acl, const class_t *cl,
                         const field_t *field)
{
    /* If we got here we actually found a field, let's check if we are allowed
     * to access it */

    if (field_is_private(field)) {
        if (acl != cl) {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Trying to access a private field from an external class");
        }
    } else if (field_is_protected(field)) {
        if ((acl != cl) && !class_is_parent(cl, acl) && !same_package(cl, acl))
        {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Trying to access a protected field from a non-child class "
                    "of a different package");
        }
    } else if (!field_is_public(field)) {
        if (!same_package(cl, acl)) {
            // Field is package visible, if ACC_PUBLIC is set no check is needed
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Trying to access a package-visible field from a different "
                    "package");
        }
    }
} // verify_field_access()
