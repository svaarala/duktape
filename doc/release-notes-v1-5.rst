=========================
Duktape 1.5 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* Low memory improvements: support for ROM strings and objects, reduce string
  memory footprint by 4 bytes.

* Debugger improvements: heap object inspection, application specific
  commands and notifications, duk_debugger_pause() API, DukLuv-based
  debug proxy for easier packaging of a proxy into an application.

* Improve error instance .stack format, minor changes to error message
  strings.

* CommonJS module improvements: module.filename and module.name support,
  improve stack trace formatting of require() calls and module wrapper
  functions.

* Emscripten compatibility improvements (Emscripten code can now be executed
  without fixups): RegExp parser accepts unescaped curly braces, Function
  ``.toString()`` output format has been modified slightly to match Emscripten
  expectations.

* Minor fixes, performance, and portability improvements.

Upgrading from Duktape 1.4.x
============================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v1.4.x.  Note the following:

* When a debugger is attached and Duktape is in a paused state garbage
  collection is now disabled by default.  As a result, garbage created during
  the paused state will not be collected immediately, but may remain in the
  Duktape heap until the next mark-and-sweep pass after resuming execution.
  This also postpones finalizer execution for such garbage.  Preventing
  garbage collection during paused state allows debugger heap inspection
  commands to work reliably for even temporary values created using e.g. Eval.

* When serializing duk_tval numbers, the debugger implementation now uses
  plain integer dvalues instead of full IEEE number dvalues when it's safe to
  do so without loss of information.  For example, if you Eval "1+2" the
  result (3) is serialized as a plain integer.  This is allowed by the
  debugger protocol but hasn't been done before, so it may have an effect on
  a debug client which assumes, for instance, that Eval result numbers are
  always in IEEE double format.

* Because the format of error ``.stack`` property has been changed in this
  release, any code parsing the stack trace format may need changes.

* Because the Function ``.toString()`` output format has been changed in this
  release (to be more Emscripten compatible), any code expecting a specific
  ``.toString()`` output format may need changes.

There are bug fixes and other minor behavioral changes which may affect some
applications, see ``RELEASES.rst`` for details.

Known issues
============

This release has the following known issues worth noting:

* Non-compliant behavior for array indices near 2G or 4G elements.

* RegExp parser is strict and won't accept some real world RegExps which
  are technically not compliant with ECMAScript E5/E5.1 specification.

* Final mantissa bit rounding issues in the internal number-to-string
  conversion.

* On FreeBSD 10.x (at least 10.1 and 10.2): Clang with ``-m32`` generates
  incorrect code for union assignments needed by Duktape's 8-byte packed
  value encoding (see
  https://github.com/svaarala/duktape/blob/master/misc/clang_aliasing.c).
  The issue can be detected by defining ``DUK_OPT_SELF_TESTS``.  A workaround
  is to avoid packed types in this case by defining ``DUK_OPT_NO_PACKED_TVAL``.

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
    test-bi-function-nonstd-caller-prop: fail; 178 diff lines; requires special feature options: DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY
    test-bi-global-parseint: fail; 108 diff lines; known issue: rounding differences for parsing integers larger than 2^53
    test-bi-json-dec-types: fail; 21 diff lines; known issue: '\x' should be allowed by eval() but not by JSON.parse(), Duktape rejects '\x' in both
    test-bi-json-enc-proplist-dups: fail; 8 diff lines; known issue: JSON.stringify() can be given a property list to serialize; duplicates should be eliminated but Duktape (and other engines) will happily serialize a property multiple times
    test-bi-json-enc-proxy: fail; 18 diff lines; known issue: JSON enumeration behavior for Proxy targets is incomplete and uses 'enumerate' trap instead of 'ownKeys' trap
    test-bi-number-proto-toexponential: fail; 75 diff lines; known issue: corner case rounding errors in toExponential()
    test-bi-number-proto-tostring: fail; 46 diff lines; known issue: expect strings to be checked, but probably Duktape rounding issues
    test-bi-proxy-object-tostring: fail; 6 diff lines; known issue: Object class handling for Proxy objects is incomplete
    test-bi-regexp-gh39: fail; 5 diff lines; known issue: requires leniency for non-standard regexps
    test-bug-dataview-buffer-prop: fail; 20 diff lines; known issue: DataView .buffer property misleading when DataView argument is not an ArrayBuffer (custom behavior)
    test-bug-enum-shadow-nonenumerable: fail; 12 diff lines; known issue: corner case enumeration semantics, not sure what correct behavior is (test262 ch12/12.6/12.6.4/12.6.4-2)
    test-bug-invalid-oct-as-dec: fail; 16 diff lines; known issue: V8/Rhino parse invalid octal constants as decimal values, Duktape doesn't at the moment
    test-bug-json-parse-__proto__: fail; 18 diff lines; known issue: when ES6 __proto__ enabled, JSON.parse() parses '__proto__' property incorrectly when a specially crafted reviver is used
    test-bug-numconv-1e23: fail; 10 diff lines; known issue: corner case in floating point parse rounding
    test-bug-numconv-denorm-toprec: fail; 7 diff lines; known issue: in a denormal corner case toPrecision() can output a zero leading digit
    test-bug-tonumber-u0000: fail; 7 diff lines; known issue: '\u0000' should ToNumber() coerce to NaN, but now coerces to zero like an empty string
    test-dev-bound-thread-start-func: fail; 13 diff lines; known issue: initial function of a new coroutine cannot be bound
    test-dev-func-cons-args: fail; 18 diff lines; known issue: corner cases for 'new Function()' when arguments and code are given as strings
    test-dev-lightfunc-accessor: fail; 50 diff lines; requires special feature options: DUK_OPT_LIGHTFUNC_BUILTINS
    test-dev-lightfunc-finalizer: fail; 8 diff lines; requires special feature options: DUK_OPT_LIGHTFUNC_BUILTINS
    test-dev-lightfunc: fail; 459 diff lines; requires special feature options: DUK_OPT_LIGHTFUNC_BUILTINS
    test-dev-yield-after-callapply: fail; 8 diff lines; known issue: yield() not allowed when function called via Function.prototype.(call|apply)()
    test-lex-unterminated-hex-uni-escape: fail; 29 diff lines; known issue: unterminated hex escapes should be parsed leniently, e.g. '\uX' -> 'uX' but Duktape now refuses to parse them
    test-numconv-parse-misc: fail; 12 diff lines; known issue: rounding corner case for 1e+23 (parses/prints as 1.0000000000000001e+23)
    test-numconv-tostring-gen: fail; 257 diff lines; known issue: rounding corner cases in number-to-string coercion
    test-numconv-tostring-misc: fail; 6 diff lines; known issue: rounding corner case, 1e+23 string coerces to 1.0000000000000001e+23
    test-regexp-empty-quantified: fail; 15 diff lines; known issue: a suitable empty quantified (e.g. '(x*)*') causes regexp parsing to terminate due to step limit
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
