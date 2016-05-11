=========================
Duktape 2.0 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* FIXME

The release has API incompatible changes, see upgrading notes below.

Upgrading from Duktape 1.5.x
============================

There are API incompatible changes in this release.  Whenever possible the
incompatible changes cause a compilation error (or warning) so that fixing
call sites should be straightforward.  Below are instructions on how to
migrate from 1.5.x to 2.0.0.  There are also bug fixes and other minor
behavioral changes which may affect some applications, see ``RELEASES.rst``
for details.

There are backwards compatible providers for some removed/modified API calls
in ``extras/duk-v1-compat``.

Supporting Duktape 1.x and Duktape 2.x simultaneously
-----------------------------------------------------

You can use the ``DUK_VERSION`` define to support both Duktape 1.x and 2.x
in the same application.  For example::

    #if (DUK_VERSION >= 20000)
    rc = duk_safe_call(ctx, my_safe_call, NULL, 1 /*nargs*/, 1 /*nrets*/);
    #else
    rc = duk_safe_call(ctx, my_safe_call, 1 /*nargs*/, 1 /*nrets*/);
    #endif

If you're developing against Duktape master before 2.x release, ``DUK_VERSION``
is set to 19999 so that you can use::

    #if (DUK_VERSION >= 19999)
    rc = duk_safe_call(ctx, my_safe_call, NULL, 1 /*nargs*/, 1 /*nrets*/);
    #else
    rc = duk_safe_call(ctx, my_safe_call, 1 /*nargs*/, 1 /*nrets*/);
    #endif

DUK_OPT_xxx feature option support removed
------------------------------------------

FIXME.

print() and alert() globals removed
-----------------------------------

The ``print()`` and ``alert()`` globals were removed because they depended on
stdout/stderr which is a portability issue on some platforms.  Further, even
if stdout/stderr is available, it's not always the appropriate place for debug
printouts, so it's cleaner if the application provides its own debug/console
logging functions.

To migrate:

* If you don't use ``print()`` or ``alert()`` no action is needed; they simply
  won't be a part of the global object anymore.

* If a simple ``print()`` and/or ``alert()`` suffices, you can use something
  like this::

      static duk_ret_t my_print(duk_context *ctx) {
          duk_push_string(ctx, " ");
          duk_insert(ctx, 0);
          duk_join(ctx, duk_get_top(ctx) - 1);
          fprintf(stdout, "%s\n", duk_to_string(ctx, -1));  /* 'stderr' for alert() */
          fflush(stdout);  /* may or may not want to flush, depends on application */
          return 0;
      }

      /* And after Duktape heap creation (or after each new thread with a
       * fresh global environment):
       */

      duk_push_c_function(ctx, my_print, DUK_VARARGS);
      duk_put_global_string(ctx, "print");

* If you do need ``print()`` and/or ``alert()`` with the Duktape 1.x
  semantics you can include the following extra into your compilation:
  ``extras/print-alert``.

duk_safe_call() userdata
------------------------

There's a new userdata argument for ``duk_safe_call()``::

    /* Duktape 1.x */
    typedef duk_ret_t (*duk_safe_call_function) (duk_context *ctx);
    duk_int_t duk_safe_call(duk_context *ctx, duk_safe_call_function func, duk_idx_t nargs, duk_idx_t nrets);

    /* Duktape 2.x */
    typedef duk_ret_t (*duk_safe_call_function) (duk_context *ctx, void *udata);
    duk_int_t duk_safe_call(duk_context *ctx, duk_safe_call_function func, void *udata, duk_idx_t nargs, duk_idx_t nrets);

The additional userdata argument makes it easier to pass a C pointer to the
safe-called function without the need to push a pointer onto the value stack.
Multiple C values can be passed by packing them into a stack-allocated struct
and passing a pointer to the struct as the userdata.

To upgrade:

* Add a userdata argument to duk_safe_call() call sites.  If no relevant
  userdata exists, pass a NULL.

* Add a userdata argument to safe call targets.  If no relevant userdata
  exists, just ignore the argument.

* If a call site needs to support both Duktape 1.x and Duktape 2.x, use
  a DUK_VERSION preprocessor check::

      #if (DUK_VERSION >= 20000)
      duk_ret_t my_safe_call(duk_context *ctx, void *udata) {
      #else
      duk_ret_t my_safe_call(duk_context *ctx) {
      #endif
          /* Ignore 'udata'. */
      }

      /* ... */

      #if (DUK_VERSION >= 20000)
      rc = duk_safe_call(ctx, my_safe_call, NULL, 1 /*nargs*/, 1 /*nrets*/);
      #else
      rc = duk_safe_call(ctx, my_safe_call, 1 /*nargs*/, 1 /*nrets*/);
      #endif

duk_dump_context_stdout() and duk_dump_context_stderr() removed
---------------------------------------------------------------

These two API calls were helpers based on ``duk_push_context_dump()`` which
would write the context dump directly to stdout/stderr.  Having a dependency
on stdout/stderr is a portability concern so the calls were removed in
Duktape 2.x.

To upgrade:

* Replace ``duk_dump_context_stdout()`` with an explicit call sequence like::

      duk_push_context_dump(ctx);
      printf("%s\n", duk_to_string(ctx, -1));
      duk_pop(ctx);

  Similarly for ``duk_dump_context_stderr()``.

* Alternatively, include extras/duk-v1-compat into your compilation to add back
  the removed API calls.

duk_debugger_attach() and duk_debugger_attach_custom() merged
-------------------------------------------------------------

The ``duk_debugger_attach_custom()`` API call in Duktape 1.x has been renamed
to ``duk_debugger_attach()`` to eliminate an unnecessary API call variant from
the public API.  The remaining debugger attach call always includes an
AppRequest callback argument.

To upgrade:

* ``duk_debugger_attach_custom()`` call sites: rename API call to
  ``duk_debugger_attach()``; no argument changes are needed.

* ``duk_debugger_attach()`` call sites: add a NULL ``request_cb`` callback
  argument.

* If a call site needs to support both Duktape 1.x and Duktape 2.x::

      /* Alternative #1: conditional call name. */
      #if (DUK_VERSION >= 20000)
          duk_debugger_attach(
      #else
          duk_debugger_attach_custom(
      #endif
              read_cb,
              write_cb,
              peek_cb,
              read_flush_cb,
              write_flush_cb,
              request_cb,  /* NULL OK if not necessary */
              detached_cb,
              udata);

      /* Alternative #2: conditional request_cb argument. */
          duk_debugger_attach(
              read_cb,
              write_cb,
              peek_cb,
              read_flush_cb,
              write_flush_cb,
      #if (DUK_VERSION >= 20000)
              request_cb,  /* NULL OK if not necessary */
      #endif
              detached_cb,
              udata);

Debug protocol version bumped from 1 to 2
-----------------------------------------

Because there are small incompatible changes in the debug protocol in this
release, the debug protocol version has been bumped from 1 to 2.  The version
is provided by the ``DUK_DEBUG_PROTOCOL_VERSION`` constant, and also appears
in the debug protocol version identification string.

To upgrade:

* Review the debug protocol changes and ensure debug client has corresponding
  changes.

* Update debug client code to support both versions 1 and 2, or version 2 only.

Debugger print/alert forwarding removed
---------------------------------------

Forwarding of ``print()`` and ``alert()`` calls, enabled using config option
``DUK_USE_DEBUGGER_FWD_PRINTALERT``, was removed as part of removing the calls
themselves.  Also debugger notifications Print (0x02) and Alert (0x03) were
deprecated.

To upgrade:

* No changes are needed, but print/alert notification support can be removed
  from a debug client.

* If you rely on print/alert forwarding in your debugger setup, you can add
  custom print/alert forwarding by implementing print and alert yourself and
  using AppNotify (``duk_debugger_notify()``) to forward print/alert text.

Known issues
============

FIXME.

Raw issues from test runs
=========================

API tests
---------

FIXME.

Ecmascript tests
----------------

FIXME.

test262
-------

FIXME.
