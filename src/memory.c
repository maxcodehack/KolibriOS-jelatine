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

/** \file memory.c
 * Garbage-collector and allocator implementation */

#include "wrappers.h"

#include "class.h"
#include "header.h"
#include "loader.h"
#include "memory.h"
#include "thread.h"
#include "util.h"
#include "vm.h"

#include "java_lang_ref_WeakReference.h"

/******************************************************************************
 * Type definitions                                                           *
 ******************************************************************************/

/** An object in the finalizer list */

struct finalizable_t {
    struct finalizable_t *next; ///< Next object in the list
    uintptr_t ref; ///< Reference to the finalized object
};

/** Typedef for the struct finalizable_t */
typedef struct finalizable_t finalizable_t;

/** Represents a free chunk of memory belonging to the small bin */

struct small_chunk_t {
    struct small_chunk_t *next; ///< The next chunk in the list
};

/** Typedef for the struct small_chunk_t */
typedef struct small_chunk_t small_chunk_t;

/** Represents a free chunk of memory belonging to the large bin */

struct large_chunk_t {
    struct large_chunk_t *next; ///<  Next chunk in the list
    size_t size; ///<  Size of the chunk in bytes
};

/** Typedef for the struct large_chunk_t */
typedef struct large_chunk_t large_chunk_t;

/** Defines the number of entries in the chunk-bin */
#define BIN_ENTRIES (16)

/** Defines the size of the largest chunk belonging to the small bin */
#define BIN_MAX_SIZE (BIN_ENTRIES * sizeof(jword_t))

/** Initial fraction of the provided heap to use */
#define HEAP_INIT_FRACTION (16)

/** Number of slots to add to the temporary root stack when growing it */
#define TEMP_ROOT_INC (4)

/** Structure representing the heap object
 *
 * Note that the last field not only points to the last word in the Java-heap
 * but also to a fake object-header used as a sentinel by the garbage-collector
 */

struct heap_t {
    bool collect; ///< True if the collector can run
    size_t size; ///< Current heap size (in bytes)
    size_t max_size; ///< Maximum heap size (in bytes)

    void *memory; ///< Generic heap used by the VM
    uintptr_t start; ///< First word of the heap
    uintptr_t end; ///< First word after the end of the heap
    uintptr_t perm; ///< First word of the permanent allocation area

    uint8_t *bitmap; ///< Memory bitmap

    java_lang_ref_WeakReference_t *weakref_list; ///< Weak references list

    large_chunk_t *large_bin; ///< Bin used for storing large free chunks
    small_chunk_t *bin[BIN_ENTRIES]; ///< Bin used for storing small free chunks

#if JEL_FINALIZER
    uintptr_t finalizer; ///< A reference to the finalizer thread's object
    finalizable_t *finalizable; ///< List of finalizer objects still alive
    finalizable_t *finalizing; ///< Objects waiting to be finalized
#endif // JEL_FINALIZER
};

/** Typedef for the struct heap_t */
typedef struct heap_t heap_t;

/******************************************************************************
 * Local functions prototypes                                                 *
 ******************************************************************************/

static uintptr_t gc_alloc(size_t);
static void gc_purge_bin( void );
static void gc_mark( void );
static void gc_mark_finalizable( void );
static void gc_sweep(size_t);
static void gc_purge_weakref_list( void );
static void gc_grow(uintptr_t, size_t);

// Bitmap management functions

static inline void bitmap_set(uintptr_t);
static inline void bitmap_clear(uintptr_t);
static inline bool bitmap_get(uintptr_t);

// Chunk management functions

static uintptr_t get_chunk(size_t);
static void put_chunk(uintptr_t, size_t);

#ifndef NDEBUG
void print_bin( void );
#endif // !NDEBUG

/******************************************************************************
 * Local declarations                                                         *
 ******************************************************************************/

/** The virtual machine heap */
static heap_t heap;

/******************************************************************************
 * Heap implementation                                                        *
 ******************************************************************************/

/** Initializes the heap, throws an exception upon failure
 *
 * Note that if a heap of less than 32 KiB is requested the function will
 * default to 32 KiB anyway as mandated by the CLDC configuration. If you want
 * to use a heap smaller than 32 KiB you have to explicitely change this
 * function, do it at your own risk. All the pointers in the heap structure are
 * explicitely aligned to 4 or 8-bytes boundaries depending on the target
 * machine
 *
 * \param size Size in bytes of the unified heap */

void gc_init(size_t size)
{
    void *unified_heap;
    uint8_t *bitmap;
    size_t init_size;
    size_t heap_size; // Actual size of the heap in jwords

    // Grow the heap size if it is smaller than the canonical 32KiB CLDC heap
    size = (size < 32768) ? 32768 : size;

    /* Truncate the size to a multiple of 4 or 8 bytes, so we won't bother about
     * alignment problems, we assume that malloc() will already return a
     * properly aligned block */
    size = size_ceil(size, sizeof(jword_t));
    heap_size = (size * sizeof(jword_t) * 8) / ((sizeof(jword_t) * 8) + 1);
    heap_size = size_floor(heap_size, sizeof(jword_t));
    init_size = size_ceil(heap_size / HEAP_INIT_FRACTION, sizeof(jword_t));

    unified_heap = malloc(size);
    memset(unified_heap, 0, size);
    bitmap = (uint8_t *) unified_heap + heap_size;

    if (unified_heap == NULL) {
        dbg_error("Out of memory, cannot allocate the unified heap.");
        vm_fail();
    }

    // Initialize the heap structure
    heap.size = init_size;
    heap.max_size = heap_size;
    heap.memory = unified_heap;
    heap.start = (uintptr_t) unified_heap;
    heap.end = (uintptr_t) unified_heap + init_size;
    heap.perm = heap.start + heap_size;
    heap.weakref_list = NULL;
    heap.bitmap = bitmap;
    heap.large_bin = NULL;

    memset(heap.bin, 0, sizeof(small_chunk_t *) * BIN_ENTRIES);

    // Initialize the first free chunk
    put_chunk((uintptr_t) unified_heap, init_size);

    /* Disable the collector, we will re-enable it once all the other global
     * structures have been initialized */
    heap.collect = false;

#if JEL_FINALIZER
    heap.finalizer = JNULL;
    heap.finalizable = NULL;
    heap.finalizing = NULL;
#endif // JEL_FINALIZER
} // gc_init()

/** Disposes the unified heap */

void gc_teardown( void )
{
    free(heap.memory);
} // gc_teardown()

/** Enables or disables the collector
 * \param enable new status of the collector */

void gc_enable(bool enable)
{
    heap.collect = enable;
} // gc_enable()

/** Allocates a managed object of the specified class
 *
 * If not enough memory is avaible an exception is thrown, note that the
 * returned reference points to the object header, not to the first word of the
 * object.
 *
 * \param cl A pointer to the object class
 * \returns A reference to the newly created object, throws an exception upon
 * failure */

uintptr_t gc_new(class_t *cl)
{
    uintptr_t ptr;
    header_t *header; // Pointer to the new object's header
    size_t size; // Size of the object (in words) to be allocated
    uint32_t ref_n;

    tm_lock();

#if SIZEOF_VOID_P == (SIZEOF_JWORD_T / 2)
    ref_n = size_ceil(class_get_ref_n(cl), 2);
#else
    ref_n = class_get_ref_n(cl);
#endif

    size = (ref_n * sizeof(uintptr_t)) + sizeof(header_t)
           + size_ceil(class_get_nref_size(cl), sizeof(jword_t));

    assert((size >= sizeof(jword_t)) && (size % sizeof(jword_t) == 0));

    ptr = gc_alloc(size);
    ptr += ref_n * sizeof(uintptr_t); // Point to the header
    bitmap_set(ptr);
    header = (header_t *) ptr;
    *header = header_create_object(cl);

#if JEL_PRINT
    if (opts_get_print_memory()) {
        fprintf(stderr, "NEW PTR: %p SIZE: %zu\n", (void *) ptr, size);
    }
#endif // JEL_PRINT

    tm_unlock();

    return ptr;
} // gc_new()

/** Creates a new array of the specified type
 * \param type The basic array class type requested
 * \param count The number of elements
 * \returns A reference to the newly created array */

uintptr_t gc_new_array_nonref(array_type_t type, int32_t count)
{
    class_t *cl = bcl_array_class(type);
    size_t size;
    array_t *array;
    uintptr_t ptr;

    if (type == T_BOOLEAN) {
        size = size_div_inf(count, 8);
    } else {
        size = count * array_elem_size(type);
    }

    size = size_ceil(sizeof(array_t) + size, sizeof(jword_t));
    tm_lock();
    ptr = gc_alloc(size);
    bitmap_set(ptr);
    array = (array_t *) ptr;
    array->header = header_create_object(cl);
    array->length = count;
    tm_unlock();

#if JEL_PRINT
    if (opts_get_print_memory()) {
        fprintf(stderr, "NEW PTR: %p SIZE: %zu\n", (void *) ptr, size);
    }
#endif // JEL_PRINT

    return ptr;
} // gc_new_array_nonref()

/** Creates a new array of references
 * \param cl A pointer to the corresponding class
 * \param count The number of elements
 * \returns A pointer to the newly created array */

uintptr_t gc_new_array_ref(class_t *cl, int32_t count)
{
    ref_array_t *array;
    uintptr_t ptr;
    size_t size;

#if SIZEOF_VOID_P == (SIZEOF_JWORD_T / 2)
    size = size_ceil(count, 2) * sizeof(uintptr_t);
#else
    size = count * sizeof(uintptr_t);
#endif // SIZEOF_VOID_P == (SIZEOF_JWORD_T / 2)

    tm_lock();
    ptr = gc_alloc(sizeof(ref_array_t) + size);
    ptr += size;
    bitmap_set(ptr);
    array = (ref_array_t *) ptr;
    array->header = header_create_object(cl);
    array->length = count;
    tm_unlock();

#if JEL_PRINT
    if (opts_get_print_memory()) {
        fprintf(stderr, "NEW PTR: %p SIZE: %zu\n", (void *) ptr,
                sizeof(ref_array_t) + size);
    }
#endif // JEL_PRINT

    return ptr;
} // gc_new_array_ref()

/** Helper function used for creating multi-dimensional arrays
 * \param cl A pointer to the corresponding class
 * \param dimensions The number of dimensions to be created
 * \param counts A pointer to the element counts
 * \returns A pointer to the newly created array */

uintptr_t gc_new_multiarray(class_t *cl, uint8_t dimensions, jword_t *counts)
{
    uintptr_t *references;
    uintptr_t ref;
    int32_t count = *((int32_t *) counts);
    int32_t i;

    if (count == 0) {
        return JNULL;
    }

    if (dimensions == 1) {
        if (cl->elem_type != PT_REFERENCE) {
            return gc_new_array_nonref(prim_to_array_type(cl->elem_type),
                                       count);
        } else {
            return gc_new_array_ref(cl, count);
        }
    } else {
        ref = gc_new_array_ref(cl, count);
        references = array_ref_get_data((array_t *) ref);

        thread_push_root(&ref);

        for (i = 0; i < count; i++) {
            references[-i] = gc_new_multiarray(cl->elem_class, dimensions - 1,
                                               counts + 1);
        }

        thread_pop_root();

        return ref;
    }
} // gc_new_multiarray()

#if JEL_FINALIZER

/** Stores a reference to the finalizer's thread Java object, it will be used
 * to wake up the finalizer thread when a new object is ready to be finalized
 * \param A reference to the finalizer thread's Thread object */

void gc_register_finalizer(uintptr_t ref)
{
    heap.finalizer = ref;
} // gc_register_finalizer()

/** Registers a newly created finalizable object
 * \param ref A reference to the object to be registered */

void gc_register_finalizable(uintptr_t ref)
{
    finalizable_t *fin;

    thread_push_root(&ref);
    tm_lock();
    fin = gc_malloc(sizeof(finalizable_t));
    fin->ref = ref;
    fin->next = heap.finalizable;
    heap.finalizable = fin;
    thread_pop_root();
    tm_unlock();
} // gc_register_finalizable()

/** Returns the first finalizable object in the list
 * \returns A reference to the first object waiting to be finalized, JNULL if
 * none are pending */

uintptr_t gc_get_finalizable( void )
{
    thread_t *self = thread_self();
    uintptr_t ref = JNULL;
    finalizable_t *fin;

    monitor_enter(self, heap.finalizer);

    if (heap.finalizing == NULL) {
        thread_wait(heap.finalizer, 0, 0);
    }

    assert(heap.finalizing != NULL);
    fin = heap.finalizing;
    heap.finalizing = fin->next;
    monitor_exit(self, heap.finalizer);
    ref = fin->ref;
    gc_free(fin);

    return ref;
} // gc_get_finalizable()

#endif // JEL_FINALIZER

/** Adds a new object to the weak reference list
 * \param ref A reference to the object to be added to the weak reference list
 */

void gc_register_weak_ref(java_lang_ref_WeakReference_t *ref)
{
    tm_lock();
    ref->next = heap.weakref_list;
    heap.weakref_list = ref;
    tm_unlock();
} // gc_register_weak_ref()

/** Grow the heap by size bytes (if possible).
 *
 * A new chunk will be added to the chunk bin holding the new memory area
 * starting from \a end. \a end might be different from the current heap end so
 * that the last free chunk can be grown directly
 * \param end A pointer to the last free chunk of memory
 * \param size The amount of bytes to be added to the heap */

void gc_grow(uintptr_t end, size_t size)
{
    if (heap.end + size > heap.perm) {
        heap.end = heap.perm;
    } else {
        heap.end += size;
    }

    heap.size = heap.end - heap.start;
    put_chunk(end, heap.end - end);
} // gc_grow()

/** Launches the garbage collector on the provided heap structure
 * \param grow If non null, specifies the amount of bytes of the next
 * allocation, the heap will be grown after the collection to accomodate it if
 * not enough free space was reclaimed */

void gc_collect(size_t grow)
{
    tm_lock();

#if JEL_PRINT
    if (opts_get_print_memory()) {
       fprintf(stderr, "GARBAGE COLLECTION\n");
    }
#endif // JEL_PRINT

    if (heap.collect != false) {
        tm_stop_the_world(); // Wait for all threads to stop

        // Mark garbage collected objects
        gc_mark();
        gc_mark_finalizable();

        // Mark and purge of non-garbage collected structures
        gc_purge_weakref_list();
        jsm_purge();
        tm_purge();
        gc_purge_bin();

        gc_sweep(grow);
    } else {
        gc_grow(heap.end, grow); // Just grow the heap
    }

    // If a collection happened this will also restart the stopped threads
    tm_unlock();
} // gc_collect()

/** Helper function used for implementing Runtime.freeMemory(), returns
 * the amount of free memory in bytes
 * \returns The amount of free memory in bytes */

size_t gc_free_memory( void )
{
    small_chunk_t *schunk;
    large_chunk_t *lchunk;
    size_t size;
    uint32_t i;

    tm_lock();
    size = 0;

    for (i = 0; i < BIN_ENTRIES; i++) {
        schunk = heap.bin[i];

        while (schunk != NULL) {
            size += (i + 1) * sizeof(jword_t);
            schunk = schunk->next;
        }
    }

    lchunk = heap.large_bin;

    while (lchunk != NULL) {
        size += lchunk->size;
        lchunk = lchunk->next;
    }

    tm_unlock();
    return size;
} // gc_free_memory()

/** Helper function used for implementing Runtime.totalMemory(), returns
 * the amount of memory available to the VM in bytes
 * \returns The amount of memory available to the VM in bytes */

size_t gc_total_memory( void )
{
    return heap.size;
} // gc_total_memory()

/** Check if a reference is a pointer and recursively gc_mark it if it is
 * \param ref A reference to a Java object */

void gc_mark_potential(uintptr_t ref)
{
    /* If it is not aligned to a JWORD_SIZE boundary then this is definetely
     * not a reference to a Java object */
    if (ref & (sizeof(jword_t) - 1)) {
        return;
    }

    // If it exceeds the heap bounds this is not a reference to a Java object
    if ((ref < heap.start) || (ref >= heap.end)) {
        return;
    }

    /* If the corresponding entry in the bitmap is not marked then this is not
     * a reference to a Java (or C) object */
    if (!bitmap_get(ref)) {
        return;
    }

    /* If we got here then we have something that may be a pointer and actually
     * points to something, let's gc_mark it */
    gc_mark_reference(ref);
} // gc_mark_potential()

/** Marks the object pointed by ref and recursively all the objects
 * pointed by the references it contains
 * \param ref A reference to a Java object */

#if !JEL_POINTER_REVERSAL

void gc_mark_reference(uintptr_t ref)
{
    class_t *cl;
    header_t *header, *new_header;
    uintptr_t *references;
    uintptr_t new_ref;
    uint32_t ref_n;

    if (ref == JNULL) {
        return;
    }

    header = (header_t *) ref;

    if (!header_is_object(header)) {
        return;
    }

    cl = header_get_class(header);

    if (header_is_marked(header)) {
        return;
    } else {
        header_set_mark(header);
    }

    if (class_is_array(cl) && (cl->elem_type == PT_REFERENCE)) {
        ref_n = array_get_ref_n((array_t *) header);
    } else {
        ref_n = class_get_ref_n(cl);
    }

    references = ((uintptr_t *) header) - ref_n;

    for (size_t i = 0; i < ref_n; i++) {
        new_ref = references[i];

        if (new_ref == JNULL) {
            continue;
        } else {
            new_header = (header_t *) new_ref;

            if (header_is_marked(new_header)) {
                continue;
            } else {
                gc_mark_reference(new_ref);
            }
        }
    }
} // gc_mark_reference()

#else

void gc_mark_reference(uintptr_t ref)
{
    class_t *cl;
    uintptr_t prev, curr, tmp;
    header_t *header, *tmp_header;
    jword_t count;
    uint32_t ref_n;
    bool ref_array;

    if (ref == JNULL) {
        return;
    }

    header = (header_t *) ref;

    if (!header_is_object(header) || header_is_marked(header)) {
        return;
    }

    curr = ref;
    prev = JNULL;

    header = (header_t *) curr;
    header_create_gc_counter(header);

    while (1) {
        cl = bcl_get_class_by_id(header_get_class_index(header));

        if (class_is_array(cl) && (cl->elem_type == PT_REFERENCE)) {
            ref_array = true;
            ref_n = ((array_t *) header)->length;
        } else {
            ref_array = false;
            ref_n = class_get_ref_n(cl);
        }

        count = header_get_count(header, ref_array);

        if (count < ref_n) {
            // We're not done yet with the current object
            tmp = *((uintptr_t *) curr - count - 1);

            if (tmp == JNULL) {
                // Null reference, skip to the next object
                header_set_count(header, count + 1, ref_array);
                continue;
            }

            tmp_header = (header_t *) tmp;

            if (!header_is_marked(tmp_header)) {
                /* This object is not marked, gc_mark it, push the previous
                 * object and set this one as the current */
                *((uintptr_t *) curr - count - 1) = prev;
                prev = curr;
                curr = tmp;
                header = tmp_header;
                header_create_gc_counter(header);
                continue;
            } else {
                // This object has already been marked, skip to the next one
                header_set_count(header, count + 1, ref_array);
                continue;
            }
        } else {
            // We're done with the current object, pop the previous
            tmp = curr;
            curr = prev;
            header = (header_t *) curr;

            // We returned to the root pointer, return to the caller
            if (curr == JNULL) {
                return;
            }

            cl = bcl_get_class_by_id(header_get_class_index(header));

            if (class_is_array(cl) && (cl->elem_type == PT_REFERENCE)) {
                ref_array = true;
            } else {
                ref_array = false;
            }

            count = header_get_count(header, ref_array);
            prev = *((uintptr_t *) curr - count - 1);
            *((uintptr_t *) curr - count - 1) = tmp;
            header_set_count(header, count + 1, ref_array);
        }
    }
} // gc_mark_reference()

#endif // !JEL_POINTER_REVERSAL

/** Allocates a chunk of the specified size (in jwords) from the bin, calls the
 * garbage collector if necessary and throws an exception if unable to satisfy
 * the request. The returned memory will have already been cleared
 * \param size The chunk size in jwords
 * \returns A chunk of memory carved from the garbage collected heap */

static uintptr_t gc_alloc(size_t size)
{
    uintptr_t ptr = get_chunk(size);

    if (ptr == 0) {
        // get_chunk() failed... collect and retry
        gc_collect(size);
        ptr = get_chunk(size);

        if (ptr == 0) {
            dbg_error("Out of memory. Try giving the VM a larger heap with the"
                      " --size <size_in_bytes> option.");
            vm_fail();
        }
    }

    memset((jword_t *) ptr, 0, size);
    return ptr;
} // gc_alloc()

/** Purges the free chunks bin
 *
 * This function also clears the free chunks left in the bin so that the
 * garbage collector can reclaim the space they hold and aggregate them with
 * other potentially free chunks surrounding them */

static void gc_purge_bin( void ) {
    small_chunk_t *schunk;
    large_chunk_t *lchunk;
    char *free_space;
    size_t free_size;

    // Empty the fixed-size bins

    for (size_t i = 0; i < BIN_ENTRIES; i++) {
        schunk = heap.bin[i];
        heap.bin[i] = NULL;

        while (schunk != NULL) {
            free_space = (char *) schunk;
            free_size = (i + 1) * sizeof(jword_t);
            schunk = schunk->next;
            memset(free_space, 0, free_size); // Clear the fred space
        }
    }

    // Empty the large bin
    lchunk = heap.large_bin;
    heap.large_bin = NULL;

    while (lchunk != NULL) {
        free_space = (char *) lchunk;
        free_size = lchunk->size;
        lchunk = lchunk->next;
        memset(free_space, 0, free_size); // Clear the fred space
    }
} // gc_purge_bin()

/** Executes the 'gc_mark' phase of the garbage collector
 *
 * The gc_mark phase consists in scanning the root objects (threads and stacks
 * basically) for references to objects in the heap. Each of these object is
 * then marked as live and its references explored in turn and so on until all
 * the live objects are found and marked. The recursive descent algorythm uses
 * the pointer-reversal trick to avoid using an implicit/explicit stack,
 * the gc is thus completely self-contained and cannot fail due to a stack
 * overflow */

static void gc_mark( void )
{
    bcl_mark();
    jsm_mark();
    tm_mark();
} // gc_mark()

/** After the gc_mark phase executed checks the finalizable obejcts. The ones which
 * haven't been marked and thus are dead are 'resurrected' by marking them and
 * moved to the list of objects to be finalized. Once the finalizer thread has
 * processed them they will be lost forever */

static void gc_mark_finalizable( void )
{
#if JEL_FINALIZER
    thread_t *self = thread_self();
    finalizable_t *curr = heap.finalizable;
    finalizable_t *prev = NULL;
    finalizable_t *next;

    while (curr != NULL) {
        if (!header_is_marked((header_t *) (curr->ref))) {
            // Add the object to the queue of the objects being finalized
            next = curr->next;
            curr->next = heap.finalizing;
            heap.finalizing = curr;

            if (curr->next == NULL) {
                monitor_enter(self, heap.finalizer);
                thread_notify(heap.finalizer, false);
                monitor_exit(self, heap.finalizer);
            }

            if (prev == NULL) {
                heap.finalizable = next;
            } else {
                prev->next = next;
            }

            curr = next;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }

    // Resurrect the objects which are being finalized
    curr = heap.finalizing;

    while (curr != NULL) {
        gc_mark_reference(curr->ref);
        curr = curr->next;
    }
#endif // JEL_FINALIZER
} // gc_mark_finalizable()

/** Scans the heap for live objects and reclaims dead ones replenishing
 * the free list
 * \param size The size of the allocation which triggered the collection, if
 * no chunk of free space larger or equal to this value is fred the heap must
 * be grown of at least this value */

static void gc_sweep(size_t size)
{
    class_t *cl;
    header_t *header;
    uintptr_t scan = heap.start;
    uintptr_t start; // Start of a new object
    uintptr_t end = heap.start; // End of the previous object
    size_t reclaimed = 0;
    size_t in_use = 0;
    size_t max_size = 0;
    size_t nref_size, ref_n;
    bool is_java;
#if JEL_POINTER_REVERSAL
    uint32_t id;
#endif // JEL_POINTER_REVERSAL

    while (scan < heap.end) {
        if ((*((uintptr_t *) scan) & ((1 << HEADER_RESERVED) - 1)) == 0) {
            scan += sizeof(jword_t);

            if (scan >= heap.end) {
                break;
            } else {
                continue;
            }
        }

        header = (header_t *) scan; // We found a header

        if (header_is_object(header)) {
            assert(bitmap_get(scan));
            is_java = true;

#if JEL_POINTER_REVERSAL
            if (header_is_marked(header)) {
                id = header_get_class_index(header);
                header_restore(header, bcl_get_class_by_id(id));
                header_set_mark(header);
            }
#endif // JEL_POINTER_REVERSAL

            cl = header_get_class(header);

            if (class_is_array(cl)) {
                nref_size = array_get_nref_size((array_t *) header);
                ref_n = array_get_ref_n((array_t *) header);
            } else {
                nref_size = class_get_nref_size(cl);
                ref_n = class_get_ref_n(cl);
            }
        } else {
            is_java = false;
            ref_n = 0;
            nref_size = header_get_size(header);
        }

        nref_size = size_ceil(nref_size, sizeof(jword_t));
#if SIZEOF_VOID_P == (SIZEOF_JWORD_T / 2)
        ref_n = size_ceil(ref_n, 2);
#endif // SIZEOF_VOID_P == (SIZEOF_JWORD_T / 2)

        if (header_is_marked(header)) {
            if (is_java) {
                header_clear_mark(header);
            }

            start = scan - ref_n * sizeof(uintptr_t);

            /* If there was some free space between the previous object and
             * this one reclaim it */
            if (start - end >= sizeof(jword_t)) {
                if (start - end > max_size) {
                    max_size = start - end;
                }

                put_chunk(end, start - end);
                reclaimed += start - end;
            }

            // Skip to the next object
            scan += sizeof(header_t) + nref_size;
            end = scan;
            in_use += ref_n * sizeof(uintptr_t) + sizeof(header_t) + nref_size;
        } else {
            // This is a dead object remove it from the bitmap
            assert(header_is_object(header));
            bitmap_clear(scan);
            scan += sizeof(header_t) + nref_size;
        }
    }

    if (heap.end - end > max_size) {
        max_size = heap.end - end;
    }

    reclaimed += heap.end - end;

    // Check if we have fred enough memory, otherwise we grow the heap
    if ((max_size > size) && (reclaimed > in_use / 2)) {
        put_chunk(end, heap.end - end);
    } else {
        if (reclaimed < in_use / 2) {
            size = size_max(size, in_use / 2 - reclaimed);
            size = size_ceil(size, sizeof(jword_t));
        }

        gc_grow(end, size);
    }

#if JEL_PRINT
    if (opts_get_print_memory()) {
       fprintf(stderr, "GARBAGE COLLECTION in_use = %zu reclaimed = %zu\n",
               in_use, reclaimed);
    }
#endif // JEL_PRINT
} // gc_sweep()

/** Allocates a chunk of memory for holding C objects, throws an
 * exception upon failure, the returned memory has already been zeroed.
 *
 * If the allocation size is zero the function will return a NULL pointer.
 * The allocation size is rounded to a multiple of 4 or 8 bytes depending on
 * the target
 *
 * \param size The size in bytes of the chunk to be allocated
 * \returns A pointer to the newly allocated memory */

void *gc_malloc(size_t size)
{
    uintptr_t ptr;
    header_t *header;

    if (size == 0) {
        return NULL;
    }

    size = size_ceil(size, sizeof(jword_t)) + sizeof(header_t);
    tm_lock();
    ptr = gc_alloc(size);
    header = (header_t *) ptr;
    *header = header_create_c(size - sizeof(header_t));

#if JEL_PRINT
    if (opts_get_print_memory()) {
       fprintf(stderr, "MALLOC PTR: %p SIZE: %zu bytes\n",
               (void *) (ptr + sizeof(jword_t)), size - sizeof(header_t));
    }
#endif // JEL_PRINT

    tm_unlock();

    return (void *) (ptr + sizeof(header_t));
} // gc_malloc()

/** Allocates a chunk of permanent memory for holding C objects, throws an
 * exception upon failure, the returned memory has already been zeroed.
 *
 * The memory allocated with this function cannot be fred with gc_free()
 *
 * \param size The size in bytes of the chunk to be allocated
 * \returns A pointer to the newly allocated memory */

void *gc_palloc(size_t size)
{
    uintptr_t ptr = 0;

    if (size == 0) {
        return NULL;
    }

    size = size_ceil(size, sizeof(jword_t));
    tm_lock();

    if (heap.perm - size >= heap.end) {
        heap.perm -= size;
        ptr = heap.perm;
        memset((jword_t *) ptr, 0, size);
    }

    tm_unlock();

    // Fallback to a normal allocation if we're out of permanent space
    if (ptr == 0) {
        ptr = (uintptr_t) gc_malloc(size);
    }

#if JEL_PRINT
    if (opts_get_print_memory()) {
       fprintf(stderr, "PALLOC PTR: %p SIZE: %zu bytes\n", (void *) ptr, size);
    }
#endif // JEL_PRINT

    return (void *) ptr;
} // gc_palloc()

/** Frees a previously allocated C object.
 *
 * If \a ptr is NULL no action is taken
 *
 * \param ptr A pointer to the object to be fred */

void gc_free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    assert(((uintptr_t) ptr >= heap.start) && ((uintptr_t) ptr < heap.perm));

    tm_lock();
    header_t *header = (header_t *) ((uintptr_t) ptr - sizeof(header_t));

#ifdef JEL_PRINT
    if (opts_get_print_memory()) {
       fprintf(stderr, "FREE PTR: %p SIZE = %zu bytes\n",
               ptr, header_get_size(header));
    }
#endif // JEL_VERBOSE_GC

    put_chunk((uintptr_t) header, header_get_size(header) + sizeof(header_t));
    tm_unlock();
} // gc_free()

/** Purges the weak reference list by cleaning all the weak references
 * which point to weakly reachable (or non-reachable) objects and removing all
 * the non-reachable weak references */

static void gc_purge_weakref_list( void )
{
    java_lang_ref_WeakReference_t *curr, *prev, list;
    header_t *referent;

    prev = &list;
    prev->next = NULL;
    curr = heap.weakref_list;

    while (curr != NULL) {
        if (header_is_marked(&(curr->header))) {
            /*
             * This weak reference is marked, if the referent is marked too
             * we can leave it alone, otherwise the referent is only weakly
             * reacheable so we have to clear the weak reference pointing to it
             */
            referent = (header_t *) curr->referent;

            if (!header_is_marked(referent)) {
                curr->referent = JNULL;
            }

            prev = curr;
        } else {
            // This reference is not marked, remove it from the queue
            prev->next = curr->next;
        }

        curr = curr->next;
    }

    heap.weakref_list = list.next;
} // heap_purge_weakref_list()

/** Set an entry in the bitmap to 1
 * \param ptr A pointer in the garbage collected heap */

static inline void bitmap_set(uintptr_t ptr)
{
    uintptr_t offset = (ptr - heap.start) / sizeof(jword_t);

    heap.bitmap[offset >> 3] |= 1 << (offset & 0x7);
} // bitmap_set()

/** Clear an entry in the bitmap
 * \param ptr A pointer in the garbage collected heap */

static inline void bitmap_clear(uintptr_t ptr)
{
    uintptr_t offset = (ptr - heap.start) / sizeof(jword_t);

    heap.bitmap[offset >> 3] &= ~(1 << (offset & 0x7));
} // bitmap_clear()

/** Read an entry of the bitmap
 * \param ptr A pointer in the garbage collected heap */

static inline bool bitmap_get(uintptr_t ptr)
{
    uintptr_t offset = (ptr - heap.start) / sizeof(jword_t);

    return (heap.bitmap[offset >> 3] >> (offset & 0x7)) & 1;
} // bitmap_get()

/** Pulls a chunk from the bin
 * \param size The minimum size (in words) requested
 * \returns A chunk if one large enough is avaible, otherwise NULL */

static uintptr_t get_chunk(size_t size)
{
    small_chunk_t *schunk;
    large_chunk_t *lchunk, *curr, *prev;
    uintptr_t res;
    uint32_t id;

    if (size <= BIN_MAX_SIZE) {
        // Try finding an exact match
        id = (size / sizeof(jword_t)) - 1;

        while (id < BIN_ENTRIES) {
            if (heap.bin[id] != NULL) {
                schunk = heap.bin[id];
                heap.bin[id] = schunk->next;
                res = (uintptr_t) schunk;
                put_chunk(res + size, (id + 1) * sizeof(jword_t) - size);
                return res;
            } else {
                id++;
            }
        }

        // Try finding a larger chunk
        if (heap.large_bin != NULL) {
            // We found a larger block, chop it and return the relevant part
            lchunk = heap.large_bin;
            heap.large_bin = lchunk->next;
            res = (uintptr_t) lchunk;
            put_chunk(res + size, lchunk->size - size);
            return res;
        }
    } else {
        // Find an appropriate block in the large chunk list
        prev = NULL;
        curr = heap.large_bin;

        while (curr != NULL) {
            if (curr->size >= size) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    heap.large_bin = curr->next;
                }

                res = (uintptr_t) curr;
                put_chunk(res + size, curr->size - size);

                return res;
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    return 0;
} // get_chunk()

/** Put a free memory chunk in the bin
 * \param chunk A pointer to the memory chunk to be put in the bin
 * \param size The size of the chunk, can be zero in which case the function
 * won't do anything */

static void put_chunk(uintptr_t chunk, size_t size)
{
    uint32_t id;
    small_chunk_t *schunk;
    large_chunk_t *lchunk;

    assert(size % sizeof(jword_t) == 0);

    if (size == 0) {
        return;
    } else if (size <= BIN_MAX_SIZE) {
        id = (size / sizeof(jword_t)) - 1;
        schunk = (small_chunk_t *) chunk;
        schunk->next = heap.bin[id];
        heap.bin[id] = schunk;
    } else {
        lchunk = (large_chunk_t *) chunk;
        lchunk->next = heap.large_bin;
        lchunk->size = size;
        heap.large_bin = lchunk;
    }
} // put_chunk()

#ifndef NDEBUG

/** Prints information on the free chunk's bin, used for debug purposes */

void print_bin( void )
{
    large_chunk_t *lchunk;
    small_chunk_t *schunk;

    fprintf(stderr, "print_bin()\n");

    for (size_t i = 0, j; i < BIN_ENTRIES; i++) {
        schunk = heap.bin[i];
        j = 0;

        while (schunk != NULL) {
            j++;
            schunk = schunk->next;
        }

        fprintf(stderr, "heap->bin[size = %zu] = %zu\n",
                (i + 1) * sizeof(jword_t), j);
    }

    fprintf(stderr, "heap->large_bin = \n");
    lchunk = heap.large_bin;

    while (lchunk != NULL) {
        fprintf(stderr, "\tsize = %zu\n", lchunk->size);
        lchunk = lchunk->next;
    }

   fprintf(stderr, "\n");
} // print_bin()

#endif // !NDEBUG
