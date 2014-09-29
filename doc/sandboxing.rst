==================
Sandboxing Duktape
==================

Overview
========

Sandboxed environments allow execution of untrusted code with two broad
goals in mind:

1. Security: prevent untrusted code from doing unsafe operations such as
   accessing memory directly, causing segfaults, etc.

2. Availability: prevent untrusted code from hogging resources, e.g.
   consuming all available memory or entering an infinite loop.

For some sandbox environments it's sufficient to protect against accidents,
e.g. user code accidentally entering an infinite loop.  In other environments
the executed code may be potentially hostile, which is of course much more
challenging to secure against.

Duktape provides mechanisms to allow these goals to be achieved for running
untrusted Ecmascript code.  All C code is expected to be trusted and must be
carefully written with these sandboxing goals in mind.

This document describes best practices for Duktape sandboxing.

.. note:: This document is in a rough draft state.  Duktape 1.0 does not yet
          have full support for sandboxing, e.g. there is no bytecode
          execution timeout yet.  Sandboxing shortcomings will be fixed in
          later versions.

Suggested measures
==================

Isolation approaches
--------------------

There are two basic alternatives to sandboxing Ecmascript code with Duktape:

* Use a separate Duktape heap for each sandbox

* Use a separate Duktape thread (with a separate global environment) for
  each sandbox

Pros and cons of using a Duktape heap for sandboxing:

* Duktape heaps cannot exchange values or object references except through
  explicit serialization, so values or references don't accidentally leak
  from one heap to another.

* Each Duktape heap can be assigned its own memory pool, which allows separate
  memory limits to be placed for each sandbox.

* Multiple native threads can be used in parallel to execute code in different
  heaps.  Only one native thread may be active at a time for each particular
  heap, however.

* One downside is that there is some per-heap overhead which accumulates for
  each sandbox.  If the memory pool for a heap is pre-allocated, some of the
  pool will be unused.  If memory is not pre-allocated, actual memory usage
  is quite tightly bound because of reference counting.

Pros and cons of using a Duktape thread for sandboxing:

* Duktape threads can exchange values and object references.  This can be
  useful at times, but is also a risk for sandboxing.

* If you create a thread with a separate global environment
  (``duk_push_thread_new_globalenv()``), two threads can be isolated to
  a large extent.  It's still possible to leak values and references
  between threads through Duktape/C functions, carelessly written finalizers,
  and so on.

* All threads in a certain heap share the same memory pool which means that
  one sandbox can starve other sandboxes of memory.

* Only a single native thread can execute Ecmascript code at a time.

These two approaches can of course be mixed: you can have multiple heaps,
each with one or more sandboxed threads.

Replace the global object
-------------------------

The first thing you should do is replace the global object with a minimal
replacement, providing only those bindings that are absolutely necessary
for the sandboxed environment.  Sometimes this means the full E5 bindings;
sometimes it means just a few bindings that sandboxed code is expecting.

In general you should be confident that every exposed binding is safe from
both security and availability viewpoints.

Risky bindings:

* The ``Duktape`` object provides access to Duktape internals in several ways
  which is not ideal.  It may also gain new properties in new Duktape versions,
  which may be easy to accidentally overlook, so the safest default is to hide
  it from sandboxed code.  You can still cherry pick individual functions to
  be exposed directly or through a wrapper.

* ``Duktape.Buffer`` allows creation of buffers and internal keys (through
  buffer-to-string coercion) and thus provides access to internal properties.
  See separate section on internal properties.

* ``Duktape.dec()`` allows decoding of string data into a buffer value and thus
  provides access to internal properties.

* ``Duktape.act()`` provides access to calling functions which may matter to
  some sandboxing environments.

* ``Duktape.fin()`` provides access to setting and getting a finalizer.  Since
  a finalizer may run in a different thread than where it was created,
  finalizers are a sandboxing risk.

You should also:

* Remove the ``require`` module loading function in the global object.
  If you need module loading in the sandbox, it's better to write a specific,
  constrained module loader for that environment.

Restrict access to internal properties
--------------------------------------

Internal properties are intended to be used by Duktape and user C code
to store "hidden properties" in objects.  The mechanism currently relies on
using strings with an invalid UTF-8 encoding which cannot normally be
created by Ecmascript code.  Such properties should be non-writable and
non-configurable when possible, but it's still a risk to let user code
access them.

If Ecmascript code has access to buffer values, it can easily create internal
keys and then access internal properties, e.g.::

    // With access to Duktape.dec: decodes to \xFFfoo, invalid UTF-8 data
    var key = Duktape.dec('hex', 'ff666f6f');

    // With an arbitrary buffer value 'buf' (with length >= 1)
    buf[0] = 0xff;  // create invalid utf-8 prefix
    var key = String(buf).substring(0, 1) + 'foo';

The risk in being able to access a certain internal property depends on the
internal property in question.  Some internal properties are non-writable and
non-configurable, so the sandboxed code can only read the property value; quite
often this is not an issue by itself.  If the value of an internal property can
be modified, concrete security issues may arise.  For instance, if an internal
property stores a raw pointer to a native handle (such as a ``FILE *``),
changing its value can lead to a potentially exploitable segfault.

To prevent access to internal keys:

* Ensure that sandboxed code has no direct access to buffer values, either
  by creating one using ``Duktape.Buffer`` or through some C binding which
  returns a buffer value in some way.

* Ensure that sandboxed code has minimal access to objects with potentially
  dangerous keys like raw pointers.

* If user code needs to deal with buffers, provide access through an accessor
  object without giving direct access to the underlying buffer.

The fact that access to buffer values provides access to internal properties
is not ideal.  There are several future work issues to improve this situation,
e.g. to prevent access to internal properties from Ecmascript code even with
the correct internal string key.

Restrict access to function instances
-------------------------------------

In some environments giving the user code access to calling functions can be
dangerous.  For instance, if user code gets access to a function it cannot
normally see through bindings in the global object, it can call that function
and perhaps sidestep sandboxing.

Prevent access to function references in the call stack:

* Prevent access to ``Duktape.act()`` which provides programmatic access to
  the call stack and its function references.

* If ``DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY`` is enabled, the ``caller``
  property provides access to calling functions.  Don't use this option
  with sandboxing, or at least carefully control the ``caller`` property
  values seen by the sandboxed code.

* The ``_tracedata`` internal property of error objects contains references
  to calling functions.  Because this property is internal, sandboxed code
  has no access to it as long as access to internal properties is prevented
  in general.

Restrict access to finalizers
-----------------------------

Allowing user to code to run a finalizer is dangerous: a finalizer can
execute in another thread than where it was created, so it can potentially
breach sandboxing.

Suggestions for sandboxing:

* Don't give user code access to ``Duktape.fin()``.

* Don't give user code access to internal properties: with access to
  internal properties, user code can read/write the internal finalizer
  reference directly.

* Write finalizers very carefully.  Make minimal assumptions on which
  thread they run, i.e. which global object they see.

* For sandboxed environments it may be sensible to make all finalizers
  native code so that they can access the necessary thread contexts
  regardless of the finalizer thread.

Use the bytecode execution timeout mechanism
--------------------------------------------

**XXX: Bytecode execution timeout not yet implemented in Duktape 1.0.**

The bytecode execution timeout mechanism allows a user callback to interact
with the bytecode executor to forcibly abort execution if a script has been
running for too long.  The mechanism relies on Duktape/C functions always
returning to the bytecode executor within a reasonable time so that the
execution timeout check can be done from time to time.  (Because there is only
one execution thread, the executor cannot interrupt on-going Duktape/C calls
otherwise.)

Duktape tries to place execution time and recursion depth limits on risky
internal operations.  For instance, there is a sanity limit on the number of
operations executed during regexp matching.  When these internal limits are
hit, a ``RangeError`` is thrown.  User code can catch such an error and
continue execution.  However, the error will return control to the bytecode
executor so that the execution timeout mechanism can kick in if necessary.

Use a fixed size memory pool for the sandbox
--------------------------------------------

You should usually restrict the amount of memory that the sandbox can use.

One common approach is to use a pooled memory allocator to impose a hard
limit on the memory available to the sandbox.  The memory can be split into
memory areas of a fixed size or a free-list based approach can be used.

Another approach is to use wrappers around standard ``malloc``, ``realloc``,
and ``free`` and keep track of total allocated memory.  One difficulty with
this is that ``realloc`` calls are not given the original allocation size
so you need to track that separately e.g. by prepending a small header to
every allocated memory block.

Review your C bindings for safety
---------------------------------

Review every C binding exposed to the sandbox.  There should be no way to
violate the safety goals through the C binding.  In particular:

* It shouldn't be possible to cause memory unsafe behavior.

* It shouldn't be possible to execute for an unreasonable amount of time
  within the C binding.

* It shouldn't be possible to access internal properties indirectly
  through the C binding.

Particular issues to look out for:

* Check typing of all arguments.  Avoid ``NULL`` pointers by using the
  "require" variants of getters (e.g. ``duk_require_lstring()`` instead
  of ``duk_require_string()``).

* Check every loop for termination.  Add a sanity termination limit if
  a loop is suspect.  Your goal is to return to the bytecode executor so
  that bytecode execution timeout can happen.

* When creating buffer values, avoid returning them to the caller and
  avoid using the buffer values e.g. as property lookup keys (which could
  accidentally access an internal property).

* When calling platform APIs, ensure they can never block indefinitely.

* Also ensure that native code doesn't compromise sandboxing goals at a
  higher level.  For instance, an API call must not allow sandboxed code
  to perform unauthenticated database writes or breach memory safety
  through file I/O on a Unix device file.
