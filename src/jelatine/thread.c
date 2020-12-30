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

/** \file thread.c
 * Implementation of the virtual machine thread */

#include "wrappers.h"

#if NEED_SCHED_H
#   include "sched.h"
#endif // NEED_SCHED_H

#include "class.h"
#include "interpreter.h"
#include "kni.h"
#include "loader.h"
#include "memory.h"
#include "thread.h"
#include "vm.h"

#include "java_lang_Thread.h"

/******************************************************************************
 * Native thread wrapper function prototypes                                  *
 ******************************************************************************/

static void native_key_create(native_key_t *);
static void native_key_dispose(native_key_t);
static void native_thread_create(native_thread_t *, void *(*start)(void *),
                                 void *);
static void native_thread_exit( void );
static void native_thread_yield( void );
static void native_mutex_create(native_mutex_t *);
static void native_mutex_dispose(native_mutex_t *);
static void native_mutex_lock(native_mutex_t *);
static void native_mutex_unlock(native_mutex_t *);
static void native_cond_create(native_cond_t *);
static void native_cond_dispose(native_cond_t *);
static void native_cond_wait(native_cond_t *, native_mutex_t *);
static void native_cond_timed_wait(native_cond_t *, native_mutex_t *,
                                   uint64_t, uint32_t);
static void native_cond_signal(native_cond_t *);
static void native_cond_broadcast(native_cond_t *);

/******************************************************************************
 * Native thread wrapper functions                                            *
 ******************************************************************************/

 /** Creates a new native thread-local storage key
  * \param key The address of the key to be initialized */

static void native_key_create(native_key_t *key)
{
#if JEL_THREAD_POSIX && !defined(TLS)
    pthread_key_create(key, NULL);
#elif JEL_THREAD_PTH
    pth_key_create(key, NULL);
#endif
} // native_key_create()

/** Disposes of a native thread-local storage key
 * \param key The key to be disposed of */

static void native_key_dispose(native_key_t key)
{
#if JEL_THREAD_POSIX && !defined(TLS)
    pthread_key_delete(key);
#elif JEL_THREAD_PTH
    pth_key_delete(key);
#endif
} // native_key_dispose()

/** Spawn a new native thread
 * \param thread A pointer to the native thread structure
 * \param start The new thread entry point
 * \param arg A pointer to be passed to the new thread */

static void native_thread_create(native_thread_t *thread,
                                 void *(*start)(void *), void *arg)
{
#if JEL_THREAD_POSIX
    int res;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    res = pthread_create(thread, &attr, start, arg);

    if (res) {
        dbg_error("Unable to create a new thread");
        vm_fail();
    }

    pthread_attr_destroy(&attr);
#elif JEL_THREAD_PTH
    pth_attr_t attr;

    attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);

    *thread = pth_spawn(attr, start, arg);

    if (*thread == NULL) {
        dbg_error("Unable to create a new thread");
        vm_fail();
    }

    pth_attr_destroy(attr);
#endif
} // native_thread_create()

/** Terminates the calling native thread */

static void native_thread_exit( void )
{
#if JEL_THREAD_POSIX
    pthread_exit(NULL);
#elif JEL_THREAD_PTH
    pth_exit(NULL);
#endif
} // native_thread_exit()

/** Yield execution of the current native thread */

static void native_thread_yield( void )
{
#if JEL_THREAD_POSIX
#   if HAVE_PTHREAD_YIELD_NP
    pthread_yield_np();
#   elif HAVE_PTHREAD_YIELD
    pthread_yield();
#   elif HAVE_SCHED_YIELD
    ; // NOP
#   endif // JEL_THREAD_POSIX
#elif JEL_THREAD_PTH
    pth_yield(NULL);
#endif
} // native_thread_yield()

/** Creates a new recursive native mutex
 * \param mutex A pointer to a native mutex structure */

static void native_mutex_create(native_mutex_t *mutex)
{
#if JEL_THREAD_POSIX
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
#elif JEL_THREAD_PTH
    pth_mutex_init(mutex);
#endif
} // native_mutex_create()

/** Disposes of a native mutex
 * \param mutex A pointer to the native mutex structure */

static void native_mutex_dispose(native_mutex_t *mutex)
{
#if JEL_THREAD_POSIX
    pthread_mutex_destroy(mutex);
#endif // JEL_THREAD_POSIX
} // native_mutex_dispose()

/** Acquire the lock on a native mutex
 * \param mutex A pointer to the native mutex structure */

static void native_mutex_lock(native_mutex_t *mutex)
{
#if JEL_THREAD_POSIX
    pthread_mutex_lock(mutex);
#elif JEL_THREAD_PTH
    pth_mutex_acquire(mutex, FALSE, NULL);
#endif
} // native_mutex_lock()

/** Release the lock on a native mutex
 * \param mutex A pointer to the native mutex structure */

static void native_mutex_unlock(native_mutex_t *mutex)
{
#if JEL_THREAD_POSIX
    pthread_mutex_unlock(mutex);
#elif JEL_THREAD_PTH
    pth_mutex_release(mutex);
 #endif
} // native_mutex_unlock()

/** Creates a native condition variable
 * \param cond A pointer to the native condition variable structure */

static void native_cond_create(native_cond_t *cond)
{
#if JEL_THREAD_POSIX
    pthread_cond_init(cond, NULL);
#elif JEL_THREAD_PTH
    pth_cond_init(cond);
#endif
} // native_cond_dispose()

/** Disposes of a native condition variable
 * \param cond A pointer to the native condition variable structure */

static void native_cond_dispose(native_cond_t *cond)
{
#if JEL_THREAD_POSIX
    while (pthread_cond_destroy(cond) != 0) {
        ;
    }
#elif JEL_THREAD_PTH
    /* HACK, GROSS: This relies on internal information of GNU/Pth, we should
     * find a better way for checking that there are no more waiters */
    while (cond->cn_waiters != 0) {
        pth_yield(NULL);
    }
#endif
} // native_cond_dispose()

/** Wait on the specified native condition variable until the variable is
 * signaled
 * \param cond A pointer to a native condition variable
 * \param mutex A pointer to a native mutex structure */

static void native_cond_wait(native_cond_t *cond, native_mutex_t *mutex)
{
#if JEL_THREAD_POSIX
    pthread_cond_wait(cond, mutex);
#elif JEL_THREAD_PTH
    pth_cond_await(cond, mutex, NULL);
#endif
} // native_cond_wait()

/** Wait on the specified native condition variable until the variable is
 * signaled or the specified timout expires
 * \param cond A pointer to a native condition variable
 * \param mutex A pointer to a native mutex structure
 * \param millis The timeout in milliseconds
 * \param nanos The timeout in nanoseconds */

static void native_cond_timed_wait(native_cond_t *cond, native_mutex_t *mutex,
                                   uint64_t millis, uint32_t nanos)
{
#if JEL_THREAD_POSIX
    if ((millis == 0) && (nanos == 0)) {
        pthread_cond_wait(cond, mutex);
    } else {
        struct timespec req = get_time_with_offset(millis, nanos);

        pthread_cond_timedwait(cond, mutex, &req);
    }
#elif JEL_THREAD_PTH
    if ((millis == 0) && (nanos == 0)) {
          pth_cond_await(cond, mutex, NULL);
    } else {
        pth_event_t ev;

        ev = pth_event(PTH_EVENT_TIME,
                       pth_timeout(millis / 1000,
                                   (millis % 1000) * 1000000 + nanos));
        pth_cond_await(cond, mutex, ev);
        pth_event_free(ev, PTH_FREE_ALL);
    }
#else
struct timespec req2;
    struct timespec req = {
        millis / 1000,
        (millis % 1000) * 1000000 + nanos
    };

    // Threading is disabled
    //nanosleep(&req, NULL);
#endif
} // native_cond_timed_wait()

/** Signal a thread waiting on a native condition variable
 * \param cond A pointer to a native condition variable */

static void native_cond_signal(native_cond_t *cond)
{
#if JEL_THREAD_POSIX
    pthread_cond_signal(cond);
#elif JEL_THREAD_PTH
    pth_cond_notify(cond, false);
#endif
} // native_cond_signal()

/** Signals all the threads waiting on a native condition variable
 * \param cond A pointer to the native condition variable */

static void native_cond_broadcast(native_cond_t *cond)
{
#if JEL_THREAD_POSIX
    pthread_cond_broadcast(cond);
#elif JEL_THREAD_PTH
    pth_cond_notify(cond, true);
#endif
} // native_cond_broadcast()

/******************************************************************************
 * Thread manager                                                             *
 ******************************************************************************/

/** Payload passed to a new thread's thread_start() routine. Holds all the
 * information necessary to bootstrap the thread. */

struct thread_payload_t {
    method_t *run; ///< The run() method of the thread
    uintptr_t *ref; ///< A pointer to the thread object
    native_thread_t thread; ///< Newly created thread
    native_cond_t cond; ///< Synchronization condition variable
};

/** Typedef for the struct thread_payload_t type */
typedef struct thread_payload_t thread_payload_t;

/** Represents a Java monitor */

struct monitor_t {
    struct monitor_t *next; ///< Next monitor in the list
    uintptr_t ref; ///< Object associated with this monitor
    thread_t *owner; ///< Owner thread, NULL if the monitor is unlocked
    size_t count; ///< Number of times the monitor was acquired
    native_cond_t *cond; ///< Condition variable associated with the monitor
};

/** Typedef for the struct monitor_t type */
typedef struct monitor_t monitor_t;

/** Define the minimum capacity (i.e. number of buckets) of the monitor table */
#define TM_CAPACITY (4)

/** Thread manager, contains all the threads and is used to interact with them
 * globally */

struct thread_manager_t {
    native_mutex_t lock; ///< Global VM lock
#if JEL_THREAD_PTH
    size_t switch_n; ///< Number of context switches
#endif // JEL_THREAD_PTH
    size_t active; ///< Number of active threads
    thread_t *queue; ///< Doubly-linked queue of active threads
    size_t capacity; ///< Capacity of the monitor hash-table
    size_t entries; ///< Number of used entries of the monitor hash-table
    monitor_t *buckets; ///< Buckets of the monitor hash-table
};

/** Typedef for the struct thread_manager_t type */
typedef struct thread_manager_t thread_manager_t;

/******************************************************************************
 * Globals                                                                    *
 ******************************************************************************/

/** Thread-local self pointer of a thread */
JEL_TLS native_key_t self;

/** Thread manager singleton */
static thread_manager_t tm;

/******************************************************************************
 * Thread manager implementation                                              *
 ******************************************************************************/

/** Number of global lock acquisition/release couples before yielding when
 * using GNU/Pth */
#define PTH_SWITCH_N (500)

/** Initialize the thread manager, this does not include the initialization of
 * the Java monitors subsystem because the thread manager is initialized before
 * the heap manager */

void tm_init( void )
{
    memset(&tm, 0, sizeof(thread_manager_t));

#if JEL_THREAD_PTH
    if (!pth_init()) {
        vm_fail();
    }

    tm.switch_n = PTH_SWITCH_N;
#endif // JEL_THREAD_PTH

    /* When using multi-threading the global machine lock is recursive so that
     * the code can enter more than one syncrhonized sections safely */
    native_mutex_create(&tm.lock);
    native_key_create(&self);
} // tm_init()

/** Tears down the thread manager */

void tm_teardown( void )
{
#if JEL_THREAD_POSIX
    thread_t *thread;

    /* FIXME: We should wait for all threads to complete instead of canceling
     * them */

    tm_lock();
    tm_stop_the_world();

    for (thread = tm.queue; thread != NULL; thread = thread->next) {
        pthread_cancel(thread->native);
    }

    tm_unlock();
#endif // JEL_THREAD_POSIX

    native_key_dispose(self);
    native_mutex_dispose(&tm.lock);

#if JEL_THREAD_PTH
    pth_kill();
#endif // JEL_THREAD_PTH
} // tm_teardown()

/** Register a new thread in the thread manager
 * \param thread A pointer to the new thread */

void tm_register(thread_t *thread)
{
    thread->next = tm.queue;
    tm.queue = thread;
    tm.active++;
} // tm_register()

/** Unregister a dying thread
 * \param thread A thread which has finished execution */

void tm_unregister(thread_t *thread)
{
    thread_t *prev, *curr;

    prev = NULL;
    curr = tm.queue;

    while (thread != curr) {
        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL) {
        tm.queue = curr->next;
    } else {
        prev->next = curr->next;
    }

    tm.active--;
} // tm_unregister_thread()

/** Returns the number of active threads in the system
 * \returns Returns the number of active threads */

uint32_t tm_active( void )
{
#if JEL_FINALIZER
    return tm.active - 1;
#else
    return tm.active;
#endif // JEL_FINALIZER
} // tm_active()

/** Marks all the references reacheable from the threads */

void tm_mark( void )
{
    jword_t *scan;
    thread_t *thread;

    for (thread = tm.queue; thread != NULL; thread = thread->next) {
        gc_mark_reference(thread->obj);

        // Mark the temporary roots
        for (size_t i = 0; i < thread->roots.used; i++) {
            gc_mark_potential(*(thread->roots.pointers[i]));
        }

        // Crawl the stack
        if (thread->stack != NULL) {
            for (scan = thread->stack; scan < thread->sp; scan++) {
                gc_mark_potential(*((uintptr_t *) scan));
            }
        }

        // Mark the pending exception
        gc_mark_reference(thread->exception);
    }
} // tm_mark()

/** Hash function used for object references
 * \param ref A reference to a Java object */

static size_t tm_hash(uintptr_t ref)
{
#if SIZEOF_VOID_P == 4
    return (ref >> 2);
#else
    return (ref >> 3);
#endif
} // tm_hash()

/** Purge the monitor table by clearing the entries associated with dead
 * objects, also purge the associated conditions */

void tm_purge( void )
{
    size_t hash, entries = 0;
    monitor_t *entry, *other;
    header_t *header;

    for (size_t i = 0; i < tm.capacity; i++) {
        entry = tm.buckets + i;

        if (entry->ref) {
            header = (header_t *) entry->ref;

            if (!header_is_marked(header)) {
                /* The object referenced by this monitor is dead, let's purge
                 * the monitor then */

                if (entry->cond) {
                    native_cond_dispose(entry->cond);
                    gc_free(entry->cond);
                }

                memset(entry, 0, sizeof(monitor_t));
            } else {
                entries++;
            }
        }
    }

    tm.entries = entries;

    /* This first phase will scan the buckets looking for the remaining
     * monitors. If a monitor is not in the bucket corresponding to its hash
     * value we check the bucket corresponding to the hash value. If it is
     * empty the monitor is moved there, if it is full but the monitor there
     * does not correspond to the hash value the two monitors are swapped.
     * This phase also clears the 'next' field of the monitors so that we can
     * re-chain them in the next phase */

    for (size_t i = 0; i < tm.capacity; i++) {
        entry = tm.buckets + i;

        if (entry->ref) {
            entry->next = NULL;
            hash = tm_hash(entry->ref) & (tm.capacity - 1);

            if (i != hash) {
                other = tm.buckets + hash;

                if (other->ref == JNULL) {
                    *other = *entry;
                    memset(entry, 0, sizeof(monitor_t));
                } else if ((tm_hash(other->ref) & (tm.capacity - 1)) != hash) {
                    monitor_t dummy = *other;
                    *other = *entry;
                    *entry = dummy;
                }
            }
        }
    }

    /* We look for the entries that are in the buckets which are not associated
     * with their hash value. Those entries are chained after the entries in
     * the buckets associated with their hash value */

    for (size_t i = 0; i < tm.capacity; i++) {
        entry = tm.buckets + i;

        if (entry->ref) {
            hash = tm_hash(entry->ref) & (tm.capacity - 1);

            if (i != hash) {
                entry->next = tm.buckets[hash].next;
                tm.buckets[hash].next = entry;
            }
        }
    }
} // tm_purge()

/** Rehash the monitor table, growing or shrinking it
 * \param grow true if the table must be grown, false if it must be shrinked */

static void tm_rehash(bool grow)
{
    size_t capacity, hash;
    size_t i, j;
    monitor_t *buckets, *entry;

    if (grow) {
        capacity = tm.capacity * 2;
    } else {
        capacity = tm.capacity / 2;
    }

    buckets = gc_malloc(capacity * sizeof(monitor_t));

    for (i = 0; i < tm.capacity; i++) {
        entry = tm.buckets + i;

        if (entry->ref) {
            j = hash = tm_hash(entry->ref) & (capacity - 1);

            while (buckets[j].ref != JNULL) {
                j = (j + 1) & (capacity - 1);
            }

            buckets[j].ref = entry->ref;
            buckets[j].owner = entry->owner;
            buckets[j].count = entry->count;
            buckets[j].cond = entry->cond;

            // Chain the entry if there was a clash
            if (j == hash) {
                buckets[j].next = NULL;
            } else {
                buckets[j].next = buckets[hash].next;
                buckets[hash].next = buckets + j;
            }
        }
    }

    gc_free(tm.buckets);
    tm.capacity = capacity;
    tm.buckets = buckets;
} // tm_rehash()

#if !JEL_THREAD_NONE

/** Try to gain the ownership of the machine-wide lock.
 * This function will return only when the caller has the ownership of the lock
 * in the meantime the calling thread is considered blocked. It is safe to call
 * this function repeatedly from the same thread as long as tm_unlock() is
 * called the same number of times to release the lock */

void tm_lock( void )
{
    thread_may_block();
    native_mutex_lock(&tm.lock);
    thread_resumes();
} // tm_lock()

/** Release the ownership of the machine-wide lock */

void tm_unlock( void )
{
    native_mutex_unlock(&tm.lock);

#if JEL_THREAD_PTH
    if (--tm.switch_n == 0) {
        tm.switch_n = PTH_SWITCH_N;
        pth_yield(NULL);
    }
#endif // JEL_THREAD_PTH
} // tm_unlock()

/** Waits for all threads to stop. The calling thread must have taken the VM
 * lock for this to happen safely */

void tm_stop_the_world( void )
{
    thread_t *self = thread_self();
    thread_t *thread;
    bool stopped = false;

    // Inform all threads that they must stop
    self->blocked = true;

    for (thread = tm.queue; thread != NULL; thread = thread->next) {
        thread->stopped = true;
    }

    // Wait for all the threads to stop
    while (!stopped) {
        stopped = true;

        for (thread = tm.queue; thread != NULL; thread = thread->next) {
            stopped &= thread->blocked;
        }

        thread_yield();
    }

    self->blocked = false;
} // tm_stop_the_world()

#endif // !JEL_THREAD_NONE

/******************************************************************************
 * Monitors                                                                   *
 ******************************************************************************/

/** Initialization of the Java monitors subsystem */

void monitor_init( void )
{
    tm.capacity = TM_CAPACITY;
    tm.buckets = gc_malloc(TM_CAPACITY * sizeof(monitor_t));
} // monitor_init()

/** Implements the MONITOR_ENTER opcode
 * \param thread The thread requesting the monitor
 * \param ref A Java object reference */

void monitor_enter(thread_t *thread, uintptr_t ref)
{
    monitor_t *entry;
    size_t hash;
    bool done;

    do {
        done = true;
        tm_lock();

        hash = tm_hash(ref) & (tm.capacity - 1);

        // Check if the monitor is already present
        entry = tm.buckets + hash;

        while (entry && (entry->ref != ref)) {
            entry = entry->next;
        }

        if (entry) {
            // The monitor is already present
            if (entry->owner == NULL) {
                entry->owner = thread;
                entry->count = 1;
            } else if (entry->owner == thread) {
                entry->count++;
            } else {
                // This monitor is contended, spin
                tm_unlock();
                thread_yield();
                done = false;
            }
        } else {
            // The monitor is not there, let's insert a new one
            size_t i = hash;

            while (tm.buckets[i].ref != JNULL) {
                i = (i + 1) & (tm.capacity - 1);
            }

            entry = tm.buckets + i;
            entry->ref = ref;
            entry->owner = thread;
            entry->count = 1;

            // Chain the entry if there was a clash
            if (i == hash) {
                entry->next = NULL;
            } else {
                entry->next = tm.buckets[hash].next;
                tm.buckets[hash].next = entry;
            }

            // Check if we must grow the table
            tm.entries++;

            if (tm.entries == tm.capacity) {
                tm_rehash(true);
            } else if (tm.entries < (tm.capacity / 4)) {
                tm_rehash(false);
            }
        }
    } while (!done);

    tm_unlock();
} // monitor_enter()

/** Implements the MONITOR_EXIT opcode
 * \param thread A pointer to the thread relinquishing the monitor
 * \param ref The object on which the monitor was taken
 * \returns true if everything went well, false if \a thread didn't own the
 * monitor and thus MONITOR_EXIT would fail */

bool monitor_exit(thread_t *thread, uintptr_t ref)
{
    monitor_t *entry;
    size_t hash;
    bool done = false;

    tm_lock();
    hash = tm_hash(ref) & (tm.capacity - 1);

    // Look for the monitor
    entry = tm.buckets + hash;

    while (entry && (entry->ref != ref)) {
        entry = entry->next;
    }

    if (entry) {
        if (entry->owner == thread) {
            if (entry->count == 1) {
                entry->owner = NULL;
            }

            entry->count--;
            done = true;
        }
    }

    tm_unlock();

    return done;
} // monitor_exit()

/******************************************************************************
 * Thread implementation                                                      *
 ******************************************************************************/

/** Sets the thread's own ::thread_t structure
 * \param thread A pointer to the thread's own structure */

static void thread_set_self(thread_t *thread)
{
    native_key_set(&self, thread);
} // thread_set_self()

/** Push a temporary root on the current thread's stack
 * \param ref A pointer to an object reference to be pushed on the temporary
 * root stack */

void thread_push_root(uintptr_t *ref)
{
    thread_t *thread = thread_self();
    uintptr_t **pointers;

    thread->roots.pointers[thread->roots.used++] = ref;

    /* Grow the stack so that a free slot is always available when entering
     * this function */
    if (thread->roots.used == thread->roots.capacity) {
        pointers = gc_malloc((thread->roots.capacity + 1)
                             * sizeof(uintptr_t *));
        memcpy(pointers, thread->roots.pointers,
               thread->roots.capacity * sizeof(uintptr_t *));
        gc_free(thread->roots.pointers);
        thread->roots.pointers = pointers;
        thread->roots.capacity++;
    }
} // thread_push_root()

/** Pop a temporary root from a thread's stack */

void thread_pop_root( void )
{
    thread_self()->roots.used--;
} // thread_pop_root()

/** Initializes all the thread-local structures required for normal operation
 * of a thread including the thread's self reference
 * \param thread A pointer to the thread's own structure */

void thread_init(thread_t *thread)
{
    memset(thread, 0, sizeof(thread_t));
    thread_set_self(thread);
    native_cond_create(&thread->cond);
} // thread_init()

/** Creates the main thread. This function doesn't return until the main thread
 * has finished execution as it uses the current native/green thread instead of
 * creating a new one
 * \param thread The main thread pre-allocated structure
 * \param run The main method
 * \param args A pointer to a reference to the arguments array
 * \returns The reference to an uncaught exception if the main method terminated
 * abruptly or JNULL */

uintptr_t thread_create_main(thread_t *thread, method_t *run, uintptr_t *args)
{
    class_t *thread_cl = bcl_find_class("java/lang/Thread");
    size_t stack_size = opts_get_stack_size();

    /* Contrary to the startup of a 'regular' thread we do not initialize nor
     * register the thread as the main thread has already been initialized and
     * registered even though it is only partially initialized */

    // Build the stack
    thread->stack = gc_malloc(stack_size);
    thread->sp = thread->stack;
    thread->fp = (stack_frame_t *) ((char *) thread->stack + stack_size);

    /* Create the first Java thread structure, since the first thread is not
     * created by VMThread.start() we have to do it by hand */
    thread->obj = gc_new(thread_cl);
    JAVA_LANG_THREAD_REF2PTR(thread->obj)->vmThread = (uintptr_t) thread;
    JAVA_LANG_THREAD_REF2PTR(thread->obj)->priority = 5;
    JAVA_LANG_THREAD_REF2PTR(thread->obj)->name =
        JAVA_LANG_STRING_PTR2REF(jstring_create_from_utf8("Thread-0"));

    // HACK: Push the arguments on top of the stack
    *((uintptr_t *) thread->sp) = *args;

    // Run the new thread's code
    interpreter(run);

    // Mark the thread as dead, none can join on it anymore after this point
    tm_lock();
    native_cond_broadcast(&thread->cond);
    tm_unregister(thread);
    JAVA_LANG_THREAD_REF2PTR(thread->obj)->vmThread = JNULL;
    tm_unlock();

    gc_free(thread->roots.pointers);
    gc_free(thread->stack);

    return thread->exception;
} // thread_create_main()

/** Implements the functionality required by the java.lang.Thread.sleep()
 * method, waiting for the number of milliseconds specified by \a ms
 * \param ms The number of milliseconds to wait */

void thread_sleep(int64_t ms)
{
    thread_t *self = thread_self();

    tm_lock();

    if (!self->interrupted) {
        native_cond_t cond; // Dummy condition variable

        native_cond_create(&cond);
        self->cond_int = &cond;
        thread_may_block();
        native_cond_timed_wait(&cond, &tm.lock, ms, 0);
        thread_resumes();
        self->cond_int = NULL;
    }

    if (self->interrupted) {
        self->interrupted = false;
        KNI_ThrowNew("java/lang/InterruptedException", NULL);
    }

    tm_unlock();
} // thread_sleep()

#if !JEL_THREAD_NONE

/** Startup function used as the entry point of a new thread. Both the payload
 * passed in \a arg and the new thread structures (including the stack) will be
 * fred by this function upon ending execution
 * \param arg A pointer to the thread payload structure
 * \returns Nothing important */

static void *thread_start(void *arg)
{
    size_t stack_size = opts_get_stack_size();
    thread_payload_t *payload = (thread_payload_t *) arg;
    method_t *run = payload->run;
    uintptr_t *ref = payload->ref;
    thread_t thread;
    int exception;

    // Initialize the new thread
    thread_init(&thread);

    // Build the stack
    thread.stack = gc_malloc(stack_size);
    thread.sp = thread.stack;
    thread.fp = (stack_frame_t *) ((char *) thread.stack + stack_size);

    // Create the temporary roots area
    thread.roots.capacity = THREAD_TMP_ROOTS;
    thread.roots.pointers = gc_malloc(THREAD_TMP_ROOTS * sizeof(uintptr_t *));

    tm_lock();

    // Link the java.lang.Thread object to the VM thread and register it
    thread.obj = *ref;
    JAVA_LANG_THREAD_REF2PTR(thread.obj)->vmThread = (uintptr_t) &thread;
    JAVA_LANG_THREAD_REF2PTR(thread.obj)->priority = 5;
    tm_register(&thread);

    // Inform the parent thread that we're starting execution
    thread.native = payload->thread;
    native_cond_signal(&payload->cond);
    tm_unlock();

    // HACK: Push the 'this' pointer on top of the stack
    *((uintptr_t *) thread.stack) = thread.obj;

    c_try {
        interpreter(run); // Run the new thread's code
    } c_catch (exception) {
        c_print_exception(exception);
        c_clear_exception();
        /* FIXME: The thread must shut down but the rest of the VM should be
         * kept running and joining threads must be woken up. */
        vm_fail();
    }

    // Mark the thread as dead, none can join on it anymore after this point
    tm_lock();

    native_cond_broadcast(&thread.cond);
    tm_unregister(&thread);
    JAVA_LANG_THREAD_REF2PTR(thread.obj)->vmThread = JNULL;

    /* We can safely release the stack and root pointers now that the thread
     * is not visible anymore to the garbage collector */
    gc_free(thread.roots.pointers);
    gc_free(thread.stack);

    tm_unlock();

    // Wait for the joining thread to wake up
    native_cond_dispose(&thread.cond);

    if (thread.exception != JNULL) {
        // TODO: Print the exception and stack trace
        dbg_error("Uncaught exception");
        vm_fail();
    }

    native_thread_exit();

    return NULL;
} // thread_start()

/** Creates a new thread executing the provided method
 * \param ref The java.lang.Thread object associated with this thread
 * \param run The first method executed by this thread */

void thread_launch(uintptr_t *ref, method_t *run)
{
    thread_payload_t *payload;

    // Prepare the thread payload
    payload = gc_malloc(sizeof(thread_payload_t));
    payload->run = run;
    payload->ref = ref;
    native_cond_create(&payload->cond);

    // We take the global lock for we will be waiting for the new thread
    tm_lock();

    // FIXME: Check if thread creation fails and shut down the machine 'cleanly'
    native_thread_create(&payload->thread, thread_start, payload);

    // Wait for the new thread
    thread_may_block();
    native_cond_wait(&payload->cond, &tm.lock);
    thread_resumes();
    tm_unlock();

    // Cleanup
    native_cond_dispose(&payload->cond);
    gc_free(payload);
} // thread_launch()

/** Flags the calling thread as potentially blocked. This function must be
 * called every time a thread might block for a long time in order to avoid
 * livelocks. Two examples might be when the thread takes a lock or when it
 * calls a blocking native method like recv(). When the thread has finished the
 * blocking operation thread_resumes() must be called */

void thread_may_block( void )
{
    thread_t *self = thread_self();

    assert(!self->blocked);
    self->blocked = true;
} // thread_may_block()

/** Restores a thread to its steady state after a blocking operation. Before
 * returning however this function checks if a global stop-the-world has been
 * requested, if so it waits until it is asked to resume execution. This
 * function must be always called after a blocking operation and coupled with a
 * previous call to  thread_may_block() */

void thread_resumes( void )
{
    thread_t *self = thread_self();

    assert(self->blocked);
    self->blocked = false;

    // Check if a stop-the world has been requested
    if (self->stopped) {
        /* Mark the thread as blocked again so that the tm_stop_the_world() can
         * make sure that this thread is effectively blocked */
        self->blocked = true;

        /* The global lock is already held by the thread that called
         * tm_stop_the_world() so this thread will block until the lock has
         * been released */
        native_mutex_lock(&tm.lock);
        native_mutex_unlock(&tm.lock);

        self->stopped = false;
        self->blocked = false;
    }
} // thread_resumes()

/** Interrupts a thread. This function is used for implementing the
 * java.lang.Thread.interrupt() method and can interrupt immediately the
 * java.lang.Object.wait() and java.lang.Thread.sleep() methods but only
 * asynchronously the java.lang.Thread.join() method
 * \param thread A pointer to the thread to be interrupted */

void thread_interrupt(thread_t *thread)
{
    tm_lock();
    thread->interrupted = true;

    if (thread->cond_int) {
        native_cond_signal(thread->cond_int);
    }

    tm_unlock();
} // thread_interrupt()

/** Causes the currently executing thread object to temporarily pause and allow
 * other threads to execute */

void thread_yield( void )
{
    native_thread_yield();
} // thread_yield()

/** Joins on the passed thread
 * \param thread A pointer to the reference to the Java thread the function
 * will join on */


void thread_join(uintptr_t *thread)
{
    thread_t *self = thread_self();
    thread_t *target;

    tm_lock();

    if (!self->interrupted) {
        target = (thread_t *) JAVA_LANG_THREAD_REF2PTR(*thread)->vmThread;

        if (target != NULL) {
            self->cond_int = &target->cond;
            thread_may_block();
            native_cond_wait(&target->cond, &tm.lock);
            thread_resumes();
            self->cond_int = NULL;
        }
    }

    if (self->interrupted) {
        self->interrupted = false;
        KNI_ThrowNew("java/lang/InterruptedException", NULL);
    }

    tm_unlock();
} // thread_join()

/** Implements the functionality required by java.lang.Object.wait() and
 * friends, waiting on an object until it is notified by notify() or notifyAll()
 * or until a timeout occurs (if \a nanos or \a millis or both ar non-zero)
 * \param ref The object on which to wait
 * \param millis Number of milliseconds before a timeout occurs
 * \param nanos Number of nanoseconds before a timeout occurs
 * \returns true if everything goes well, false if the monitor associated with
 * \a ref was not owned \a thread. In the latter case an
 * IllegalMonitorStateException should be thrown */

bool thread_wait(uintptr_t ref, uint64_t millis, uint32_t nanos)
{
    thread_t *self = thread_self();
    monitor_t *entry;
    size_t hash;

    tm_lock();

    // Look for the monitor
    hash = tm_hash(ref) & (tm.capacity - 1);
    entry = tm.buckets + hash;

    while (entry && (entry->ref != ref)) {
        entry = entry->next;
    }

    if (entry && !self->interrupted) {
        if ((entry->owner == self) && (entry->count == 1)) {
            // Release the lock and wait
            entry->owner = NULL;
            entry->count = 0;

            if (entry->cond == NULL) {
                entry->cond = gc_malloc(sizeof(native_cond_t));
                native_cond_create(entry->cond);
            }

            self->cond_int = entry->cond;
            thread_may_block();

            if ((millis == 0) && (nanos == 0)) {
                native_cond_wait(entry->cond, &tm.lock);
            } else {
                native_cond_timed_wait(entry->cond, &tm.lock, millis, nanos);
            }

            thread_resumes();
            self->cond_int = NULL;

            // Re-acquire the monitor
            tm_unlock();
            monitor_enter(self, ref);
        }
    } else {
        tm_unlock();
    }

    if (self->interrupted) {
        self->interrupted = false;
        KNI_ThrowNew("java/lang/InterruptedException", NULL);
    }

    return entry ? true : false;
} // thread_wait()

/** Implements the functionliaty required by java.lang.Object.notify()
 * \param ref The object used for the notification
 * \param broadcast If true, wake up all the threads waiting on this object,
 * otherwise wake up only one thread
 * \returns true if everything goes well, false if the monitor associated with
 * \a ref was not owned by \a thread. In the latter case an
 * IllegalMonitorStateException should be thrown */

bool thread_notify(uintptr_t ref, bool broadcast)
{
    thread_t *self = thread_self();
    monitor_t *entry;
    size_t hash;
    bool res = false;

    tm_lock();

    // Look for the monitor
    hash = tm_hash(ref) & (tm.capacity - 1);
    entry = tm.buckets + hash;

    while (entry && (entry->ref != ref)) {
        entry = entry->next;
    }

    if (entry) {
        if (entry->owner == self) {
            // Notify the waiting thread
            if (entry->cond != NULL) {
                if (broadcast) {
                    native_cond_broadcast(entry->cond);
                } else {
                    native_cond_signal(entry->cond);
                }
            }

            res = true;
        }
    }

    tm_unlock();
    return res;
} // thread_notify()

#endif // JEL_THREAD_POSIX || JEL_THREAD_PTH

