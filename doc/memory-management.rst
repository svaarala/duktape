=================
Memory management
=================

Overview
========

Duktape memory management is based on the following basic concepts:

* **Allocation functions**.
  The user provides a set of functions for allocating, reallocating, and
  freeing blocks of memory.  These "raw" functions can be used directly, but
  the implementation also provides variants which behave the same as the raw
  functions externally but force a garbage collection if an allocation
  attempt fails due to out of memory.  Both of these variants are used
  internally, and can also be used by external code.

* **Heap element tracking**.
  Actual memory management happens on the heap level.  Heap elements
  are tracked after being allocated, which allows unreachable elements
  to be freed by reference counting or mark-and-sweep garbage collection.
  Freeing a heap causes all related allocations to be freed, regardless of
  their reference count or reachability.

* **Reference counting and mark-and-sweep**.
  These algorithms are used to detect which heap elements can be
  freed.  A finalizer method may be executed when an element is
  about to be freed by reference counting or mark-and-sweep.

This document covers the memory management related aspects of the
implementation:

* The raw allocation functions and their behavior

* The heap memory layout (for "tracked" allocations)

* Details of the reference counting algorithm

* Details of the mark-and-sweep algorithm

* Implementation notes, such as how to manage reference counting
  correctly, how code must be structured to work correctly with
  potential ``longjmp()``\ s, etc

Duktape supports three basic models for memory management; one of these
is selected during build:

#. Reference counting and mark-and-sweep, has reclamation for reference loops

#. Reference counting alone, has no reclamation for reference loops

#. Mark-and-sweep alone, has reclamation for reference loops but memory
   usage fluctuates considerably between mark-and-sweep collections

At a high level, the implementation code must ensure that reference counts
and heap element reachability are consistently and correctly updated where
reference relationships are changed.  In particular, reachability and
reference counts must be correct whenever an operation which may cause a
``longjmp()`` or a garbage collection is performed.  This is very tricky in
practice.  There is a "GC torture" compilation option to shake out memory
management bugs.

Some terminology
================

Heap element
  The term "heap-allocated element" or "heap element" is used to refer broadly
  to all memory allocations which are automatically tracked.  The term
  "heap-allocated object" or "heap object" is not used because it is easy to
  confuse with other notions of an "object".  In particular, all ECMAScript
  objects are heap elements, but there are other heap element types too.
  Heap-allocated elements subject to memory management are:

  * ``duk_hstring``

  * ``duk_hobject`` and its subtypes

  * ``duk_hbuffer``

  Only ``duk_hobject`` contains further internal references to other heap
  elements.  These references are kept in the object property table and the
  object internal prototype pointer.  Currently only ``duk_hobject`` or its
  subtypes may have a finalizer.

  Heap elements have a **stable pointer** which means that the (main) heap
  element is not relocated during its lifetime.  Auxiliary allocations
  referenced by the heap element (such as an object property table) can be
  reallocated/relocated.

Reference
  A pointer from a source heap element to a target heap element.  The
  reference count of the target heap element must be incremented when
  a reference is created and decremented when the reference is removed.
  Only ``duk_hobject`` heap elements currently contain references,
  either through object properties (keys and values) or the object
  internal prototype reference.

Borrowed reference
  A pointer from a source heap element to a target heap element which
  is not reflected in the target element's reference count.  Borrowed
  references can be used when an actual reference is guaranteed to
  exist somewhere while the borrowed reference is in use.  If this cannot
  be guaranteed, the resulting bugs will be very difficult to diagnose.

Weak reference
  A pointer to a target heap element which is not reflected in the target
  element's reference count.  A weak reference can exist even when no other
  references to the target exists, and does not prevent collection of the
  target.  However, if the target is collected, the weak reference must be
  deleted to avoid dangling pointers.

  Currently there is no user visible support for weak references as such.
  Weak references would be useful for e.g. cache data structures.  However,
  there are specialized internal weak references which need to be taken into
  account.  For instance, there is a "string access cache" which optimizes
  access to individual characters of strings.  This cache weakly references
  heap strings and must be updated when strings are collected.

Finalizer
  Objects (``duk_hobject`` and its subtypes) stored in the heap may have a
  finalizer, which is called when the object is about to be freed.  This
  allows user code to e.g. free native resources related to the object.
  A finalizer could, for instance, close a native socket or free memory
  allocated outside Duktape tracking.  Finalizers are not required or
  supported by the E5 standard.  Finalizers require a separate implementation
  mechanism for reference counting and mark-and-sweep; these two
  implementations need to coexist peacefully.

Allocation functions
====================

Raw functions
-------------

When creating an ``duk_heap``, three memory allocation related functions
are associated with the heap: ``alloc``, ``realloc``, and ``free``.
The related typedefs are::

  typedef void *(*duk_alloc_function) (void *udata, size_t size);
  typedef void *(*duk_realloc_function) (void *udata, void *ptr, size_t size);
  typedef void (*duk_free_function) (void *udata, void *ptr);

The semantics of these functions are essentially the same as their ANSI C
equivalents.  In particular:

* The return value for a zero-sized ``alloc`` and ``realloc`` may be
  ``NULL`` or some non-``NULL``, unique pointer value.  Whatever the return
  value is, it must be accepted by ``realloc`` and ``free``.

* ``realloc(NULL, size)`` is equivalent to ``malloc(size)``.

* ``realloc(ptr, 0)`` is equivalent to ``free(ptr)`` (assuming ``ptr``
  is not ``NULL``), and must either return ``NULL`` or some non-``NULL``
  unique pointer value accepted by ``realloc`` and ``free``.

* ``free(NULL)`` is a no-op.

The default implementations map directly to the corresponding ANSI C
functions (``udata`` is ignored).  If the platform allocator does not
fulfill the ANSI C requirements, replacement functions must be provided
by user code.

The memory returned by the allocation and reallocation functions must be
properly aligned to support Duktape data structures.  In particular, it
must be possible, as far as alignment is concerned, to store a ``double``
or an ``int64_t`` at the start of the returned memory.  This does always
imply alignment by 8: on x86 there is usually no alignment requirement at
all, while on ARM alignment by 4 usually suffices.  Even when not strictly
required, some level of alignment is often good for performance.
(Technically these alignment requirements differ from the ANSI C
requirements, especially when the allocation size is smaller than 8 bytes,
but these cases don't really matter with Duktape.)

Internal macros
---------------

The following internal macros use the raw allocation functions and do
not trigger garbage collection or any other side effects:

* ``DUK_ALLOC_RAW``

* ``DUK_REALLOC_RAW``

* ``DUK_FREE_RAW``

The natural downside of using these functions is that an allocation or
reallocation may fail even if some memory would be available after a
garbage collection.

The following internal macros may trigger a garbage collection (even
when not strictly out of memory):

* ``DUK_ALLOC``

* ``DUK_ALLOC_ZEROED``

* ``DUK_REALLOC``

* ``DUK_REALLOC_INDIRECT``

* ``DUK_FREE``

Triggering a garbage collection has a wide set of possible side effects.
If a finalizer is executed, arbitrary ECMAScript or even native code
may run.  Garbage collection side effects are discussed in detail in a
separate section below.

Memory reallocation (e.g. ``DUK_REALLOC()``) has a particularly nasty
interaction with garbage collection.  Mark-and-sweep side effects may
potentially change the original pointer being reallocated.  This must
be taken into account when retrying the reallocation operation.
There is a separate macro for these cases, ``DUK_REALLOC_INDIRECT()``,
see detailed discussion below.

Note that even if user code is allocating buffers to be used outside
of automatic memory management, the garbage collection triggering
variants are usually preferable because memory pressure is then communicated
properly between user allocations and Duktape managed allocations.  Use the
raw variants only when invoking a garbage collection would be detrimental;
this is rarely the case, especially for user code.

Because a (non-raw) memory allocation or reallocation may invoke garbage
collection, any function or macro call which allocates memory directly or
indirectly may have such side effects.  Any direct or indirect checked
memory allocations may also throw an out-of-memory error (leading ultimately
to a ``longjmp()``).

Public API
----------

The heap-associated memory allocation functions can also be called by user
code through the exposed API.  This is useful for e.g. C functions which
need temporary buffers.  Note, however, that such allocations are, of course,
not automatically managed so care must be taken to avoid memory leaks caused
by e.g. errors (``longjmp()``\ s) in user code and the functions it calls.

The raw API calls behave essentially as direct wrappers for the memory
management functions registered into the heap.  The API calls providing
garbage collection are unchecked and simply return a ``NULL`` on errors.
A ``NULL`` is only returned when an allocation request cannot be satisfied
even after garbage collection.  Expect in fatal errors, the API calls are
guaranteed to return and will hide e.g. errors thrown by finalizer functions.

Another alternative, perhaps more robust, is to push a ``buffer`` object into
the value stack; the buffer will be automatically memory managed.  Also, if
the buffer is a fixed size one, a stable pointer can be obtained after
allocation and passed anywhere in user code without further checks.  The
buffer is viable until it is no longer reachable (i.e. is pushed off the value
stack and is not stored in any reachable object or variable).

The public API is::

  /* no garbage collection */
  void *duk_alloc_raw(duk_context *ctx, size_t size);
  void duk_free_raw(duk_context *ctx, void *ptr);
  void *duk_realloc_raw(duk_context *ctx, void *ptr, size_t size);

  /* may cause garbage collection, doesn't longjmp() */
  void *duk_alloc(duk_context *ctx, size_t size);
  void duk_free(duk_context *ctx, void *ptr);
  void *duk_realloc(duk_context *ctx, void *ptr, size_t size);

DUK_REALLOC() issues with mark-and-sweep; DUK_REALLOC_INDIRECT()
----------------------------------------------------------------

There is a subtle gotcha when using DUK_REALLOC().  If the initial attempt
to reallocate fails, the DUK_REALLOC() implementation will trigger a
mark-and-sweep and then retry the reallocation.  This does not work if the
mark-and-sweep may have an effect on the original pointer being reallocated.
In that case, the second attempt to reallocate will use an invalid "original
pointer"!

A more conrete example of reallocating a valuestack (``thr->valstack``):

* Calling code calls ``DUK_REALLOC(thr, thr->valstack, new_size)``.
  Assume that the value of ``thr->valstack`` is ``P1`` at this point.

* The ``DUK_REALLOC()`` implementation attempts to use the raw realloc,
  giving ``P1`` as its pointer argument.  This attempt fails.

* A mark-and-sweep is triggered.  The mark-and-sweep invokes a number
  of finalizer methods, which cause **the same valstack** to be resized.
  This resize succeeds, and ``thr->valstack`` pointer is updated to ``P2``.

* The ``DUK_REALLOC()`` implementation retries the raw realloc, again
  giving ``P1`` as its pointer argument.  Here, ``P1`` is a garbage
  pointer and the realloc call has undefined behavior.

The correct pointer for the second realloc would be ``P2``.  However,
the helper behind the macro doesn't know where the pointer came from.

A naive approach is to use an indirect realloc function which gets a
pointer to the storage location of the pointer being reallocated
(e.g. ``(void **) &thr->valstack``).  The realloc implementation then
re-lookups the current pointer right before every reallocation, which
works correctly even if the pointer has changed by garbage collection.
Note that heap headers have stable pointers so that the header which
contains the pointer is never relocated so the location of the pointer
itself never changes.  Even so, this approach suffers from C type-punning
and strict aliasing issues.  Such issues could be fixed by changing all
the base pointers to a union but this would be very invasive, of course.

The current solution is to use an indirect realloc function which gets
a callback function with a userdata pointer as its argument.  The
callback is used to request for the current value of the pointer being
reallocated.  This bloats code to be strict aliasing compatible, but
is the most portable way.

Implications:

* DUK_REALLOC_RAW() can be used reliably for anything, but is not guaranteed
  to succeed (even if memory would be available after garbage collection).

* DUK_REALLOC() can be used reliably for pointers which are guaranteed not to
  be affected by mark-and-sweep -- considering that mark-and-sweep runs
  arbitrary code, including even arbitrary native function, e.g. as part of
  object finalization.

* DUK_REALLOC_INDIRECT() (or DUK_ALLOC() + DUK_FREE()) should be used for
  pointers which are not stable across a mark-and-sweep.  The storage
  location of such pointers must be stable, e.g. reside in the meain
  allocation of a heap object.

Heap structure
==============

Overview
--------

All heap-allocated elements must be recorded in the ``duk_heap``, either as
part of the string table (for ``duk_hstring`` elements) or as part of the
"heap allocated" list (or temporary work queues).  This is required so that
all allocated elements can always be enumerated and freed, regardless of their
reference counts or reachability.

Heap elements which are currently in use somewhere must have a positive
reference count, and they must be reachable through the actual reachability
roots starting from the ``duk_heap`` structure.  These form the actual
reachability graph from a garbage collection point of view; any objects
tracked by the heap but not part of the reachability graph are garbage
and can be freed.  Such objects, assuming reference counts are correct,
either have a zero reference count or belong to a reference cycle.

The following figure summarizes the elements managed by a single
heap structure, with arrows indicating basic reachability or
ownership relationships::


                 All non-string heap elements reside in one of the
                 following object lists:

                 * "heap allocated"
                 * "refzero work list"
                 * "finalization work list"

                 +-------------+  h_next  +-------------+  h_next
         .------>| duk_hobject |<-------->| duk_hbuffer |<--------> ...
         |       +-------------+ (h_prev) +-------------+ (h_prev)
         |
 +==========+    (Above illustrates "heap allocated", there are
 | duk_heap |    similar lists for "refzero" and "finalization")    
 +==========+                    
    |    |
    |    |
    |    |       All duk_hstrings reside in the string table.
    |    |
    |    |       +--------+
    |    |       : string :      +-------------+
    |    +------>: intern :----->| duk_hstring |
    |    |       : table  :      +-------------+
    |    |       +--------+         ^      ^
    |    |                          |      :
    |    |       +------+           |      :
    |    +------>: strs :-----------'      :
    |    |       +------+   (built-in      :
    |    |                   strings)      :
    |    |       +--------+                :
    |    `------>: string :                :
    |            : access :- - - - - - - - '
    |            : cache  :  (weak refs)
    |            +--------+
    |
    |
    |    (reachability graph roots)
    |
    |     +-------------+
    +---> | duk_hthread |     heap_thread: internal thread, also used
    |     +-------------+                  for (some) finalization
    |
    |     +-------------+
    `---> | duk_hthread |     curr_thread: currently running thread
          +-------------+
           |
           |
           |    +----------+      +-------------+
           +--->: builtins :----->| duk_hobject |
           |    +----------+      +-------------+
           |                            |
           |                            +--> object properties
           |                            |
           |                            `--> (type specific)
           +-->  object properties
           |
           +-->  value stack
           |
           +-->  call stack -->  duk_activations (linked list)
           |                       |
           |                       `--> duk_catchers (linked list)
           |
           |                          +-------------+
           `-->  resumer -----------> | duk_hthread |
              (another duk_hthread    +-------------+
               or NULL)

Notation::

   +=====+          +-----+          +-----+
   | xxx |          | xxx |          : xxx :
   +=====+          +-----+          +-----+    

   backbone       heap element      auxiliary

(Many details are omitted from the figure; for instance, there are
back pointers and duplicate pointers for faster access which are not
illustrated at all.)

The primary memory management models relate to the figure as follows
(omitting details such as recursion depth limits, finalization, interaction
between reference counting and mark-and-sweep, etc):

* Reference counting works by inspecting a reference count field which
  is a part of the header of every heap allocated element (including
  strings).  Whenever a reference is removed, the reference count of
  the target is decreased, and if the reference count becomes zero, the
  target object can be freed.  Before freeing, any outgoing references
  from object must be iterated and the reference count of the target
  heap elements needs to be decreased, possibly setting off a cascade
  of further "refzero" situations.  Note that incoming references don't
  need to be considered: if reference counts are correct and the reference
  count of the current object is zero, there cannot be any live incoming
  references.

* Mark-and-sweep works by traversing the reachability graph originating
  from the ``duk_heap`` structure referenced, marking all reachable objects,
  and then walking the comprehensive "heap allocated" list to see which
  objects are unreachable and can be freed.

The only "backbone" element which is not itself a heap element is the
``duk_heap`` object.  Heap elements include both internal and external
objects which may reference each other in an arbitrary conceptual graph.
Finally, auxiliary elements are either struct members or additional
allocations "owned" by the main heap element types.  They are an integral
part of their parent element and cannot be referenced directly by other
elements.  They are freed when their parent is freed.

The primary roots for reachability are the threads referenced by the heap
object.  In particular, the currently running thread is reachable, and the
thread structure maintains a pointer to the thread which resumed the current
thread (if any).  All heap element references ultimately reside in:

* Object properties

* Thread value stack

* Thread call stack (including catchers)

* Thread resumer reference

* Compiled function constant table

* Compiled function inner function table

These references form the heap-level reachability graph, and provides
the basis for mark-and-sweep collection.

There are, of course, temporary references to both heap-allocated and
non-heap-allocated memory areas in CPU registers and the stack frames
of the C call stack.  Such references must be very carefully maintained:
an abrupt completion (concretely, a ``longjmp()``) will unwind the C
stack to some catch point (concretely, a ``setjmp()``) and any such
references are lost.  Also, any unreachable heap elements may be freed
if a mark-and-sweep is triggered directly or indirectly.  See separate
discussion on error handling and memory management.

Heap elements
-------------

All heap tracked elements have a shared header structure, ``duk_heaphdr``,
defined in ``duk_heaphdr.h``.  String elements use a smaller
``duk_heaphdr_string`` header which is a prefix of ``duk_heaphdr``.
The difference between these two headers is that ``duk_heaphdr_string``
does not contain next/previous links required to maintain heap allocated
objects in a single or double linked list.  These are not needed because
strings are always kept in the heap-level string intern table, and are
thus enumerable (regardless of their reachability) through the string
intern hash table.

Heap-allocated elements are always allocated with a fixed size, and are
never reallocated (and hence never moved) during their life cycle.  This
allows all heap-allocated elements to be pointed to with *stable pointers*.
Non-fixed parts of an element are allocated separately and pointed to by
the main heap element.  Such allocations are "owned" by the heap element
and are automatically freed when the heap element is freed.  The upside of
having stable pointers is simplicity and compatibility with existing
allocators.  The downside is that memory fragmentation may become an issue
over time because there is no way to compact the heap.  The full size of
the fixed part of the heap element needs to be known at the time of
allocation.

Normally, heap elements are typed by the tagged value (``duk_tval``)
which holds the heap pointer, or if the heap element reference is in
a struct field, the field is usually already correctly typed through its
C type (e.g. a field might have the type "``duk_hcompfunc *``").
However, heap elements do have a "heap type" field as part of the
``h_flags`` field of the header; this is not normally used, but is
needed by e.g. reference counting.  As a separate issue, some heap types
(such as ``duk_hobject``) have "sub-types" with various extended memory
layouts; these are not reflected in the heap type.

The current specific heap element types are:

* ``duk_hstring`` (heap type ``DUK_HTYPE_STRING``):

  + Fixed size allocation consisting of a header with string
    data following the header.  Header only contains a 'next'
    pointer (uses ``duk_heaphdr_string``).

  + No references to other heap elements.

* ``duk_hobject`` (heap type ``DUK_HTYPE_OBJECT``):

  + Fixed size allocation consisting of a header, whose size
    depends on the object type (``duk_hobject``, ``duk_hthread``,
    ``duk_hcompfunc``, ``duk_hnatfunc``, etc).

  + The specific "sub type" and its associated struct definition
    can be determined using object flags, using the macros:

    - ``DUK_HOBJECT_IS_COMPFUNC``
    - ``DUK_HOBJECT_IS_NATFUNC``
    - ``DUK_HOBJECT_IS_THREAD``
    - (and other sub types added later)
    - If none of the above are true, the object is a plain object
      (``duk_hobject`` without any extended structure)

  + Properties are stored in a separate, dynamic allocation, and contain
    references to other heap elements.

  + For ``duk_hcompfunc``, function bytecode, constants, and
    references to inner functions are stored in a fixed ``duk_hbuffer``
    referenced by the ``duk_hcompfunc`` header.  These provide
    further references to other heap elements.

  + For ``duk_hthread`` the heap header contains references to the
    value stack, call stack, etc, which provide references to other heap
    elements.

* ``duk_hbuffer`` (heap type ``DUK_HTYPE_BUFFER``):

  + Fixed buffer (``DUK_HBUFFER_HAS_DYNAMIC()`` is false):

    - Fixed size allocation consisting of a header with buffer data
      following the header.

  + Dynamic buffer (``DUK_HBUFFER_HAS_DYNAMIC()`` is true):

    - Fixed size allocation consisting of a header with a pointer to
      the current buffer allocation following the header.

    - Buffer data is allocated separately and the buffer may be resized.
      The address of the buffer data may change during a resize.

  + No references to other heap elements.

String table
============

String interning
----------------

All strings are `interned`__ into the hash level string table: only one,
immutable copy of any particular string is ever stored at a certain
point in time.

.. __: http://en.wikipedia.org/wiki/String_interning

When a new string is constructed e.g. by string concatenation, the
string table is checked to see if the resulting string has already been
interned.  If yes, the existing string is used; if not, the string is added
to the string table.  Regardless, the string is represented by an
``duk_hstring`` pointer which is stable for the lifetime of the string.

String interning has many nice features:

* When a string is interned, precomputations can be done and stored as
  part of the string representation.  For example, a string hash can be
  precomputed and used elsewhere in e.g. hash tables.  Other precomputations
  would also be possible, e.g. numeric conversions (not currently used).

* Strings can be compared using direct pointer comparisons without comparing
  actual string data, since at any given time, a given string can only have
  one ``duk_hstring`` instance with a stable address.

* Memory is saved for strings which occur multiple times.  For instance,
  object properties of the same name are simply referenced with a string
  pointer instead of storing multiple instances of the same property name.

But, there are downsides as well:

* String manipulation is slower because any intermediate, referenceable
  results need to be interned (which implies string hashing, a lookup
  from the string table, etc).  This can be mitigated e.g. by doing string
  concatenation of multiple parts in an atomic fashion.

* For small strings which only occur once or twice in the heap, there is
  additional overhead in the interned ``duk_hstring`` heap element compared
  to simply storing the string in an object's property table, for instance.

* Using string values as "data buffers" which are continuously manipulated
  (appended or predended to, sliced, etc) is very inefficient and causes a
  lot of garbage collection churn.  Buffer objects should be used instead,
  but these are not part of the ECMAScript standard.

Memory management of strings
----------------------------

Interned strings are garbage collected normally when they are no longer needed.
They are later re-interned if they are needed again; at this point they usually
get a different pointer than before.

String table algorithm
----------------------

The string table structure is similar to the "entry part" of the
``duk_hobject`` property allocation:

* Closed hash table (probe sequences).  Probe sequences use an initial
  index based on string hash value, and a probe step looked up from a
  precomputed table of step values using a string hash value based index.

* Hash table size is rounded upwards to a prime in a precomputed
  sequence.  Hash table load factor is kept within a certain range
  by resizing whenever necessary.

* Deleted entries are explicitly marked DELETED to avoid breaking
  hash probe chains.  DELETED entries are eliminated on rehashing,
  and are counted as "used" entries before a resize to ensure there
  are always NULL entries in the string table to break probe sequences.

For more details, see:

* ``hstring-design.rst`` for discussion on the string hash algorithm.

* ``hobject-design.rst``, entry part hash algorithm, for discussion on
  the basic closed hash structure.

.. note:: This discussion should be expanded.

Reference counting
==================

Introduction
------------

For background, see:

* http://en.wikipedia.org/wiki/Reference_counting

In basic reference counting each heap object has a reference count field
which indicates how many other objects in the heap point to this object.
Whenever a new reference is created, its target object's reference count
is incremented; whenever a reference is destroyed, its target object's
reference counter is decreased.  If a reference count goes to zero when it
is decreased, the object can be freed directly.  When the object is freed,
any heap objects it refers to need to have their reference counts decremented,
which may trigger an arbitrarily long chain of objects to be freed recursively.

There are variations of reference counting where objects are not freed immediately
after their reference count goes to zero.  Objects-to-be-freed can be managed in
a work list and freed later.  However, for our purposes it is useful to free any
reference counted objects as soon as possible (otherwise we could just use the
mark-and-sweep collector).

There are also reference counting variants which handle reference loops
correctly without resorting to mark-and-sweep.  These seem to be too complex
in practice for a small interpreter.

Reference counting increases code size, decreases performance due to
reference count updates, and increases heap header size for every object.
On the other hand it minimizes variance in memory usage (compared to plain
mark-and-sweep, even an incremental one) and is very useful for small
scripts running without a pre-allocated heap.  Reference counting also reduces
the impact of having non-relocatable heap elements: memory fragmentation still
happens, but is comparable to memory fragmentation encountered by ordinary
C code.

Reference count field
---------------------

The reference count field is embedded into the ``duk_heaphdr`` structure
whose layout varies depending on the memory management model chosen for
the build.  The reference count field applies to all heap allocated elements,
including strings, so it appears in the header before the next/previous
pointers required for managing non-string heap elements.

The current struct definitions are in ``duk_heaphdr.h``.  Two structures
are defined:

* ``duk_heaphdr``: applies to all heap elements except strings.

* ``duk_heaphdr_string``: applies to strings, beginning of struct matches
  ``duk_heaphdr``.

The reference count field must have enough bits to ensure that it will never
overflow.  This is easy to satisfy by making the field as large as a data
pointer type.  Currently ``size_t`` is used which is technically incorrect
(one could for instance have a platform with maximum allocation size of
32 bits but a memory space of 64 bits).

Reference count macros
----------------------

Macros:

* ``DUK_TVAL_INCREF``

* ``DUK_TVAL_DECREF``

* ``DUK_HEAPHDR_INCREF``

* ``DUK_HEAPHDR_DECREF``

* and a bunch of heap element type specific INCREF/DECREF macros and
  helpers, defined in ``heaphdr.h``

Notes on macro semantics:

* The macros are optimized for performance and don't tolerate a ``NULL``
  pointer by default.  There are ``_ALLOWNULL`` variants for cases where
  NULLs may actually occur.

* An ``INCREF`` is guaranteed not to have any side effects.

* A ``DECREF`` may have a wide variety of side effects.

  + ``DECREF`` may free the target object and an arbitrary number of other
    objects whose reference count drops to zero as a result.

  + If a finalizer is invoked, arbitrary C or ECMAScript code is
    executed which may have essentially arbitrary side effects,
    including triggering the mark-and-sweep garbage collector.

  + The mark-and-sweep garbage collector may also be voluntarily
    invoked at the end of "refzero" handling.

  + Any ``duk_tval`` pointers pointing to dynamic structures (like
    a value stack) may be invalidated; heap element pointers are not
    affected because they are stable.

See discussion on "side effects" below for more particulars on the
implementation impact.

Updating reference counts
-------------------------

Updating reference counts is a bit tricky.  Some important rules:

* Whenever a ``longjmp()`` or garbage collection may occur, reachability
  and reference counts must be correct.

* If a reference count drops to zero, even temporarily, the target is
  *immediately* freed.  If this is not desired, ``INCREF``/``DECREF``
  order may need to be changed.

* A ``DECREF`` call may invalidate *any* ``duk_tval`` pointers to
  resizable locations, such as the value stack.  It may also invalidate
  indices to object property structures if a property allocation is
  resized.  So, ``DECREF`` must be called with utmost care.

Note that it is *not enough* to artificially increase a target's reference
count to prevent the object from being freed, at least when mark-and-sweep
collection is also enabled.  Mark-and-sweep may be triggered very easily,
and *will* free an unreachable object, regardless of its reference count,
unless specific measures are taken to avoid it.  In fact, mark-and-sweep
*must* collect unreachable objects with a non-zero reference count, to deal
with reference loops which cannot be collected using reference counting
alone.  Even if mark-and-sweep issues were avoided (perhaps with a flag
preventing collection), if a reference count is artificially increased
without there being a corresponding, actual heap-based reference to the
target, there must be a guarantee that the reference count is also decreased
later.  This would require a ``setjmp()`` catchpoint.

Specific considerations:

* ``DECREF`` + ``INCREF`` on the same target object is dangerous.  If the
  refcount drops to zero between the calls, the object is freed.  It's
  usually preferable to do ``INCREF`` + ``DECREF`` instead to avoid this
  potential issue.

The INCREF algorithm
--------------------

The ``INCREF`` algorithm is very simple:

1. If the target reference is ``NULL`` or the target is not a heap element,
   return.

2. Increase the target's reference count by one.

The practical implementation depends on whether ``INCREF`` is used on a
tagged value pointer or a heap element pointer.

The DECREF algorithm
--------------------

The ``DECREF`` algorithm is a bit more complicated:

1. If the target reference is ``NULL`` or the target is not a heap element,
   return.

2. Decrease the target's reference count by one.

3. If the reference count dropped to zero:

   a. If mark-and-sweep is currently running, ignore and return.
      (Note: mark-and-sweep is expected to perform a full reachability
      analysis and have correct reference counts at the end of the
      mark-and-sweep algorithm.)

   b. If the target is a string, remove the string from the string table,
      remove any weak references (e.g. from string access cache), and
      then free the string structure.

   c. If the target is a buffer:

      1. Remove the buffer from the "heap allocated" list, free any related
         allocations (if the buffer is dynamic, the separately allocated
         buffer), and then free the buffer structure.

   d. Else the target is an object:

      1. This case is relatively complicated, see code for details:

         * If the object doesn't have a finalizer, queue it to "refzero list".
           If no-one is processing refzero_list now, process it until it
           becomes empty; new objects may be queued as previous ones are
           refcount finalized and freed.  When the list is empty, run any
           pending finalizers queued up during the process.  If a previous
           call is already processing the list, just queue the object and
           finish.

         * If the object has a finalizer, queue it to finalize_list.  If
           no-one is processing the refzero_list or finalize_list, process
           the finalize_list directly.  Otherwise just queue the object and
           finish.

The REFZERO algorithm
---------------------

The ``DECREF`` algorithm ensures that only one instance of the "refzero"
algorithm may run at any given time.  The "refzero" work list model is used
to avoid an unbounded C call stack depth caused by a cascade of reference
counts which drop to zero.

See code for details, also see ``doc/side-effects.rst``.

Background on the refzero algorithm, limiting C recursion depth
---------------------------------------------------------------

When a reference count drops to zero, the heap element will be freed.  If the
heap element contains references (like an ECMAScript object does), all target
elements need to be ``DECREF``'d before the element is freed.  These ``DECREF``
calls may cause the reference count of further elements to drop to zero; this
"cascade" of zero reference counts may be arbitrarily long.  Since we need to
live with limited and sometimes very small C stacks in some embedded
environments (some environments may have less than 64 kilobytes of usable
stack), the reference count zero handling must have a limited C recursion
level to work reliably.

This is currently handled by using a "work list" model.  Heap elements whose
reference count has dropped to zero are placed in a "to be freed" work list
(see ``duk_heap`` structure, ``refzero_list`` member in ``duk_heap.h``).  The
list is then freed using a loop which frees one element at a time until the
list is free.  New elements may be added to the list while it is being iterated.
The C recursion level is fixed.

The ``h_prev``/``h_next`` fields of the ``duk_heaphdr`` structure, normally
used for the "heap allocated" list, are used for the "refzero" work list.
Because ``duk_hstring``\ s do not have embedded references so they are freed
directly when their reference count drops to zero.

*Finalization* of an object whose refcount becomes zero is very useful for
e.g. freeing any native resources or handles associated with an object.
For instance, socket or file handles can be closed when the object is being
freed.  The finalizer is an internal method associated with an ``duk_hobject``
which is called just before the object is freed either by reference counting
or by the mark-and-sweep collector.  The finalizer gets a reference to the
object in question, and may "rescue" the reference.

There are many side effects to consider, see ``doc/side-effects.rst``.

Mark-and-sweep
==============

Introduction
------------

For background, see:

* http://en.wikipedia.org/wiki/Garbage_collection_(computer_science)

The variant used is a "stop the world" mark-and-sweep collector, which
is used instead of an incremental one for simplicity and small footprint.
When combined with reference counting, the mark-and-sweep collector is
only required for handling reference cycles anyway, so the particular
variant is not that important.  A definite downside of a "stop the world"
collector is that it introduces an annoying pause in application behavior
which is otherwise avoided by reference counting.

The mark-and-sweep algorithm used has support for:

* object finalization (requires two collector passes)

* object compaction (in emergency mode)

* string table resizing

An "emergency mode" is provided for situations where allocation fails
repeatedly, even after a few ordinary mark-and-sweep attempts.  In
emergency mode the collector tries to find memory even by expensive
means (such as forcibly compacting object property allocations).

Control flags are also provided to limit side effects of mark-and-sweep,
which is required to implement a few critical algorithms: resizing the
string table, and resizing object property allocation.  During these
operations mark-and-sweep must avoid interfering with the object being
resized.

Mark-and-sweep flags
--------------------

Mark-and-sweep control flags are defined in ``duk_heap.h``, e.g.:

* ``DUK_MS_FLAG_EMERGENCY``

* ``DUK_MS_FLAG_NO_FINALIZERS``

* ``DUK_MS_FLAG_NO_OBJECT_COMPACTION``
  
In addition to the explicitly requested flags, the bit mask in
``ms_base_flags`` in ``duk_heap`` is bitwise ORed into the requested flags
to form effective flags.  The flags added to the "base flags" control
restrictions on mark-and-sweep side effects, and are used for certain
critical sections.

To protect against such side effects, the critical algorithms:

* Store the original value of ``heap->ms_base_flags``

* Set the suitable restriction flags into ``heap->ms_base_flags``

* Attempt the allocation / reallocation operation, *without throwing errors*

* Restore the ``heap->ms_base_flags`` to its previous value

* Examine the allocation result and act accordingly

It is important not to throw an error without restoring the base flags field.
See ``duk_heap.h`` for the flag details.

Heap header flags
-----------------

The following flags in the heap element header are used for controlling
mark-and-sweep:

* ``DUK_HEAPHDR_FLAG_REACHABLE``:
  element is reachable through the reachability graph.

* ``DUK_HEAPHDR_FLAG_TEMPROOT``:
  element's reachability has been marked, but its children have not been
  processed; this is required to limit the C recursion level.

* ``DUK_HEAPHDR_FLAG_FINALIZABLE``:
  element is not reachable after the first marking pass (see algorithm),
  has a finalizer, and the finalizer has not been called in the previous
  mark-and-sweep round; object will be moved to the finalization work
  list and will be considered (temporarily) a reachability root.

* ``DUK_HEAPHDR_FLAG_FINALIZED``:
  element's finalizer has been executed, and if still unreachable, object
  can be collected.  The finalizer will not be called again until this
  flag is cleared; this prevents accidental re-entry of the finalizer
  until the object is explicitly rescued and this flag cleared.

These are referred to as ``REACHABLE``, ``TEMPROOT``, ``FINALIZABLE``,
and ``FINALIZED`` below for better readability.  All the flags are clear
when a heap element is first allocated.  Explicit "clearing passes" are
avoided by careful handling of the flags so that the flags are always in
a known state when mark-and-sweep begins and ends.

Basic algorithm
---------------

The mark-and-sweep algorithm is triggered by a failed memory allocation
either in "normal" mode or "emergency" mode.  Emergency mode is used if
a normal mark-and-sweep pass did not resolve the allocation failure; the
emergency mode is a more aggressive attempt to free memory.  Mark-and-sweep
is controlled by a set of flags.  The effective flags set is a bitwise OR
of explicit flags and "base flags" stored in ``heap->ms_base_flags``.
The "base flags" essentially prohibit specific garbage collection operations
when a certain critical code section is active.

The mark-and-sweep algorithm is as follows:

1. The ``REACHABLE`` and ``TEMPROOT`` flags of all heap elements are
   assumed to be cleared at this point.

   * Note: this is the case for all elements regardless of whether they
     reside in the string table, the "heap allocated" list, the "refzero"
     work list, or anywhere else.

2. **Mark phase**.
   The reachability graph is traversed recursively, and the ``REACHABLE``
   flags is set for all reachable elements.  This is complicated by the
   necessity to impose a limit on maximum C recursion depth:

   a. At the beginning the heap level flag
      ``DUK_HEAP_FLAG_MARKANDSWEEP_RECLIMIT_REACHED`` is asserted to be
      cleared.

   b. The reachability graph of the heap is traversed with a depth-first
      algorithm:

      1. Marking starts from the reachability roots:

         * the heap structure itself (including the current thread, its
           resuming thread, etc)

         * the "refzero_list" for reference counting

      2. If the reachability traversal hits the C recursion limit
         (``mark_and_sweep_recursion_limit`` member of the heap) for
         some heap element ``E``:

         a. The ``DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED`` flag is set.

         b. The reachability status of ``E`` is updated, but its internal
            references are not processed (to avoid further recursion).

         c. The ``TEMPROOT`` flag is set for ``E``, indicating that it
            should be processed later.

      3. Unreachable objects which need finalization (but whose finalizers
         haven't been executed in the last round) are marked FINALIZABLE
         and are marked as reachable with the normal recursive marking
         algorithm.

      4. The algorithm of step 2 (handling ``TEMPROOT`` markings) is
         repeated to ensure reachability graph has been fully processed
         (elements are marked reachable and TEMPROOT flags are set),
         also for the objects just marked FINALIZABLE.

   c. While the ``DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED`` flag is
      set for the heap:

      1. Clear the ``DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED`` flag
         of the heap.

      2. Scan all elements in the "heap allocated" or "refzero work list"
         (note that "refzero work list" *must* be included here but not
         in the sweep phase).  For each element with the ``TEMPROOT`` flag set:

         a. Clear the ``TEMPROOT`` flag.

         b. Process the internal references of the element recursively,
            imposing a similar recursion limit as before (i.e. setting
            the ``DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED`` flag etc).

3. **Sweep phase 1 (refcount adjustments)**.
   Inspect all heap elements in the "heap allocated" list (string table
   doesn't need to be considered as strings have no internal references):

   a. If the heap element would be freed in sweep phase 2 (i.e., element
      is not reachable, and has no finalizer which needs to be run):

      1. Decrease reference counts of heap elements the element points to,
         but don't execute "refzero" queueing or the "refzero" algorithm.
         Any elements whose refcount drops to zero will be dealt with by
         mark-and-sweep and objects in the refzero list are handled by
         reference counting.

4. **Sweep phase 2 (actual freeing)**.
   Inspect all heap elements in the "heap allocated" list and the string
   table (note that objects in the "refzero" work list are NOT processed
   and thus never freed here):

   a. If the heap element is ``REACHABLE``:

      1. If ``FINALIZED`` is set, the object has been rescued by the finalizer.
         This requires no action as such, but can be debug logged.

      2. Clear ``REACHABLE`` and ``FINALIZED`` flags.

      3. Continue with next heap element.

   b. Else the heap element is not reachable, and:

      1. If the heap element is an ``duk_hobject`` (its heap type is
         ``DUK_HTYPE_OBJECT``) and the object has a finalizer (i.e. it
         has the internal property ``_Finalizer``), and the ``FINALIZED``
         flag is not set:

         a. Move the heap element from "heap allocated" to "to be finalized"
            work list.

         b. Continue with next heap element.

      2. Free the element and any of its "auxiliary allocations".

      3. Continue with next heap element.

5. For every heap element in the "refzero" work list:

   a. Clear the element's ``REACHABLE`` flag.
      (See notes below why this seemingly unnecessary step is in fact necessary.)

6. If doing an emergency mark-and-sweep and object compaction is not
   explicitly prohibited by heap flags:

   a. Compact the object's property allocation in the hopes of freeing
      memory for the emergency.

7. If string table resize is not explicitly prohibited by heap flags:

   a. Compact and rehash the string table.  This can be controlled by build
      flags as it may not be appropriate in all environments.

8. Finish.

   a. All ``TEMPROOT`` and ``REACHABLE`` flags are clear at this point.

   b. All "heap allocated" elements either (a) are reachable and have a
      non-zero reference count, or (b) were finalized and their reachability
      status is unknown.

   c. The "to be finalized" list is empty.

   d. No object in the "refzero" work list has been freed.

9. Execute pending finalizers unless finalizer execution is prevented or an
   earlier call site is already finalizing objects.  Finalizer execution is
   outside of mark-and-sweep prevention lock, so mark-and-sweep may run while
   finalizers are being processed.  However, rescue decisions are postponed
   until the finalize_list is empty to avoid incorrect rescue decisions caused
   by finalize_list being treated as a reachability root.

Notes:

* Elements on the refzero list are considered reachability roots, as we need
  to preserve both the object itself (which happens automatically because we
  don't sweep the refzero_list) and its children.  (This is no longer relevant
  because refzero_list is always NULL when mark-and-sweep runs.)

* Elements marked FINALIZABLE are considered reachability roots to ensure
  that their children (e.g. property values) are not swept during the
  sweep phase.  This would obviously be problematic for running the finalizer,
  regardless of whether the object would be rescued or not.

* While mark-and-sweep is running:

  + Another mark-and-sweep cannot execute.

  + A ``DECREF`` resulting in a zero reference count is not processed at all
    (other than updating the refcount).  The object is not placed into the
    "refzero" work list, as mark-and-sweep is assumed to be a comprehensive
    pass, including running finalizers.

* Finalizers are executed after the sweep phase to ensure that finalizers
  have as much available memory as possible.  Since Duktape 2.1 mark-and-sweep
  runs outside the mark-and-sweep algorithm, and mark-and-sweep may run while
  finalizers are being processed, with the limitation that rescue decisions
  are postponed until finalize_list is empty.

* The sweep phase is divided into two separate scans: one to adjust refcounts
  and one to actually free the objects.  If these were performed in a single
  heap scan, refcount adjustments might refer to already freed heap elements
  (dangling pointers).  This may happen even without reference counting bugs
  for unreachable reference loops.

* Clearing the ``REACHABLE`` flags explicitly for objects in the "refzero"
  list is necessary:

  + The "refzero" work list is not processed at all in the sweep phase but the
    marking phase could theoretically mark objects in the "refzero" work list.
    Since the sweeping phase is the only place where ``REACHABLE`` flags are
    cleared, some object in the "refzero" work list might be left with its
    ``REACHABLE`` flag set at the end of the algorithm.  At first it might seem
    that this can never happen if reference counts are correct: all objects in
    the "refzero" work list are unreachable by definition.  However, this is not
    the case for objects with finalizers.  (As of Duktape 2.1 refzero_list is
    freed inline without side effects, so it's always NULL when mark-and-sweep
    runs.)

Interactions between reference counting and mark-and-sweep
==========================================================

If mark-and-sweep is triggered e.g. by an out-of-memory condition, reference
counting is essentially "disabled" for the duration of the mark-and-sweep
phase:

* Reference counts are updated normally.  In fact, mark-and-sweep uses the
  same refcount macros to update element refcounts while freeing them.

* If a reference count reaches zero due after a ``DECREF`` operation, the
  object is not freed nor is it placed on the "refzero" work list because
  mark-and-sweep is expected to deal with the object directly.

If the "refzero" algorithm is triggered first (with some objects in the
"refzero" work list), since Duktape 2.1 mark-and-sweep is not triggered while
the refzero_list is being processed as refzero_list handling is side effect
free.

Finalizer behavior
==================

General notes:

* If reference counting is used, finalizers are called either when reference
  count drops to zero, or when mark-and-sweep wants to collect the object
  (which is required for circular references and may also happen if reference
  counts have been incorrectly updated for whatever reason).

* If mark-and-sweep is used, finalizers are called only when mark-and-sweep
  wants to collect the object.

* Regardless of whether reference counting or mark-and-sweep (or both) is
  used, finalizers are executed for all objects (even reachable ones) when
  a heap is freed.

* Finalizer may reinstate a reference to the target object.  In this case the
  object is "rescued" and its finalizer may be called again if it becomes
  unreachable again.  Regardless of whether an object is rescued or not,
  it's a good practice to make the finalizer re-entrant, i.e. allow multiple
  finalizer calls even if the finalizer doesn't rescue the object.

* Finalizers are guaranteed to run when objects are collected or when a heap
  is destroyed forcibly.  The Duktape API ``duk_destroy_heap()`` call runs a
  few rounds of mark-and-sweep to allow finalizers for unreachable objects to
  run normally, and then runs finalizers for all objects on the heap_allocated
  list regardless of their reachability status.  This allows user code to e.g.
  free any native resources more or less reliably even for reachable objects.

* The finalizer return value is ignored.  Also, if the finalizer throws an
  error, this is only debug logged but is considered to be a successful
  finalization.

* Finalizers are always executed using ``heap->heap_thread`` in Duktape 2.1.
  Before Duktape 2.0 the thread used depended on whether the object was
  finalized via refcounting or mark-and-sweep.

* The finalizer may technically launch other threads and do arbitrary things
  in general, but it is a good practice to make the finalizer very simple and
  unintrusive.  Ideally it should only operate on the target object and its
  properties.

* A finalizer should not be able to terminate any threads in the active call
  stack, in particular the thread which triggered a finalization or the
  finalizer thread (if these are different).

Voluntary mark-and-sweep interval
=================================

There are many ways to decide when to do a voluntary mark-and-sweep pass:
byte count based, object count based, probabilistic, etc.

The current approach is to count the number of heap objects and heap strings
kept at the end of a mark-and-sweep pass, and initialize the voluntary sweep
trigger count based on that as (the computation actually happens using fixed
point arithmetic)::

  trigger_count = ((kept_objects + kept_strings) * MULT) + ADD

  // MULT and ADD are tuning parameters

The trigger count is decreased on every memory (re)allocation and free, to
roughly measure allocation activity.  If the trigger count is below zero when
memory is about to be (re)allocated, a a voluntary mark-and-sweep pass is
done.  When ``MULT`` is 1 and ``ADD`` is 0, a voluntary sweep is done when
the number of alloc/free operations matches the previous heap object/string
count.

When reference counting is enabled, ``MULT`` can be quite large (e.g. 10)
because only circular references need to be swept.  When reference counting
is not enabled, ``MULT`` should be closer to 1 (or even below).  The ``ADD``
tuning parameter is not that important; its purpose is to avoid too frequent
mark-and-sweep on very small heaps and to counteract some inaccuracy of fixed
point arithmetic.

Implementation issues
=====================

Error handling
--------------

When a ``longjmp()`` takes place, the C stack is unwound and all references
to the unwound part of the stack are lost.  To avoid memory leaks and other
correctness issues, care must be taken to:

* Ensure that the reference count of every heap-allocated element is
  correct whenever entering code which may ``longjmp()``.

* Ensure that all heap-allocated objects which should be subject to
  automatic garbage collection are reachable whenever entering code
  which may ``longjmp()``.

* Use a ``setjmp()`` catchpoint whenever control must be regained to
  clean up properly.

To avoid the need for ``setjmp()`` catchpoints, many innermost helper
functions return error codes rather than throwing errors.  This makes
error handling a bit easier.

Side effects of memory management
---------------------------------

See ``doc/side-effects.rst``.

Misc notes
==========

Garbage collection of value stacks
----------------------------------

While an ECMAScript function is running, the value stack frame allocated
for it has a minimum size matching the "register count" of the function.
All of these registers are reachable from a mark-and-sweep viewpoint, even
if the values held by the registers are never referenced by the bytecode
of the function.

For instance, any temporaries created during expression evaluation may
leave unused but technically reachable values behind.  Consider for
instance::

   function f(x,y,z) {
     var w = (x + y) + z;
   }

the bytecode created for this will:

* Compute ``x + y`` into a temporary register ``T``.

* Compute ``T + z`` into the register allocated for ``w``.

Before exiting the function, ``T`` is reachable for mark-and-sweep.  If
``T`` is a heap element (e.g. a string), it has a positive reference count.

The situation is fixed if the function exits or the temporary register ``T``
is reused by the evaluation of another expression, so this is not usually a
relevant issue.  However:

* If a function runs in an infinite loop, such references may never become
  collectable.  Consider, for instance, a main event loop which never exits.

* Even if the function eventually exits, such references may cause an
  out-of-memory situation before the function exits.  The out-of-memory
  situation may not be recoverable using garbage collection because the
  values are technically reachable until the exit.

There is currently no actual solution to this issue, but any code containing
an infinite loop should be structured to avoid "dangling values", e.g. by
using an auxiliary function for any computations::

  function stuff() {
    // ...
  }

  function infloop() {
    for (;;) {
      stuff();
    }
  }

The issue could be fixed technically by:

* Making the function use an actual stack of values instead of direct
  register references.  This would make function evaluation slower.

* Add a bytecode instruction to "wipe" any registers above a certain
  index to ensure they contain no bogus references.  These could be
  issued after expression evaluation or in loop headers.  This would
  bloat bytecode.

Function closures are reference loops by default
------------------------------------------------

Function closures contain a reference loop by default::

  var f = function() {};
  print(f.prototype.constructor === f);  // --> true

Unless user code explicitly sets a different ``f.prototype``, every
function closure requires a mark-and-sweep to be collected which makes
plain reference counting unattractive if there are a lot of function
temporaries.  Such temporaries will then be reachable and only freed
when the heap is destroyed.  This should be fixed in the future somehow
if possible.

Requirements for tracking heap allocated objects
------------------------------------------------

Mark-and-sweep only requires a single (forward) linked list to track
objects.  Objects are inserted at the head, and scanned linearly
during mark and sweep.  The sweep phase can remove an object by keeping
track of its predecessor when traversing the list.  The same applies
to work lists.

Reference counting requires the ability to remove an arbitrarily
chosen object to be removed from the heap allocated list.  To do
this efficiently, a double linked list is needed to avoid scanning
the list from the beginning.

Future work
===========

* During object property allocation resize, don't prevent compaction of
  other objects in mark-and-sweep.

* Special handling for built-in strings and objects, so that they can be
  allocated from a contiguous buffer, only freed when heap is freed.

* Incremental mark-and-sweep at least as an option in semi real-time
  environments.

* Optimize reference count handling in performance critical code sections.
  For instance:

  - a primitive to INCREF a slice of tagged values would be useful

  - often the target of an INCREF can be assumed to be non-NULL; a fast
    path macro could assert for this but avoid otherwise checking for it

* Develop a fix for the function temporary register reachability issue.

* Develop a fix for function instance prototype reference loop issue.

* Add a figure of where objects may reside (string table, heap allocated,
  refzero work list, mark-and-sweep to be finalized work list).
