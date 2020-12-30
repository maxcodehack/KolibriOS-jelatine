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

/** \file thread.h
 * C object used to implement the java.lang.Thread class */

/** \def JELATINE_THREAD_H
 * thread.h inclusion macro */

#ifndef JELATINE_THREAD_H
#   define JELATINE_THREAD_H (1)

#include "wrappers.h"

#include "method.h"

/******************************************************************************
 * Native threads wrapper type definitions                                    *
 ******************************************************************************/

/** \typedef native_key_t
 * Represents a thread-local storage key of the host machine */

/** \typedef native_thread_t
 * Represents a thread structure of the host machine */

/** \typedef native_mutex_t
 * Represents a mutex structure of the host machine */

/** \typedef native_cond_t
 * Represents a condition variable structure of the host machine */

#if JEL_THREAD_POSIX
#   ifdef TLS
typedef void *native_key_t;
#   else
typedef pthread_key_t native_key_t;
#   endif // TLS
typedef pthread_t native_thread_t;
typedef pthread_mutex_t native_mutex_t;
typedef pthread_cond_t native_cond_t;
#elif JEL_THREAD_PTH
typedef pth_key_t native_key_t;
typedef pth_t native_thread_t;
typedef pth_mutex_t native_mutex_t;
typedef pth_cond_t native_cond_t;
#else
typedef void *native_key_t;
typedef int native_thread_t;
typedef int native_mutex_t;
typedef int native_cond_t;
#endif

/** \def JEL_TLS
 * Thread-local storage class if one is available, otherwise set to an empty
 * definition */

#ifdef TLS
#   define JEL_TLS TLS
#else
#   define JEL_TLS
#endif // TLS

/******************************************************************************
 * Native threads wrapper inlined functions                                   *
 ******************************************************************************/

 /** Returns the value of a native thread-local storage key
  * \param key The TLS key to be read
  * \returns The value held in the TLS key */

static inline void *native_key_get(native_key_t key)
{
#if JEL_THREAD_POSIX
#   ifdef TLS
    return key;
#else
    return pthread_getspecific(key);
#endif // TLS
#elif JEL_THREAD_PTH
    return pth_key_getdata(key);
#else
    return key;
#endif
} // native_key_get()

/** Sets the value of native thread-local storage key
 * \param key A pointer to the TLS key
 * \param value The value to be stored in the key */

static inline void native_key_set(native_key_t *key, void *value)
{
#if JEL_THREAD_POSIX
#   ifdef TLS
    *key = value;
#   else
    pthread_setspecific(*key, value);
#   endif // TLS
#elif JEL_THREAD_PTH
    pth_key_setdata(*key, value);
#else
    *key = value;
#endif
} // native_key_set()

/******************************************************************************
 * Type definitions                                                           *
 ******************************************************************************/

/** Represents a single method frame on the stack */

struct stack_frame_t {
    struct class_t *cl; ///< Class associated with this frame
    method_t *method; ///< Method associated with this frame
    const uint8_t *pc; ///< Current PC
    jword_t *locals; ///< Old locals pointer
};

/** Typedef for struct stack_frame_t */
typedef struct stack_frame_t stack_frame_t;

/** Defines the initial number of temporary root pointers available */
#define THREAD_TMP_ROOTS (2)

/** Virtual machine thread */

struct thread_t {
    struct thread_t *next; ///< Next thread in the list
    bool blocked; ///< Indicates whethever the thread might be blocked
    bool stopped; ///< Indicates whethever the thread must stop execution

    uintptr_t obj; ///< The associated java.lang.Thread object

    jword_t *stack; ///< Global stack pointer
    jword_t *sp; ///< Stack pointer
    stack_frame_t *fp; ///< Frame pointer
    const uint8_t *pc; ///< Saved program counter

    uintptr_t exception; ///< Java exception

    struct {
#if !NDEBUG
        const char *file; ///< File where the exception was thrown
        int line; ///< Line where the exception was thrown
#endif // !NDEBUG
        uintptr_t data; ///< Class type of the exception
        char *description; ///< Exception description
        jmp_buf *buf; ///< Closest jump buffer
    } c_exception; ///< C pseudo-exception

    struct {
        uint16_t capacity; ///< Available temporary root slots
        uint16_t used; ///< Used temporary root slots
        uintptr_t **pointers; ///< Array of pointers to the roots
    } roots; ///< Area used for storing temporary root pointers

    native_thread_t native;  ///< Embedded native thread
    native_cond_t cond; ///< Embedded native condition variable
    native_cond_t *cond_int; ///< Condition on which this thread is waiting
    bool interrupted; ///< True if this thread was interrupted

#if JEL_PRINT
    size_t call_depth; ///< Depth of the current function call
#endif // JEL_PRINT
};

/** Typedef for struct thread_t */
typedef struct thread_t thread_t;

/******************************************************************************
 * Globals                                                                    *
 ******************************************************************************/

extern JEL_TLS native_key_t self;

/******************************************************************************
 * Thread manager interface                                                   *
 ******************************************************************************/

extern void tm_init( void );
extern void tm_teardown( void );
extern void tm_register(thread_t *);
extern void tm_unregister(thread_t *);
extern uint32_t tm_active( void );
extern void tm_mark( void );
extern void tm_purge( void );

#if !JEL_THREAD_NONE
extern void tm_lock();
extern void tm_unlock();
extern void tm_stop_the_world( void );
#else // JEL_THREAD_NONE
static inline void tm_lock( void ) {}
static inline void tm_unlock( void ) {}
static inline void *tm_get_lock( void ) { return NULL; }
static inline void tm_stop_the_world( void ) {}
#endif // !JEL_THREAD_NONE

/******************************************************************************
 * Monitor interface                                                          *
 ******************************************************************************/

extern void monitor_init( void );
extern void monitor_enter(thread_t *, uintptr_t);
extern bool monitor_exit(thread_t *, uintptr_t);

/******************************************************************************
 * Thread interface                                                           *
 ******************************************************************************/

extern void thread_push_root(uintptr_t *);
extern void thread_pop_root( void );
extern void thread_init(thread_t *);
extern uintptr_t thread_create_main(thread_t *, method_t *, uintptr_t *);
extern void thread_sleep(int64_t);

#if !JEL_THREAD_NONE

extern void thread_launch(uintptr_t *, method_t *);
extern void thread_may_block( void );
extern void thread_resumes( void );
extern void thread_interrupt(thread_t *);
extern void thread_yield( void );
extern void thread_join(uintptr_t *);
extern bool thread_wait(uintptr_t, uint64_t, uint32_t);
extern bool thread_notify(uintptr_t, bool);

#else // JEL_THREAD_NONE

static inline void thread_launch(uintptr_t ref, method_t *run)
{
    return;
} // thread_launch()

static inline void thread_may_block( void ) {}
static inline void thread_resumes( void ) {}

static inline void thread_yield( void ) {}
static inline void thread_join(uintptr_t *thread) {}

static inline bool thread_wait(uintptr_t ref, uint64_t millis, uint32_t nanos)
{
    return true;
} // thread_wait()

static inline bool thread_notify(uintptr_t ref, bool broadcast)
{
    return true;
} // thread_notify()

#endif

/******************************************************************************
 * Thread inlined functions                                                   *
 ******************************************************************************/

/** Return a pointer to the execution thread's ::thread_t structure
 * \returns A pointer to this thread own structure */

static inline thread_t *thread_self( void )
{
    return native_key_get(self);
} // thread_self()

#endif // JELATINE_THREAD_H
