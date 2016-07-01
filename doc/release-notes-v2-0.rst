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

To upgrade:

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

Built-in CommonJS module framework removed
------------------------------------------

The built-in CommonJS module loading framework consisting of ``require()``,
``Duktape.modSearch()`` and ``Duktape.modLoaded`` was removed; a module
framework isn't always needed, and when it is, it's difficult for a single
framework to match the very different use cases.

To upgrade:

* If you don't use the built-in module loading framework, no action is needed.

* If you do use the built-in module loading framework and want to continue
  using a module loader with Duktape 1.x semantics, you can include the
  following extra into your compilation: ``extras/module-duktape``.

* If you're upgrading, there are also other alternatives to module loading.
  For example, the ``extras/module-node`` module loader provides Node.js-like
  semantics with a more flexible module resolution and loading process.

Duktape.Logger, duk_log(), and duk_log_va() removed
---------------------------------------------------

The built-in logging framework consisting of ``Duktape.Logger``, ``duk_log()``,
and ``duk_log_va()`` were removed because they depended on stdout/stderr which
is a portability issue on some platforms.  The logging framework also didn't
always match user expectations: for some uses it was too simple (lacking e.g.
expressive backend configuration); for other uses it was too complex (too
high a ROM/RAM footprint for some embedded uses).  Sometimes an existing API
like ``console.log()`` was preferred while in other cases a platform specific
logging binding was more appropriate.

To upgrade:

* If you don't need ``Duktape.Logger`` or the C logging API calls, no action
  is needed.

* If you do need ``Duktape.Logger`` and/or the C logging API calls with
  Duktape 1.x semantics, you can include the following extra into your
  compilation: ``extras/logging``.

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

Duktape specific error codes removed from API
---------------------------------------------

Duktape specific error codes were removed from the public API and from
internals.  These error codes were not very widely used, and they didn't
have an Ecmascript counterpart (for example, a ``DUK_ERR_API_ERROR`` mapped
to a plain ``Error`` object) which was confusing.  The removed error codes
and defines are:

* ``DUK_ERR_UNIMPLEMENTED_ERROR`` / ``DUK_RET_UNIMPLEMENTED_ERROR``

* ``DUK_ERR_UNSUPPORTED_ERROR`` / ``DUK_RET_UNSUPPORTED_ERROR``

* ``DUK_ERR_INTERNAL_ERROR`` / ``DUK_RET_INTERNAL_ERROR``

* ``DUK_ERR_ALLOC_ERROR`` / ``DUK_RET_ALLOC_ERROR``

* ``DUK_ERR_ASSERTION_ERROR`` / ``DUK_RET_ASSERTION_ERROR``

* ``DUK_ERR_API_ERROR`` / ``DUK_RET_API_ERROR``

* ``DUK_ERR_UNCAUGHT_ERROR`` / ``DUK_RET_UNCAUGHT_ERROR``

Duktape API related errors were also changed to map to either a ``TypeError``
or ``RangeError`` instead of a plain ``Error``:

* A ``RangeError`` is used when an argument is out of bounds; for example:
  a value stack index is out of bounds, pop count is too large, not enough
  value stack items for call argument count.

* A ``TypeError`` is used when a value has incorrect type, and is thrown by
  for example ``duk_require_boolean()``.  ``TypeError`` is also typically
  used when nothing else applies.

To upgrade:

* If you use the custom error codes (``DUK_ERR_INTERNAL_ERROR`` etc) in your
  code, convert to using standard error codes (``DUK_ERR_TYPE_ERROR``, etc).

* If you depend on API errors mapping to a plain ``Error``, revise such code
  to accept also ``TypeError`` or ``RangeError``.  (In general depending on a
  specific error type should be only be done when it's absolute necessary.)

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

File I/O Duktape C API calls were removed
-----------------------------------------

Some platform don't have file I/O API calls (even ANSI), while on others they
are present but don't actually map to the file system (instead, a platform
specific API is used to access the actual file system).  Finally, there are
character encoding issues with ANSI C file I/O APIs e.g. on Windows, so that
the built-in file I/O support didn't always work as expected.

To improve portability, the following Duktape C API calls depending on
platform file I/O (fopen() etc) were removed (moved to extras):

* duk_push_string_file()

* duk_compile_file()

* duk_pcompile_file()

* duk_eval_file()

* duk_eval_file_noresult()

* duk_peval_file()

* duk_peval_file_noresult()

To upgrade:

* If you don't use these API calls, no action is needed.

* If you use these API calls you can e.g. implement a helper to push a file
  as a string (like ``duk_push_string_file()``) and then implement any needed
  compile/eval helpers based on that.

* Alternatively, you can include the following extra into your compilation:
  ``extras/duk-v1-compat``.  The extra provides Duktape 1.x compatible
  file-related API call bindings.

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

Debugger detached callback has a duk_context pointer argument
-------------------------------------------------------------

The debugger detached callback is allowed to immediately reattach the debugger
session.  However, the detached callback didn't have a ``duk_context *``
argument in Duktape 1.x so that the relevant context pointer needed to be passed
e.g. via the udata argument which is awkward.

In Duktape 2.x an explicit context argument was added::

    /* Duktape 1.x */
    typedef void (*duk_debug_detached_function) (void *udata);

    /* Duktape 2.x */
    typedef void (*duk_debug_detached_function) (duk_context *ctx, void *udata);

To upgrade:

* If you're using ``duk_debugger_attach()``, add an additional ``duk_context *``
  argument to the detached callback.

* If support for both Duktape 1.x and 2.x is desired, use::

      #if DUK_VERSION >= 20000
      void my_detached_cb(duk_context *ctx, void *udata) {
      #else
      void my_detached_cb(void *udata) {
      #end
          /* ... */
      }

Debugger print/alert and logger forwarding removed
--------------------------------------------------

Forwarding of ``print()``, ``alert()``, and log writes, enabled using config
options ``DUK_USE_DEBUGGER_FWD_PRINTALERT`` and ``DUK_USE_DEBUGGER_FWD_LOGGING``,
was removed as part of removing the bindings themselves.  Also debugger
notifications Print (0x02), Alert (0x03), Log (0x04) were deprecated.

To upgrade:

* No changes are needed, but print/alert and logger notification support can
  be removed from a debug client.

* If you rely on print/alert or logger forwarding in your debugger setup, you
  can add custom print/alert or logger forwarding by implementing print/alert
  or logging yourself and using AppNotify (``duk_debugger_notify()``) to
  forward print/alert or logger text.

Debug print config options changed
----------------------------------

Debug print related config options were reworked as follows:

* Debug prints no longer automatically go to ``stderr``.  Instead, an
  application must define ``DUK_USE_DEBUG_WRITE()`` in ``duk_config.h``
  when ``DUK_USE_DEBUG`` is enabled.  The macro is called to write debug log
  lines; there's no default provider to avoid platform I/O dependencies.
  Using a user-provided macro removes a dependency on platform I/O and also
  allows debug logs to be filtered and redirected in whatever manner is most
  useful for the application.  Example provider::

      #define DUK_USE_DEBUG_WRITE(level,file,line,func,msg) do { \
              fprintf(stderr, "D%ld %s:%ld (%s): %s\n", \
                      (long) (level), (file), (long) (line), (func), (msg)); \
          } while (0)

  See http://wiki.duktape.org/HowtoDebugPrints.html for more information.

* Debug level options ``DUK_USE_DPRINT``, ``DUK_USE_DDPRINT``, and
  ``DUK_DDDPRINT`` were replaced with a single config option
  ``DUK_USE_DEBUG_LEVEL`` with a numeric value:

  - 0 is minimal logging (matches ``DUK_USE_DPRINT``)

  - 1 is verbose logging (matches ``DUK_USE_DDPRINT``)

  - 2 is very verbose logging (matches ``DUK_USE_DDDPRINT``)

To upgrade:

* If you're not using debug prints, no action is needed.

* If you're using debug prints:

  - Add a ``DUK_USE_DEBUG_WRITE()`` to your ``duk_config.h``.  By itself it
    won't enable debug prints so it's safe to add even when debug prints are
    disabled.

  - Convert debug level options from ``DUK_USE_{D,DD,DDD}PRINT`` to the
    equivalent ``DUK_USE_DEBUG_LEVEL`` (0, 1, or 2).

Fatal error and panic handling reworked
---------------------------------------

The following changes were made to fatal error and panic handling:

* Fatal error function signature was simplied from::

      /* Duktape 1.x */
      void func(duk_context *ctx, duk_errcode_t code, const char *msg);

  to::

      /* Duktape 2.x */
      void func(void *udata, const char *msg);

  where the ``udata`` argument is the userdata argument given in heap creation.

* ``duk_fatal()`` error code argument was removed to match the signature
  change.

* The entire concept of "panic errors" was removed and replaced with calls to
  the fatal error mechanism.  There's a user-registered (optional) fatal error
  handler in heap creation, and a built-in default fatal error handler which
  is called if user code doesn't provide a fatal error handler.

  Some fatal errors, currently assertion failures, happen without a Duktape
  heap/thread context so that a user-registered handler cannot be called
  (there's no heap reference to look it up).  For these errors the default
  fatal error handler is always called, with the userdata argument as ``NULL``.
  The default fatal error handler can be replaced by editing ``duk_config.h``.

To upgrade:

* If you're not providing a fatal error handler nor using a custom panic
  handler, no action is needed -- however, providing a fatal error handler
  in heap creation is **strongly recommended**, see
  http://wiki.duktape.org/HowtoFatalErrors.html for instructions.

  The default fatal error handler will by default cause an intentional
  segfault; to improve this behavior define ``DUK_USE_FATAL_HANDLER()``
  in your ``duk_config.h``.

* If you have a fatal error handler, update its signature::

      /* Duktape 1.x */
      void my_fatal(duk_context *ctx, duk_errcode_t error_code, const char *msg) {
          /* ... */
      }

      /* Duktape 2.x */
      void my_fatal(void *udata, const char *msg) {
          /* ... */
      }

* If you're using ``duk_fatal()`` API calls, remove the error code argument::

      /* Duktape 1.x */
      duk_fatal(ctx, DUK_ERR_INTERNAL_ERROR, "assumption failed");

      /* Duktape 2.x */
      duk_fatal(ctx, "assumption failed");

* If you have a custom panic handler in your ``duk_config.h``, convert it to
  a default fatal error handler, also provided by ``duk_config.h``.  Both
  Duktape 1.x panic handler and Duktape 2.x default fatal error handler apply
  to all Duktape heaps (rather than a specific Duktape heap).

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
