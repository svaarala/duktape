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
untrusted ECMAScript code.  All C code is expected to be trusted and must be
carefully written with these sandboxing goals in mind.

This document describes best practices for Duktape sandboxing.

There's a YAML config file with some useful default options for sandboxing,
and comments on what options you might consider:

* ``config/examples/security_sensitive.yaml``

.. note:: This document described the current status of sandboxing features
          which is not yet a complete solution.

Suggested measures
==================

Isolation approaches
--------------------

There are two basic alternatives to sandboxing ECMAScript code with Duktape:

* Use a separate Duktape heap for each sandbox.

* Use a separate Duktape thread (with a separate global environment) for
  each sandbox.

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
  each sandbox; ROM built-ins can mitigate some of that effect.  If the memory
  pool for a heap is pre-allocated and not shared between heaps (in a thread
  safe manner), some of the pool will be unused.  If memory is not
  pre-allocated, actual memory usage is quite tightly bound because of
  reference counting.

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

* Only a single native thread can execute ECMAScript code at a time.

These two approaches can of course be mixed: you can have multiple heaps,
each with one or more sandboxed threads.

Define a native stack check macro (DUK_USE_NATIVE_STACK_CHECK)
--------------------------------------------------------------

This macro allows a sandbox environment to minimize chances of a native
stack overrun more accurately than plain stack depth limits (such as
DUK_USE_NATIVE_CALL_RECLIMIT).  See:

* Config option description:
  https://github.com/svaarala/duktape/blob/master/config/config-options/DUK_USE_NATIVE_STACK_CHECK.yaml

* Example stack check call in (duk_cmdline_stack_check):
  https://github.com/svaarala/duktape/blob/master/examples/cmdline/duk_cmdline.c

Disable verbose errors
----------------------

Verbose error messages may cause sandboxing security issues:

* When ``DUK_USE_PARANOID_ERRORS`` is not set, offending object/key is
  summarized in an error message of some rejected property operations.
  If object keys contain potentially sensitive information, you should
  enable this option.  Disable ``DUK_USE_PARANOID_ERRORS``.

* When stack traces are enabled an attacker may gain useful information from
  the stack traces.  Further, access to the internal ``_Tracedata`` property
  provides access to call chain functions even when references to them are not
  available directly.  Disable ``DUK_USE_TRACEBACKS``.

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
  be exposed directly or through a wrapper.  You can copy a reference to the
  ``Duktape`` object e.g. to the global stash which will then be accessible for
  C code.

* ``Duktape.act()`` provides access to calling functions which may matter to
  some sandboxing environments.

* ``Duktape.fin()`` provides access to setting and getting a finalizer.  Since
  a finalizer may run in a different thread than where it was created,
  finalizers are a sandboxing risk.  It's also possible to override or unset a
  finalizer which the sandbox relies on.

* Since Duktape 2.x buffer bindings no longer provide a way create hidden
  Symbols (called "internal strings" in Duktape 1.x) which allow access to
  internal properties.  See separate section on internal properties.

You should also:

* Remove the ``require`` module loading function in the global object
  (since Duktape 2.x it's no longer present by default).  If you need
  module loading in the sandbox, it's better to write a specific,
  constrained module loader for that environment.

Restrict access to internal properties
--------------------------------------

Internal properties are used by Duktape and user C code to store "hidden
properties" in objects.  The mechanism currently relies on "hidden Symbols"
(called "internal keys" or "internal strings" in Duktape 1.x).  These are
strings whose internal representation contains invalid UTF-8/CESU-8 data
(see ``doc/symbols.rst`` for description of the current formats).  Because
all standard ECMAScript strings are represented as CESU-8, such strings cannot
normally be created by ECMAScript code.  The properties are also never
enumerated or otherwise exposed to ECMAScript code (not even by
``Object.getOwnPropertySymbols()``) so that the only way to access them from
ECMAScript code is to have access to a hidden Symbol acting as the property key.

C code can create hidden Symbols very easily, which can provide a way to access
internal properties.  For example::

    // Assume an application native binding returns an internal key pushed
    // using duk_push_string(ctx, "\xff" "Value"):
    var key = getDangerousKey();

    // Access a Date instance's internal value, not normally accessible.
    print('Date internal value is:', new Date()[key]);

The risk in being able to access a certain internal property depends on the
internal property in question.  Some internal properties are non-writable and
non-configurable, so the sandboxed code can only read the property value; quite
often this is not an issue by itself.  If the value of an internal property can
be modified, concrete security issues may arise.  For instance, if an internal
property stores a raw pointer to a native handle (such as a ``FILE *``),
changing its value can lead to a potentially exploitable segfault.

Since Duktape 2.x ECMAScript code cannot create hidden Symbols using standard
ECMAScript code and the built-in bindings alone.  To prevent access to hidden
Symbols, ensure that no native bindings provided by the sandboxing environment
accidentally return such strings.  The easiest way to ensure this is to make
sure all strings pushed on the value stack are properly CESU-8 encoded.

It's also good practice to ensure that sandboxed code has minimal access to
objects with potentially dangerous properties like raw pointers.

.. note:: There's a future work issue, potentially included in Duktape 3.x,
          for preventing access to internal properties from ECMAScript code
          even when using the correct hidden Symbol as a lookup key.

Restrict access to function instances
-------------------------------------

In some environments giving the user code access to calling functions can be
dangerous.  For instance, if user code gets access to a function it cannot
normally see through bindings in the global object, it can call that function
and perhaps sidestep sandboxing.

Prevent access to function references in the call stack:

* Prevent access to ``Duktape.act()`` which provides programmatic access to
  the call stack and its function references.

* If ``DUK_USE_NONSTD_FUNC_CALLER_PROPERTY`` is enabled, the ``caller``
  property provides access to calling functions.  Don't use this option
  with sandboxing, or at least carefully control the ``caller`` property
  values seen by the sandboxed code.

* The ``_Tracedata`` internal property of error objects contains references
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
  thread they run, i.e. which global object they see.  It's also best
  practice to tolerate re-entry (although Duktape 1.4.0 and above has
  a guarantee of no re-entry unless object is rescued).

* For sandboxed environments it may be sensible to make all finalizers
  native code so that they can access the necessary thread contexts
  regardless of the finalizer thread.

Sanitize built-in prototype objects
-----------------------------------

Plain values inherit implicitly from built-in prototype objects.  For instance,
string values inherit from ``String.prototype``, which allows one to access
string methods with a plain base value::

    print("foo".toUpperCase());

Duktape uses the original built-in prototype functions in these inheritance
situations.  There is currently no way to replace these built-ins so that the
replacements would be used for instead (see
``test-dev-sandbox-prototype-limitation.js``).

As a result, sandboxed code will always have access to the built-in prototype
objects which participate in implicit inheritance:

* ``Boolean.prototype``: through plain booleans such as ``true``

* ``Number.prototype``: through numbers such as ``123``

* ``String.prototype``: through strings such as ``"foo"``

* ``Object.prototype``: through object literals such as ``{}``

* ``Array.prototype``: through array literals such as ``[]``

* ``Function.prototype``: through function expressions and declarations,
  such as ``function(){}``

* ``RegExp.prototype``: through RegExp literals such as ``/foo/``

* ``Error.prototype`` and all subclasses like ``URIError.prototype``:
  through explicit construction (if constructors visible) or implicitly
  through internal errors, e.g. ``/foo\123/`` which throws a SyntaxError

* ``Uint8Array.prototype``: through buffer values (if available); since
  there is no buffer literal, user cannot construct buffer values directly

* ``Duktape.Pointer.prototype`` through pointer values (if available); since
  there is no pointer literal, user cannot construct pointer values directly

It's not sufficient to avoid exposing these prototype objects in a replacement
global object: Duktape will use the original built-in prototype objects
regardless when dealing with plain value inheritance.  It is possible, however,
to delete individual properties of the prototype objects, e.g.::

    delete String.prototype.toUpperCase

This will cause the original example to fail::

    delete String.prototype.toUpperCase
    print("foo".toUpperCase());  // TypeError: call target not an object

Suggestions for sandboxing:

* Be aware that user code can access built-in prototypes through implicit
  inheritance through various plain values.

* Sanitize built-in prototype objects by deleting unnecessary methods.

**XXX: This will probably need improvement.  There may need to be API to
replace all built-in values.  They are kept in an internal array so perhaps
just exposing a primitive to set arbitrary values in the array would be
sufficient (though cryptic).  Some work in https://github.com/svaarala/duktape/pull/566.**

Use the bytecode execution timeout mechanism
--------------------------------------------

Duktape 1.1 added a simple bytecode execution timeout mechanism, see
``DUK_USE_EXEC_TIMEOUT_CHECK`` in http://wiki.duktape.org/ConfigOptions.html.

The mechanism and its limitations is described in a separate section below.

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
violate the safety goals through the C binding.  In particular, it shouldn't
be possible to:

* Cause memory unsafe behavior regardless of call arguments.

* Execute for an unreasonable amount of time.

* Access internal properties directly or indirectly.

* Push internal strings directly or indirectly.

Particular issues to look out for:

* Check typing of all arguments.  Avoid ``NULL`` pointers by using the
  "require" variants of getters (e.g. ``duk_require_lstring()`` instead
  of ``duk_require_string()``).

* Check every loop for termination.  Add a sanity termination limit if
  a loop is suspect.  Your goal is to return to the bytecode executor so
  that bytecode execution timeout can happen.

* When creating string values, ensure they're properly CESU-8 (or UTF-8)
  encoded.  This ensures internal strings, providing access to internal
  properties, are not created by accident.

* When calling platform APIs, ensure they can never block indefinitely.

* Also ensure that native code doesn't compromise sandboxing goals at a
  higher level.  For instance, an API call must not allow sandboxed code
  to perform unauthenticated database writes or breach memory safety
  through file I/O on a Unix device file.

Use bytecode dump/load carefully
--------------------------------

Because Duktape doesn't validate bytecode being loaded, loading invalid
bytecode may lead to memory unsafe behavior -- even exploitable
vulnerabilities.  To avoid such issues:

* Use bytecode dump/load only when it is really necessary e.g. for
  performance.  An alternative to bytecode dump/load is to compile
  on-the-fly which is usually not a performance bottleneck.  You can
  use e.g. minification to obfuscate code.

* Ensure bytecode being loaded has been compiled with the same Duktape
  version and same Duktape configuration options.  Major and minor versions
  must match; patch version may vary as bytecode format doesn't change in
  patch versions.

* Ensure integrity of bytecode being loaded e.g. by checksumming or signing.

* If bytecode is transported over the network or other unsafe media,
  use cryptographic means (keyed hashing, signatures, or similar) to
  ensure an attacker cannot cause crafted bytecode to be loaded.

Bytecode dump/load is only available through the C API, so there are
no direct sandboxing considerations for executing ECMAScript code.
However, if a Duktape/C function uses bytecode dump/load, ensure that
it doesn't accidentally expose the facility to ECMAScript code.

See ``bytecode.rst`` for more discussion on bytecode limitations and
best practices.

Bytecode execution timeout details
==================================

This section describes the bytecode execution timeout mechanism in detail,
and illustrates the limitations in the current Duktape 1.1 version of the
mechanism.

The current mechanism provides some protection against accidental errors
like infinite loops, but is not a reliable mechanism against deliberately
malicious code.

Current implementation
----------------------

* The bytecode executor calls the user callback whenever it goes into the
  bytecode executor interrupt handler.  The interval between interrupts
  varies from one bytecode instruction (e.g. when debugging) to several
  hundred thousand bytecode instructions (e.g. when running normally).

* When the user callback indicates a timeout the bytecode executor throws
  a ``RangeError``.  This error is propagated like any other error.

* ECMAScript code (try-catch-finally) may catch the error, but before a
  catch/finally clause actually executes, another ``RangeError`` is thrown
  by the bytecode executor.  The executor makes sure an execution interrupt
  happens before the catch/finally (or any other ECMAScript code) executes.
  For this approach to work, it's important that the user callback keeps
  indicating a timeout until the ``RangeError`` has fully bubbled through
  to the original protected call.

* Duktape/C functions can catch the error by using a protected call.
  They have a chance to clean up any native resources, with the limitation
  that if they make any ECMAScript calls, they will immediatelly throw
  a new ``RangeError``.  When a Duktape/C function returns control to Duktape,
  a ``RangeError`` is thrown as soon as ECMAScript code would be executed.

* ECMAScript finalizers are triggered but will always immediately throw a
  ``RangeError`` so they cannot be reliably used in case of execution timeouts.
  Duktape/C finalizers work normally; however, if they invoke the bytecode
  executor by running ECMAScript code, a ``RangeError`` is immediately thrown.

Using the mechanism from application code
-----------------------------------------

The concrete application code to use this mechanism can be e.g. as follows:

* Before entering untrusted code, record a start timestamp.  Then call the
  untrusted code using e.g. ``duk_pcall()``.

* On each execution timeout macro call, check if too much time has elapsed
  since the start timestamp.  If so, return 1.  Keep returning 1 until the
  original protected call exits.

* Once the protected call has exited, clear the execution timeout state.

The ``duk`` command line tool illustrates this approach.

Limitation: C code must not block during cleanup
------------------------------------------------

The timeout mechanism allows C code to clean up resources, e.g.::

    FILE *f = fopen("file.txt", "rb");

    ret = duk_pcall(ctx, 0 /*nargs*);
    /* ... */

    if (f) {
        fclose(f);
    }

This is a useful feature to allow C code to reliably free non-memory resources
not tracked by finalizers.  Finalizers can only be used, but are only executed
if they're Duktape/C functions: ECMAScript finalizers will immediately throw a
``RangeError`` because of the execution timeout.

C code must be careful to avoid entering an infinite loop (or blocking for an
unreasonable amount of time) to avoid subverting the timeout mechanism::

    ret = duk_pcall(ctx, 0 /*nargs*);
    /* ... */

    /* Infinite loop, prevents propagating RangeError outwards. */
    for(;;) {}

This limitation is not easy to fix because allowing C code to clean up is a
basic guarantee offered at the moment.

Limitation: timeout checks are only made when executing ECMAScript code
-----------------------------------------------------------------------

Execution timeout checks are only made by the bytecode executor, i.e. when
executing ECMAScript code.  No timeout checks are made when executing C code.
Any C code that goes into an infinite loop or blocks for an unreasonable
amount of time will essentially subvert the timeout mechanism.

Relevant C code includes:

* Application Duktape/C functions.

* Duktape internals, such as built-in functions, regexp compiler and executor,
  etc.

As an example, the following ECMAScript code would cause a Duktape internal
to run for a very long time::

    var a = []; a[1e9] = 'x';

    // Results in a huge string: [ null, null, ..., null, "x" ]
    var tmp = JSON.stringify();

Duktape places on internal sanity limit for some operations, such as regexp
execution taking too many steps.  When that happens a ``RangeError`` is
thrown.  Although user code can catch such an error, it returns control to
the executor so that the bytecode execution timeout can kick in if necessary.

However, not all internal algorithms are currently protected like this.
For instance, many Array built-ins can be abused to execute for a very
long time.

To fix this limitation quite a lot of work is needed.  Every built-in must
be made to cooperate with the execution timeout mechanism, either by applying
its own sanity timeout or by calling the user execution timeout callback to
see if it's time to abort.

Limitation: timeout check is made only every Nth bytecode instruction
---------------------------------------------------------------------

Execution timeout is only checked after every Nth bytecode instruction.
Technically, it is only checked when a Duktape executor interrupt happens,
which usually happens e.g. very few hundred thousand opcodes.  In special
cases like when a debugger is attached the interval can be much higher.

When doing heavy operations like matching regexps or some Array operations,
it may take very long (measured in wall clock time) for the opcode interval
to be triggered and a timeout be noticed.

Future work
-----------

* Add an API call for execution timeout instead of a macro.  The API timeout
  can be applied to the entire heap, or perhaps just a single call.

* Allow stacking of timeouts, so that some internal operation may apply a
  local timeout.

* Allow ECMAScript code to execute a function with a timeout.

* Better finalizer support, e.g. execute finalizers normally or avoid
  executing finalizers at all until the timeout error has been handled.
  This requires the ability to postpone finalizer execution, which is also
  a useful feature for timing sensitive environments.

* Improve built-ins so that they can cooperate with the timeout mechanism
  for operations that take a very long time (like regexp execution, some
  Array algorithms, etc).

* Allow user Duktape/C code to cooperate with the timeout mechanism in a
  similar fashion.

* Make timeout callback handling a bit more intelligent so that the callback
  is called e.g. when returning from a risky built-in (or perhaps any function
  call).
