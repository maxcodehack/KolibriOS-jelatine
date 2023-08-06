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

/** \file class.c
 * Class related functions implementation */

#include "wrappers.h"

#include "array.h"
#include "class.h"
#include "memory.h"
#include "method.h"
#include "header.h"
#include "util.h"

/******************************************************************************
 * Interface manager implementation                                           *
 ******************************************************************************/

/** Creates an interface manager
 * \returns A new interface manager */

interface_manager_t *im_create( void )
{
    interface_manager_t *im = gc_palloc(sizeof(interface_manager_t));

    return im;
} // im_create()

/** Adds a new interface to the manager
 * \param im A pointer to the interface manager
 * \param cl A pointer to an interface */

void im_add(interface_manager_t *im, class_t *cl)
{
    class_t **interfaces;

    for (size_t i = 0; i < im->entries; i++) {
        if (cl == im->interfaces[i]) {
            return;
        }
    }

    interfaces = gc_malloc(sizeof(class_t *) * (im->entries + 1));
    memcpy(interfaces, im->interfaces, sizeof(class_t *) * im->entries);
    interfaces[im->entries] = cl;
    gc_free(im->interfaces);

    im->interfaces = interfaces;
    im->entries++;
} // im_add()

/** Flattens the interface manager internal representation turning it into a
 * permanent structure
 * \param im A pointer to the interface manager */

void im_flatten(interface_manager_t *im)
{
    class_t **interfaces = gc_palloc(sizeof(class_t *) * im->entries);

    memcpy(interfaces, im->interfaces, sizeof(class_t *) * im->entries);
    gc_free(im->interfaces);
    im->interfaces = interfaces;
} // im_flatten()

/** Checks if an interface is present in the interface manager
 * \param im A pointer to the interface manager
 * \param interface A pointer to a class
 * \returns true if the specified interface is present, false otherwise */

bool im_is_present(interface_manager_t *im, class_t *interface)
{
    for (size_t i = 0; i < im->entries; i++) {
        if (im->interfaces[i] == interface) {
            return true;
        }
    }

    return false;
} // im_is_present()

/******************************************************************************
 * Class implementation                                                       *
 ******************************************************************************/

/** Returns true if \a child is a child class of \a parent
 * \param parent The potentially parent class
 * \param child The potentially child class
 * \returns true if \a child inherits from \a parent, false otherwise  */

bool class_is_parent(const class_t *parent, const class_t *child)
{
    do {
        if (parent == child->parent) {
            return true;
        }

        child = child->parent; // Climb up in the hierarchy
    } while (child != NULL);

    return false;
} // class_is_parent()

/** Adds a field to the class
 * \param cl A pointer to a class being loaded
 * \param info A pointer to the information of the new field
 * \param attr A pointer to the field attributes of the new field */

void class_add_field(class_t *cl, const field_info_t *info,
                     const field_attributes_t *attr)
{
    field_t *field = cl->fields + cl->fields_n;

    field->access_flags = info->access_flags;
    field->name = info->name;
    field->descriptor = info->descriptor;

    if (field_is_static(field) && attr->constant_value_found) {
        /* Store the constant value index in the offset making it non-zero,
         * this will be used during initialization when static fields are
         * laid out and initialized */
        field->offset = attr->constant_value_index;
    }

    cl->fields_n++;
} // class_add_field()

/** Finds a field by its name and descriptor, this function will only
 * look for the field inside the specified class, not its parents
 * \param cl A pointer to the class where to look for the field
 * \param name The field name
 * \param descriptor The descriptor name
 * \param stat true if the field looked for is static, false otherwise
 * \returns A pointer to the field or NULL if none was found */

field_t *class_get_field(const class_t *cl, const char *name,
                         const char *descriptor, bool stat)
{
    field_t *field;

    for (size_t i = 0; i < cl->fields_n; i++) {
        field = cl->fields + i;

        if ((strcmp(field->name, name) == 0)
            && (strcmp(field->descriptor, descriptor) == 0)
            && (field_is_static(field) == stat))
        {
            return field;
        }
    }

    return NULL;
} // class_get_field()

/** This function unloads the class initializer method
 * \param cl A pointer to a class_t structure */

void class_purge_initializer(class_t *cl)
{
    method_purge(mm_get(cl->method_manager, "<clinit>", "()V"));
} // class_purge_initializer()

