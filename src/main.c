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

/** \file main.c
 * main() implementation of the command-line launcher and global data used by
 * the virtual machine */

#include "wrappers.h"

#include "vm.h"
#include "memory.h"
#include "utf8_string.h"

#ifdef USE_SDL_MAIN
#define main SDL_main

#endif

#ifdef USE_SDL
	#include <SDL/SDL.h>
	#ifndef USE_SDL_MAIN	
		#undef main 
	#endif
#endif

#if defined(USE_WIN_SOCK)
	#include <winsock2.h>
#endif

/******************************************************************************
 * Local function declarations                                                *
 ******************************************************************************/

static void parse_command_line_args(int, char *[]);

#if defined (WIN32) && defined(USE_SDL)

static void safe_AllocConsole(){
	AllocConsole();
}
#endif
/** Self explanatory
 * \param argc Number of arguments
 * \param argv Argument list */

int main(int argc, char *argv[])
{
    int jargs_n;
    char **jargs;
#if defined (WIN32) && defined(USE_SDL)
	safe_AllocConsole();
#endif

#if defined(USE_WIN_SOCK)
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	
	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		fprintf(stderr,"\nfailed WSAStartup!ret=%i",err);
	}
#endif

	parse_command_line_args(argc, argv);

    if (opts_get_version()) {
        printf("%s\n", PACKAGE_STRING);
    } else if (opts_get_help() || argc == 1 || opts_get_main_class() == NULL) {
        printf("Usage: jelatine [OPTIONS]... CLASSNAME [ARGUMENTS]...\n"
               "\n"
               "where options include:\n"
               "    -b, --bootclasspath <single directory or JAR>\n"
               "    -c, --classpath <colon separated list of directories and JARs>\n"
               "    -s, --size <size of the heap in bytes>\n"
               "    --stack-size <size of a thread's stack in bytes>\n"
               "\n"
               "    -h, --help      display this help and exit\n"
               "    --version       output version information and exit\n"
 #if JEL_TRACE
               "\n"
               "    --trace-methods trace method invocations\n"
               "    --trace-opcodes trace opcode execution\n"
#endif // JEL_TRACE
#if JEL_PRINT
               "\n"
               "    --print-methods print method invocations\n"
               "    --print-opcodes print opcode execution\n"
               "    --print-memory print memory operations\n"
#endif // JEL_PRINT
            );
    } else {
        jargs = opts_get_jargs();

        if (jargs != NULL) {
            jargs_n = 0;

            while (jargs[jargs_n] != NULL) {
                jargs_n++;
            }

            opts_set_jargs_n(jargs_n);
        } else {
            opts_set_jargs_n(0);
        }

        vm_run();
    }
#ifdef USE_SDL
	if (SDL_WasInit(SDL_INIT_VIDEO)==SDL_INIT_VIDEO)
	{
		SDL_Quit();
	}
#endif

#if defined(USE_WIN_SOCK)
	WSACleanup();
#endif
    exit(0);
} // main()

/** Parses the command-line args, the results are stored in the appropriate
 * global variables
 * \param argc Same as main()'s argc
 * \param argv Same as main()'s argv */

static void parse_command_line_args(int argc, char *argv[])
{
    int i = 1;

    while (i < argc) {
        if (((strcmp("-c", argv[i]) == 0)
             || (strcmp("--classpath", argv[i]) == 0))
            && (i + 1 < argc))

        {
            opts_set_classpath(argv[i + 1]);
            i += 2;
        } else if (((strcmp("-b", argv[i]) == 0)
                    || (strcmp("--bootclasspath", argv[i]) == 0))
                   && (i + 1 < argc))
        {
            opts_set_boot_classpath(argv[i + 1]);
            i += 2;
        } else if (((strcmp("-s", argv[i]) == 0)
                    || (strcmp("--size", argv[i]) == 0))
                   && (i + 1 < argc))
        {
            opts_set_heap_size(atoi(argv[i + 1]));
            i += 2;
        } else if ((strcmp("--stack-size", argv[i]) == 0) && (i + 1 < argc)) {
            size_t stack_size = atoi(argv[i + 1]);

            opts_set_stack_size(size_ceil(stack_size, sizeof(jword_t)));
            i += 2;
        } else if ((strcmp("-h", argv[i]) == 0)
                   || (strcmp("--help", argv[i]) == 0))
        {
            opts_set_help(true);
            i++;
        } else if (strcmp("--version", argv[i]) == 0) {
            opts_set_version(true);
            i++;
#if JEL_PRINT
        } else if (strcmp("--print-opcodes", argv[i]) == 0) {
            opts_set_print_opcodes(true);
            i++;
        } else if (strcmp("--print-methods", argv[i]) == 0) {
            opts_set_print_methods(true);
            i++;
        } else if (strcmp("--print-memory", argv[i]) == 0) {
            opts_set_print_memory(true);
            i++;
#endif // JEL_PRINT
#if JEL_TRACE
        } else if (strcmp("--trace-opcodes", argv[i]) == 0) {
            opts_set_trace_opcodes(true);
            i++;
        } else if (strcmp("--trace-methods", argv[i]) == 0) {
            opts_set_trace_methods(true);
            i++;
#endif // JEL_TRACE
        } else {
            break; // Move to the class-name
        }
    }

    if (i < argc) {
        opts_set_main_class(utf8_slashify(argv[i]));
        i++;

        if (i < argc) {
            opts_set_jargs_n(i - argc);
            opts_set_jargs(argv + i);
        }
    }
} // parse_command_line_args()

