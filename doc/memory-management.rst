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
  to be freed by reference counting and/or mark-and-sweep garbage collection.
  Freeing a heap causes all related allocations to be freed, regardless of
  their reference count or reachability.

* **Reference counting and/or mark-and-sweep**.
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
  confuse with other notions of an "object".  In particular, all Ecmascript
  objects are heap elements, but there are other heap element types too.
  Heap-allocated elements subject to memory management are:

  * ``duk_hstring``

  * ``duk_hobject`` and its subtypes

  * ``duk_hbuffer``

  Only ``duk_hobject`` contains further internal references to other heap
  elements.  These references are kept in the object property table and the
  object internal prototype pointer.  Currently only ``duk_hobject`` may
  have a finalizer.

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
If a finalizer is executed, arbitrary Ecmascript or even native code
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
                 * "mark-and-sweep finalization work list"

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
           +-->  call stack
           |
           +-->  catch stack
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

* Thread call stack

* Thread catch stack

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
C type (e.g. a field might have the type "``duk_hcompiledfunction *``").
However, heap elements do have a "heap type" field as part of the
``h_flags`` field of the header; this is not normally used, but is
needed by e.g. reference counting.  As a separate issue, some heap types
(such as ``duk_hobject``) have "sub-types" with various extended memory
layouts; these are not reflected in the heap type.

The current specific heap element types are:

* ``duk_hstring`` (heap type ``DUK_HTYPE_STRING``):

  + Fixed size allocation consisting of a header with string
    data following the header.  Header does not contain next/previous
    pointers (uses ``duk_heaphdr_string``).

  + No references to other heap elements.

* ``duk_hobject`` (heap type ``DUK_HTYPE_OBJECT``):

  + Fixed size allocation consisting of a header, whose size
    depends on the object type (``duk_hobject``, ``duk_hthread``,
    ``duk_hcompiledfunction``, or ``duk_hnativefunction``).

  + The specific "sub type" and its associated struct definition
    can be determined using object flags, using the macros:

    - ``DUK_HOBJECT_IS_COMPILEDFUNCTION``
    - ``DUK_HOBJECT_IS_NATIVEFUNCTION``
    - ``DUK_HOBJECT_IS_THREAD``
    - If none of the above are true, the object is a plain object
      (``duk_hobject`` without any extended structure)

  + Properties are stored in a separate, dynamic allocation, and contain
    references to other heap elements.

  + For ``duk_hcompiledfunction``, function bytecode, constants, and
    references to inner functions are stored in a fixed ``duk_hbuffer``
    referenced by the ``duk_hcompiledfunction`` header.  These provide
    further references to other heap elements.

  + For ``duk_hthread`` the heap header contains references to the
    value stack, call stack, catch stack, etc, which provide references
    to other heap elements.

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
  but these are not part of the Ecmascript standard.

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

* and a bunch of heap element type specific INCREF/DECREF macros,
  defined in ``heaphdr.h``

Notes on macro semantics:

* The macros tolerate ``NULL`` pointers, which are simply ignored.  This
  reduces caller code size but requires a pointer check which is unnecessary
  in the vast majority of cases.

* An ``INCREF`` is guaranteed not to have any side effects.

* A ``DECREF`` may have a wide variety of side effects.

  + ``DECREF`` may free the target object and an arbitrary number of other
    objects whose reference count drops to zero as a result.

  + If a finalizer is invoked, arbitrary C or Ecmascript code is
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

   b. If the target is a string:

      1. Remove the string from the string table.

      2. Remove any references to the string from the "string access cache"
         (which accelerates character index to byte index conversions).
         Note that this is a special, internal "weak" reference.

      3. Free the string.  There are no auxiliary allocations to free
         for strings.

      4. Return.

   c. If the target is a buffer:

      1. Remove the buffer from the "heap allocated" list.

      2. If the buffer is dynamic, free the auxiliary buffer (which is
         allocated separately).

      3. Free the buffer.

      4. Return.

   d. Else the target is an object:

      1. Move the object from the "heap allocated" list to the "refzero" work
         list.  Note that this prevents the mark-and-sweep algorithm from
         freeing the object (the "sweep" phase does not affect objects in the
         "refzero" work list).

      2. If the "refzero" algorithm is already running, return.

      3. Else, call the "refzero" algorithm to free pending objects.
         The refzero algorithm returns when the entire work list has
         been successfully cleared.

      4. Return.

The REFZERO algorithm
---------------------

The ``DECREF`` algorithm ensures that only one instance of the "refzero"
algorithm may run at any given time.  The "refzero" work list model is used
to avoid an unbounded C call stack depth caused by a cascade of reference
counts which drop to zero.

The algorithm is as follows:

1. While the "refzero" work list is not empty:

   a. Let ``O`` be the element at the head of the work list.
      Note:

      * ``O`` is always an object, because only objects are placed in the work list.

      * ``O`` must not be removed from the work list yet.  ``O`` must be on the
        work list in case a finalizer is executed, so that a mark-and-sweep
        triggered by the finalizer works correctly (concretely: to be able to
        clear the ``DUK_HEAPHDR_FLAG_REACHABLE`` of the object.)

   b. If ``O`` is an object (this is always the case, currently), and has a
      finalizer (i.e. has a ``_Finalizer`` internal property):

      1. Create a ``setjmp()`` catchpoint.

      2. Increase the reference count of ``O`` temporarily by one (back to 1).

      3. Note: the presence of ``O`` in the "refzero" work list is enough to
         guarantee that the mark-and-sweep algorithm will not free the object
         despite it not being reachable.

      4. Call the finalizer method.  Ignore the return value and a possible
         error thrown by the finalizer (except for debug logging an error).
         Any error or other ``longjmp()`` is caught by the  ``setjmp()``
         catchpoint.  Note:

         * The thread used for finalization is currently the thread which
           executed ``DECREF``.  *This is liable to be changed later.*

      5. Regardless of how the finalizer finishes, decrease the reference
         count of ``O`` by one.

      6. If the reference count of ``O`` is non-zero, the object has been
         "rescued" and:

         a. Place the object back into the "heap allocated" list (and debug
            log the object as "rescued").

         b. Continue the while-loop with the next object.

   c. Remove ``O`` from the work list.

   d. Call ``DECREF`` for any references that ``O`` contains (this is
      called "refcount finalization" in the source).  Concretely:

      * String: no internal references.

      * Buffer: no internal references.

      * Object: properties contain references; specific sub-types (like
        ``duk_hthread``) contain further references.

      * Note: this step is recursive with respect to ``DECREF`` but not
        the "refzero" algorithm: a ``DECREF`` is executed inside a
        ``DECREF`` which started the "refzero" algorithm, but the inner
        ``DECREF`` doesn't restart the "refzero" algorithm.  Recursion is
        thus limited to two levels.

   e. Free any auxiliary references (such as object properties) contained
      in ``O``, and finally ``O`` itself.

2. Check for a voluntary mark-and-sweep.

Notes:

* "Churning" the work list requires that the type of a heap element can be
  determined by looking at the heap header.

  + This is one of the rare places where this would be necessary: usually the
    tagged type of a ``duk_tval`` is sufficient to type an arbitrary value,
    and when following pointer references from one heap element to another,
    the pointers themselves are typed.

  + Right now, this type determination is not actually needed because only
    object (``duk_hobject``) values will be placed in the work list.

* The finalizer thread selection is not a trivial issue, especially for
  mark-and-sweep.  See discussion under mark-and-sweep.

* Because the reference count is artifially increased by one during finalization,
  the object being finalized cannot encounter a "refcount drops to zero"
  situation while being finalized (assuming of course that all ``INCREF`` and
  ``DECREF`` calls are properly "nested").

* If mark-and-sweep is triggered during finalization, the target may or
  may not be reachable, but will have a non-zero reference count in
  either case due to the artificial ``INCREF`` in the finalization
  algorithm.  The reference count is inconsistent with the actual reference
  count in the reachability graph but this is not an issue for mark-and-sweep.
  In any case, mark-and-sweep will not free any object in the "refzero" work
  list, regardless of its reachability status, so mark-and-sweep during
  REFZERO is not a problem.

* Although finalization increases C call stack size, another finalization
  triggered by reference counting cannot occur while finalization for one
  object is in progress: any objects whose refcounts drop to zero during
  finalization are simply placed in the refzero work list and dealt with
  when the object being finalization has been fully processed.  However,
  there can still be **two** active finalizers at the same time, one initiated
  by reference counting and another by a mark-and-sweep triggered inside
  REFZERO.

Background on the refzero algorithm, limiting C recursion depth
---------------------------------------------------------------

When a reference count drops to zero, the heap element will be freed.  If the
heap element contains references (like an Ecmascript object does), all target
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
directly when their reference count drops to zero.  This is fortunate, because
strings don't have ``h_prev``/``h_next`` fields at all.

*Finalization* of an object whose refcount becomes zero is very useful for
e.g. freeing any native resources or handles associated with an object.
For instance, socket or file handles can be closed when the object is being
freed.  The finalizer is an internal method associated with an ``duk_hobject``
which is called just before the object is freed either by reference counting
or by the mark-and-sweep collector.  The finalizer gets a reference to the
object in question, and may "rescue" the reference.

Mark-and-sweep may be triggered during the "refzero" algorithm, currently
only by finalization.  If mark-and-sweep is triggered, it must not touch any
object in the "refzero" work list (i.e. any object whose reference count is
zero, but which has not yet been processed).

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

Mark-and-sweep control flags are defined in ``duk_heap.h``:

* ``DUK_MS_FLAG_EMERGENCY``

* ``DUK_MS_FLAG_NO_STRINGTABLE_RESIZE``

* ``DUK_MS_FLAG_NO_FINALIZERS``

* ``DUK_MS_FLAG_NO_OBJECT_COMPACTION``
  
In addition to the explicitly requested flags, the bit mask in
``mark_and_sweep_base_flags`` in ``duk_heap`` is bitwise ORed into the
requested flags to form effective flags.  The flags added to the "base
flags" control restrictions on mark-and-sweep side effects, and are used
for certain critical sections.

To protect against such side effects, the critical algorithms:

* Store the original value of ``heap->mark_and_sweep_base_flags``

* Set the suitable restriction flags into ``heap->mark_and_sweep_base_flags``

* Attempt the allocation / reallocation operation, *without throwing errors*

* Restore the ``heap->mark_and_sweep_base_flags`` to its previous value

* Examine the allocation result and act accordingly

It is important not to throw an error without restoring the base flags field.

The concrete flags used are:

* String table resize:

  + ``DUK_MS_FLAG_NO_STRINGTABLE_RESIZE``: prevents another stringtable
    resize attempt when one is already running

  + ``DUK_MS_FLAG_NO_FINALIZERS``: prevent finalizers from adding new
    interned strings to the string table, possibly requiring a resize

  + ``DUK_MS_FLAG_NO_OBJECT_COMPACTION``: prevent object compaction,
    because object compaction may lead to an array part being abandoned,
    which leads to string interning of array keys.

* Object property allocation resize:

  + ``DUK_MS_FLAG_NO_FINALIZERS``: prevent finalizers from manipulating
    the properties of any object.  It would suffice to protect only the
    object being resized, but a finalizer may potentially operate on any
    set of objects; hence no finalizers are executed at all.

  + ``DUK_MS_FLAG_NO_OBJECT_COMPACTION``: prevent objects from being
    compacted (i.e., resized).  It would suffice to protect only the
    object being resized from a recursive resize; this is currently not
    done, however, but would be easy to fix.

Heap header flags
-----------------

The following flags in the heap element header are used for controlling
mark-and-sweep:

* ``DUK_HEAPHDR_FLAG_REACHABLE``:
  element is reachable through the reachability graph

* ``DUK_HEAPHDR_FLAG_TEMPROOT``:
  element's reachability has been marked, but its children have not been
  processed; this is required to limit the C recursion level

* ``DUK_HEAPHDR_FLAG_FINALIZABLE``:
  element is not reachable after the first marking pass (see algorithm),
  has a finalizer, and the finalizer has not been called in the previous
  mark-and-sweep round; object will be moved to the finalization work
  list and will be considered (temporarily) a reachability root

* ``DUK_HEAPHDR_FLAG_FINALIZED``:
  element's finalizer has been executed, and if still unreachable, object
  can be collected

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
of explicit flags and "base flags" stored in ``heap->mark_and_sweep_base_flags``.
The "base flags" essentially prohibit specific garbage collection operations
(like finalizers) when a certain critical code section is active.

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

8. Run finalizers:

   a. While the "to be finalized" work queue is not empty:

      1. Select object from head of the list.

      2. Set up a ``setjmp()`` catchpoint.

      3. Execute finalizer.  Note:

         * The thread used for this is the currently running thread
           (``heap->curr_thread``), or if no thread is running,
           ``heap->heap_thread``.  This is liable to change in the future.

      4. Ignore finalizer result (except for logging errors).

      5. Mark the object ``FINALIZED``.

      6. Move the object back to the "heap allocated" list.  The object will
         be collected on the next pass if it is still unreachable.  (Regardless
         of actual reachability, the ``REACHABLE`` flag of the object is clear
         at this point.)

9. Finish.

   a. All ``TEMPROOT`` and ``REACHABLE`` flags are clear at this point.

   b. All "heap allocated" elements either (a) are reachable and have a
      non-zero reference count, or (b) were finalized and their reachability
      status is unknown.

   c. The "to be finalized" list is empty.

   d. No object in the "refzero" work list has been freed.

Notes:

* Elements on the refzero list are considered reachability roots, as we need
  to preserve both the object itself (which happens automatically because we
  don't sweep the refzero_list) and its children.  If the refzero list elements
  were not considered reachability roots, their children might be swept by the
  sweep phase.  This would be problematic for processing the objects in the
  refzero list, regardless of whether they have a finalizer or not, as some
  references would be dangling pointers.

* Elements marked FINALIZABLE are considered reachability roots to ensure
  that their children (e.g. property values) are not swept during the
  sweep phase.  This would obviously be problematic for running the finalizer,
  regardless of whether the object would be rescued or not.

* While mark-and-sweep is running:

  + Another mark-and-sweep cannot execute.

  + A ``DECREF`` resulting in a zero reference count is not processed at all.
    The object is not placed into the "refzero" work list, as mark-and-sweep
    is assumed to be a comprehensive pass, including running finalizers.

* Finalizers are executed after the sweep phase to ensure that finalizers
  have as much available memory as possible.  Since mark-and-sweep is
  running, if a finalizer runs out of memory, no memory can be reclaimed
  as recursive mark-and-sweep is explicitly blocked.  This is probably a
  very minor issue in practice.

* Finalizers could be executed from their work list after the mark-and-sweep
  has finished to allow mark-and-sweep to run if mark-and-sweep is required
  by a finalizer.  The mark-and-sweep could then append more objects to be
  finalized into the "to be finalized" work list; this is not a problem.
  However, since finalizers are used with a rather limited scope, this is not
  currently done.

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
    the case for objects with finalizers.

  + A finalizer call made by the "refzero" algorithm makes the object reachable
    again (through the finalizer thread value stack; the finalizer method itself
    can also create reachable references for the target).  If a mark-and-sweep
    is triggered during finalization, the target will be marked ``REACHABLE``
    during the mark phase.  Thus, ``REACHABLE`` flags of "refzero" work list
    elements must be cleared explicitly after or during the sweep phase.

Note that there is a small "hole" in the reclamation right now, when
mark-and-sweep finalizers are used:

* If a finalizer executed by mark-and-sweep removes a reference to another
  object (not the object being finalized), causing the target object's
  reference count to drop to zero, the object is *not* placed in the
  "refzero" work list, as mark-and-sweep is still running.

* As a result, the object will be unreachable and will not be freed by
  the reference count algorithm, regardless of whether the object was part
  of a reference loop.  Instead, the next mark-and-sweep will free the object.
  If the object has a finalizer, the finalizer will be called later than
  would be preferable.

* This is not ideal but will not result in memory leaks, so it's not really
  worth fixing right now.

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
"refzero" work list), mark-and-sweep may be triggered while the "refzero"
algorithm is running.  In more detail:

* A ``DECREF`` happens while neither mark-and-sweep nor "refzero" algorithm
  is running.

* A reference count reaches zero, and the object is placed on the "refzero"
  work list and the "refzero" algorithm is invoked.

* The "refzero" algorithm cannot trigger another "refzero" algorithm to
  execute recursively.  Instead, the work list is churned until it becomes
  empty.  Any objects whose reference count reaches zero are added to the
  work list, though, so will be processed eventually.

* The "refzero" algorithm may trigger a mark-and-sweep while it is running,
  e.g. by running a finalizer which runs out of memory:

  + This mark-and-sweep will mark any elements in the "refzero" work list
    but will not free them.

  + While the mark-and-sweep is running, no new elements are placed into
    the "refzero" work list, even if their reference count reaches zero.
    Instead, the mark-and-sweep algorithm is assumed to deal with them.

  + The mark-and-sweep algorithm may also execute finalizers, so two
    finalizers (but no more) can be running simultaneously, though on
    different objects.

  + Another recursive mark-and-sweep run cannot happen.

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

* The thread running a finalizer is not very logical right now and is liable
  to change:

  + Reference counting: the thread which executed ``DECREF`` is used as the
    finalizer thread.

  + Mark-and-sweep: the thread which caused mark-and-sweep is used as the
    finalizer thread; if there is no active thread, ``heap->heap_thread``
    is used instead.

* The finalizer may technically launch other threads and do arbitrary things
  in general, but it is a good practice to make the finalizer very simple and
  unintrusive.  Ideally it should only operate on the target object and its
  properties.

* A finalizer should not be able to terminate any threads in the active call
  stack, in particular the thread which triggered a finalization or the
  finalizer thread (if these are different).

Finalizer thread selection is currently not optimal; there are several
approaches:

* The thread triggering mark-and-sweep is not a good thread for finalization,
  as it may be from a different conceptual virtual machine, and may thus have
  a different global context (global object) than where the finalized object
  was created.

* A heap-level dedicated finalizer thread has a similar problem: the finalizer
  will run in a different global context than where the finalized object was
  created.

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

The trigger count is decreased on every memory (re)allocation, and for every
object processed by the refzero algorithm.  If the trigger reaches zero when
memory is about to be (re)allocated, a voluntary mark-and-sweep pass is done.
When ``MULT`` is 1 and ``ADD`` is 0, a voluntary sweep is done when the number
of "operations" matches the previous heap object/string count.

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

Automatic memory management may be triggered by various operations, and has
a wide variety of side effects which must be taken into account by calling
code.  This affects internal code in particular, which must be very careful
not to reference dangling pointers, deal with valstack and object property
allocation resizes, etc.

The fundamental triggers for memory management side effects are:

* An attempt to ``alloc`` or ``realloc`` memory may trigger a garbage
  collection.  A collection is triggered by an out-of-memory condition,
  but a voluntary garbage collection also occurs periodically.  A ``free``
  operation cannot, at the moment, trigger a collection.

* An explicit request for garbage collection.

* A ``DECREF`` operation which drops the target heap element reference
  count to zero triggers the element (and possibly a bunch of other
  elements) to be freed, and may invoke a number of finalizers.  Also,
  a mark-and-sweep may be triggered (e.g. by finalizers or voluntarily).

The following primitives do not trigger any side effects:

* An ``INCREF`` operation never causes a side effect.

* A ``free`` operation never causes a side effect.

Because of finalizers, the side effects of a ``DECREF`` and a mark-and-sweep
are potentially the same as running arbitrary C or Ecmascript code,
including:

* Calling (further) finalizer functions (= running arbitrary Ecmascript and C code).

* Resizing object allocations, value stacks, catch stacks, call stacks, buffers,
  object property allocations, etc.

* Compacting object property allocations, abandoning array parts.

* In particular:

  + Any ``duk_tval`` pointers referring any value stack may be invalidated,
    because any value stack may be resized.  Value stack indices are OK.

  + Any ``duk_tval`` pointers referring any object property values may be
    invalidated, because any property allocation may be resized.  Also,
    any indices to object property slots may be invalidated due to
    "compaction" which happens during a property allocation resize.

  + Heap element pointers are stable, so they are never affected.

The side effects can be avoided by many techniques:

* Refer to value stack using a numeric index.

* Make a copy of an ``duk_tval`` to a C local to ensure the value can still
  be used after a side effect occurs.  If the value is primitive, it will
  OK in any case.  If the value is a heap reference, the reference uses a
  stable pointer which is OK as long as the target is still reachable.

* Re-lookup object property slots after a potential side effect.

Misc notes
==========

Garbage collection of value stacks
----------------------------------

While an Ecmascript function is running, the value stack frame allocated
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
