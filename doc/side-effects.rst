============
Side effects
============

Overview
========

Duktape is a single threaded interpreter, so when the internal C code deals
with memory allocations, pointers, and internal data structures it is safe
to assume, for example, that pointers are stable while they're being used and
that internal state and data structures are not modified simultaneously from
other threads.

However, many internal operations trigger quite extensive side effects such
as resizing the value stack (invalidating any pointers to it) or clobbering
the current heap error handling (longjmp) state.  There are a few primary
causes for the side effects, such as memory management reallocating data
structures, finalizer invocation, and Proxy trap invocation.  The primary
causes are also triggered by a lot of secondary causes.  The practical effect
is that any internal helper should be assumed to potentially invoke arbitrary
side effects unless there's a specific reason to assume otherwise.

Some of the side effects can be surprising when simply looking at calling
code, which makes side effects an error prone element when maintaining Duktape
internals.  Incorrect call site assumptions can cause immediate issues like
segfaults, assert failures, or valgrind warnings.  But it's also common for
an incorrect assumption to work out fine in practice, only to be triggered by
rare conditions like voluntary mark-and-sweep or a unrecoverable out-of-memory
error happening in just the right place.  Such bugs have crept into the code
base several times -- they're easy to make and hard to catch with tests or
code review.

This document describes the different side effects, how they may be triggered,
what mechanisms are in place to deal with them internally, and how tests try
to cover side effects.

Basic side effect categories
============================

Primary causes
--------------

Side effects are ultimately caused by:

* A refcount dropping to zero, causing a "refzero cascade" where a set of
  objects is refcount finalized and freed.  If any objects in the cascade
  have finalizers, the finalizer calls have a lot of side effects.  Object
  freeing itself is nearly side effect free, but does invalidate any pointers
  to unreachable but not-yet-freed objects which are held at times.

* Mark-and-sweep similarly frees objects and can make finalizer calls.
  Mark-and-sweep may also resize/compact the string table and object property
  tables.  The set of mark-and-sweep side effects are likely to slowly change
  over time (e.g. better emergency GC capabilities).

* Error throwing overwrites heap-wide error handling state, and causes a long
  control transfer.  Concrete impact on call site is that e.g. calling code
  may not be able to store/restore internal flags or counters if an error gets
  thrown.  Almost anything involving a memory allocation, property operation,
  etc may throw.

Any operation doing a DECREF may thus have side effects.  Any operation doing
anything to cause a mark-and-sweep (like allocating memory) may similarly have
side effects.  Finalizers cause the most wide ranging side effects, but even
with finalizers disabled there are significant side effects in mark-and-sweep.

Full side effects
-----------------

The most extensive type of side effect is arbitrary code execution, caused
by e.g. a finalizer or a Proxy trap call (and a number of indirect causes).
The potential side effects are very wide:

* Because a call is made, the value stack may be grown (but not shrunk) and
  its base pointer may change.  As a result, any duk_tval pointers to the
  value stack are (potentially) invalidated.  Since Duktape 2.2 duk_activation
  and duk_catcher structs are allocated separately and have a stable pointer.
  Before Duktape 2.2 duk_activations were held in a call stack and duk_catchers
  in a catch stack, and their pointers might be invalidated by side effects.

* Value stack allocated size may grow or shrink.  However, value stack bottom,
  top, and reserved space won't change.

* An error throw may happen, clobbering heap longjmp state.  This is a
  problem particularly in error handling where we're dealing with a previous
  throw.  A long control transfer may skip intended cleanup code.

* A new thread may be resumed and yielded from.  The resumed thread may even
  duk_suspend().

* A native thread switch may occur, for an arbitrarily long time, if any
  function called uses duk_suspend() and duk_resume().  This is not currently
  supported for finalizers, but may happen, for example, for Proxy trap calls.

* Because called code may operate on any object (except those we're certain
  not to be reachable), objects may undergo arbitrary mutation.  For example,
  object properties may be added, deleted, or modified; dynamic and external
  buffer data pointers may change; external buffer length may change.  An
  object's property table may be resized and its base pointer may change,
  invalidating both pointers to the property table.  Object property slot
  indices may also be invalidated due to object resize/compaction.

The following will be stable at all times:

* Value stack entries in the current activation won't be unwound or modified.
  Similarly, the current call stack and catch stack entries and entries below
  them won't be unwound or modified.

* All heap object (duk_heaphdr) pointers are valid and stable regardless of
  any side effects, provided that the objects in question are reachable and
  correctly refcounted for.  Called code cannot (in the absence of bugs)
  remove references from previous activations in the call stack and thread
  resume chain.

* In particular, while duk_tval pointers to the value stack may change, if
  an object pointer is encapsulated in a duk_tval, the pointer to the actual
  object is still stable.

* All string data pointers, including external strings.  String data is
  immutable, and can't be reallocated or relocated.

* All fixed buffer data pointers, because fixed buffer data follows the stable
  duk_heaphdr directly.  Dynamic and external buffer data pointers are not
  stable.

Side effects without finalizers, but with mark-and-sweep allowed
----------------------------------------------------------------

If code execution side effects (finalizer calls, Proxy traps, getter/setter
calls, etc) are avoided, most of the side effects are avoided.  In particular,
refzero situations are then side effect free because object freeing has no
side effects beyond memory free calls.

The following side effects still remain:

* Refzero processing still frees objects whose refcount reaches zero.
  Any pointers to such objects will thus be invalidated.  This may happen
  e.g. when a borrowed pointer is used and that pointer loses its backing
  reference.

* Mark-and-sweep may reallocate/compact the string table.  This affects
  the string table data structure pointers and indices/offsets into them.
  Strings themselves are not affected (but unreachable strings may be freed).

* Mark-and-sweep may reallocate/compact object property tables.  All property
  keys and values will remain reachable, but pointers and indices to an object
  property table may be invalidated.  This mostly affects property code which
  often finds a property's "slot index" and then operates on the index.

* Mark-and-sweep may free unreachable objects, invalidating any pointers to
  them.  This affects only objects which have been allocated and added to
  heap_allocated list.  Objects not on heap_allocated list are not affected
  because mark-and-sweep isn't aware of them; such objects are thus safe from
  collection, but at risk for leaking if an error is thrown, so such
  situations are usually very short lived.

Other side effects don't happen with current mark-and-sweep implementation.
For example, the following don't happen (but could, if mark-and-sweep scope
and side effect lockouts are changed):

* Thread value stack is never reallocated and all pointers to duk_tvals remain
  valid; duk_activation and duk_catcher pointers are stable in Duktape 2.2.
  (This could easily change if mark-and-sweep were to "compact" the value stack
  in an emergency GC.)

The mark-and-sweep side effects listed above are not fundamental to the
engine and could be removed if they became inconvenient.  For example, it's
nice that emergency GC can compact objects in an attempt to free memory, but
it's not a critical feature (and many other engines don't do it either).

Side effects with finalizers and mark-and-sweep disabled
--------------------------------------------------------

When both finalizers and mark-and-sweep are disabled, the only remaining side
effects come from DECREF (plain or NORZ):

* Refzero processing still frees objects whose refcount reaches zero.
  Any pointers to such objects will thus be invalidated.  This may happen
  e.g. when a borrowed pointer is used and that pointer loses its backing
  reference.

When DECREF operations happen during mark-and-sweep they get handled specially:
the refcounts are updated normally, but the objects are never freed or even
queued to refzero_list.  This is done because mark-and-sweep will free any
unreachable objects; DECREF still gets called because mark-and-sweep finalizes
refcounts of any freed objects (or rather other objects they point to) so that
refcounts remain in sync.

Controls in place
=================

Finalizer execution, pf_prevent_count
-------------------------------------

Objects with finalizers are queued to finalize_list and are processed later
by duk_heap_process_finalize_list().  The queueing doesn't need any side
effect protection as it is side effect free.

duk_heap_process_finalize_list() is guarded by heap->pf_prevent_count which
prevents recursive finalize_list processing.  If the count is zero on entry,
it's bumped and finalize_list is processed until it becomes empty.  New
finalizable objects may be queued while the list is being processed, but
only the first call will process the list.  If the count is non-zero on entry,
the call is a no-op.

The count can also be bumped upwards to prevent finalizer execution in the
first place, even if no call site is currently processing finalizers.  If the
count is bumped, there must be a reliable mechanism of unbumping the count or
finalizer execution will be prevented permanently.

Because only the first finalizer processing site processes the finalize_list,
using duk_suspend() from a finalizer or anything called by a finalizer is not
currently supported.  If duk_suspend() were called in a finalizer, finalization
would be stuck until duk_resume() was called.  Processing finalizers from
multiple call sites would by itself be relatively straightforward (each call
site would just process the list head or notice it is NULL and finish);
however, at present mark-and-sweep also needs to be disabled while finalizers
run.

Mark-and-sweep prevent count, ms_prevent_count
----------------------------------------------

Stacking counter to prevent mark-and-sweep.  Also used to prevent recursive
mark-and-sweep entry when mark-and-sweep runs.

Mark-and-sweep running, ms_running
----------------------------------

This flag is set only when mark-and-sweep is actually running, and doesn't
stack because recursive mark-and-sweep is not allowed.

The flag is used by DECREF macros to detect that mark-and-sweep is running
and that objects must not be queued to refzero_list or finalize_list; their
refcounts must still be updated.

Mark-and-sweep flags, ms_base_flags
-----------------------------------

Mark-and-sweep base flags from duk_heap are ORed to mark-and-sweep argument
flags.  This allows a section of code to avoid e.g. object compaction
regardless of how mark-and-sweep gets triggered.

Using the base flags is useful when mark-and-sweep by itself is desirable
but e.g. object compaction is not.  Finalizers are prevented using a
separate flag.

Calling code must restore the flags reliably -- e.g. catching errors or having
assurance of no errors being thrown in any situation.  It might be nice to
make this easier by allowing flags to be modified, the modification flagged,
and for error throw handling to do the restoration automatically.

Creating an error object, creating_error
----------------------------------------

This flag is set when Duktape internals are creating an error to be thrown.
If an error happens during that process (which includes a user errCreate()
callback), the flag is set and avoids recursion.  A pre-allocated "double
error" object is thrown instead.

Call stack unwind or handling an error, error_not_allowed
---------------------------------------------------------

This flag is only enabled when using assertions.  It is set in code sections
which must be protected against an error being thrown.  This is a concern
because currently the error state is global in duk_heap and doesn't stack,
so an error throw (even a caught and handled one) clobbers the state which
may be fatal in code sections working to handle an error.

DECREF NORZ (no refzero) macros
-------------------------------

DECREF NORZ (no refzero) macro variants behave the same as plain DECREF macros
except that they don't trigger side effects.  Since Duktape 2.1 NORZ macros
will handle refzero cascades inline (freeing all the memory directly); however,
objects with finalizers will be placed in finalize_list without finalizer
calls being made.

Once a code segment with NORZ macros is complete, DUK_REFZERO_CHECK_{SLOW,FAST}()
should be called.  The macro checks for any pending finalizers and processes
them.  No error catcher is necessary: error throw path also calls the macros and
processes pending finalizers.  (The NORZ name is a bit of a misnomer since
Duktape 2.1 reworks.)

Mitigation, test coverage
=========================

There are several torture test options to exercise side effect handling:

* Triggering a mark-and-sweep for every allocation (and in a few other places
  like DECREF too).

* Causing a simulated finalizer run with error throwing and call side effects
  every time a finalizer might have executed.

Some specific cold paths like out-of-memory errors in critical places are
difficult to exercise with black box testing.  There is a small set of
DUK_USE_INJECT_xxx config options which allow errors to be injected into
specific critical functions.  These can be combined with e.g. valgrind and
asserts, to cover assertions, memory leaks, and memory safety.

Operations causing side effects
===============================

The main reasons and controls for side effects are covered above.  Below is
a (non-exhaustive) list of common operations with side effects.  Any internal
helper may invoke some of these primitives and thus also have side effects.

DUK_ALLOC()

* May trigger voluntary or emergency mark-and-sweep, with arbitrary
  code execution side effects.

DUK_REALLOC()

* May trigger voluntary or emergency mark-and-sweep, with arbitrary
  code execution side effects.

* In particular, if reallocating e.g. the value stack, the triggered
  mark-and-sweep may change the base pointer being reallocated!
  To avoid this, the DUK_REALLOC_INDIRECT() call queries the base pointer
  from the caller for every realloc() attempt.

DUK_FREE()

* No side effects at present.

Property read, write, delete, existence check

* May invoke getters, setters, and Proxy traps with arbitrary code execution
  side effects.

* Memory allocation is potentially required for every operation, thus causing
  arbitrary code execution side effects.  Memory allocation is obviously
  needed for property writes, but any other operations may also allocate
  memory e.g. to coerce a number to a string.

Value stack pushes

* Pushing to the value stack is side effect free.  The space must be allocated
  beforehand, and a pushed value is INCREF'd if it isn't primitive, and INCREF
  is side effect free.

* A duk_check_stack() / duk_require_stack() + push has arbitrary side effects
  because of a potential reallocation.

Value stack pops

* Popping a value may invoke a finalizer, and thus may cause arbitrary code
  execution side effects.

Value stack coercions

* Value stack type coercions may, depending on the coercion, invoke methods
  like .toString() and .valueOf(), and thus have arbitrary code execution
  side effects.  Even failed attempts may cause side effects due to memory
  allocation attempts.

* In specific cases it may be safe to conclude that a coercion is side effect
  free; for example, doing a ToNumber() conversion on a plain string is side
  effect free at present.  (This may not always be the case in the future,
  e.g. if numbers become heap allocated.)

* Some coercions not involving an explicit method call may require an
  allocation call -- which may then trigger a voluntary or emergency
  mark-and-sweep leading to arbitrary code execution side effects.

INCREF

* No side effects at present.  Object is never freed or queued anywhere.

DECREF_NORZ

* No side effects other than freeing one or more objects, strings, and
  buffers.  The freed objects don't have finalizers; objects with finalizers
  are queued to finalize_list but finalizers are not executed.

* Queries finalizer existence which is side effect free.

* When mark-and-sweep is running, DECREF_NORZ adjusts target refcount but
  won't do anything else like queue object to refzero_list or free it; that's
  up to mark-and-sweep.

DECREF

* If refcount doesn't reach zero, no side effects.

* If refcount reaches zero, one or more objects, strings, and buffers are
  freed which is side effect free.  Objects with finalizers are queued to
  finalize_list, and the list is processed when the cascade of objects without
  finalizers has been freed.  Finalizer execution had arbitrary code execution
  side effects.

* Queries finalizer existence which is side effect free.

* When mark-and-sweep is running, DECREF adjusts target refcount but won't
  do anything else.

* All objects on finalize_list have an artificial +1 refcount bump, so that
  they can never trigger refzero processing (assuming refcounts are correct).
  This allows refzero code to assume a refzero object is on heap_allocated.

duk__refcount_free_pending()

* As of Duktape 2.1 no side effects, just frees objects without a finalizer
  until refzero_list is empty.  (Equivalent in Duktape 2.0 and prior would
  process finalizers inline.)

* Recursive entry is prevented; first caller processes a cascade until it's
  done.  Pending finalizers are executed after the refzero_list is empty
  (unless prevented).  Finalizers are executed outside of refzero_list
  processing protection so DECREF freeing may happen normally during finalizer
  execution.

Mark-and-sweep

* Queries finalizer existence which is side effect free.

* Object compaction.

* String table compaction.

* Recursive entry prevented.

* Executes finalizers after mark-and-sweep is complete (unless prevented),
  which has arbitrary code execution side effects.  Finalizer execution
  happens outside of mark-and-sweep protection, and may trigger mark-and-sweep.
  However, when mark-and-sweep runs with finalize_list != NULL, rescue
  decisions are postponed to avoid incorrect rescue decisions caused by
  finalize_list being (artificially) treated as a reachability root; in
  concrete terms, FINALIZED flags are not cleared so they'll be rechecked
  later.

Error throw

* Overwrites heap longjmp state, so an error throw while handling a previous
  one is a fatal error.

* Because finalizer calls may involve error throws, finalizers cannot be
  executed in error handling (at least without storing/restoring longjmp
  state).

* Memory allocation may involve side effects or fail with out-of-memory, so
  it must be used carefully in error handling.  For example, creating an object
  may potentially fail, throwing an error inside error handling.  The error
  that is thrown is constructed *before* error throwing critical section
  begins.

* Protected call error handling must also never throw (without catching) for
  sandboxing reasons: the error handling path of a protected call is assumed
  to never throw.

* ECMAScript try-catch handling isn't currently fully protected against out of
  memory: if setting up the catch execution fails, an out-of-memory error is
  propagated from the try-catch block.  Try-catch isn't as safe as protected
  calls for sandboxing.  Even if catch execution setup didn't allocate memory,
  it's difficult to write script code that is fully memory allocation free
  (whereas writing C code which is allocation free is much easier).

* Mark-and-sweep without error throwing or (finalizer) call side effects is
  OK.

Debugger message writes

* Code writing a debugger message to the current debug client transport
  must ensure, somehow, that it will never happen when another function
  is doing the same (including nested call to itself).

* If nesting happens, memory unsafe behavior won't happen, but the debug
  connection becomes corrupted.

* There are some current issues for debugger message handling, e.g. debugger
  code uses duk_safe_to_string() which may have side effects or even busy
  loop.

Call sites needing side effect protection
=========================================

Error throw and resulting unwind

* Must protect against another error: longjmp state doesn't nest.

* Prevent finalizers, avoid Proxy traps and getter/setter calls.

* Avoid out-of-memory error throws, trial allocation is OK.

* Refzero with pure memory freeing is OK.

* Mark-and-sweep without finalizer execution is OK.  Object and string
  table compaction is OK, at least present.

* Error code must be very careful not to throw an error in any part of the
  error unwind process.  Otherwise sandboxing/protected call guarantees are
  broken, and some of the side effect prevention changes are not correctly
  undone (e.g. pf_prevent_count is bumped again!).  There are asserts in place
  for the entire critical part (heap->error_not_allowed).

Success unwind

* Must generally avoid (or protect against) error throws: otherwise state may
  be only partially unwound.  Same issues as with error unwind.

* However, if the callstack state is consistent, it may be safe to throw in
  specific places in the success unwind code path.

String table resize

* String table resize must be protected against string interning.

* Prevent finalizers, avoid Proxy traps.

* Avoid any throws, so that state is not left incomplete.

* Refzero with pure memory freeing is OK.

* Mark-and-sweep without finalizer execution is OK.  As of Duktape 2.1
  string interning is OK because it no longer causes a recursive string
  table resize regardless of interned string count.  String table itself
  protects against recursive resizing, so both object and string table
  compaction attempts are OK.

Object property table resize

* Prevent compaction of the object being resized.

* In practice, prevent finalizers (they may mutate objects) and proxy
  traps.  Prevent compaction of all objects because there's no fine
  grained control now (could be changed).

JSON fast path

* Prevent all side effects affecting property tables which are walked
  by the fast path.

* Prevent object and string table compaction, mark-and-sweep otherwise OK.

Object property slot updates (e.g. data -> accessor conversion)

* Property slot index being modified must not change.

* Prevent finalizers and proxy traps/getters (which may operate on the object).

* Prevent object compaction which affects slot indices even when properties
  are not deleted.

* In practice, use NORZ macros which avoids all relevant side effects.
