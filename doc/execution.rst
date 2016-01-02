=========
Execution
=========

Overview
========

This document describes how Duktape manages its execution state.  Some details
are omitted but the goal is to give an overall picture how execution proceeds,
what state is involved, and what are the most important internal functions
involved.

The discussion is limited to a single Duktape heap as each Duktape heap is
independent of other Duktape heaps.  At any time, only one native thread may
be actively calling into a specific Duktape heap.

Execution states and state overview
===================================

There are three conceptual execution states for a Duktape heap:

* Idle

* Executing a Duktape/C function

* Executing an Ecmascript function

This conceptual model ignores details like heap initialization and
transitions from one state to another by "call handling".

Execution state is contained mostly in three stacks:

* Call stack: used to track function calls

* Catch stack: used to track try-catch-finally and other catchpoints specific
  to the bytecode executor

* Value stack: contains the tagged values manipulated through the Duktape API
  and in the bytecode executor

In addition to these there are execution control variables in ``duk_hthread``
and ``duk_heap``.

Typical control flow
====================

Execution always begins from an idle state where no calls into Duktape are
active and user application has control.  User code may manipulate the value
stacks of Duktape contexts in this state without making any calls.  User code
may also call ``duk_debugger_cooperate()`` for integrating debugger into the
application event loop (or equivalent).

Eventually user code makes a call into either a Duktape/C function or an
Ecmascript function.  Such a call may be caused by an obvious API call like
``duk_pcall()``.  It may also be caused by a less obvious API call such as
``duk_get_prop()``, which may invoke a getter, or ``duk_to_string()`` which
may invoke a ``toString()`` coercion method.

The initial call into Duktape is usually handled using
``duk_handle_call_(un)protected()`` which can handle a call from any state
into any kind of target function.  Setting up a call involves a lot of state
changes:

* A setjmp catchpoint is needed for protected calls.

* The call stack is resized if necessary, and an activation record
  (``duk_activation``) is set up for the new call.

* The value stack is resized if necessary, and a fresh value stack frame
  is established for the call.  The calling value stack frame and the target
  frame overlap for the call arguments, so that arguments on top of the
  calling stack are directly visible on the bottom of the target stack.

* An arguments object and an explicit environment record is created if
  necessary.

* Other small book-keeping (such as recursion depth tracking) is done.

When a call returns, the state changes are reversed before returning to
the caller.  If an error occurs during the call, a ``longjmp()`` will take
place and will be caught by the current (innermost) setjmp catchpoint
without tearing down the call state; the catchpoint will have to do that.

If the target function is a Duktape/C function, the corresponding C function
is looked up and called.  The C function now has access to a fresh value stack
frame it can operate on using the Duktape API.  It can make further calls which
get handled by ``duk_handle_call_(un)protected()``.

If the target function is an Ecmascript function, the value stack is resized
for the function register count (nregs) established by the compiler during
function compilation; unlike Duktape/C functions the value stack is mostly
static for the duration of bytecode execution.  Opcode handling may push
temporaries on the value stack but they must always be popped off before
proceeding to dispatch the next opcode.

The bytecode executor has its own setjmp catchpoint.  If bytecode makes a
call into a Duktape/C function it is handled normally using
``duk_handle_call_(un)protected()``; such calls may happen also when the
bytecode executor uses the value stack API for various coercions etc.

If bytecode makes a function call into an Ecmascript function it is handled
specially by ``duk_handle_ecma_call_setup()``.  This call handler sets up a
new activation similarly to ``duk_handle_call_(un)protected()``, but instead
of doing a recursive call into the bytecode executor it returns to the bytecode
executor which restarts execution and starts executing the call target without
increasing C stack depth.  The call handler also supports tail calls where an
activation record is reused.

Both Duktape and user code may use ``duk_safe_call()`` to make protected
calls inside the current activation (or outside of any activations in the
idle state).  A safe call creates a new setjmp catchpoint but not a new
activation, so safe calls are not actual function calls.

Threading limitations
=====================

Only one native thread may call into a Duktape heap at any given time.
See ``threading.rst`` for more details.

Bytecode executor
=================

Basic functionality
-------------------

* Setjmp catchpoint which supports yield, resume, slow returns, try-catch, etc

* Opcode dispatch loop, central for performance

* Executor interrupt which facilitates script timeout and debugger integration

* Debugger support; breakpoint handling, checked and normal execution modes

Setjmp catchpoint
-----------------

The ``duk_handle_call_protected()`` and ``duk_safe_call()`` catchpoints are only
used to handle ordinary error throws which propagate out of the calling function.
The bytecode executor setjmp catchpoint handles a wider variety of longjmp call
types, and in many cases the longjmp may be handled without exiting the current
function:

* A slow break/continue uses a longjmp() so that if the break/continue crosses
  any finally clauses, they get executed as expected.  Similarly 'with' statement
  lexical environments are torn down, etc.

* A slow return uses a longjmp() so that any finally clauses, 'with' statement
  lexical environments, etc are handled appropriately.

* A coroutine resume is handled using longjmp(): the Duktape.Thread.resume()
  call adjusts the thread states (including their activations) and then uses
  this longjmp() type to restart execution in the target coroutine.

* A coroutine yield is handled using longjmp(): the Duktape.Thread.yield()
  call adjusts the states and uses this longjmp() type to restart execution
  in the target coroutine.

* An ordinary throw is handled as in ``duk_handle_call_protected()`` with the
  difference that there are both 'try' and 'finally' sites.

Returns, coroutine yields, and throws may propagate out of the initial bytecode
executor entry and outwards to whatever code called into the executor.

Opcode dispatch loop and executor interrupt
-------------------------------------------

The opcode dispatch loop is a central performance critical part of the
executor.  The dispatch loop:

* Checks for an executor interrupt.  An interrupt can be taken for every
  opcode or for every N instructions; the interrupt handler provides e.g.
  script timeout and debugger integration.  This is performance critical
  because the check occurs for every opcode dispatch.  See separate section
  below on interrupt counter handling.

* Fetches an instruction from the topmost activation's "current PC",
  and increments the PC.  Managing the "current PC" is performance critical.
  See separate section below on current PC handling.

* Decodes and executes the opcode using a large switch-case.  The most
  important opcodes are in the main opcode space (64 opcodes); more rarely
  used opcodes are "extra" opcodes and need a double dispatch.

* Usually loops back to execute further opcodes.  May also (1) call another
  Duktape/C or Ecmascript function, (2) cause a longjmp, or (3) use
  ``goto restart_execution`` to restart the executor e.g. after call stack
  has been changed.

Debugger support
----------------

Debugger support relies on:

* Executor interrupt mechanism is needed to support debugging.

* A precheck in ``restart_execution`` where debugging status and breakpoints
  are checked.  Executor then either proceeds in "normal" or "checked"
  execution.  Checked execution means running one opcode at a time, and
  calling into the interrupt handler before each to see e.g. if a breakpoint
  has been triggered.

* There's some additional support outside the executor, e.g. call stack
  unwinding code handles the "step out" logic.

See ``debugger.rst`` for details.

Call processing: duk_handle_call_(un)protected()
================================================

Call setup
----------

When handling a call, ``duk_handle_call_(un)protected()`` is given
``num_stack_args`` which indicates how many arguments have been pushed
on the current stack for the call.  The stack frame of the calling
activation looks as follows::

      top - num_stack_args - 2
           |
           |          top - num_stack_args
           |               |
           v               v
  +-----+------+--------+------+-----+------+
  | ... | func | 'this' | arg0 | ... | argN | <- top
  +-----+------+--------+------+-----+------+

To prepare the stack frame for the called function,
``duk_handle_call_(un)protected()`` does the following:

* If ``func`` is a bound function, follows the bound function chain until
  a non-bound function is found.  While following the chain, the requested
  ``this`` binding may be updated by the bound function, and arguments may be
  prepended at the ``arg0`` point.

* Coerces the ``this`` binding as specified in E5.  The ``this`` in the calling
  stack frame is the caller requested ``this`` binding.  For instance, for a
  property-based call (e.g. ``obj.method()``) this is the base object.  The
  effective ``this`` binding may be coerced (for non-strict target functions)
  or replaced during bound function handling.

* Resolves the difference between arguments requested (target function
  ``nargs``) and provided (``num_stack_args``) by filling in missing arguments
  with ``undefined`` or discarding extra arguments so that exactly ``nargs``
  arguments are present.  (Special handling is needed for vararg functions
  where ``nargs`` indicates ``num_stack_args`` arguments are used as is.)

* Finalizes the value stack "top":

  - For Duktape/C target functions the top is set to ``nargs`` (or
    ``num_stack_args`` for vararg functions).

  - For Ecmascript target functions the top is first set to ``nargs``, wiping
    any values above that, and then extended to ``nregs``.  Values above
    ``nargs`` are filled with ``undefined``.  At the end the value stack frame
    has ``nregs`` allocated and initialized entries, with ``[0, nargs-1]``
    mapping to call arguments.

* Creates a new lexical scope object if necessary; this step is postponed
  when possible and done lazily only when actually necessary.

* Creates a new activation, and switches the valstack bottom to the first
  argument.

The value stack looks as follows after call setup is complete and the new
function is ready to execute (the example is for an Ecmascript target
function)::

     (-1)     0      1          nargs-1                   nregs - 1
  +--------+------+------+-----+------+-----------+-----+-----------+
  | 'this' | arg0 | arg1 | ... | argM | undefined | ... | undefined | <- top
  +--------+------+------+-----+------+-----------+-----+-----------+

The effective ``this`` binding for the function is always stashed right below
the active value stack frame.  This interacts well with the calling convention
where the requested ``this`` binding can be coerced in-place nicely, and the
``this`` binding can also be accessed quickly.

When doing tail calls, no stacks (value stack, call stack, catch stack) may
grow in size; otherwise the point of cail talls would be defeated.  This is
ensured as follows:

* The value stack is manipulated so that the callee's first argument (``arg0``)
  will be placed in the current activation's index 0 (value stack bottom).
  The effective ``this`` binding is overwritten just below the current
  activation's value stack bottom.

* The call stack does not grow by virtue of reusing the current activation.

* The catch stack does not grow because the Ecmascript compiler never emits
  a tailcall if there is a catch stack; tail calls are not possible if a
  catch stack exists, because e.g. ``try`` and ``finally`` must be processable.
  Hence, ``duk_handle_call_(un)protected()`` simply asserts for this condition.

Call cleanup after a successful call
------------------------------------

The C return value of the called Duktape/C function indicates how many return
values are on the value stack, with negative values indicating an error which
is thrown by call handling (this is a shorthand for throwing errors).

To clean up after a call:

* The call stack and catch stack are unwound, and a best effort shrink check
  is done.  If shrinking is attempted and it fails, the error is ignored.

* The value stack is restored to the caller's configuration.  The return value
  is moved into its expected position (same as ``func`` on the input stack).
  Value stack top is configured so that the return value is at the stack top
  (for Duktape/C callers) or so that the stack top is at ``nregs`` (for
  Ecmascript callers).  A value stack shrink (or grow) check is done; shrink
  errors should be ignored silently.

* Other book-keeping variables are restored to their entry values, e.g.:
  call recursion depth, bytecode executor instruction pointer, thread state,
  current thread, etc.

Call cleanup after a failed call
--------------------------------

When an error is thrown it is caught by the nearest ``setjmp`` catch point.
If that catch point is in ``duk_handle_call_protected()`` the processing is
quite similar to success handling except that multiple call stack and catch
stack frames are potentially unwound:

* Restore the previous ``setjmp`` catchpoint so that any errors thrown during
  call cleanup are propagated outwards to avoid recursion into the same
  handler.  Note, however, that the error handling code path should never
  actually throw further errors -- doing so would break protected call
  semantics.

* The call stack and catch stack are unwound, and a best effort shrink check
  is done.

* The value stack is configured as for successful calls, except that the error
  thrown is left on the value stack instead of a return value.

* Other book-keeping variables are restored to their entry values.

If there's no catcher for the error the uncaught error causes the fatal error
handler to be called.  None of the stacks are unwound, and since the entry
values for various book-keeping variables are lost, there's no way to properly
unwind the call state afterwards.  This is OK because fatal errors are not
recoverable and there's no way to resume execution if a fatal error occurs.
It should be possible to free the Duktape heap normally but this is of little
use because it's not safe to continue execution after a fatal error in general.

Managing heap->curr_thread
--------------------------

The current thread is managed in several places:

* Call handling saves and restores ``heap->curr_thread`` whose previous value
  may be different from the call thread when an initial call is made, i.e.
  previous value is ``NULL``.

* Bytecode executor longjmp handler ultimately handles each coroutine resume
  and yield operation.  The longjmp handler will update ``heap->curr_thread``
  as a resume enters a thread and when a yield exits a thread.

* As a result, the setjmp catch point of ordinary call handling doesn't need
  to unwind multiple levels of resumers: it just needs to restore the previous
  value in case it was ``NULL``.

Current limitations in call cleanup
-----------------------------------

As of Duktape 1.4.0 the error handling path is not completely free of errors
in out-of-memory situations:

* Value stack may need to be grown during call cleanup.  This will be fixed
  so that value stack is never shrunk in call setup so that there's no need
  to grow it in cleanup.

* Unwinding activations causes lexical scope objects to be allocated which
  may fail and propagate an error from error handling.  This needs to be fixed
  e.g. so that the scope object is preallocated, see: https://github.com/svaarala/duktape/issues/476.

Misc notes
----------

* The value stack doesn't hold all the internal state relevant for an
  activation.  Some state, such as active environment records (``lex_env``
  and ``var_env``) are held in the ``duk_activation`` call stack structure.

Value stack management
======================

One value stack per thread
--------------------------

A thread has a single value stack, essentially an array of tagged values,
which is shared by the activations in the call stack.  Each activation has
a set of registers indexed relative to "frame bottom", starting from zero,
mapped to the range [regbase, regtop[ in the value stack.  The register ranges
of activations may and often do overlap (see call handling discussion).
For instance, function call arguments prepared by the caller are used directly
by the callee.

The value stack can be thought of as follows::

  size ->    _
            : :    [0,size[    allocated range
            : :    [top,size[  allocated, initialized to undefined, ignored by GC
            : :    [0,top[     active range, must be initialized for GC
  top ->    :_:
            ! ! -.
            ! !  !-- current activation
            ! !  !
  bottom -> !_! -'
            ! !
            ! !
            ! !
            ! !
  0 ->      !_!

There are several possible policies for values above "top".  The current
policy is based on concrete performance measurements, and is as follows:

* Values above "top" are not considered reachable to GC.

* Values above "top" are initialized to "undefined" (DUK_TAG_UNDEFINED).
  Whenever the "top" is decreased, previous values are set to undefined.

Overlap between activations
---------------------------

Example of value stack overlap for two Ecmascript activations during a
function call::

  size ->    _
            : :    [0,size[    allocated range
            : :    [top,size[  allocated, initialized to undefined, ignored by GC
            : :    [0,top[     active range, must be initialized for GC
  top ->    :_:
            !=! -.
            !=!  !
            !=!  !-- activation 2
            !#!  !  -.
  bottom -> !#! -'   !-- activation 1
            !:!      !
            !:!     -'
            ! !
  0 ->      !_!

The callee's activation (activation 2 in the figure) may also be smaller
than the caller's activation::

  size ->    _
            : :    [0,size[    allocated range
            : :    [top,size[  allocated, initialized to undefined, ignored by GC
            : :    [0,top[     active range, must be initialized for GC
            : :
            : :
            ::: -.
            :::  !-- activation 1
  top ->    :::  !
            !#!  !  -.
            !#!  !   !-- activation 2
  bottom -> !#!  !  -'
            !:!  !
            !:! -'
            ! !
  0 ->      !_!

When the callee returns, call handling will restore the value stack frame
to the size expected by the caller.  Values above the entries used for
call handling will be reinitialized to ``undefined``.

Call handling will also ensure that the reserved size for the value stack
never decreases as a result of the call, even if the caller has a much
smaller value stack frame.  This is important for the value stack size
guarantees provided by e.g. ``duk_require_stack()``.

Note that there is nothing in the value stack model or the execution model
in general which requires activations to share registers for parameter
passing.  It is just a convenient thing to do especially for
Ecmascript-to-Ecmascript calls: it minimizes value stack growth, minimizes
unnecessary copying of arguments (which is pointless because the caller will
never rely on the argument values after a call anyway).

When an Ecmascript function with a very large value stack frame calls
a function with a very small value stack frame, a lot of value stack
resize / wipe mechanics will happen.  It might be useful to avoid the
register overlap in such cases to improve performance.

Growing and shrinking
---------------------

The value stack allocation size grows and shrinks as required by the active
range, which changes e.g. during function calls.  Some hysteresis is applied
to minimize memory allocation activity when the value stack changes active
size.  Note that when the value stack grows or shrinks, it is reallocated and
its base pointer may change, which invalidates any outstanding pointers to
values in the stack.  For this reason, all persistent execution state refers
to registers and value stack entries by index, not by memory pointer.

Whenever there is a risk of a garbage collector run (either directly or
indirectly through an error, a finalizer run, etc) all the entries in the
[0,top[ range of the value stack must be initialized and correctly reference
counted: all active ranges of reachable threads are considered GC roots.  The
compiler and the executor should wipe any unused value stack entries as soon
as the values are no longer needed: otherwise the values will be reachable
for the GC and will prevent garbage collection.  This is easy to do e.g.
when a function call returns (just wipe the entire range of registers used
by the function) but is more difficult for a function which runs forever.

When Ecmascript functions are compiled, the compiler keeps track of how many
registers are needed by the opcodes comprising the compiled bytecode, and
this value is stored in the ``nregs`` entry of a compiled function.  While
the Ecmascript function is executing, we know that *all* register accesses
will be to valid and initialized parts of the value stack, so no grow/shrink
or other sanity checks are necessary while the function is executing.  This
does not mean that all the ``nregs`` will always be used, and any unused
registers at the top of the activation record's register range can be reused
during e.g. function calls.

The value stack is handled quite differently for C functions, which use a
traditional stack model (this is similar to how Lua manages its value stack).
Value stack grow/shrink checks are needed whenever pushing and popping values,
and the number of value stack entries needed is not known beforehand.
Arguments to C functions are placed on top of the initial C activation record
(starting from register 0).  A possible return value is left by the C code at
the top of the stack, not necessarily at position 0.  The return value of the
C function indicates whether a return value is intended or not; if not, the
return value defaults to ``undefined``.

Managing executor interrupt
===========================

The executor interrupt counter is currently tracked in
``thr->interrupt_counter``.  This seems to work well because ``thr`` is a
"hot" variable.

Another alternative would be to track the counter in an executor local
variable.  Error handling and other code paths jumping out of the executor
need to work similarly to how stack local ``curr_pc`` is handled.

Managing current PC
===================

Current approach
----------------

The current solution in Duktape 1.3 is to maintain a direct bytecode pointer
in each activation, and to keep a "cached copy" of the topmost activation's
bytecode pointer in a bytecode executor local variable ``curr_pc``.  A pointer
to the ``curr_pc`` in the stack frame (whose type is ``duk_instr_t **``) is
stored in ``thr->ptr_curr_pc`` so that when control exits the opcode dispatch
loop (e.g. when an error is thrown) the value in the stack frame can be read
and synced back into the topmost activation's ``act->curr_pc``.

Consistency depends on the compiler doing correct aliasing analysis, and
writing back the ``curr_pc`` value to the stack frame before any operation
that may potentially read it through ``thr->ptr_curr_pc``.  Using ``volatile``
would be safer but in practical testing it eliminates the performance benefit
entirely.

For the most part the bytecode executor can keep on dispatching opcodes
using ``curr_pc`` without copying the pointer back to the topmost activation.
Careful management of ``curr_pc`` and ``thr->ptr_curr_pc`` are needed in the
following situations:

* Call handling must (1) store/restore the current ``thr->ptr_curr_pc`` value,
  (2) sync the ``curr_pc`` if ``thr->ptr_curr_pc`` is non-NULL, (3) set the
  ``thr->ptr_curr_pc`` to NULL to avoid any code using it with an incorrect
  activation (not matching what ``curr_pc`` was initialized from).  This
  ensures that any side effects in the executor, such as DECREF causing a
  finalizer call or a property read causing a getter call, are handled
  correctly without the executor syncing the ``curr_pc`` at every turn.  This
  is quite important because there are a lot of potential side effects in the
  executor opcode loop.

* If any code depends on ``duk_activation`` structs (``act->curr_pc`` in
  particular) being correct, ``curr_pc`` must be synced back.  For example:
  executor interrupt, debugger handling, and error augmentation need to see
  synced state.

* The ``curr_pc`` must be synced back **and** ``thr->ptr_curr_pc`` must be
  NULLed before a longjmp that (potentially) causes a call stack unwind.
  The NULLing is important because **any** call stack unwind may have side
  effects due to e.g. finalizers for values in the unwound call stack being
  called.  If ``thr->ptr_curr_pc`` was still set at that time, call handling
  would sync ``curr_pc`` to the topmost activation, which wouldn't be the
  same activation as intended.

* NULLing of ``thr->ptr_curr_pc`` is also required for longjmps which are
  purely internal to the bytecode executor.  This is important because the
  seemingly internal longjmps may propagate outwards, may cause side effects,
  etc, all of which demand that ``thr->ptr_curr_pc`` be NULL at the time.
  Once the longjmp has been handled, the executor should reinitialize
  ``thr->ptr_curr_pc`` if bytecode execution resumes.

* Whenever the bytecode executor does a ``goto restart_execution;`` the
  ``curr_pc`` must be synced back even if the activation hasn't changed:
  the restart code will look up the topmost activation's ``act->curr_pc``
  which must be up to date.

Syncing the pointer back unnecessarily or multiple times is safe in general,
so there's no need to ensure there's exactly one sync for a certain code path.

Function bytecode is behind a stable pointer, so there are no realloc or
other side effect concerns with using direct bytecode pointers.  Because
the function being executed is always reachable, a borrowed pointer can
be used.

This approach is error prone, but it is worth the performance difference of
the alternatives.  This method of dispatch improves dispatch performance by
about 20-25% over Duktape 1.2.

Some alternatives
-----------------

* Duktape 1.3: maintain a direct bytecode pointer in each activation, and a
  "cached" copy of the topmost activation's bytecode pointer in a local
  variable of the executor.  Whenever something that might throw an error
  is executed, write the pointer back to the current activation using
  ``thr->ptr_curr_pc`` which points to the stack frame location containing
  ``curr_pc``.

* Duktape 1.2: maintain all PC values as numeric indices (not pointers and
  not pre-multiplied by bytecode opcode size).  The current PC is always
  looked up from the current activation.

* Same as Duktape 1.3 behavior but maintain a cached copy of the topmost
  activation's bytecode pointer in ``thr->curr_pc``.  The copy back operation
  is needed but doesn't need to peek into the bytecode executor stack frame.
  This works quite well because ``thr`` is a "hot" variable.  However, the
  stack local ``curr_pc`` used in Duktape 1.3 is faster.

* Use direct bytecode pointers in activations, keep a pointer to the current
  activation in the executor, and use ``act->curr_pc`` for dispatch.  There's
  no need for a copy back operation because activation states are always in
  sync.  This is faster than the Duktape 1.2 approach, but significantly
  slower than the ``thr->curr_pc`` or the Duktape 1.3 approach (part of that
  is probably because there's more register pressure).

Comparison between curr_pc alternatives
---------------------------------------

The current Duktape 1.3 approach is a bit error prone because of the need to
sync the executor local ``curr_pc`` back to ``act->curr_pc`` in multiple code
paths.  Another alternative would be to dispatch using ``act->curr_pc``
directly.  While that is faster than Duktape 1.2, it is significantly slower
than dispatching using executor local ``curr_pc`` (or ``thr->curr_pc``).

The measurements below are using ``gcc -O2`` on x64::

    # Duktape 1.3, dispatch using executor local variable curr_pc
    $ sudo nice -20 python util/time_multi.py --count 10 --mode all --verbose ./duk.O2.local_pc tests/perf/test-empty-loop.js
    Running: 2.180000 2.170000 2.180000 2.290000 2.180000 2.200000 2.190000 2.190000 2.220000 2.200000
    min=2.17, max=2.29, avg=2.20, count=10: [2.18, 2.17, 2.18, 2.29, 2.18, 2.2, 2.19, 2.19, 2.22, 2.2]

    # Duktape 1.2, dispatch using a numeric PC index
    $ sudo nice -20 python util/time_multi.py --count 10 --mode all --verbose ./duk.O2.123 tests/perf/test-empty-loop.js
    Running: 3.100000 3.100000 3.120000 3.120000 3.160000 3.300000 3.370000 3.410000 3.370000 3.390000
    min=3.10, max=3.41, avg=3.24, count=10: [3.1, 3.1, 3.12, 3.12, 3.16, 3.3, 3.37, 3.41, 3.37, 3.39]

    # Alternative; dispatch using thr->curr_pc
    $ sudo nice -20 python util/time_multi.py --count 10 --mode all --verbose ./duk.O2.thr_pc tests/perf/test-empty-loop.js
    Running: 2.310000 2.330000 2.310000 2.300000 2.400000 2.290000 2.310000 2.290000 2.300000 2.300000
    min=2.29, max=2.40, avg=2.31, count=10: [2.31, 2.33, 2.31, 2.3, 2.4, 2.29, 2.31, 2.29, 2.3, 2.3]

    # Alternative; dispatch using act->curr_pc
    $ sudo nice -20 python util/time_multi.py --count 10 --mode all --verbose ./duk.O2.act_pc tests/perf/test-empty-loop.js
    Running: 2.590000 2.580000 2.600000 2.600000 2.600000 2.660000 2.600000 2.640000 2.860000 2.860000
    min=2.58, max=2.86, avg=2.66, count=10: [2.59, 2.58, 2.6, 2.6, 2.6, 2.66, 2.6, 2.64, 2.86, 2.86]

Accessing constants
===================

The executor stores a copy of the ``duk_hcompiledfunction`` constant table
base address into a local variable ``consts``.  This reduces code footprint
and performs better compared to reading the consts base address on-the-fly
through the function reference.  Because the constants table has a stable
base address, this is easy and safe.

Accessing registers
===================

The executor currently accesses the stack frame base address (needed to read
registers) through ``thr`` as ``thr->valstack_bottom``.  This is reasonably
OK because ``thr`` is a "hot" variable.

The register base address could also be copied to a local variable as is done
for constants.  However, ``thr->valstack_bottom`` is not a stable address and
may be changed by any side effect (because any side effect can cause a value
stack resize, e.g. if a finalizer is invoked).

If a local variable were to be used, it would need to be updated when the
value stack is resized.  It's not certain if overall performance would be
improved.  This was postponed to Duktape 1.4:

* https://github.com/svaarala/duktape/issues/298
