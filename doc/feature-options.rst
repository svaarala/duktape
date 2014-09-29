=======================
Duktape feature options
=======================

Overview
========

The effective set of Duktape features is resolved in three steps:

* User defines ``DUK_OPT_xxx`` feature options.  These are essentially
  requests to enable/disable some feature.

* Duktape feature resolution in ``duk_features.h.in`` takes into account
  the requested features, the platform, the compiler, the operating system
  etc, and defines ``DUK_USE_xxx`` internal use flags.  Other parts of
  Duktape only listen to these "use flags", so that feature resolution is
  strictly contained.

* User may optionally have a ``duk_custom.h`` header which can further
  tweak the effective ``DUK_USE_xxx`` defines.  This is a last resort and
  is somewhat fragile.  See ``DUK_OPT_HAVE_CUSTOM_H`` for more discussion.

This document describes all the supported Duktape feature options and should
be kept up-to-date with new features.  The feature option list in the guide
is a subset of the most commonly needed features.

See also:

- ``low-memory.rst``: suggested options for low memory environments

- ``timing-sensitive.rst``: suggested options for timing sensitive environments

- ``src/duk_features.h.in``: resolution of feature options to use flags

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

Memory management options
=========================

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

..note:: Unimplemented as of Duktape 0.12.0.

DUK_OPT_NO_NONSTD_ARRAY_SPLICE_DELCOUNT
---------------------------------------

For better compatibility with existing code, ``Array.prototype.splice()``
has non-standard behavior by default when the second argument (deleteCount)
is not given: the splice operation is extended to the end of the array.
If this option is given, ``splice()`` will behave in a strictly
conforming fashion, treating a missing deleteCount the same as an undefined
(or 0) value.

DUK_OPT_NO_NONSTD_ARRAY_CONCAT_TRAILER
--------------------------------------

For better compatibility with existing code, ``Array.prototype.concat()``
has non-standard behavior by default for trailing non-existent elements of
the concat result, see
`test-bi-array-proto-concat-nonstd-trailing.js <https://github.com/svaarala/duktape/blob/master/ecmascript-testcases/test-bi-array-proto-concat-nonstd-trailing.js>`_.
If this option is given, ``concat()`` will behave in a strictly conforming
fashion, ignoring non-existent trailing elements in the result ``length``.

DUK_OPT_NO_NONSTD_ARRAY_MAP_TRAILER
-----------------------------------

For better compatibility with existing code, ``Array.prototype.map()``
has non-standard behavior by default for trailing non-existent elements
of the map result, see
`test-bi-array-proto-map-nonstd-trailing.js <https://github.com/svaarala/duktape/blob/master/ecmascript-testcases/test-bi-array-proto-map-nonstd-trailing.js>`_.
If this option is given, ``map()`` will behave in a strictly conforming
fashion, ignoring non-existent trailing elements in the result ``length``.

DUK_OPT_NO_COMMONJS_MODULES
---------------------------

Disable support for CommonJS modules.  Causes ``require()`` to throw an
error.

DUK_OPT_NO_ES6_OBJECT_PROTO_PROPERTY
------------------------------------

Disable the non-standard (ES6 draft) ``Object.prototype.__proto__``
property which is enabled by default.

DUK_OPT_NO_ES6_OBJECT_SETPROTOTYPEOF
------------------------------------

Disable the non-standard (ES6 draft) ``Object.setPrototypeOf`` method
which is enabled by default.

DUK_OPT_NO_ES6_PROXY
--------------------

Disable the non-standard (ES6 draft) ``Proxy`` object which is enabled
by default.

DUK_OPT_NO_JX
-------------

Disable support for the JX format.  Reduces code footprint.  An attempt
to encode or decode the format causes an error.

DUK_OPT_NO_JC
-------------

Disable support for the JC format.  Reduces code footprint.  An attempt
to encode or decode the format causes an error.

Debugging options
=================

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

DUK_OPT_NO_INTERRUPT_COUNTER
----------------------------

Disable the internal bytecode executor periodic interrupt counter.
The mechanism is used to implement e.g. execution step limit, custom
profiling, and debugger interaction.  Disabling the interrupt counter
improves bytecode execution performance very slightly but disables all
features depending on it.

.. note:: Disabled for the 1.0 release because there is no API to use it.

DUK_OPT_NO_ZERO_BUFFER_DATA
---------------------------

By default Duktape zeroes data allocated for buffer values.  Define
this to disable the zeroing (perhaps for performance reasons).

Using DUK_OPT_HAVE_CUSTOM_H and duk_custom.h
============================================

Normally you define ``DUK_OPT_xxx`` feature options and the internal
``duk_features.h`` header resolves these with platform/compiler constraints
to determine effective compilation options for Duktape internals.  The
effective options are provided as ``DUK_USE_xxx`` defines which you normally
never see.

>If you define ``DUK_OPT_HAVE_CUSTOM_H``, Duktape will include
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

Adding new feature options
--------------------------

* Add a descriptive ``DUK_OPT_xxx`` for the custom feature.  The custom
  feature should only be enabled if the feature option is explicitly given.

* Modify ``duk_features.h.in`` to detect your custom feature option and define
  appropriate internal ``DUK_USE_xxx`` define(s).  Conflicts with other
  features should be detected.  Code outside ``duk_features.h.in`` should only
  listen to ``DUK_USE_xxx`` defines so that the resolution process is fully
  contained in ``duk_features.h.in``.

Removing feature options
------------------------

* If the feature option has been a part of a stable release, add a check
  for it in ``duk_feature_sanity.h.in``.  If the option is present, the
  build should error out with a deprecation notice.  This is preferable to
  silently removing an option a user may be depending on.
