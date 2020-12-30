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

/** \file vm.c
 * Virtual machine object implementation */

#include "wrappers.h"

#include "class.h"
#include "jstring.h"
#include "interpreter.h"
#include "loader.h"
#include "memory.h"
#include "method.h"
#include "thread.h"
#include "utf8_string.h"
#include "util.h"
#include "vm.h"

#include "java_lang_Thread.h"

/******************************************************************************
 * Virtual machine related type definitions                                   *
 ******************************************************************************/

/** Represents the manager used for handling threads */

struct virtual_machine_t {
    // Tracing and printing facilities
#if JEL_TRACE
    bool trace_bytecodes; ///< True if bytecode tracing is enabled
    bool trace_methods; ///< True if method tracing is enabled
#endif // JEL_TRACE
#if JEL_PRINT
    bool print_bytecodes; ///< True if bytecode printing is enabled
    bool print_methods; ///< True if method printing is enabled
#endif // JEL_PRINT
};

/** Typedef for the struct virtual_machine_t */
typedef struct virtual_machine_t virtual_machine_t;

/******************************************************************************
 * Globals                                                                    *
 ******************************************************************************/

/** Global options of the VM */

options_t options = {
    ".", // classpath
    JEL_CLASSPATH_DIR, // boot_classpath
    128 * 1024, // heap_size
    4096, // stack_size
    NULL, // main_class
    NULL, // jargs
    0, // jargs_n

#if JEL_TRACE
    false, // trace_methods
    false, // trace_opcodes
#endif // JEL_TRACE

#if JEL_PRINT
    false, // print_methods
    false, // print_opcodes
    false, // print_memory
#endif // JEL_PRINT

    false, // version
    false // help
};

/** Global virtual-machine pointer */
virtual_machine_t vm;

/******************************************************************************
 * Local function prototypes                                                  *
 ******************************************************************************/

static void vm_init( void );
static void vm_teardown( void );

/******************************************************************************
 * Virtual machine related functions implementation                           *
 ******************************************************************************/

/** Launches the JVM */

void vm_run( void )
{
    int jargc = opts_get_jargs_n();
    char * const *jargv = opts_get_jargs();
    char *main_class;
    thread_t main_thread;
    class_t *cl, *thread_cl;
    method_t *method;
    uintptr_t args, ref;
    uintptr_t *args_data;
    int exception;

    // The main thread is initialized early because we need it for exceptions
    tm_init();
    thread_init(&main_thread);
    tm_register(&main_thread);

    c_try {
        vm_init();

        /* We initialize the temporary root pointers storage of the main thread
         * so that we can push temporary roots early in the bootstrap process */
        main_thread.roots.capacity = THREAD_TMP_ROOTS;
        main_thread.roots.pointers = gc_malloc(THREAD_TMP_ROOTS
                                               * sizeof(uintptr_t *));

        main_class = utf8_intern(opts_get_main_class(),
                                 strlen(opts_get_main_class()));

        /* Check if the 'main' string purged from the trailing '0' is in the
         * limited UTF8 format supported by Java */
        if (!utf8_check(main_class)) {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Invalid class name: %s", main_class);
        }

        /* java.lang.String, java.lang.Class, java.lang.Thread and the array
         * class [C must be preloaded as they are needed by constant string
         * objects and class objects respectively */
        bcl_preload_bootstrap_classes();

        // Resolve the bootstrap classes
        bcl_resolve_class(NULL, "java/lang/Object");
        bcl_resolve_class(NULL, "[C");
        bcl_resolve_class(NULL, "java/lang/String");
        thread_cl = bcl_resolve_class(NULL, "java/lang/Thread");

        // Enable the garbage-collector now that it is safe to do so
        gc_enable(true);

        // Setup the dummy methods
        init_dummy_methods();

#if JEL_FINALIZER
        /* Create the finalizer thread: find the jelatine.VMFinalizer class and
         * find its run() method */
        cl = bcl_resolve_class(NULL, "jelatine/VMFinalizer");
        method = mm_get(cl->method_manager, "run", "()V");
        assert(method != NULL);

        /* Create the finalizer Java thread object, since it is not created
         * by VMThread.start() we have to do it by hand */
        ref = gc_new(thread_cl);
        gc_register_finalizer(ref);
        JAVA_LANG_THREAD_REF2PTR(ref)->priority = 5;
        thread_push_root(&ref);
        // After this call has returned the finalizer thread will be running
        thread_launch(&ref, method);
        thread_pop_root();
#endif // JEL_FINALIZER

        cl = bcl_resolve_class(NULL, main_class);
        // Find the main() method
        method = mm_get(cl->method_manager, "main", "([Ljava/lang/String;)V");

        if (method == NULL) {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Unable to find the main() method in class: %s",
                    main_class);
        } else if (!method_is_public(method)
                   || method_is_native(method)
                   || !method_is_static(method))
        {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "main() method is not public, native or non-static");
        }

        method->access_flags |= ACC_MAIN;

        // Prepare the 'args' array holding command line arguments
        args = gc_new_array_ref(bcl_resolve_class(NULL, "[Ljava/lang/String;"),
                                jargc);
        args_data = array_ref_get_data((array_t *) args);
        thread_push_root(&args);

        for (int i = 0; i < jargc; i++) {
            args_data[-i] =
                JAVA_LANG_STRING_PTR2REF(jstring_create_literal(jargv[i]));
        }

        // Launch the main thread
        ref = thread_create_main(&main_thread, method, &args);
        thread_pop_root();

        if (ref != JNULL) {
            // TODO: Print the type of the unknown exception and its contents
            c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                    "Uncaught exception in main thread");
        }
    } c_catch (exception) {
        c_print_exception(exception);
        c_clear_exception();
        vm_fail();
    }

    /* The thread manager is destroyed before the other VM structures because
     * we need to stop all the running threads before doing anything else */
    tm_teardown();
    vm_teardown();
} // vm_run()

/** Initializes the basic subsystems of the virtual machine */

static void vm_init( void )
{
    // Initialize the various subsystems
    gc_init(opts_get_heap_size());
    monitor_init();
    string_manager_init(6, 2);
    jsm_init(6, 2);
    classpath_init();
    bcl_init();

    // Initialize the VM structure
    memset(&vm, 0, sizeof(virtual_machine_t));
} // vm_init()

/** Destroys the virtual machine instance */

static void vm_teardown( void )
{
    classpath_teardown();
    gc_teardown();
} // vm_teardown()
