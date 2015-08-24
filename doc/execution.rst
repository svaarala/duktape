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

Execution states
================

There are three conceptual execution states for a Duktape heap:

* Idle

* Executing a Duktape/C function

* Executing an Ecmascript function

This conceptual model ignores details like heap initialization and
transitions from one state to another by "call handling".

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

The initial call into Duktape is always handled using ``duk_handle_call()``
which can handle a call from any state into any kind of target function.
Setting up a call involves a lot of state changes:

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
get handled by ``duk_handle_call()``.

If the target function is an Ecmascript function, the value stack is resized
for the function register count (nregs) established by the compiler during
function compilation; unlike Duktape/C functions the value stack is mostly
static for the duration of bytecode execution.  Opcode handling may push
temporaries on the value stack but they must always be popped off before
proceeding to dispatch the next opcode.

The bytecode executor has its own setjmp catchpoint.  If bytecode makes a
call into a Duktape/C function it is handled normally using ``duk_handle_call()``;
such calls may happen also when the bytecode executor uses the value stack API
for various coercions etc.

If bytecode makes a function call into an Ecmascript function it is handled
specially by ``duk_handle_ecma_call_setup()``.  This call handler sets up a
new activation similarly to ``duk_handle_call()``, but instead of doing a
recursive call into the bytecode executor it returns to the bytecode executor
which restarts execution and starts executing the call target without
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

The ``duk_handle_call()`` and ``duk_safe_call()`` catchpoints are only used to
handle ordinary error throws which propagate out of the calling function.  The
bytecode executor setjmp catchpoint handles a wider variety of longjmp call
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

* An ordinary throw is handled as in ``duk_handle_call()`` with the difference
  that there are both 'try' and 'finally' sites.

Returns, coroutine yields, and throws may propagate out of the initial bytecode
executor entry and outwards to whatever code called into the executor.

Managing current PC
-------------------

Managing the current bytecode pointer is critical for opcode dispatch
performance.  The current solution in Duktape 1.3 is to keep a direct
bytecode pointer in each activation (the pointer is stable) and keep a
cached copy of the topmost activation's bytecode pointer in
``thr->curr_pc`` which can be accessed quickly because ``thr`` is a
"hot" variable compared to the current activation.

For the most part the bytecode executor can keep on dispatching opcodes
using ``thr->curr_pc`` without copying the pointer back to the topmost
activation.  However, the pointer needs to be synced (copied back) when:

* The current activation changes, i.e. a new function call is made.

* When an error is about to be thrown, to ensure any longjmp handlers
  will see correct PC values.

* When the executor interrupt is entered; in particular, the debugger
  must see an up-to-date state.

* When a ``goto restart_execution;`` occurs in bytecode dispatch, which
  happens for multiple opcodes.

Syncing the pointer back unnecessarily or multiple times is safe, however.

This is a bit error prone, but it is worth the performance difference
of the alternatives (this method of dispatch improves dispatch performance
by about 20% over Duktape 1.2).

See separate section below on PC handling alternatives.

Alternatives for managing current PC
====================================

Various alternatives
--------------------

* Duktape 1.3: maintain a direct bytecode pointer in each activation, and
  a "cached" copy of the topmost activation's bytecode pointer in
  ``thr->curr_pc``.

* Duktape 1.2: maintain all PC values as numeric indices (not pointers and
  not pre-multiplied by bytecode opcode size).  The current PC is always
  looked up from the current activation.

* Use direct bytecode pointers in activations, keep a pointer to the current
  activation in the executor, and use ``act->curr_pc`` for dispatch.  This
  is faster than the Duktape 1.2 approach, but significantly slower than
  the Duktape 1.3 approach (part of that is probably because there's more
  register pressure).

* Track the current bytecode pointer as a local variable in the executor.
  Whenever something that might throw an error is executed, write the pointer
  back to the current activation.  This is similar to the Duktape 1.3 approach
  but the current bytecode pointer is in the C stack frame rather than
  ``thr->curr_pc`` so its address needs to be stored to e.g. ``thr`` so that
  it can be "plucked out" from the stack frame when needed.

Comparison between Duktape 1.2, Duktape 1.3, and act->curr_pc
-------------------------------------------------------------

The current Duktape 1.3 approach is a bit error prone because of the need to
sync the ``thr->curr_pc`` back to ``act->curr_pc`` in multiple code paths.
Another alternative would be to dispatch using ``act->curr_pc`` directly.
While that is faster than Duktape 1.3, it is significantly slower than
dispatching using ``thr->curr_pc``.

The measurements below are using ``gcc -O2`` on x64::

    # Duktape 1.2, dispatch using a numeric PC index
    $ sudo nice -20 python util/time_multi.py --count 10 --mode all --verbose ./duk.O2.master tests/perf/test-empty-loop.js
    Running: 3.100000 3.100000 3.120000 3.120000 3.160000 3.300000 3.370000 3.410000 3.370000 3.390000
    min=3.10, max=3.41, avg=3.24, count=10: [3.1, 3.1, 3.12, 3.12, 3.16, 3.3, 3.37, 3.41, 3.37, 3.39]

    # Duktape 1.3, dispatch using thr->curr_pc
    $ sudo nice -20 python util/time_multi.py --count 10 --mode all --verbose ./duk.O2.thr_pc tests/perf/test-empty-loop.js
    Running: 2.310000 2.330000 2.310000 2.300000 2.400000 2.290000 2.310000 2.290000 2.300000 2.300000
    min=2.29, max=2.40, avg=2.31, count=10: [2.31, 2.33, 2.31, 2.3, 2.4, 2.29, 2.31, 2.29, 2.3, 2.3]

    # Alternative; dispatch using act->curr_pc (no thr->curr_pc at all)
    $ sudo nice -20 python util/time_multi.py --count 10 --mode all --verbose ./duk.O2.act_pc tests/perf/test-empty-loop.js
    Running: 2.590000 2.580000 2.600000 2.600000 2.600000 2.660000 2.600000 2.640000 2.860000 2.860000
    min=2.58, max=2.86, avg=2.66, count=10: [2.59, 2.58, 2.6, 2.6, 2.6, 2.66, 2.6, 2.64, 2.86, 2.86]

Because the difference is rather large, the ``thr->curr_pc`` dispatch is
used in Duktape 1.3.
