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

/** \file classfile.c
 * Class-file wrapper implementation */

#include "wrappers.h"

#include "class.h"
#include "classfile.h"
#include "memory.h"
#include "util.h"
#include "vm.h"

/******************************************************************************
 * Classpath type definitions                                                 *
 ******************************************************************************/

/** Represents a generic path, it might be a directory path or JAR file. The
 * \a jar field is updated on demand */

struct path_t {
    const char *str; ///< The path to the directory or JAR file
#ifdef JEL_JARFILE_SUPPORT
    ZZIP_DIR *jar; ///< The JAR file handle
#endif // JEL_JARFILE_SUPPORT
};

/** Typedef for the struct path_t */
typedef struct path_t path_t;

/** Represents the virtual machine classpath */

struct classpath_t {
    size_t entries; ///< Entries in the application classpath
    path_t boot; ///< Boot classpath for system classes
    path_t user[]; ///< Classpath for the application classes
};

/** Typedef for the struct classpath_t */
typedef struct classpath_t classpath_t;

/******************************************************************************
 * Globals                                                                    *
 ******************************************************************************/

/** Global classpath information */
static classpath_t *classpath;

/******************************************************************************
 * Local functions prototypes                                                 *
 ******************************************************************************/

static void set_classpath_string(path_t *, const char *);

/******************************************************************************
 * Classpath implementation                                                   *
 ******************************************************************************/

/** Set the classpath string provided in \a str
 * \param path The path to be set
 * \param str The input string */

static void set_classpath_string(path_t *path, const char *str)
{
#if JEL_JARFILE_SUPPORT
    size_t len = strlen(str);

    if ((len > 4) && (strcmp(str + len - 4, ".jar") == 0)) {
        // This is a JAR file
        path->jar = zzip_dir_open(str, 0);

        if (path->jar == NULL) {
            c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                    "Unable to open JAR file: %s", str);
        }
    }
#endif // JEL_JARFILE_SUPPORT

    path->str = str;
} // set_classpath_string()

/** Initializes the classpath by processing the related options */

void classpath_init( void )
{
    size_t count = 1;
    char *bcp = opts_get_boot_classpath();
    char *cp = opts_get_classpath();
    char *idx = cp;

    // Count the number of directories/JAR files in the classpath
    while (*(idx = cstrchrnul(idx, ':')) != '\0') {
        count++;
        *idx = '\0';
        idx++;
    }

    classpath = gc_palloc(sizeof(classpath_t) + sizeof(path_t) * (count + 1));
    classpath->entries = count;

    set_classpath_string(&classpath->boot, bcp);
    idx = cp;

    for (size_t i = 0; i < count; i++) {
        idx = cstrchrnul(idx, ':');
        set_classpath_string(&classpath->user[i], cp);
        idx++;
        cp = idx;
    }
} // classpath_init()

/** Tear down the classpath */

void classpath_teardown( void )
{
#if JEL_JARFILE_SUPPORT
    for (size_t i = 0; i < classpath->entries; i++) {
        if (classpath->user[i].jar != NULL) {
            zzip_dir_close(classpath->user[i].jar);
        }
    }
#endif // JEL_JARFILE_SUPPORT
} // classpath_teardown()

/******************************************************************************
 * Class file implementation                                                  *
 ******************************************************************************/

/** Opens a class file by looking it up in the specified classpath
 * \param name The class name
 * \param cp The classpath used for the class lookup
 * \returns A new classfile object, throws an exception upon failure */

static class_file_t *cf_open_with_classpath(const char *name, const path_t *cp)
{
#if JEL_JARFILE_SUPPORT
    ZZIP_FILE *zzip_file;
#endif // JEL_JARFILE_SUPPORT
    FILE *file = NULL;
    class_file_t *cf;
    char *path;
    size_t cp_len;
    size_t cl_len = strlen(name);

#if JEL_JARFILE_SUPPORT
    if (cp->jar != NULL) {
        // Append the .class suffic to the class name
        path = gc_malloc(cl_len + strlen(".class") + 1);

        strncpy(path, name, cl_len + 1);
        strncat(path, ".class", cl_len + strlen(".class") + 1);
        zzip_file = zzip_file_open(cp->jar, path, 0);

        if (zzip_file == NULL) {
            return NULL;
        }

        cf = gc_malloc(sizeof(class_file_t));
        cf->file.compressed = zzip_file;
        cf->jar = true;
    } else {
#endif // JEL_JARFILE_SUPPORT
        /* Append the class name to the classpath and add the .class suffix to
         * the resulting string */
        cp_len = strlen(cp->str);
        path = gc_malloc(cp_len + cl_len + strlen(".class") + 2);
        strncpy(path, cp->str, cp_len + 1);
        strncat(path, "/", cp_len + 2);
        strncat(path, name, cp_len + cl_len + 2);
        strncat(path, ".class", cp_len + cl_len + strlen(".class") + 2);

        file = fopen(path, "rb");

        if (file == NULL) {
            return NULL;
        }

        cf = gc_malloc(sizeof(class_file_t));
        cf->file.plain = file;
#if JEL_JARFILE_SUPPORT
    }
#endif // JEL_JARFILE_SUPPORT

    gc_free(path);

    return cf;
} // cf_open_with_classpath()

/** Opens a class file with the specified class name (in the internal class
 * format) looking in the classpath directory or JAR file, throws an exception
 * if the class file cannot be found or opened.
 * \param name The class name */

class_file_t *cf_open(const char *name)
{
    class_file_t *cf = NULL;

    if ((strncmp(name, "java/", 5) == 0)
        || (strncmp(name, "javac/", 6) == 0)
        || (strncmp(name, "javax/", 6) == 0)
        || (strncmp(name, "jelatine/", 9) == 0))
    {
        cf = cf_open_with_classpath(name, &classpath->boot);
    } else {
        for (size_t i = 0; i < classpath->entries; i++) {
            cf = cf_open_with_classpath(name, &classpath->user[i]);

            if (cf != NULL) {
                break;
            }
        }
    }

    if (cf == NULL) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR, "Cannot find class %s\n", name);
    }

    return cf;
} // cf_open()

/** Closes a class file and disposes the relevant structure
 * \param cf A pointer to the class_file_t structure to be closed and disposed
 */

void cf_close(class_file_t *cf)
{
#if JEL_JARFILE_SUPPORT
    if (cf->jar) {
        zzip_file_close(cf->file.compressed);
    } else {
#endif
        fclose(cf->file.plain);
#if JEL_JARFILE_SUPPORT
    }
#endif // JEL_JARFILE_SUPPORT

    gc_free(cf);
} // class_file_close()

/** Reads a single-byte value from the provided class-file, throws an exception
 * if the end of the file has been already reached
 * \param cf A pointer to an open class-file
 * \returns A single-byte constant read from the class-file */

uint8_t cf_load_u1(class_file_t *cf)
{
    uint8_t data;
    size_t res;

#if JEL_JARFILE_SUPPORT
    if (cf->jar) {
        res = zzip_file_read(cf->file.compressed, &data, 1);
    } else {
#endif // JEL_JARFILE_SUPPORT
        res = fread(&data, 1, 1, cf->file.plain);
#if JEL_JARFILE_SUPPORT
    }
#endif // JEL_JARFILE_SUPPORT

    if (res != 1) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Unable to read from a class file");
    }

    return data;
} // cf_load_u1()

/** Reads a two-bytes value from the provided class-file, throws an exception if
 * the end of the file has been already reached
 * \param cf A pointer to an open class-file
 * \returns A two-bytes constant read from the class-file */

uint16_t cf_load_u2(class_file_t *cf)
{
    uint8_t data[2];
    size_t res;

#if JEL_JARFILE_SUPPORT
    if (cf->jar) {
        res = zzip_file_read(cf->file.compressed, data, 2);
    } else {
#endif // JEL_JARFILE_SUPPORT
        res = fread(data, 1, 2, cf->file.plain);
#if JEL_JARFILE_SUPPORT
    }
#endif // JEL_JARFILE_SUPPORT

    if (res != 2) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Unable to read from a class file");
    }

    return (data[0] << 8) | data[1];
} // cf_load_u2()

/** Reads a four-bytes value from the provided class-file, throws an exception
 * if the end of the file has been already reached
 * \param cf A pointer to an open class-file
 * \returns A four-bytes constant read from the class-file */

uint32_t cf_load_u4(class_file_t *cf)
{
    uint8_t data[4];
    size_t res;

#if JEL_JARFILE_SUPPORT
    if (cf->jar) {
        res = zzip_file_read(cf->file.compressed, data, 4);
    } else {
#endif // JEL_JARFILE_SUPPORT
        res = fread(data, 1, 4, cf->file.plain);
#if JEL_JARFILE_SUPPORT
    }
#endif // JEL_JARFILE_SUPPORT

    if (res != 4) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Unable to read from a class file");
    }

    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
} // cf_load_u4()

/** Seeks to the position specified by the \a offset parameter, throws an
 * exception if the position is out of the file's range. This function's
 * semantic is the same as fseek() but only the SEEK_SET and SEEK_CUR whence
 * modes are supported
 * \param cf A pointer to an open class-file
 * \param offset The new position in the class-file
 * \param whence From where to start seeking */

void cf_seek(class_file_t *cf, long offset, int whence)
{
    long res;

#if JEL_JARFILE_SUPPORT
    if (cf->jar) {
        res = zzip_seek(cf->file.compressed, offset, whence);
    } else {
#endif // JEL_JARFILE_SUPPORT
        res = fseek(cf->file.plain, offset, whence);
#if JEL_JARFILE_SUPPORT
    }
#endif // JEL_JARFILE_SUPPORT

    if (res < 0) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Unable seek in a class file");
    }
} // cf_seek()

/** Returns the position inside the current class-file
 * \param cf A pointer to an open class-file
 * \returns An integer holding the current position inside the class-file */

long cf_tell(class_file_t *cf)
{
    long pos;

#if JEL_JARFILE_SUPPORT
    if (cf->jar) {
        pos = zzip_tell(cf->file.compressed);
    } else {
#endif // JEL_JARFILE_SUPPORT
        pos = ftell(cf->file.plain);
#if JEL_JARFILE_SUPPORT
    }
#endif // JEL_JARFILE_SUPPORT

    if (pos < 0) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Unable to get the current offset in a class file");
    }

    return pos;
} // cf_tell()

#if JEL_JARFILE_SUPPORT

/** Return a ZZIP_FILE handle for the requested resource or NULL if none was
 * found matching the passed name
 * \param resource A string representing the resource name
 * \returns A pointer to a ZZIP_FILE handle or NULL upon failure */

ZZIP_FILE *jar_get_resource(const char *resource)
{
    ZZIP_FILE *zzip_file = NULL;

    for (size_t i = 0; i < classpath->entries; i++) {
        if (classpath->user[i].jar != NULL) {
            zzip_file = zzip_file_open(classpath->user[i].jar, resource, 0);

            if (zzip_file != NULL) {
                break;
            }
        }
    }

    return zzip_file;
} // jar_get_resource()

#endif // JEL_JARFILE_SUPPORT
