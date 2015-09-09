=======================
Duktape feature options
=======================

Overview
========

The effective set of Duktape features is resolved in three steps in Duktape 1.x
(this process will change in Duktape 2.x):

* User defines ``DUK_OPT_xxx`` feature options.  These are essentially
  requests to enable/disable some feature.  (These will be removed in
  Duktape 2.x and ``DUK_USE_xxx`` flags will be used directly.)

* Duktape feature resolution in the default "auto-detecting" ``duk_config.h``
  (previously internal ``duk_features.h.in``) takes into account the
  requested features, the platform, the compiler, the operating system
  etc, and defines ``DUK_USE_xxx`` internal use flags.  Other parts of
  Duktape only listen to these "use flags", so that feature resolution is
  strictly contained.

* The final ``DUK_USE_xxx`` flags can be tweaked in several ways:

  - The generated ``duk_config.h`` header can be edited directly (manually,
    through scripting, etc).

  - The ``genconfig`` utility can be used to generate a ``duk_config.h``
    header with user-supplied option overrides given either as YAML config
    file(s) or C header snippets included in the config header.

  - User may optionally have a ``duk_custom.h`` header which can tweak the
    defines (see ``DUK_OPT_HAVE_CUSTOM_H`` for more discussion; this feature
    option will be removed in Duktape 2.x.)

Starting from Duktape 1.3 an external ``duk_config.h`` is required; it may
be a prebuilt multi-platform header or a user-modified one.  Duktape 2.x
will remove support for ``DUK_OPT_xxx`` feature options entirely.

This document describes all the supported Duktape feature options and should
be kept up-to-date with new features.  The feature option list in the guide
is a subset of the most commonly needed features.

See also:

- ``low-memory.rst``: suggested options for low memory environments

- ``timing-sensitive.rst``: suggested options for timing sensitive environments

- ``src/duk_config.h`` (in the distributable): resolution of feature options
  to use flags

Feature option naming
=====================

Feature options that enable a certain (default) feature are named::

  DUK_OPT_MY_FEATURE

Feature options that disable a (default) feature are named::

  DUK_OPT_NO_MY_FEATURE

Both flags are reserved at the same time.  One of the options will match
the default behavior, so it won't actually be implemented.

Some feature options have a value associated with them.  This is the case
for e.g. ``DUK_OPT_PANIC_HANDLER`` or ``DUK_OPT_FORCE_ALIGN``.  These are
handled case by case.

Avoid using words like "disable" in the feature naming.  This will lead to
odd names if the default behavior changes and a "no disable" flag is needed.

Platform and portability options
================================

DUK_OPT_DLL_BUILD
-----------------

Add this define to both Duktape and application build when Duktape is compiled
as a DLL.  This is especially critical on Windows: the option makes Duktape use
``__declspec(dllexport)`` and ``__declspec(dllimport)`` for public symbols.
While this is not currently needed for Unix platforms, it should always be used
if you build as a DLL.

DUK_OPT_FORCE_ALIGN
-------------------

Use ``-DDUK_OPT_FORCE_ALIGN=4`` or ``-DDUK_OPT_FORCE_ALIGN=8`` to force a
specific struct/value alignment instead of relying on Duktape's automatic
detection.  This shouldn't normally be needed.

DUK_OPT_FORCE_BYTEORDER
-----------------------

Use this to skip byte order detection and force a specific byte order:
``1`` for little endian, ``2`` for ARM "mixed" endian (integers little
endian, IEEE doubles mixed endian), ``3`` for big endian.  Byte order
detection relies on unstandardized platform specific header files, so
this may be required for custom platforms if compilation fails in
endianness detection.

DUK_OPT_NO_FILE_IO
------------------

Disable use of ANSI C file I/O which might be a portability issue on some
platforms.  Causes ``duk_eval_file()`` to throw an error, makes built-in
``print()`` and ``alert()`` no-ops, and suppresses writing of a panic
message to ``stderr`` on panic.  This option does not suppress debug
printing so don't enable debug printing if you wish to avoid I/O.

DUK_OPT_HAVE_CUSTOM_H
---------------------

Enable user-provided ``duk_custom.h`` customization header (see below for
details).  Not recommended unless really necessary.

DUK_OPT_PANIC_HANDLER(code,msg)
-------------------------------

Provide a custom panic handler, see detailed description below.

DUK_OPT_DECLARE
---------------

Provide declarations or additional #include directives to be used when
compiling Duktape.  You may need this if you set ``DUK_OPT_PANIC_HANDLER``
to call your own panic handler function (see example below).  You can also
use this option to cause additional files to be included when compiling
Duktape.

DUK_OPT_SEGFAULT_ON_PANIC
-------------------------

Cause the default panic handler to cause a segfault instead of using
``abort()`` or ``exit()``.  This is useful when debugging with valgrind,
as a segfault provides a nice C traceback in valgrind.

DUK_OPT_USER_INITJS
-------------------

Provide a string to evaluate when a thread with new built-ins (a new global
environment) is created.  This allows you to make minor modifications to the
global environment before any code is executed in it.  The value must be a
string, e.g.::

    -DDUK_OPT_USER_INITJS='"this.foo = 123"'

Errors in the initialization code result in a fatal error.

(This option will most likely be deprecated in favor of an actual callback
which provides much more flexibility for extending the global environment,
implementing sandboxing, etc.)

DUK_OPT_SETJMP
--------------

Force ``setjmp/longjmp`` for long control transfers.

The default long control transfer provider is ``setjmp/longjmp`` because it
is the most portable option.  When a better provider is known for a platform,
Duktape may default to that (e.g. ``_setjmp/_longjmp`` is the default for
OSX/iPhone, see GH-55).  With this feature option you can force Duktape to
explicitly use ``setjmp/longjmp`` even in these cases.

The downside of ``setjmp/longjmp`` is that signal mask saving behavior is not
specified and varies between platforms.  Signal mask saving may have a
significant performance impact so you may want to force a specific provider
if performance matters for your application.

DUK_OPT_UNDERSCORE_SETJMP
-------------------------

Force ``_setjmp/_longjmp`` for long control transfers.  This ensures signal
mask is not saved which can be a lot faster if ``setjmp/longjmp`` saves the
signal mask (this varies between platforms).  See comments in
``DUK_OPT_SETJMP``.

DUK_OPT_SIGSETJMP
-----------------

Force ``sigsetjmp/siglongjmp`` with ``savesigs == 0`` for long control
transfers (i.e. signal mask not saved/restored).  See comments in
``DUK_OPT_SETJMP``.

DUK_OPT_FASTINT
---------------

Add internal support for 48-bit signed integer duk_tval with transparent
semantics.  Transparency means that neither Ecmascript code nor application
using the Duktape API will be aware of whether a number is represented as
an IEEE double or a 48-bit signed integer internally.  Integers are promoted
to IEEE doubles and vice versa when necessary.

The internal implementation provides fast paths for performance critical
sections.  Such fast paths support "fastint" values and use integer math
when possible.  Other internal parts are not aware of the "fastint" type
and will automatically coerce a fastint to a double when necessary (even
when integer math would be possible).

This option increases code size slightly, but improves performance a great
deal on platforms with soft float arithmetic.  If a platform has hard floats,
this option may reduce overall performance because of the additional costs of
checking for integer/double conversion, etc.

Performance options
===================

DUK_OPT_JSON_STRINGIFY_FASTPATH
-------------------------------

Enable JSON.stringify() fast path.  The fast path is not fully portable in
Duktape 1.3, so it is not enabled by default.

Memory management options
=========================

DUK_OPT_EXTERNAL_STRINGS
------------------------

Enable support for external strings.  An external string requires a Duktape
heap allocation to store a minimal string header, with the actual string
data being held behind a pointer (similarly to how dynamic buffers work).

This is needed to use ``DUK_OPT_EXTSTR_INTERN_CHECK`` and/or
``DUK_OPT_EXTSTR_FREE``.

DUK_OPT_NO_PACKED_TVAL
----------------------

Don't use the packed 8-byte internal value representation even if otherwise
possible.  The packed representation has more platform/compiler portability
issues than the unpacked one.

DUK_OPT_DEEP_C_STACK
--------------------

By default Duktape imposes a sanity limit on the depth of the C stack because
it is often limited in embedded environments.  This option forces Duktape to
use a deep C stack which relaxes e.g. recursion limits.  Automatic feature
detection enables deep C stacks on some platforms known to have them (e.g.
Linux, BSD, Windows).

Removed in Duktape 1.3.0, use explicit config options for shallow stack
targets.

DUK_OPT_NO_REFERENCE_COUNTING
-----------------------------

Disable reference counting and use only mark-and-sweep for garbage collection.
Although this reduces memory footprint of heap objects, the downside is much
more fluctuation in memory usage.

DUK_OPT_NO_MARK_AND_SWEEP
-------------------------

Disable mark-and-sweep and use only reference counting for garbage collection.
This reduces code footprint and eliminates garbage collection pauses, but
objects participating in unreachable reference cycles won't be collected until
the Duktape heap is destroyed.  In particular, function instances won't be
collected because they're always in a reference cycle with their default
prototype object.  Unreachable objects are collected if you break reference
cycles manually (and are always freed when a heap is destroyed).

DUK_OPT_NO_VOLUNTARY_GC
-----------------------

Disable voluntary periodic mark-and-sweep collection.  A mark-and-sweep
collection is still triggered in an out-of-memory condition.  This option
should usually be combined with reference counting, which collects all
non-cyclical garbage.  Application code should also request an explicit
garbage collection from time to time when appropriate.  When this option
is used, Duktape will have no garbage collection pauses in ordinary use,
which is useful for timing sensitive applications like games.

DUK_OPT_NO_MS_STRINGTABLE_RESIZE
--------------------------------

Disable forced string intern table resize during mark-and-sweep garbage
collection.  This may be useful when reference counting is disabled, as
mark-and-sweep collections will be more frequent and thus more expensive.

DUK_OPT_GC_TORTURE
------------------

Development time option: force full mark-and-sweep on every allocation to
stress test memory management.

Low memory feature options
==========================

These options are low memory features for systems with 96-256 kB of RAM.
Unless you have very little RAM, these options are probably not relevant
to you.  They involve some compromises in e.g. performance or compliance
to reduce memory usage.

DUK_OPT_REFCOUNT16
------------------

Use a 16-bit reference count field (for low memory environments).

DUK_OPT_STRHASH16
-----------------

Use a 16-bit string hash field (for low memory environments).

DUK_OPT_STRLEN16
----------------

Use a 16-bit string length field (for low memory environments).

DUK_OPT_BUFLEN16
----------------

Use a 16-bit buffer length field (for low memory environments).

DUK_OPT_OBJSIZE16
-----------------

Use a 16-bit object entry and array part sizes (for low memory environments).
Also automatically drops support for an object hash part to further reduce
memory usage; there are rarely large objects in low memory environments simply
because there's no memory to store a lot of properties.

DUK_OPT_HEAPPTR16, DUK_OPT_HEAPPTR_ENC16, DUK_OPT_HEAPPTR_DEC16
---------------------------------------------------------------

Enable "compression" of Duktape heap pointers into an unsigned 16-bit value
and provide the macros for encoding and decoding a pointer:

- Pointers compressed are those allocated from Duktape heap, using the
  user provided allocation functions.  Also NULL pointer must encode and
  decode correctly.

- Currently it is required that NULL encodes to integer 0, and integer
  0 decodes to NULL.  No other pointer can be encoded to 0.

- DUK_OPT_HEAPPTR_ENC16(udata,p) is a macro with a userdata and ``void *``
  argument, and a ``duk_uint16_t`` return value.

- DUK_OPT_HEAPPTR_DEC16(udata,x) is a macro with a userdata and
  ``duk_uint16_t`` argument, and a ``void *`` return value.

- The userdata argument is the heap userdata value given at heap creation.

- See ``ajduk`` example in Duktape ``Makefile`` for a concrete example.

This option reduces memory usage by several kilobytes, but has several
downsides:

- It can only be applied when Duktape heap is limited in size.  For instance,
  with 4-byte aligned allocations a 256kB heap (minus one value for NULL)
  can be supported.

- Pointer encoding and decoding may be relatively complicated as they need to
  correctly handle NULL pointers and non-continuous memory maps used by some
  targets.  The macro may need to call out to a helper function in practice,
  which is much slower than an inline implementation.

Current limitations:

- Duktape internal debug code enabled with e.g. ``DUK_OPT_DEBUG`` and
  ``DUK_OPT_DPRINT`` doesn't have enough plumbing to be able to decode
  pointers.  Debug printing cannot currently be enabled when pointer
  compression is active.

DUK_OPT_DATAPTR16, DUK_OPT_DATAPTR_ENC16, DUK_OPT_DATAPTR_DEC16
---------------------------------------------------------------

Enable "compression" of arbitrary data pointers into an unsigned 16-bit value
and provide the macros for encoding and decoding a pointer:

- Pointers compressed are any void pointers in C code, not just the Duktape
  heap.  Also NULL pointer must encode and decode correctly.

- Currently it is required that NULL encodes to integer 0, and integer
  0 decodes to NULL.  No other pointer can be encoded to 0.

- DUK_OPT_DATAPTR_ENC16(udata,p) is a macro with a userdata and ``void *``
  argument, and a ``duk_uint16_t`` return value.

- DUK_OPT_DATAPTR_DEC16(udata,x) is a macro with a userdata and
  ``duk_uint16_t`` argument, and a ``void *`` return value.

- The userdata argument is the heap userdata value given at heap creation.

.. note:: This feature option is currently unimplemented, i.e. Duktape won't compress
          any data pointers at the moment.

DUK_OPT_FUNCPTR16, DUK_OPT_FUNCPTR_ENC16, DUK_OPT_FUNCPTR_DEC16
---------------------------------------------------------------

Enable "compression" of arbitrary C function pointers into an unsigned 16-bit
value and provide the macros for encoding and decoding a pointer:

- Pointers compressed are any C function pointers.  Also NULL pointer must
  encode and decode correctly.

- Currently it is required that NULL encodes to integer 0, and integer
  0 decodes to NULL.  No other pointer can be encoded to 0.

- DUK_OPT_FUNCPTR_ENC16(udata,p) is a macro with a userdata and ``void *``
  argument, and a ``duk_uint16_t`` return value.

- DUK_OPT_FUNCPTR_DEC16(udata,x) is a macro with a userdata and
  ``duk_uint16_t`` argument, and a ``void *`` return value.

- The userdata argument is the heap userdata value given at heap creation.

.. note:: This feature option is currently unimplemented, i.e. Duktape won't compress
          any function pointers at the moment.  It might not be necessary to support a
          NULL function pointer.

DUK_OPT_EXTSTR_INTERN_CHECK(udata,ptr,len)
------------------------------------------

Provide a hook for checking if data for a certain string can be used from
external memory (outside of Duktape heap, e.g. memory mapped flash).
The hook is called during string interning with the following semantics:

* The string data with no NUL termination resides at ``ptr`` and has ``len``
  bytes.  The ``udata`` argument is the heap userdata which may be ignored
  if not needed.

* If the hook returns NULL, Duktape interns the string normally, i.e.
  string data is allocated from Duktape heap.

* Otherwise the hook return value must point to a memory area which contains
  ``len`` bytes from ``ptr`` followed by a NUL byte which is **not present**
  in the input data.  Data behind the returned pointer may not change after
  the hook returns.

Notes:

* Also enable ``DUK_OPT_EXTERNAL_STRINGS`` to use this feature.

* The hook may be called several times for the same input string.  This
  happens when a string is interned, garbage collected, and then interned
  again.

* The ``DUK_OPT_EXTSTR_FREE()`` hook allows application code to detect when
  an external string is about to be freed.

* In most cases the hook should reject strings whose ``len`` is less than 4
  because there is no RAM advantage in moving so short strings into external
  memory.  The ordinary ``duk_hstring`` header followed by the data (and a
  NUL byte) has the same size as ``duk_hstring_external`` header which hosts
  a pointer instead of string data.

See ``low-memory.rst`` for more discussion how to use this feature option
in practice.

DUK_OPT_EXTSTR_FREE(udata,ptr)
------------------------------

Optional counterpart to ``DUK_OPT_EXTSTR_INTERN_CHECK``, with the following
semantics:

* Also enable ``DUK_OPT_EXTERNAL_STRINGS`` to use this feature.

* The macro is invoked when an external string is about to be freed.

* The argument ``ptr`` is a ``void *`` and points to the external string data.
  Concretely, it is the (non-NULL) value returned by
  ``DUK_OPT_EXTSTR_INTERN_CHECK``.  The ``udata`` argument is the heap
  userdata which may be ignored if not needed.

.. note:: Right now there is no API to push external strings; external strings
          come into being as a resul of DUK_OPT_EXTSTR_INTERN_CHECK() only.
          If/when this is changed, this hook will get called for every string,
          even if pushed by the user using an API call; this may need to be
          rethought at that time.

DUK_OPT_STRTAB_CHAIN, DUK_OPT_STRTAB_CHAIN_SIZE
-----------------------------------------------

Replace the default (open addressing, probing) string table structure with one
based on separate chaining.  There is a fixed-size top level hash table (whose
size is defined using ``DUK_OPT_STRTAB_CHAIN_SIZE``), with each entry in the
hash table being: (a) NULL, (b) a ``duk_hstring`` pointer, or (c) a pointer
to an array of ``duk_hstring`` pointers.  The pointer arrays are gappy (the
gaps are reused on new inserts) and are never shrunk at the moment.

This option is intended for low memory environments to make Duktape's memory
behavior match a typical pool-based allocator better:

* The top level fixed structure never changes size, so there is no hash table
  resize, and thus no need for resize temporaries.  The default string table
  algorithm needs resizing from time to time and doesn't resize in place, so
  you effectively need twice the string table size temporarily during a resize.

* The pointer arrays vary in size, but their size (typically 8 to 64 bytes,
  depending on the load factor) matches that of many other allocations which
  works well with a pooled allocator.

Ecmascript feature options
==========================

DUK_OPT_NO_AUGMENT_ERRORS
-------------------------

Don't augment Ecmascript error objects with custom fields like ``fileName``,
``lineNumber``, and traceback data.  Also disables ``Duktape.errCreate`` and
``Duktape.errThrow`` error handler callbacks.  Implies ``DUK_OPT_NO_TRACEBACKS``.

DUK_OPT_NO_TRACEBACKS
---------------------

Don't record traceback data into Ecmascript error objects (but still record
``fileName`` and ``lineNumber``).  Reduces footprint and makes error handling
a bit faster, at the cost of less informative Ecmascript errors.

DUK_OPT_NO_VERBOSE_ERRORS
-------------------------

Don't provide error message strings or filename/line information for errors
generated by Duktape.  Reduces footprint, at the cost of much less informative
Ecmascript errors.

DUK_OPT_TRACEBACK_DEPTH
-----------------------

Override default traceback collection depth.  The default is currently 10.

DUK_OPT_NO_PC2LINE
------------------

Don't record a "pc2line" map into function instances.  Without this map,
exceptions won't have meaningful line numbers (virtual machine program
counter values cannot be translated to line numbers) but function instances
will have a smaller footprint.

DUK_OPT_NO_STRICT_DECL
----------------------

**Experimental.**

Disable support for ``"use strict"`` declaration so that Ecmascript code is
always executed in non-strict mode.  Duktape/C functions remain strict.

.. note:: This mechanism is EXPERIMENTAL and the details may change
          between releases.

DUK_OPT_NO_REGEXP_SUPPORT
-------------------------

Disable support for regular expressions.  Regexp literals are treated as a
``SyntaxError``, RegExp constructor and prototype functions throw an error,
``String.prototype.replace()`` throws an error if given a regexp search value,
``String.prototype.split()`` throws an error if given a regexp separator
value, ``String.prototype.search()`` and ``String.prototype.match()`` throw an
error unconditionally.

DUK_OPT_STRICT_UTF8_SOURCE
--------------------------

Enable strict UTF-8 parsing of source code.  When enabled, non-shortest
encodings (normally invalid UTF-8) and surrogate pair codepoints are accepted
as valid source code characters.  This option breaks compatibility with
some test262 tests.

DUK_OPT_NO_OCTAL_SUPPORT
------------------------

Disable optional octal number support (Ecmascript E5/E5.1
`Annex B <http://www.ecma-international.org/ecma-262/5.1/#sec-B>`_).

DUK_OPT_NO_SOURCE_NONBMP
------------------------

Disable accurate Unicode support for non-BMP characters in source code.
Non-BMP characters are then always accepted as identifier characters.

DUK_OPT_NO_BROWSER_LIKE
-----------------------

Disable browser-like functions.  Makes ``print()`` and ``alert()`` throw an
error.  This option is confusing when used with the Duktape command line tool,
as the command like tool will immediately panic.

DUK_OPT_NO_SECTION_B
--------------------

Disable optional features in Ecmascript specification
`Annex B <http://www.ecma-international.org/ecma-262/5.1/#sec-B>`_.
Causes ``escape()``, ``unescape()``, and ``String.prototype.substr()`` to
throw an error.

DUK_OPT_NO_NONSTD_ACCESSOR_KEY_ARGUMENT
---------------------------------------

Don't give setter/getter calls the property name being accessed as
an additional, non-standard property.  See
`Property virtualization <http://duktape.org/guide.html#propertyvirtualization>`_.

DUK_OPT_NO_NONSTD_FUNC_STMT
---------------------------

Disable support for function declarations outside program or function top
level (also known as "function statements").  Such declarations are
non-standard and the strictly compliant behavior is to treat them as a
SyntaxError.  Default behavior is to treat them like ordinary function
declarations ("hoist" them to function top) with V8-like semantics.

DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY
-----------------------------------

Add a non-standard ``caller`` property to non-strict function instances
for better compatibility with existing code.  The semantics of this
property are not standardized and may vary between engines; Duktape tries
to behave close to V8 and Spidermonkey.  See
`Mozilla <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function/caller>`_
description of the property.  This feature disables tail call support.

This feature conflicts with several other features, so you should use it
only if it's absolutely necessary.

DUK_OPT_NONSTD_FUNC_SOURCE_PROPERTY
-----------------------------------

Add a non-standard ``source`` property to function instances.  This allows
function ``toString()`` to print out the actual function source.  The
property is disabled by default because it increases memory footprint.

.. note:: Unimplemented as of Duktape 1.3.0.

DUK_OPT_NO_NONSTD_ARRAY_SPLICE_DELCOUNT
---------------------------------------

For better compatibility with existing code, ``Array.prototype.splice()``
has non-standard behavior by default when the second argument (deleteCount)
is not given: the splice operation is extended to the end of the array,
see
`test-bi-array-proto-splice-no-delcount.js <https://github.com/svaarala/duktape/blob/master/tests/ecmascript/test-bi-array-proto-splice-no-delcount.js>`_.
If this option is given, ``splice()`` will behave in a strictly
conforming fashion, treating a missing deleteCount the same as an undefined
(or 0) value.

DUK_OPT_NO_NONSTD_ARRAY_CONCAT_TRAILER
--------------------------------------

For better compatibility with existing code, ``Array.prototype.concat()``
has non-standard behavior by default for trailing non-existent elements of
the concat result, see
`test-bi-array-proto-concat-nonstd-trailing.js <https://github.com/svaarala/duktape/blob/master/tests/ecmascript/test-bi-array-proto-concat-nonstd-trailing.js>`_.
If this option is given, ``concat()`` will behave in a strictly conforming
fashion, ignoring non-existent trailing elements in the result ``length``.

DUK_OPT_NO_NONSTD_ARRAY_MAP_TRAILER
-----------------------------------

For better compatibility with existing code, ``Array.prototype.map()``
has non-standard behavior by default for trailing non-existent elements
of the map result, see
`test-bi-array-proto-map-nonstd-trailing.js <https://github.com/svaarala/duktape/blob/master/tests/ecmascript/test-bi-array-proto-map-nonstd-trailing.js>`_.
If this option is given, ``map()`` will behave in a strictly conforming
fashion, ignoring non-existent trailing elements in the result ``length``.

DUK_OPT_NO_NONSTD_JSON_ESC_U2028_U2029
--------------------------------------

By default Duktape JSON.stringify() will escape U+2028 and U+2029 which
is non-compliant behavior.  This is the default to make JSON.stringify()
output valid when embedded in a web page or parsed with ``eval()``.  This
feature option enables the compliant behavior, i.e. no escaping for U+2028
and U+2029.

DUK_OPT_NO_NONSTD_STRING_FROMCHARCODE_32BIT
-------------------------------------------

By default Duktape String.fromCharCode() allows 32-bit codepoints which is
non-compliant (the E5.1 specification has a ToUint16() coercion for the
codepoints) but useful because Duktape supports non-BMP strings.  This
feature option restores the compliant behavior.

DUK_OPT_NO_NONSTD_ARRAY_WRITE
-----------------------------

By default Duktape uses a fast path for handling some property writes to
Array instances.  The fast path improves performance for common array writes
but is technically non-compliant.  There's a detectable outside difference
only when Array.prototype has conflicting numeric properties (which is very
rare in practice).  See
`tests/ecmascript/test-misc-array-fast-write.js <https://github.com/svaarala/duktape/blob/master/tests/ecmascript/test-misc-array-fast-write.js>`_
for details on the fast path conditions and behavior.

This feature option enables the compliant (but slower) behavior.

DUK_OPT_NO_COMMONJS_MODULES
---------------------------

Disable support for CommonJS modules.  Causes ``require()`` to throw an
error.

DUK_OPT_NO_ES6_OBJECT_PROTO_PROPERTY
------------------------------------

Disable the non-standard (ES6) ``Object.prototype.__proto__``
property which is enabled by default.

DUK_OPT_NO_ES6_OBJECT_SETPROTOTYPEOF
------------------------------------

Disable the non-standard (ES6) ``Object.setPrototypeOf`` method
which is enabled by default.

DUK_OPT_NO_ES6_PROXY
--------------------

Disable the non-standard (ES6) ``Proxy`` object which is enabled
by default.

DUK_OPT_NO_JX
-------------

Disable support for the JX format.  Reduces code footprint.  An attempt
to encode or decode the format causes an error.

DUK_OPT_NO_JC
-------------

Disable support for the JC format.  Reduces code footprint.  An attempt
to encode or decode the format causes an error.

DUK_OPT_LIGHTFUNC_BUILTINS
--------------------------

**Experimental.**

Force built-in functions to be lightweight functions.  This reduces
memory footprint by around 14 kB at the cost of some non-compliant
behavior.

DUK_OPT_NO_BUFFEROBJECT_SUPPORT
-------------------------------

Disable support for Node.js Buffer and Khronos/ES6 typed arrays, plain
buffers and Duktape.Buffer will still be supported.  Saves some code
footprint and may be useful for low memory targets.

C API options
=============

DUK_OPT_NO_BYTECODE_DUMP_SUPPORT
--------------------------------

Disable support for bytecode dump/load in C API, reduces code footprint.

Execution and debugger options
==============================

DUK_OPT_INTERRUPT_COUNTER
-------------------------

Enable the internal bytecode executor periodic interrupt counter.
The mechanism is used to implement e.g. execution step limit, custom
profiling, and debugger interaction.  Enabling the interrupt counter
has a small impact on execution performance.

DUK_OPT_EXEC_TIMEOUT_CHECK
--------------------------

**Experimental.**

Provide a hook to check for bytecode execution timeout.  The macro gets
a ``void *`` userdata argument (the userdata given to ``duk_heap_create()``)
and must evaluate to a ``duk_bool_t``.  Duktape calls it as::

    if (DUK_OPT_EXEC_TIMEOUT_CHECK(udata)) { ... }

The macro is called occasionally by the Duktape bytecode executor (i.e. when
executing Ecmascript code), typically from a few times per second to a hundred
times per second, but the interval varies a great deal depending on what kind
of code is being executed.

To indicate an execution timeout, the macro must return a non-zero value.
When that happens, Duktape starts to bubble a ``RangeError`` outwards
until control has been returned to the original protected call made by
the application.  Until that happens, the exec timeout macro must always
return non-zero to indicate an execution timeout is still in progress.

This mechanism and its limitations is described in more detail in
``doc/sandboxing.rst``.

This option requires ``DUK_OPT_INTERRUPT_COUNTER``.

.. note:: This mechanism is EXPERIMENTAL and the details may change
          between releases.

DUK_OPT_DEBUGGER_SUPPORT
------------------------

Enable support for Duktape debug protocol (see ``doc/debugger.rst``) and the
debug API calls (``duk_debugger_attach()``, ``duk_debugger_detach()`` etc).
This adds about 10kB of code footprint at the moment.

This option requires ``DUK_OPT_INTERRUPT_COUNTER``.

DUK_OPT_DEBUGGER_FWD_PRINTALERT
-------------------------------

Forward calls to the built-in ``print()`` and ``alert()`` function to the
debug client.

DUK_OPT_DEBUGGER_FWD_LOGGING
----------------------------

Forward log writes using the built-in logging framework to the debug client.
Forwarding happens from the ``Duktape.Logger.prototype.info()`` etc calls
before the ``raw()`` function is called, so that logging is forwarded even
if you replace the backend.

DUK_OPT_TARGET_INFO
-------------------

Define a freeform human readable string to describe the target device (e.g.
"Arduino Yun").  This string will be sent as part of version/target info in
the debugger protocol and shows up in the debugger UI.

DUK_OPT_DEBUGGER_DUMPHEAP
-------------------------

Support the DumpHeap command.  This is optional because the command is not
always needed.  The command also has a relatively large footprint (about 10%
of debugger code); in absolute terms it's about 1kB of code footprint.

DUK_OPT_DEBUGGER_TRANSPORT_TORTURE
----------------------------------

Development time option: force debugger transport torture.  Concretely this
now causes Duktape to read/write debug protocol data in 1-byte increments,
which stresses message parsing and transport code.

Debugging options
=================

Options for development time debug prints and such.  Not to be confused with
debugger options.

DUK_OPT_SELF_TESTS
------------------

Perform run-time self tests when a Duktape heap is created.  Catches
platform/compiler problems which cannot be reliably detected during
compile time.  Not enabled by default because of the extra footprint.

DUK_OPT_ASSERTIONS
------------------

Enable internal assert checks.  These slow down execution considerably
so only use when debugging.

DUK_OPT_DEBUG
-------------

Enable debug code in Duktape internals.  Without this option other
debugging options (such as ``DUK_OPT_DPRINT``) have no effect.

DUK_OPT_DPRINT
--------------

Enable debug printouts.

DUK_OPT_DDPRINT
---------------

Enable more debug printouts.

DUK_OPT_DDDPRINT
----------------

Enable even more debug printouts.  Not recommended unless you have
grep handy.

DUK_OPT_DPRINT_COLORS
---------------------

Enable coloring of debug prints with
`ANSI escape codes <http://en.wikipedia.org/wiki/ANSI_escape_code>`_.
The behavior is not sensitive to terminal settings.

DUK_OPT_DPRINT_RDTSC
--------------------

Print RDTSC cycle count in debug prints if available.

DUK_OPT_DEBUG_BUFSIZE
---------------------

Debug code uses a static buffer as a formatting temporary to avoid side
effects in debug prints.  The static buffer is large by default, which may
be an issue in constrained environments.  You can set the buffer size
manually with this option.  Example::

    -DDUK_OPT_DEBUG_BUFSIZE=2048

DUK_OPT_NO_ZERO_BUFFER_DATA
---------------------------

By default Duktape zeroes data allocated for buffer values.  Define
this to disable the zeroing (perhaps for performance reasons).

DUK_OPT_SHUFFLE_TORTURE
-----------------------

Development time option: force compiler to shuffle every possible opcode
to stress shuffle behavior which is otherwise difficult to test for
comprehensively.

Using DUK_OPT_HAVE_CUSTOM_H and duk_custom.h
============================================

Normally you define ``DUK_OPT_xxx`` feature options and the internal
``duk_features.h`` header resolves these with platform/compiler constraints
to determine effective compilation options for Duktape internals.  The
effective options are provided as ``DUK_USE_xxx`` defines which you normally
never see.

If you define ``DUK_OPT_HAVE_CUSTOM_H``, Duktape will include
``duk_custom.h`` after determining the appropriate ``DUK_USE_xxx`` defines
but before compiling any code.  The ``duk_custom.h`` header, which you
provide, can then tweak the active ``DUK_USE_xxx`` defines freely.  See
``duk_features.h`` for the available defines.

This approach is useful when the ``DUK_OPT_xxx`` feature options don't
provide enough flexibility to tweak the build.  The downside is that you can
easily create inconsistent ``DUK_USE_xxx`` flags, the customization header
will be version specific, and you need to peek into Duktape internals to
know what defines to tweak.

Using DUK_OPT_PANIC_HANDLER
===========================

The default panic handler will print an error message to stdout unless I/O is
disabled by ``DUK_OPT_NO_FILE_IO``.  It will then call ``abort()`` or cause a
segfault if ``DUK_OPT_SEGFAULT_ON_PANIC`` is defined.

You can override the entire panic handler by defining
``DUK_OPT_PANIC_HANDLER``.  For example, you could add the following to your
compiler options::

    '-DDUK_OPT_PANIC_HANDLER(code,msg)={printf("*** %d:%s\n",(code),(msg));abort();}'

You can also use::

    '-DDUK_OPT_PANIC_HANDLER(code,msg)={my_panic_handler((code),(msg))}'

which calls your custom handler::

    void my_panic_handler(int code, const char *msg) {
        /* Your panic handling.  Must not return. */
    }

The ``DUK_OPT_PANIC_HANDLER`` macro is used internally by Duktape, so your
panic handler function needs to be declared for Duktape compilation to avoid
compiler warnings about undeclared functions.  You can "inject" a declaration
for your function into Duktape compilation with::

    '-DDUK_OPT_DECLARE=extern void my_panic_handler(int code, const char *msg);'

After this you might still get a compilation warning like "a noreturn function
must not return" as the compiler doesn't know your panic handler doesn't
return.  You can fix this by either using a (compiler specific) "noreturn"
declaration, or by modifying the panic handler macro to something like::

    '-DDUK_OPT_PANIC_HANDLER(code,msg)={my_panic_handler((code),(msg));abort()}'

As ``abort()`` is automatically a "noreturn" function the panic macro body
can no longer return.  Duktape always includes ``stdlib.h`` which provides
the ``abort()`` prototype so no additional include files are needed.

Memory management alternatives
==============================

There are three supported memory management alternatives:

* **Reference counting and mark-and-sweep (default)**: heap objects are
  freed immediately when they become unreachable except for objects
  participating in unreachable reference cycles.  Such objects are freed by
  a periodic voluntary, stop the world mark-and-sweep collection.
  Mark-and-sweep is also used as the emergency garbage collector if memory
  allocation fails.

* **Reference counting only**: reduces code footprint and eliminates garbage
  collection pauses, but objects in unreachable reference cycles are not
  collected until the Duktape heap is destroyed.  This alternative is not
  recommended unless the reference cycles are not an issue.  See notes below.

* **Mark-and-sweep only**: reduces code footprint and memory footprint (heap
  headers don't need to store a reference count), but there is more memory
  usage variance than in the default case.  The frequency of voluntary, stop
  the world mark-and-sweep collections is also higher than in the default
  case where reference counting is expected to handle almost all memory
  management.

When using only reference counting it is important to avoid creating
unreachable reference cycles.  Reference cycles are usually easy to avoid in
application code e.g. by using only forward pointers in data structures.  Even
if reference cycles are necessary, garbage collection can be allowed to work
simply by breaking the cycles before deleting the final references to such objects.
For example, if you have a tree structure where nodes maintain references to
both children and parents (creating reference cycles for each node) you could
walk the tree and set the parent reference to ``null`` before deleting
the final reference to the tree.

Unfortunately every Ecmascript function instance is required to be in a
reference loop with an automatic prototype object created for the function.
You can break this loop manually if you wish.  For internal technical reasons,
named function expressions are also in a reference loop; this loop cannot be
broken from user code and only mark-and-sweep can collect such functions.
See `Limitations <http://duktape.org/guide.html#limitations>`_.

Development notes
=================

This section only applies if you customize Duktape internals and wish to
submit a patch to be included in the mainline distribution.

Adding new config options
-------------------------

* Add a descriptive ``DUK_USE_xxx`` for the custom feature.  Use only this
  define inside Duktape source code (never add any compiler/platform #ifdefs
  inside Duktape).

* Add config option metadata for genconfig; see existing metadata files in
  ``config/config-options/``.  Remember to add a useful default value and
  a good description for the new option.  Ensure config option documentation
  still builds and your option looks good in the documentation.

Removing config options
-----------------------

* If the feature option has been a part of a stable release, the first step
  is to mark the option deprecated in the option metadata.  An option should
  be deprecated for one minor release before being removed.

* The next step is to mark the option removed in the option metadata.  The
  option is never completely removed from the metadata, so that it is possible
  to autogenerate checks for removed options.  This is useful so that users can
  be warned that options they're using are no longer available.
