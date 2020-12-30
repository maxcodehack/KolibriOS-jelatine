/*
 *
 * Copyright  1990-2006 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

/*=========================================================================
 * SYSTEM:    Verifier
 * SUBSYSTEM: System support functions
 * FILE:      sys_support.c
 * OVERVIEW:  Routines for system support functions. The routines
 *            are for retrieving system class path and for certain
 *            Windows specific file parsing functions.
 *
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include "wrappers.h"

#include "oobj.h"
#include "jar.h"
#include "typedefs.h"
#include "sys_api.h"
#include "path.h"

/*=========================================================================
 * Globals and extern declarations
 *=======================================================================*/

static cpe_t **saved_classpath;
static cpe_t **saved_classpath_end;

extern zip_t * getZipEntry(char *zipFile, int len);

/*=========================================================================
 * FUNCTION:      sysGetClassPath
 * OVERVIEW:      Returns the system class path using the getenv() system
 *                call for retrieving the CLASSPATH environment.
 * INTERFACE:
 *   parameters:  none
 *
 *   returns:     Pointer to cpe_t struct ptr (see path.h)
 *=======================================================================*/
cpe_t **
sysGetClassPath(void)
{
    struct stat sbuf;
    int length;
    zip_t *zipEntry;
    bool includedDot = false;

    if (saved_classpath == 0) {
        char *cps, *s;
        int ncpe = 1;
        cpe_t **cpp;
        if ((cps = getenv("CLASSPATH")) == 0) {
            cps = ".";
        }
        if ((cps = strdup(cps)) == 0) {
            return 0;
        }
        for (s = cps; *s != '\0'; s++) {
            if (*s == PATH_SEPARATOR) {
                ncpe++;
            }
        }
        /* We add 2 since we automatically append "." to the list, and we
         * need a NULL element at the end.
         * We add an extra 10 to allow pushing and popping of the classpath.
         * Generally, we'll need two, at most, but we can afford to be
         * a little extravagant.
         */
        cpp = saved_classpath = sysMalloc((ncpe + 2 + 10) * sizeof(cpe_t *));
        if (cpp == 0) {
            return 0;
        }
        while (cps && *cps) {
            char *path = cps;
            cpe_t *cpe;
            if ((cps = strchr(cps, PATH_SEPARATOR)) != 0) {
                *cps++ = '\0';
            }
            if (*path == '\0') {
                path = ".";
            }
            cpe = sysMalloc(sizeof(cpe_t));
            if (cpe == 0) {
                return 0;
            }
            length = strlen(path);
            if (JAR_DEBUG && verbose) {
                jio_fprintf(stderr, "SysGetClassPath: Length : %d\n", length);
                jio_fprintf(stderr, "SysGetClasspath: Path : %s\n", path);
            }
            if (stat(path, &sbuf) < 0) {
                /* we don't have to do anything */
            } else if (sbuf.st_mode & S_IFDIR) {
                /* this is a directory */
                cpe->type = CPE_DIR;
                cpe->u.dir = path;
                if (strcmp(path, ".") == 0) {
                    includedDot = true;
                }

                /* restore only for a valid directory */
                *cpp++ = cpe;

                if (JAR_DEBUG && verbose)
                    jio_fprintf(stderr, "SysGetClassPath: Found directory [%s]\n", path);

            } else if (isJARfile (path, length)) {
                /* this looks like a JAR file */
                /* initialize the zip structure */

                if (JAR_DEBUG && verbose)
                    jio_fprintf(stderr, "SysGetClassPath: Found .JAR file [%s]\n", path);

                /* Create the zip entry for searching the JAR
                 * directories. If the zip entry is NULL, it
                 * would indicate that we ran out of memory
                 * and would have exited already.
                 */
                zipEntry = getZipEntry(path, length);

                /* search for the JAR directories */
                if (findJARDirectories(zipEntry, &sbuf)) {
                    /* this is a JAR file - initialize the cpe */

                    if (JAR_DEBUG && verbose)
                        jio_fprintf(stderr, "SysGetClassPath: JAR directories OK in [%s]\n", zipEntry->name);

                    zipEntry->type = 'j';
                    cpe->type = CPE_ZIP;
                    cpe->u.zip = zipEntry;
                    /* restore entry only for a valid JAR */
                    *cpp++ = cpe;
                }
            }
        }
        if (!includedDot) {
            cpe_t *cpe = sysMalloc(sizeof(cpe_t));
            cpe->type = CPE_DIR;
            cpe->u.dir = ".";
            *cpp++ = cpe;
        }
        *cpp = 0;
        saved_classpath_end = cpp;
    }

    return saved_classpath;
}

/* Puts this directory at the beginning of the class path */

void
pushDirectoryOntoClassPath(char* directory)
{
    cpe_t **ptr;

    cpe_t *cpe = sysMalloc(sizeof(cpe_t));
    cpe->type = CPE_DIR;
    cpe->u.dir = directory;

    sysGetClassPath();          /* just in case not done yet */

    /* Note that saved_classpath_end points to the NULL at the end. */
    saved_classpath_end++;
    for (ptr = saved_classpath_end; ptr > saved_classpath; ptr--) {
        ptr[0] = ptr[-1];
    }
    ptr[0] = cpe;
}

/* Puts this jar file at the beginning of the class path */

void
pushJarFileOntoClassPath(zip_t *zipEntry)
{
    cpe_t **ptr;

    cpe_t *cpe = sysMalloc(sizeof(cpe_t));
    cpe->type = CPE_ZIP;
    cpe->u.zip = zipEntry;

    sysGetClassPath();          /* just in case not done yet */

    /* Note that saved_classpath_end points to the NULL at the end. */
    saved_classpath_end++;
    for (ptr = saved_classpath_end; ptr > saved_classpath; ptr--) {
        ptr[0] = ptr[-1];
    }
    ptr[0] = cpe;
}


/* Pop the first element off the class path */
void
popClassPath()
{
    cpe_t **ptr;

    sysFree(*saved_classpath);

    --saved_classpath_end;
    for (ptr = saved_classpath; ptr <= saved_classpath_end; ptr++) {
        /* This copies all of the elements, including the NULL at the end */
        ptr[0] = ptr[1];
    }
}

