Duktape threading
=================

Overview
========

This document describes native threading options that can be used
with Duktape.

The basic threading rules are as follows.

Only one active native thread at a time per Duktape heap
--------------------------------------------------------

A Duktape heap can only be accessed by one native thread at a time.  There
are no concurrency protection mechanisms inside Duktape so whenever two
threads call into Duktape, even when seemingly not interacting with the same
objects, the results are unpredictable and memory unsafe behavior may happen.

The simplest way to use multiple threads is for only one native thread to call
into Duktape API at a time.  If a call is made, e.g. using ``duk_call()``,
control flow is transferred to Duktape for the duration of the call.  Within
that call further ECMAScript and Duktape/C function calls can be made, and
multiple Duktape threads (coroutines, not to be confused with native
threads) can execute.  Once the initial call returns it's safe to call into
the Duktape API from another native thread, as long as there's some
application mechanism (e.g. a mutex) in place to ensure only one thread
does so.

A native thread can be suspended and resumed later
--------------------------------------------------

It's also possible for a Duktape/C call inside an active call stack to be
temporarily suspended using ``duk_suspend()`` and later resumed using
``duk_resume()``.  Between the two API calls the suspended native thread
must not make any other Duktape API calls.  The application must ensure
that no other thread is using Duktape when ``duk_resume()`` is called,
e.g. using a mutex.

The suspend/resume mechanism allows a blocking Duktape/C function to release
control of the Duktape heap so that other native threads can execute other
code within the same heap (also suspending if they encounter blocking
operations).

Multiple native threads can execute code on separate Duktape heaps
------------------------------------------------------------------

Multiple threads can use different Duktape heaps: each heap is entirely
independent.

Although Duktape itself is fully re-entrant, there are some situations
(discussed separately below) which can limit re-entrancy and prevent
running multiple threads at the same time, even when they run code in
separate Duktape heaps.

Platform limitations on re-entrancy
===================================

Lack of variadic preprocessor macros
------------------------------------

When variadic preprocessor macros are not available, Duktape currently
passes some fields through globals in that case.  This creates a race
condition where error information (``__FILE__`` and ``__LINE__``, for
instance) can be corrupted for errors thrown in different Duktape heaps.

This should not cause unsafe behavior but may corrupt error messages
or tracebacks.

.. note:: This limitation can be fixed by passing these values through
          duk_context (duk_hthread) instead of globals, but at the moment
          Duktape public API macros don't access duk_hthread directly.
          There is no longer a reason for this limitation because duktape.h
          sees the internal structures (even when compiling application
          code).  A fix is scheduled for Duktape 1.1 release.

Non-re-entrant system calls
---------------------------

Duktape uses some system calls which don't always have re-entrant variants
(or perhaps the re-entrant variants don't work).  This mainly impacts the
Date built-in, which uses ``gmtime_r()`` and ``localtime_r()`` on UNIX when
they are available, but falls back to ``gmtime()`` and ``localtime()`` if
the platform doesn't support them.

The impact on multithreading behavior depends on the non-re-entrant system
calls in question.

A few workarounds:

* Implement your own re-entrant native functions (e.g. date/time functions) for
  those not provided by your platform.  You'll need to change Duktape internals
  to make Duktape use the replacements.

* Replace the built-ins (such as ``Date``) entirely with a replacement
  written specifically for your platform.  This approach may allow you to
  avoid changes to Duktape internals as the non-re-entrant calls won't be
  used.
