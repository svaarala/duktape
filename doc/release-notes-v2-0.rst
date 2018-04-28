=========================
Duktape 2.0 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* New tools/configure.py frontend tool replaces genconfig.py for configuring
  and preparing Duktape sources for build.

* Buffer handling has been simplified: Duktape.Buffer has been removed and is
  replaced by Uint8Array, plain buffers now behave like Uint8Array objects.
  Node.js Buffer behavior aligned with more recent Node.js Buffer API.

* Implement more ES2015 and ES2016 functionality, and align some ES5.1
  semantics with ES2015/ES2016.  Implement WHATWG Encoding API with
  TextEncoder() and TextDecoder() bindings.

* Some incompatible API changes, and several API additions.  API and config
  changes to avoid I/O dependencies (such as printf() and fopen()) in core
  Duktape code to simplify porting.

* More configuration flexibility in dropping Duktape specific functionality
  from build, e.g. coroutines and finalization.

* Disabled ECMAScript bindings are no longer present (instead of being present
  but throwing a TypeError).

* Built-in functionality moved to optional extras: print/alert bindings,
  logging, and module loader.  New optional extras include a Node.js-like
  module loader and a 'console' binding.

* Bug fixes, performance and footprint improvements.

The release has API incompatible changes, see upgrading notes below.

Upgrading from Duktape 1.x
==========================

There are API incompatible changes in this release.  Whenever possible the
incompatible changes cause a compilation error (or warning) so that fixing
call sites should be straightforward.  Below are instructions on how to
migrate from 1.x to 2.0.0.  There are also bug fixes and other minor
behavioral changes which may affect some applications, see ``RELEASES.rst``
for details.

There are backwards compatible providers for some removed/modified API calls
in ``extras/duk-v1-compat``.

Supporting Duktape 1.x and Duktape 2.x simultaneously
-----------------------------------------------------

For C code you can use the ``DUK_VERSION`` define to support both Duktape 1.x
and 2.x in the same application.  For example::

    #if (DUK_VERSION >= 20000)
    rc = duk_safe_call(ctx, my_safe_call, NULL, 1 /*nargs*/, 1 /*nrets*/);
    #else
    rc = duk_safe_call(ctx, my_safe_call, 1 /*nargs*/, 1 /*nrets*/);
    #endif

If you're developing against Duktape master before 2.0 release, ``DUK_VERSION``
is set to 19999 so that you can use::

    #if (DUK_VERSION >= 19999)
    rc = duk_safe_call(ctx, my_safe_call, NULL, 1 /*nargs*/, 1 /*nrets*/);
    #else
    rc = duk_safe_call(ctx, my_safe_call, 1 /*nargs*/, 1 /*nrets*/);
    #endif

Similarly for ECMAScript code you can::

    var plainBuffer;
    if (Duktape.version >= 19999) {
        plainBuffer = Uint8Array.plainOf(bufferObject);
    } else {
        plainBuffer = Duktape.Buffer(bufferObject);
    }

Or you can detect features specifically::

    var plainBuffer = (typeof Uint8Array.plainOf === 'function' ?
                       Uint8Array.plainOf : Duktape.Buffer)(bufferObject);

DUK_OPT_xxx feature option support removed
------------------------------------------

Duktape 2.0 no longer supports ``DUK_OPT_xxx`` options given via the compiler
command line.  Instead, all options are encoded in ``duk_config.h``.

To use custom Duktape options, use the ``tools/configure.py`` tool to create
a customized ``duk_config.h`` and prepared Duktape sources matching the
configuration.  For example to enable assertions and fastint support::

    $ python2 tools/configure.py \
          --output-directory /tmp/output \
          --source-directory src-input \
          --config-metadata config \
          -DDUK_USE_FASTINT \
          -DDUK_USE_ASSERTIONS

    # Prepared duk_config.h header and Duktape sources (duktape.h and
    # duktape.c) are in /tmp/output.  Compile normally with your application.

    $ gcc -std=c99 -Wall -o/tmp/test -I/tmp/output /tmp/output/duktape.c \
          my_application.c -lm

See http://wiki.duktape.org/Configuring.html for details and examples.

To upgrade:

* If you're using the Duktape default configuration and no ``DUK_OPT_xxx``
  compiler options, no actions are needed.

* Otherwise, remove ``DUK_OPT_xxx`` options from the compilation command and
  add a ``tools/configure.py`` pre-step to your build.  Add the equivalent
  ``DUK_USE_xxx`` options to ``tools/configure.py`` argument list; for example
  ``-DDUK_USE_FASTINT``.

* If you're using a ``duk_custom.h`` header there are three simple approaches:

  - To embed your custom header into ``duk_config.h`` statically, use
    ``--fixup-file duk_custom.h`` in ``tools/configure.py`` options.

  - To include your custom header at compilation time, similarly to
    ``DUK_OPT_HAVE_CUSTOM_H``, use ``--fixup-line '#include "duk_custom.h"'``
    in ``tools/configure.py`` options.

  - Finally, you can in some cases remove your custom header and use
    equivalent config options for ``tools/configure.py``.

Config option changes
---------------------

There are several new config options and some existing config options have
been removed.

To upgrade:

* Review any ``DUK_OPT_xxx`` or ``DUK_USE_xxx`` options in use against
  ``config/config-options/*.yaml``.

Built-ins disabled in configuration are now absent
--------------------------------------------------

If a built-in is disabled when running ``configure.py``, it won't be present
in the ECMAScript environment.  For example, with ``-UDUK_USE_ES6_PROXY``::

    duk> new Proxy()
    ReferenceError: identifier 'Proxy' undefined
        at [anon] (duk_js_var.c:1262) internal
        at global (input:1) preventsyield
    duk> typeof Proxy
    = "undefined"

In Duktape 1.x the binding was present but would just throw an Error when
invoked::

    duk> new Proxy()
    Error: unknown error (rc -1)
        at Proxy () native strict construct preventsyield
        at global (input:1) preventsyield
    duk> typeof Proxy
    = "function"

The revised behavior saves footprint and allows scripts to detect
supported built-ins reliably using e.g.::

    if (typeof Proxy === 'function') {
        // supported
    }

To upgrade:

* In most cases no action is needed.  If your code relies on the builtins
  being present but throwing an error (which seems unlikely), such call
  sites need to be fixed.

Tooling changes
---------------

There are some tooling changes in this release:

* The distributable now includes raw sources in ``src-input/`` and some
  tooling in ``tools/``.  This allows Duktape sources to be modified and
  re-amalgamated directly from the distributable.  The distributable still
  includes sources prepared using default configuration (``src/``,
  ``src-noline/``, and ``src-separate``) and some configuration examples.

* The tooling includes a new ``tools/configure.py`` tool which creates
  a ``duk_config.h`` and matching prepared sources simultaneously.  This
  allows use of ROM built-ins from the distributable.  Previously ROM
  built-ins required a manual ``dist.py --rom-support ...`` command.

* The ``make_dist.py`` utility in Duktape main repo has been renamed to
  ``dist.py`` and no longer supports ``--rom-support``,
  ``--rom-auto-lightfunc``, and ``--user-builtin-metadata`` options.  Use
  the  ``tools/configure.py`` tool instead, which supports these options.
  However, ``--user-builtin-metadata`` has been renamed ``--builtin-file``.

* The ``config/genconfig.py`` has been relocated to ``tools/genconfig.py`` in
  the distributable.  It can still be used as a standalone tool, but using
  configure.py is recommended instead.

To upgrade:

* If you're just using the default sources and ``duk_config.h`` in the
  distributable, no changes are needed.

* If you're using ``genconfig.py`` considering using ``tools/configure.py``
  instead.  If you keep on using ``genconfig.py``, update path to
  ``tools/genconfig.py``.

* If you're using ROM built-ins via ``make_dist.py``, change your build to
  use ``tools/configure.py`` instead, and change ``--user-builtins-metadata``
  option argument(s) to ``--builtin-file``.

Dist package file changes
-------------------------

* Configuration metadata is now in unpacked form in ``dist/config`` to match
  the Duktape master repo and to make config files more convenient to patch.
  The ``dist/tools/genconfig.py`` tool no longer accepts a tar.gz metadata
  argument.

* The pre-built ``duk_config.h`` examples have been removed as somewhat
  useless.  Use ``dist/tools/configure.py`` (or ``dist/tools/genconfig.py)``
  to generate ``duk_config.h`` files.

* ``dist/duk_build_meta.json`` has been renamed to ``dist/duk_dist_meta.json``
  for clarity.  It no longer contains string data scanned from source files.
  This metadata is now in source directories, e.g.
  ``dist/src/duk_source_meta.json`` as the string set potentially depends
  on options used to prepare sources.

* Source metadata, e.g. ``dist/src/metadata.json``, has been renamed to
  ``dist/src/duk_source_meta.json`` for clarity.  The metadata contains
  Duktape version information, strings scanned from source files, and for
  combined (amalgamated) sources the line number metadata.

Buffer behavior changes
-----------------------

There are a lot of buffer behavior changes in the 2.0 release; see detailed
changes below and in RELEASES.rst.  Here's a summary of changes:

* ``Duktape.Buffer`` has been removed.  Plain buffers now behave like
  ``Uint8Array`` instances to the extent possible.  They don't have a property
  table, however, which causes some limitations.  Plain buffers ToObject()
  coerce to an actual Uint8Array object with the same backing buffer.  There
  are many small changes to how plain buffers are treated by standard built-ins
  as a result.  For example, string coercion (``String(plainBuffer)``) now
  mimics Uint8Array and usually results in the string ``[object Uint8Array]``.

* Plain buffers have an inherited ``.buffer`` getter property which returns an
  ArrayBuffer object backing to the same underlying plain buffer.  Because
  there is no property table for plain buffers, each ``.buffer`` access creates
  a new ArrayBuffer instance.

* When ``duk_push_buffer_object()`` creates an automatic ArrayBuffer for a
  view (such as Uint8Array), the ArrayBuffer's .byteOffset will be set to 0
  and its .byteLength will be set to view.byteOffset + view.byteLength.  This
  ensures that accessing the ArrayBuffer at view.byteOffset returns the same
  value as when accessing view at index 0, which is the usual relationship
  between a view and its backing ArrayBuffer.  Up to Duktape 1.6.x the
  ArrayBuffer's .byteOffset and .byteLength would be the same as the view's.

* Non-standard properties, such as virtual indices and ``.length`` have been
  removed from ArrayBuffer and DataView.  The ``.byteOffset``, ``.byteLength``,
  ``.BYTES_PER_ELEMENT``, and ``.buffer`` properties of view objects are now
  inherited getters to match ES2015.  The ``.length`` property remains a virtual
  own property, however (it is a getter in ES2015).

* Default ECMAScript built-ins no longer provide the ability to do a 1:1
  buffer-to-string coercion where the buffer bytes are used directly as the
  internal string bytes.  Instead, an encoding (usually UTF-8) is always
  involved, and U+FFFD replacement characters are used when invalid inputs
  are encountered.  See https://github.com/svaarala/duktape/issues/1005.
  C code can still do 1:1 conversions using ``duk_buffer_to_string()`` or
  by pushing a raw string directly, and can expose such a binding to
  ECMAScript code.

* Node.js Buffer binding has been aligned more with Node.js v6.9.1 (from
  Node.js v0.12.1) and some (but not all) behavior differences to actual
  Node.js have been fixed.

* Disabling ``DUK_USE_BUFFEROBJECT_SUPPORT`` allows use of plain buffers in
  the C API, and allows manipulation of plain buffers in ECMAScript code via
  their virtual properties (index properties, ``.length``, etc).  Plain buffers
  still inherit from ``Uint8Array.prototype``, but won't Object coerce.  All
  ArrayBuffer, typed array, and Node.js Buffer methods will be missing, including
  ``Uint8Array.allocPlain()``.  Duktape custom built-ins operating on plain
  buffers (like Duktape.dec() with hex or base-64 encoding) continue to work.
  (This behavior is not guaranteed and may change even in minor versions.)

To upgrade:

* If you're using buffers in general, review http://wiki.duktape.org/HowtoBuffers.html
  which has been updated for Duktape 2.0.

* If you're using standard ArrayBuffers and typed arrays, no changes should
  normally be necessary, however see technical changes in RELEASES.rst.

* If you're using the Node.js Buffer binding, review the following:

  - Node.js Buffer ``.concat()`` always returns a buffer copy, even for a
    one-element input array which had special handling in Node.js v0.12.1.

  - Node.js Buffer.prototype ``.toString()`` now decodes the input buffer
    using UTF-8, emitting replacement characters for invalid UTF-8 sequences.

  - Review Buffer code for Node.js Buffer changes between Node.js versions
    v0.12.1 and v6.9.1 in general.

* If you're using plain buffers, review their usage especially in ECMAScript
  code, in particular:

  - Because plain buffers now mimic Uint8Array (a view), they are treated as
    initializer values when used as typed array constructor arguments.  For
    example, ``new Uint32Array(plainBuffer)`` will create a new Uint32Array
    rather than a view into the plain buffer.

  - To create a view into the plain buffer, use the same approach as with a
    Uint8Array, e.g. ``new Uint32Array(plainBuffer.buffer)``.

* Regardless of buffer type(s) in use:

  - One important change is that ``String(plainBuffer)`` and ``duk_to_string()``
    for a buffer does not work as before, use new ``duk_buffer_to_string()``
    C API call instead.  There's no equivalent function for the default
    ECMAScript built-ins.

  - Another important change is that plain buffers, like Uint8Array objects,
    boolean coerce to ``true`` regardless of buffer size (zero or larger) and
    contents.

* If you're using ``Duktape.Buffer``, the following new built-ins replace its
  functionality (and more):

  - ``Uint8Array.allocPlain()``: to allocate a new (fixed) plain buffer

  - ``Uint8Array.plainOf()``: to get the underlying plain buffer of any
    buffer object (without making a copy)

  - However, these bindings are intentionally missing if buffer object support
    has been disabled in Duktape configuration.

Some detailed changes, not exhaustive; see ``RELEASES.rst`` and
``tests/ecmascript/test-bi-plain-buffer-*.js`` for even more detail:

* ``typeof plainBuffer`` is now ``object`` instead of ``buffer``.

- ``plainBuffer instanceof Uint8Array`` is true.

* Plain buffer Object.prototype.toString() now usually, assuming no overridden
  .toString(), yields ``[object Uint8Array]`` instead of ``[object Buffer]``.

* Plain buffer inherits from Uint8Array.prototype instead of
  Duktape.Buffer.prototype.

* For a plain buffer ``duk_to_string()`` no longer creates a string with the
  same underlying bytes, but results in ``[object Uint8Array]`` instead
  (unless ``.toString()`` or ``.valueOf()`` has been overridden); in
  particular, using a plain buffer as an object property key is misleading
  as ``obj[buf]`` is (usually) equivalent to ``obj['[object Uint8Array]']``.
  ``duk_to_buffer()`` for a string still results in a plain buffer with the
  same underlying bytes as before.

* A new ``duk_buffer_to_string()`` API call converts any buffer value to a
  string with the same underlying bytes as in the buffer (like
  ``duk_to_string()`` did in Duktape 1.x).  ECMAScript built-ins no longer
  have this ability directly.

* ``duk_to_boolean()`` for a plain buffer: always true, even if the buffer
  has zero length.

* ``duk_to_primitive()`` for plain buffer: usually coerces to the string
  ``[object Uint8Array]`` because plain buffers are not considered a primitive
  value.

* ``duk_is_primitive()`` for a plain buffer is now false to match how
  ``duk_to_primitive()`` deals with plain buffers (i.e. coerces them rather
  than returning them as is).

* When a plain buffer is used as the "this" binding of a function call, it is
  ToObject() coerced to an actual Uint8Array if the call target is non-strict.
  This mimics what happens to e.g. plain strings.  Lightfuncs have also been
  revised to behave the same way (in Duktape 1.x they would not be ToObject()
  coerced in this situation).

* ``new ArrayBuffer(plainBuffer)`` no longer creates a new ArrayBuffer with
  the same underlying plain buffer; instead, the plain buffer gets coerced to
  zero and creates a zero-length ArrayBuffer.  This matches how a Uint8Array
  argument is handled in ``new ArrayBuffer()``.

- ``new Buffer(plainBuffer)`` no longer special cases plain buffer and gets
  treated like an Uint8Array: a fresh Buffer with matching ``.length`` is
  created and index elements are copied into the result buffer (in effect
  making an actual buffer copy).

- ``ArrayBuffer.isView(nodejsBuffer)`` is now true to reflect the fact that
  Node.js Buffers are Uint8Arrays in newer Node.js versions.

* ``new Uint32Array(plainBuffer)`` and other typed array constructors use the
  argument plain buffer as an initializer (like Uint8Array), which causes a
  copy to be created.

* ``new DataView(plainBuffer)`` is rejected and DataView() in general rejects
  any other argument than an actual ArrayBuffer.

* ``typedarray.prototype.subarray()`` accepts a plain buffer and the resulting slice
  is a Uint8Array because plain buffers cannot represent a view offset/length.

* Node.js ``Buffer.prototype.slice()`` accepts a plain buffer and the result is a
  Node.js Buffer (which itself is a special Uint8Array instance).

* ``plainBuffer.valueOf()`` ordinarily backed by ``Object.prototype.valueOf()``
  returns ``Object(plainBuffer)``, i.e. converts plain buffer to an actual
  Uint8Array.  This matches normal ``Object.prototype.valueOf()`` behavior, e.g.
  plain string is coerced into a String object.

- ``JSON.stringify()`` now recognizes plain buffers like Uint8Array instances;
  the result is typically ``{"0":XXX,"1":XXX,....}`` without a ``.toJSON()``
  implementation, as the virtual index properties are enumerable for Uint8Arrays.

* ``Object.freeze()`` not allowed for plain buffers or buffer objects (Duktape
  1.x allowed silently) because array index elements cannot be made non-writable.
  This is an internal limitation and failing with a TypeError signals this to the
  caller (and matches how e.g. V8 handles ``Object.freeze(new Uint8Array(4))``).

- Typed array ``.subarray()`` and Node.js Buffer ``.slice()`` result internal
  prototype is now set to the default prototype of the result type (e.g. initial
  value of ``Uint8Array.prototype`` if the input is an Uint8Array) rather than
  being copied from the argument.

* Node.js ``Buffer`` and ``Buffer.prototype`` methods now accept plain buffers.

* A plain buffer is accepted as a constructor "replacement value".

Pointer behavior changes
------------------------

There are very minor changes to pointer value behavior:

* ``plainPointer instanceof Duktape.Pointer`` now evaluates to ``true``
  (``false`` in Duktape 1.x).

To upgrade:

* If you're using pointer values in ECMAScript code, check pointer handling.

Lightfunc behavior changes
--------------------------

There are very minor changes to lightfunc value behavior:

* ``duk_is_primitive()`` now returns false for lightfuncs; this is more in
  line with how lightfuncs behave in ECMAScript ToPrimitive() coercion and
  matches how plain buffers work in Duktape 2.x.

* ``[[DefaultValue]]`` coercion now considers lightfuncs non-primitive
  (previously considered primitive and thus accepted as ``[[DefaultValue]]``
  result).

* When a lightfunc is used as the "this" binding of a function call, it is
  ToObject() coerced to a full function when the call target is non-strict.
  Duktape 1.x would not coerce the lightfunc to an object in this situation;
  the change was made to match plain buffer behavior.  Note that because
  lightfuncs themselves are considered strict functions, this only happens
  when the call target is not a lightfunc but the "this" binding is.

* A lightfunc is accepted as a constructor "replacement value".

To upgrade:

* If you're using lightfuncs, review their handling.

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
have an ECMAScript counterpart (for example, a ``DUK_ERR_API_ERROR`` mapped
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

duk_error(), duk_error_va(), duk_throw(), duk_fatal() have a return value
-------------------------------------------------------------------------

The prototype return value for these error throwers was changed from ``void``
to ``duk_ret_t`` which allows for idioms like::

    if (argvalue < 0) {
        return duk_error(ctx, DUK_ERR_TYPE_ERROR,
                         "invalid arg: %d", (int) argvalue);
    }

To upgrade:

* Without an explicit cast to ``(void) duk_error(...)`` you may get some new
  compiler warnings.  Fix by adding the void cast, or convert the call sites
  to use the ``return duk_error(...)`` idiom where applicable.

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

duk_to_defaultvalue() removed
-----------------------------

The ``duk_to_defaultvalue()`` API call was rather technical: it invoked the
internal ``[[DefaultValue]]`` algorithm which is used in ES5.1 as part of
the ToPrimitive() coercion (``duk_to_primitive()``).  ES2015 no longer specifies
``[[DefaultValue]]`` which has been folded into ToPrimitive().  The API call
thus no longer makes much sense.

To upgrade:

* If you're using ``duk_to_defaultvalue()`` (which is unlikely), you can in
  most cases replace it with ``duk_to_primitive()``.  The main difference
  is that ``duk_to_primitive()`` accepts all argument types (returning
  those considered primitive as is) while ``duk_to_defaultvalue()`` rejects
  primitive value arguments.  See the ES5.1/ES2015 specifications for exact
  differences between the two.

* Here's an example replacement.  Replace this::

      duk_to_defaultvalue(ctx, idx, hint);

  with::

      duk_require_type_mask(ctx, idx, DUK_TYPE_MASK_OBJECT |
                                      DUK_TYPE_MASK_BUFFER |
                                      DUK_TYPE_MASK_LIGHTFUNC);
      duk_to_primitive(ctx, idx, hint);

* Alternatively, include extras/duk-v1-compat into your compilation to add back
  the removed API call.

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

Debugger command callstack index changes
----------------------------------------

Debug command callstack indexes have been made mandatory where appropriate to
simplify the protocol.  Affected commands are: GetVar, PutVar, GetLocals, and
Eval.

To upgrade:

* Review debug client handling of callstack indices when sending affected
  commands.

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

Internal duk_harray affects debugger array inspection
-----------------------------------------------------

Duktape 2.x introduces an internal ``duk_harray`` type to represent arrays.
The array ``.length`` property is no longer stored in the property table of
the array but is a C struct field in ``duk_harray`` and the property visible
to ECMAScript code is virtual.

As a result, array ``.length`` is not visible when inspecting ordinary array
properties using e.g. GetObjPropDesc or GetObjPropDescRange.  Instead, array
``.length`` is an artificial property ``"length"`` returned by GetHeapObjInfo.

To upgrade:

* If the debug client uses array ``.length`` for e.g. UI purposes, ensure
  the artificial property ``"length"`` is used instead.

Other debugger changes
----------------------

* Artificial properties renamed for consistency with internal renaming:

  - ``compiledfunction`` -> ``compfunc``

  - ``nativefunction`` -> ``natfunc``

  - ``bufferobject`` -> ``bufobj``

  - ``bound`` -> ``boundfunc``

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

  The default fatal error handler will by default call ``abort()`` with no
  error message to ``stdout`` or ``stderr``.  To improve this behavior define
  ``DUK_USE_FATAL_HANDLER()`` in your ``duk_config.h``.

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

InitJS support removed
----------------------

Both Duktape InitJS (``DUK_USE_BUILTIN_INITJS``) and user InitJS
(``DUK_USE_USER_INITJS``) were removed.  Duktape built-in InitJS is no
longer needed (and was never used for very much).  User InitJS was rarely
used and it's not a full solution because custom environment initialization
may also involve native initialization code which isn't supported by the
mechanism.

To upgrade:

* Duktape built-in InitJS removal requires no user code changes.

* If you're using the user InitJS option, call sites need to be modified to
  run the init code explicitly on heap/thread creation.

Enumeration order changes
-------------------------

Enumeration order for ``Object.getOwnPropertyNames()`` has been changed to
match ES2015/ES2016 ``[[OwnPropertyKeys]]`` enumeration order, which is:

* Array indices in ascending order

* Normal (non-array-index) property keys in insertion order

* Symbols in insertion order

While not required by ES2015/ES2016, the same enumeration order is also used in
Duktape 2.x for ``for-in``, ``Object.keys()``, and ``duk_enum()``.  A related
change is that ``duk_enum()`` flags ``DUK_ENUM_ARRAY_INDICES_ONLY`` and
``DUK_ENUM_SORT_ARRAY_INDICES`` can now be used independently.

The revised enumeration order makes enumeration behavior more predictable
and matches other modern engines.  In particular, sparse arrays (arrays
without an internal array part) now enumerate identically to dense arrays.

To upgrade:

* Check application code for enumeration assumptions.

Symbol support related changes
------------------------------

Small changes related to adding symbol support:

* Symbols are represented as strings with an invalid UTF-8 representation (like
  internal keys in Duktape 1.x), and they behave like strings in the C API just
  like internal keys did in Duktape 1.x.  For example,  ``duk_is_string()`` is
  true for symbols.  Symbols can be distinguished using ``duk_is_symbol()``.

* Symbol support is currently **experimental**.  The core semantics have been
  implemented but it's likely some of the internal details will change in future
  releases.  The C API may also need changes (for example to the typing model)
  in future releases; in its current form symbols behave just like internal
  strings in the C API.

* Being experimental, the ``Symbol`` built-in is disabled by default; enable via
  config option ``DUK_USE_SYMBOL_BUILTIN``.  Symbols can be created from C code
  even when the ``Symbol`` built-in is disabled, and symbol semantics (like
  ``typeof`` and coercion behavior) are currently enabled.

* Internal properties are now called "hidden symbols" and adopt some ES2015 Symbol
  behaviors, e.g. ``typeof internalKey === 'symbol`` and ``"" + internalKey``
  is now a TypeError.  Internal keys are different from normal ES2015 Symbols in
  that they can't be enumerated from ECMAScript code even with
  ``Object.getOwnPropertySymbols()``.

* The ``DUK_ENUM_INCLUDE_INTERNAL`` C API flag has been renamed
  ``DUK_ENUM_INCLUDE_HIDDEN``.

Other incompatible changes
--------------------------

Incompatible changes (not exhaustive, also see RELEASES.rst):

* Normal and constructor function call argument limit is now 255, down from
  the previous 511.

* If a user function is called using the identifier 'eval', such a call won't
  get tailcall optimized even if otherwise possible.

* ``duk_gc()`` no longer accepts a NULL context pointer for consistence with
  other API calls.  A NULL pointer not causes memory unsafe behavior, as with
  all other API calls.

* ``duk_def_prop()`` now ToPropertyKey() coerces its argument rather than
  requiring the key to be a string.  This allows e.g. numbers to be used as
  property keys.

* ``duk_char_code_at()`` and ``String.charCodeAt()`` now return 0xFFFD (Unicode
  replacement character) if the string cannot be decoded as extended UTF-8,
  previously an error was thrown.  This situation never occurs for standard
  ECMAScript strings or valid UTF-8 strings.

* ``duk_get_length()`` now allows the ``size_t`` rather than the unsigned 32-bit
  integer range for the target value's ``.length``.

* Legacy octal literal handling has been improved to match more closely with
  ES2015 Annex B.  Octal look-alike decimal literals like "0778" and "0778.123"
  are now allowed.

* Legacy octal escape handling in string literals has been improved to match
  more closely with ES2015 Annex B and other engines: "\078" is not accepted and
  is the same as "\u00078", "\8" and "\9" are accepted as literal "8" and "9"
  (even in strict mode).

* The NetBSD pow() workaround option ``DUK_USE_POW_NETBSD_WORKAROUND`` has been
  generalized and renamed to ``DUK_USE_POW_WORKAROUNDS``.

* When using a Proxy as a for-in target the "ownKeys" trap is invoked instead
  of the "enumerate" trap in ES2016.  Duktape now follows this behavior.  The
  "enumerate" trap has been obsoleted.  Key enumerability is also now checked
  when "ownKeys" trap is used in Object.keys() and for-in.

* Bound function internal prototype is copied from the target function to match
  ES2015 requirements; in ES5 (and Duktape 1.x) bound function internal prototype
  is always set to Function.prototype.

* Function.prototype.toString() output has been changed to match ES2015
  requirements.  For example ``function foo() {"ecmascript"}`` is now
  ``function foo() { [ecmascript code] }``.

* Function ``.name`` and ``.length`` properties are now non-writable,
  non-enumerable, but configurable, to match revised ES2015 semantics.  Previously
  the properties were also non-configurable.

* Function ``.fileName`` property is now non-writable, non-enumerable, and
  configurable.  Previously it was writable, non-enumerable, and configurable.
  While this is not required by ES2015 (as the property is non-standard), it has
  been aligned with ``.name``.  Direct assignment to the property will be
  rejected, but you can set it using e.g.
  ``Object.defineProperty(func, 'fileName', { value: 'myFilename.js' });``.

* Error instance ``.fileName`` and ``.lineNumber`` property attributes are
  also non-writable, non-enumerable, but configurable.  This only matters when
  tracebacks are disabled and concrete properties are used instead of the
  inherited accessors.

* Object constructor methods like Object.keys(), Object.freeze(), etc now
  follow more lenient ES2015 coercion semantics: non-object arguments are either
  coerced to objects or treated like non-extensible objects with no own
  properties.

* RegExp.prototype follows ES2015 behavior more closely: it is no longer a RegExp
  instance, .source, .global, .ignoreCase, and .multiline are now inherited
  getters, etc.  However, leniency to allow e.g. RegExp.prototype.source (from
  ES2017 draft) is supported for real world code compatibility.

* Duktape.info() now returns an object rather than an array.  The object has
  named properties like ``.type`` and ``.enext`` for the internal fields which
  is easier to version and work with.  The names of the properties are not
  under version guarantees and may change in an incompatible fashion in even a
  minor release.

* Memory management without mark-and-sweep is no longer supported; relying on
  only refcounting was error prone because reference cycles or debugger use
  could result in leaked garbage that would only get collected on heap
  destruction.

Known issues
============

* Some non-compliant behavior for array indices near 2G or 4G elements.

* RegExp parser is strict and won't accept some real world RegExps which
  are technically not compliant with ECMAScript E5/E5.1 specification
  but allowed in ES2015 Annex B.

* Final mantissa bit rounding issues in the internal number-to-string
  conversion.

Raw issues from test runs
=========================

API tests
---------

::

    test-to-number.c: fail; 15 diff lines; known issue: number parsing bug for strings containing NUL characters (e.g. '\u0000')

ECMAScript tests
----------------

::

    test-bi-array-proto-push: fail; 30 diff lines; known issue: array length above 2^32-1 not supported
    test-bi-array-push-maxlen: fail; 17 diff lines; known issue: array length above 2^32-1 not supported
    test-bi-date-tzoffset-brute-fi: fail; 12 diff lines; known issue: year 1970 deviates from expected, Duktape uses equiv. year for 1970 on purpose at the moment; requires special feature options: test case has been written for Finnish locale
    test-bi-function-nonstd-caller-prop: fail; 175 diff lines; requires special feature options: DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
    test-bi-global-parseint: fail; 89 diff lines; known issue: rounding differences for parsing integers larger than 2^53
    test-bi-json-enc-proplist-dups: fail; 8 diff lines; known issue: JSON.stringify() can be given a property list to serialize; duplicates should be eliminated but Duktape (and other engines) will happily serialize a property multiple times
    test-bi-json-enc-proxy: fail; 13 diff lines; known issue: JSON enumeration behavior for Proxy targets is incomplete and uses 'enumerate' trap instead of 'ownKeys' trap
    test-bi-number-proto-toexponential: fail; 75 diff lines; known issue: corner case rounding errors in toExponential()
    test-bi-number-proto-tostring: fail; 46 diff lines; known issue: expect strings to be checked, but probably Duktape rounding issues
    test-bi-proxy-object-tostring: fail; 6 diff lines; known issue: Object class handling for Proxy objects is incomplete
    test-bi-regexp-gh39: fail; 7 diff lines; known issue: requires leniency for non-standard regexps
    test-bi-symbol-coercion: fail; 9 diff lines; known issue: no @@toPrimitive coercion yet
    test-bi-typedarray-misc-inherited-accessors: fail; 21 diff lines; known issue: typed array .length etc not yet inherited accessors (ES2015 requirement)
    test-bug-date-timeval-edges: fail; 17 diff lines; known issue: test case depends on current timezone offset
    test-bug-enum-shadow-nonenumerable: fail; 12 diff lines; known issue: corner case enumeration semantics, not sure what correct behavior is (test262 ch12/12.6/12.6.4/12.6.4-2)
    test-bug-json-parse-__proto__: fail; 18 diff lines; known issue: when ES2015 __proto__ enabled, JSON.parse() parses '__proto__' property incorrectly when a specially crafted reviver is used
    test-bug-numconv-1e23: fail; 10 diff lines; known issue: corner case in floating point parse rounding
    test-bug-numconv-denorm-toprec: fail; 7 diff lines; known issue: in a denormal corner case toPrecision() can output a zero leading digit
    test-bug-tonumber-u0000: fail; 7 diff lines; known issue: '\u0000' should ToNumber() coerce to NaN, but now coerces to zero like an empty string
    test-dev-16bit-overflows: fail; 11 diff lines; requires special feature options: requires 16-bit field options
    test-dev-func-cons-args: fail; 18 diff lines; known issue: corner cases for 'new Function()' when arguments and code are given as strings
    test-dev-func-varmap-drop: fail; 46 diff lines; requires special feature options: debugger support must be disabled
    test-dev-lightfunc-accessor: fail; 50 diff lines; requires special feature options: DUK_USE_LIGHTFUNC_BUILTINS
    test-dev-lightfunc-finalizer: fail; 8 diff lines; requires special feature options: DUK_USE_LIGHTFUNC_BUILTINS
    test-dev-lightfunc: fail; 518 diff lines; requires special feature options: DUK_USE_LIGHTFUNC_BUILTINS
    test-dev-object-literal-method-computed: fail; 8 diff lines; known issue: computed name for object literal method shorthand not yet implemented
    test-dev-yield-after-callapply: fail; 8 diff lines; known issue: yield() not allowed when function called via Function.prototype.(call|apply)()
    test-lex-unterminated-hex-uni-escape: fail; 29 diff lines; known issue: unterminated hex escapes should be parsed leniently, e.g. '\uX' -> 'uX' but Duktape now refuses to parse them
    test-numconv-parse-misc: fail; 12 diff lines; known issue: rounding corner case for 1e+23 (parses/prints as 1.0000000000000001e+23)
    test-numconv-tostring-gen: fail; 257 diff lines; known issue: rounding corner cases in number-to-string coercion
    test-numconv-tostring-misc: fail; 6 diff lines; known issue: rounding corner case, 1e+23 string coerces to 1.0000000000000001e+23
    test-regexp-empty-quantified: fail; 16 diff lines; known issue: a suitable empty quantified (e.g. '(x*)*') causes regexp parsing to terminate due to step limit
    test-regexp-invalid-charclass: fail; 7 diff lines; known issue: some invalid character classes are accepted (e.g. '[\d-z]' and '[z-x]')
    test-stmt-for-in-lhs: fail; 29 diff lines; known issue: for-in allows some invalid left-hand-side expressions which cause a runtime ReferenceError instead of a compile-time SyntaxError (e.g. 'for (a+b in [0,1]) {...}')

test262
-------

::

    ch12/12.6/12.6.4/12.6.4-2 in non-strict mode   // diagnosed: enumeration corner case issue, see test-bug-enum-shadow-nonenumerable.js
    ch15/15.10/15.10.2/15.10.2.5/S15.10.2.5_A1_T5 in non-strict mode   // diagnosed: Duktape bug, matching /(a*)b\1+/ against 'baaaac' causes first capture to match the empty string; the '\1+' part will then use the '+' quantifier over the empty string.  As there is no handling to empty quantified now, Duktape bails out with a RangeError.
    ch15/15.10/15.10.2/15.10.2.9/S15.10.2.9_A1_T5 in non-strict mode   // diagnosed: Duktape bug, matching /(a*)b\1+/ against 'baaac' causes first capture to be empty, the '\1+' part will then quantify over an empty string leading to Duktape RangeError (there is no proper handling for an empty quantified now)
    ch15/15.4/15.4.4/15.4.4.10/S15.4.4.10_A3_T3 in non-strict mode   // diagnosed: probably Duktape bug related to long array corner cases or 'length' sign handling (C typing?)
    ch15/15.4/15.4.4/15.4.4.12/S15.4.4.12_A3_T3 in non-strict mode   // diagnosed: probably Duktape bug related to long array corner cases or 'length' sign handling (C typing?)
    ch15/15.4/15.4.4/15.4.4.14/15.4.4.14-5-12 in non-strict mode   // diagnosed: Array length over 2G, not supported right now
    ch15/15.4/15.4.4/15.4.4.14/15.4.4.14-5-16 in non-strict mode   // diagnosed: Array length over 2G, not supported right now
    ch15/15.4/15.4.4/15.4.4.14/15.4.4.14-9-9 in non-strict mode   // diagnosed: a.indexOf(<n>,4294967290) returns -1 for all indices n=2,3,4,5 but is supposed to return 4294967294 for n=2.  The cause is long array corner case handling, possibly signed length handling (C typing?)
    ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-5-12 in non-strict mode   // diagnosed: probably Duktape bug: long array corner cases (C typing?)
    ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-5-16 in non-strict mode   // diagnosed: probably Duktape bug: long array corner cases (C typing?)
    ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-8-9 in non-strict mode   // diagnosed: probably Duktape bug: long array corner cases (C typing?)
    ch15/15.4/15.4.4/15.4.4.6/S15.4.4.6_A4_T1 in non-strict mode   // diagnosed: Array .pop() fast path (can be disabled) ignores inherited array index properties
