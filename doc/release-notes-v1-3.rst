=========================
Duktape 1.3 release notes
=========================

Release overview
================

* Extended buffer support with Khronos/ES6 TypedArray API (subset of ES6 API),
  Node.js Buffer API, and related C API changes

* Move to use an external duk_config.h header to improve portability and make
  Duktape easier to use as a library

* Support module.exports

* Bytecode dump/load support

* New API calls like duk_instanceof() and duk_pnew()

* Performance improvements for e.g. bytecode opcode dispatch, JSON parsing and
  serialization, and lexer/compiler buffer handling

Upgrading from Duktape 1.2.x
============================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v1.2.x.  Note the following:

* There's a new header file, duk_config.h, which must be in the include path.
  A default duk_config.h file is present in dist src/ and src-separate/
  directories.

* Duktape.modLoaded internal format has changed as a result of module.exports
  support.

* Behavior for a CommonJS module load error has changed: the offending module
  is now de-registered instead of a partial module being cached.

* DUK_OPT_DEEP_C_STACK has been removed and Duktape defaults to deep stacks on
  all platforms.  There are explicit C stack options for platforms with a
  shallow stack (e.g. DUK_USE_NATIVE_CALL_RECLIMIT, DUK_USE_COMPILER_RECLIMIT).

* Debugger breakpoint triggering behavior has changed so that breakpoints are
  only triggered when execution enters the exact breakpoint line.  Breakpoints
  on lines without any executable code are ignored.

There are bug fixes and other minor behavioral changes which may affect some
applications, see ``RELEASES.rst`` for details.

Known issues
============

This release has the following known issues worth noting.

Ecmascript features
-------------------

* Non-compliant behavior for array indices near 2G or 4G elements.

* RegExp parser is strict and won't accept some real world RegExps which
  are technically not compliant with Ecmascript E5/E5.1 specification.

* Final mantissa bit rounding issues in the internal number-to-string
  conversion.

* If a function contains a lot of constants (over 511 strings or non-integers),
  the compiler fails to compile a try-catch statement.  The workaround is to
  place the try-catch in a separate function outside the problematic function.

Portability and platforms
-------------------------

* On some older clang/llvm versions (e.g. Clang 3.3 on FreeBSD 10):
  when ``-m32`` is used, Duktape will end up using the 8-byte packed value
  representation but clang will generate incorrect code for union assignment
  (see ``misc/clang_aliasing.c``).  The issue can be detected by defining
  ``DUK_OPT_SELF_TESTS``.  A workaround is to avoid packed types in this
  case by defining ``DUK_OPT_NO_PACKED_TVAL``.

* On some older clang/llvm versions (e.g. Clang 3.3 on FreeBSD 10):
  compilation may produce a warning "generic selections are a C11-specific
  feature".  The warning should be harmless.

* On some older clang/llvm versions (e.g. Clang 3.3 on FreeBSD 10):
  harmless compilation warning for "duk_repl_isinf" being unused.

* On some GCC versions and compilation options you may get a warning
  "variable idx_func might be clobbered by longjmp or vfork [-Wclobbered]".
  This warning seems spurious and causes no known problems.

* GCC ``-O4`` may produce a warning "assuming signed overflow does not occur
  when assuming that (X - c) > X is always false" for some assertions.  This
  warning seems spurious and causes no known problems.

* GCC ``-pedantic`` without ``-std=c99`` causes the ``unsigned long long``
  type to be used by Duktape, and an associated warning about the type.
  This is harmless and most easily fixed by simply using ``-std=c99``.

* The JSON.stringify() fast path (DUK_USE_JSON_STRINGIFY_FASTPATH) assumes
  that "%lld" format specifier is correct for the "unsigned long long" type.

Raw issues from test runs
=========================

**FIXME: to be updated**

API tests
---------

Ecmascript tests
----------------

test262
-------
