=========================
Duktape 1.3 release notes
=========================

Release overview
================

This release adds multiple new features and performance improvements:

* Extend buffer support with Khronos/ES6 TypedArray API (subset of ES6 API),
  Node.js Buffer API, external buffers, and related C API additions and
  changes; see
  http://wiki.duktape.org/HowtoBuffers.html

* Move to use an external duk_config.h header to improve portability and make
  Duktape easier to use as a library; this is a work in progress, see
  https://github.com/svaarala/duktape/blob/master/doc/duk-config.rst

* Support module.exports

* Bytecode dump/load support, and related C API additions

* New API calls ``duk_instanceof()`` and ``duk_pnew()``

* Performance improvements for e.g. bytecode opcode dispatch, JSON parsing and
  serialization, and lexer/compiler buffer handling

Upgrading from Duktape 1.2.x
============================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v1.2.x.  Note the following:

* There's a new header file, ``duk_config.h``, which must be in the include
  path. A default duk_config.h file, compatible with Duktape 1.2, is present
  in dist src/ and src-separate/ directories.

* Duktape.modLoaded internal format has changed as a result of module.exports
  support.  This should not matter unless you need to interact with
  Duktape.modLoaded directly for some reason.

* Behavior for a CommonJS module load error has changed: the offending module
  is now de-registered instead of a partial module being cached.  This allows
  calling code to retry the failing ``require()`` call.

* DUK_OPT_DEEP_C_STACK has been removed and Duktape defaults to deep stacks
  on all platforms.  There are explicit C stack options for (rare) platforms
  with a shallow stack (e.g. DUK_USE_NATIVE_CALL_RECLIMIT,
  DUK_USE_COMPILER_RECLIMIT).

* Debugger breakpoint triggering behavior has changed so that breakpoints are
  only triggered when execution enters the exact breakpoint line.  Breakpoints
  on lines without any executable code are ignored.

* Some example files have been renamed.  For example,
  ``examples/debug-trans-socket/duk_debug_trans_socket.*`` have been renamed
  to ``examples/debug-trans-socket/duk_trans_socket.*``.

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

* MSVC ``/Wp64`` produces harmless warnings when compiling for x86.  The
  warnings are caused by 64-bit incompatible code enabled for 32-bit targets;
  when you actually compile for a 64-bit target, those code paths are not
  used so the warnings are irrelevant.

* The JSON.stringify() fast path (DUK_USE_JSON_STRINGIFY_FASTPATH) assumes
  that "%lld" format specifier is correct for the "unsigned long long" type.

Raw issues from test runs
=========================

API tests
---------

See ``testcase-known-issues.yaml``::

    test-to-number.c: fail; 15 diff lines; known issue: number parsing bug for strings containing NUL characters (e.g. '\u0000')


Ecmascript tests
----------------

See ``testcase-known-issues.yaml``::

    test-bi-array-proto-push: fail; 30 diff lines; known issue: array length above 2^32-1 not supported
    test-bi-array-push-maxlen: fail; 17 diff lines; known issue: array length above 2^32-1 not supported
    test-bi-date-tzoffset-brute-fi: fail; 12 diff lines; known issue: year 1970 deviates from expected, Duktape uses equiv. year for 1970 on purpose at the moment; requires special feature options: test case has been written for Finnish locale
    test-bi-function-nonstd-caller-prop: fail; 178 diff lines; requires special feature options: DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY
    test-bi-global-parseint-oct: fail; 20 diff lines; known issue: non-standard octal behavior does not match V8/Rhino
    test-bi-global-parseint: fail; 108 diff lines; known issue: rounding differences for parsing integers larger than 2^53
    test-bi-json-dec-types: fail; 21 diff lines; known issue: '\x' should be allowed by eval() but not by JSON.parse(), Duktape rejects '\x' in both
    test-bi-json-enc-proplist-dups: fail; 8 diff lines; known issue: JSON.stringify() can be given a property list to serialize; duplicates should be eliminated but Duktape (and other engines) will happily serialize a property multiple times
    test-bi-number-proto-toexponential: fail; 75 diff lines; known issue: corner case rounding errors in toExponential()
    test-bi-number-proto-tostring: fail; 46 diff lines; known issue: expect strings to be checked, but probably Duktape rounding issues
    test-bi-regexp-gh39: fail; 5 diff lines; known issue: requires leniency for non-standard regexps
    test-bug-dataview-buffer-prop: fail; 20 diff lines; known issue: DataView .buffer property misleading when DataView argument is not an ArrayBuffer (custom behavior)
    test-bug-enum-shadow-nonenumerable: fail; 12 diff lines; known issue: corner case enumeration semantics, not sure what correct behavior is (test262 ch12/12.6/12.6.4/12.6.4-2)
    test-bug-invalid-oct-as-dec: fail; 14 diff lines; known issue: V8/Rhino parse invalid octal constants as decimal values, Duktape doesn't at the moment
    test-bug-json-parse-__proto__: fail; 18 diff lines; known issue: when ES6 __proto__ enabled, JSON.parse() parses '__proto__' property incorrectly when a specially crafted reviver is used
    test-bug-numconv-1e23: fail; 10 diff lines; known issue: corner case in floating point parse rounding
    test-bug-numconv-denorm-toprec: fail; 7 diff lines; known issue: in a denormal corner case toPrecision() can output a zero leading digit
    test-bug-tonumber-u0000: fail; 7 diff lines; known issue: '\u0000' should ToNumber() coerce to NaN, but now coerces to zero like an empty string
    test-dev-bound-thread-start-func: fail; 13 diff lines; known issue: initial function of a new coroutine cannot be bound
    test-dev-func-cons-args: fail; 18 diff lines; known issue: corner cases for 'new Function()' when arguments and code are given as strings
    test-dev-lightfunc-accessor: fail; 50 diff lines; requires special feature options: DUK_OPT_LIGHTFUNC_BUILTINS
    test-dev-lightfunc-finalizer: fail; 8 diff lines; requires special feature options: DUK_OPT_LIGHTFUNC_BUILTINS
    test-dev-lightfunc: fail; 462 diff lines; requires special feature options: DUK_OPT_LIGHTFUNC_BUILTINS
    test-dev-yield-after-callapply: fail; 8 diff lines; known issue: yield() not allowed when function called via Function.prototype.(call|apply)()
    test-lex-unterminated-hex-uni-escape: fail; 29 diff lines; known issue: unterminated hex escapes should be parsed leniently, e.g. '\uX' -> 'uX' but Duktape now refuses to parse them
    test-numconv-parse-misc: fail; 12 diff lines; known issue: rounding corner case for 1e+23 (parses/prints as 1.0000000000000001e+23)
    test-numconv-tostring-gen: fail; 257 diff lines; known issue: rounding corner cases in number-to-string coercion
    test-numconv-tostring-misc: fail; 6 diff lines; known issue: rounding corner case, 1e+23 string coerces to 1.0000000000000001e+23
    test-regexp-empty-quantified: fail; 15 diff lines; known issue: a suitable empty quantified (e.g. '(x*)*') causes regexp parsing to terminate due to step limit
    test-regexp-invalid-charclass: fail; 7 diff lines; known issue: some invalid character classes are accepted (e.g. '[\d-z]' and '[z-x]')
    test-regexp-nonstandard-patternchar: fail; 6 diff lines; known issue: other engines allow an unescaped brace to appear literally (e.g. /{/), Duktape does not (which seems correct but is against real world behavior)
    test-stmt-for-in-lhs: fail; 29 diff lines; known issue: for-in allows some invalid left-hand-side expressions which cause a runtime ReferenceError instead of a compile-time SyntaxError (e.g. 'for (a+b in [0,1]) {...}')

test262
-------

See ``test262-status.rst`` and ``test262-known-issues.yaml``.  With Ecmascript 6 and Intl module tests removed::

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
