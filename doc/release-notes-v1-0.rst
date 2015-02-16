=========================
Duktape 1.0 release notes
=========================

Release overview
================

First release with a stable API.  This release will get bug fixes at least
until 1.1.0 is released (and probably some time beyond that).

**This release is now obsolete, please upgrade to Duktape 1.1.x which has
several critical fixes not present in 1.0.x.**

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

* GCC ``-pedantic`` without -std=c99 causes the ``unsigned long long`` type
  to be used by Duktape, and an associated warning about the type.  This is
  harmless and most easily fixed by simply using ``-std=c99``.

* MinGW and Cygwin GCC compilation produces symbol visibility warnings when
  compiling Duktape from ``src-separate``.

* On platforms where ``SIZE_MAX`` is incorrectly set e.g. as ``0x7fffffff``
  compilation may fail with ``#error size_t is too small``.  This has been
  fixed in Duktape 1.1.0.  (GH-101)

Raw issues from test runs
=========================

API tests
---------

::

    test-to-number.c: fail; 15 diff lines; known issue: number parsing bug for strings containing NUL characters (e.g. '')

Ecmascript tests
----------------

::

    test-bi-date-tzoffset-brute-fi: fail; 12 diff lines; known issue: year 1970 deviates from expected, Duktape uses equiv. year for 1970 on purpose at the moment; requires special feature options: test case has been written for Finnish locale
    test-bi-function-nonstd-caller-prop: fail; 178 diff lines; requires special feature options: DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY
    test-bi-global-parseint-oct: fail; 20 diff lines; known issue: non-standard octal behavior does not match V8/Rhino
    test-bi-global-parseint: fail; 108 diff lines; known issue: rounding differences for parsing integers larger than 2^53
    test-bi-json-dec-types: fail; 21 diff lines; known issue: '\x' should be allowed by eval() but not by JSON.parse(), Duktape rejects '\x' in both
    test-bi-json-enc-proplist-dups: fail; 8 diff lines; known issue: JSON.stringify() can be given a property list to serialize; duplicates should be eliminated but Duktape (and other engines) will happily serialize a property multiple times
    test-bi-number-proto-toexponential: fail; 75 diff lines; known issue: corner case rounding errors in toExponential()
    test-bi-number-proto-tostring: fail; 46 diff lines; known issue: expect strings to be checked, but probably Duktape rounding issues
    test-bi-regexp-gh39: fail; 5 diff lines; known issue: requires leniency for non-standard regexps
    test-bug-enum-shadow-nonenumerable: fail; 12 diff lines; known issue: corner case enumeration semantics, not sure what correct behavior is (test262 ch12/12.6/12.6.4/12.6.4-2)
    test-bug-error-linenumber-2: fail; 10 diff lines; known issue: in corner cases (related to automatic semicolon insertion) throw statement error linenumber can be unexpected
    test-bug-invalid-oct-as-dec: fail; 14 diff lines; known issue: V8/Rhino parse invalid octal constants as decimal values, Duktape doesn't at the moment
    test-bug-json-parse-__proto__: fail; 18 diff lines; known issue: when ES6 __proto__ enabled, JSON.parse() parses '__proto__' property incorrectly when a specially crafted reviver is used
    test-bug-labelled-block: fail; 9 diff lines; known issue: label attached to a plain block statement can cause an INVALID opcode error
    test-bug-numconv-1e23: fail; 10 diff lines; known issue: corner case in floating point parse rounding
    test-bug-numconv-denorm-toprec: fail; 7 diff lines; known issue: in a denormal corner case toPrecision() can output a zero leading digit
    test-bug-tonumber-u0000: fail; 7 diff lines; known issue: '\u0000' should ToNumber() coerce to NaN, but now coerces to zero like an empty string
    test-bug-trycatch-many-constants: fail; 5 diff lines; known issue: out of regs for try-catch in Duktape 1.0
    test-conv-number-tostring-tonumber-roundtrip: fail; 5 diff lines; known issue: rounding corner cases
    test-dev-bound-thread-start-func: fail; 13 diff lines; known issue: initial function of a new coroutine cannot be bound
    test-dev-func-cons-args: fail; 18 diff lines; known issue: corner cases for 'new Function()' when arguments and code are given as strings
    test-dev-labelled-break: fail; 21 diff lines; known issue: label attached to a plain block statement can cause an INVALID opcode error
    test-dev-yield-after-callapply: fail; 8 diff lines; known issue: yield() not allowed when function called via Function.prototype.(call|apply)()
    test-lex-unterminated-hex-uni-escape: fail; 29 diff lines; known issue: unterminated hex escapes should be parsed leniently, e.g. '\uX' -> 'uX' but Duktape now refuses to parse them
    test-numconv-parse-misc: fail; 12 diff lines; known issue: rounding corner case for 1e+23 (parses/prints as 1.0000000000000001e+23)
    test-numconv-tostring-gen: fail; 279 diff lines; known issue: rounding corner cases in number-to-string coercion
    test-numconv-tostring-misc: fail; 6 diff lines; known issue: rounding corner case, 1e+23 string coerces to 1.0000000000000001e+23
    test-regexp-empty-quantified: fail; 15 diff lines; known issue: a suitable empty quantified (e.g. '(x*)*') causes regexp parsing to terminate due to step limit
    test-regexp-invalid-charclass: fail; 7 diff lines; known issue: some invalid character classes are accepted (e.g. '[\d-z]' and '[z-x]')
    test-regexp-nonstandard-patternchar: fail; 6 diff lines; known issue: other engines allow an unescaped brace to appear literally (e.g. /{/), Duktape does not (which seems correct but is against real world behavior)
    test-stmt-for-in: fail; 6 diff lines; known issue: for-in allows some invalid left-hand-side expressions which cause a runtime ReferenceError instead of a compile-time SyntaxError (e.g. 'for (a+b in [0,1]) {...}')

test262
-------

See ``test262-status.rst`` and ``test262-known-issues.json``.  With Ecmascript 6 and Intl module tests removed::

    annexB/B.RegExp.prototype.compile in non-strict mode   // KNOWN: RegExp.prototype.compile() not part of E5.1
    ch07/7.8/7.8.5/S7.8.5_A1.4_T1 in non-strict mode   // KNOWN: uses invalid RegExp formats, e.g. '/\1/' and '/\a/'
    ch07/7.8/7.8.5/S7.8.5_A1.4_T2 in non-strict mode   // KNOWN: uses invalid RegExp format '/\1/' (#0031)
    ch07/7.8/7.8.5/S7.8.5_A2.4_T1 in non-strict mode   // KNOWN: uses invalid RegExp format '/\1/'
    ch07/7.8/7.8.5/S7.8.5_A2.4_T2 in non-strict mode   // KNOWN: uses invalid RegExp format '/\1/' (#0031)
    ch15/15.1/15.1.2/15.1.2.2/S15.1.2.2_A5.1_T1 in non-strict mode   // KNOWN: octal input to parseInt() accepted by Duktape
    ch15/15.10/15.10.2/S15.10.2_A1_T1 in non-strict mode   // KNOWN: XML Shallow Parsing with Regular Expression: [^]]*]([^]]+])*]+.  The intent of [^]] is probably [^\]].  An unescaped ']' is not allowed in a character class, so the expression is parsed as [^] (empty inverted class) followed by a literal ']', which is a SyntaxError.  There are two other literal ']' issues.  The RegExp can be fixed to: /[^\]]*\]([^\]]+\])*\]+/.
    ch15/15.10/15.10.2/15.10.2.10/S15.10.2.10_A2.1_T3 in non-strict mode   // KNOWN: uses invalid RegExp control escape '\cX' where X is non-ASCII
    ch15/15.10/15.10.2/15.10.2.10/S15.10.2.10_A5.1_T1 in non-strict mode   // KNOWN: possible test case bug, compiles invalid RegExp '/\undefined/'
    ch15/15.10/15.10.2/15.10.2.13/S15.10.2.13_A1_T16 in non-strict mode   // KNOWN: uses invalid DecimalEscape inside a character class, '/[\12-\14]/'
    ch15/15.10/15.10.2/15.10.2.6/S15.10.2.6_A4_T7 in non-strict mode   // KNOWN: the test case has unescaped invalid PatternCharacters (^, ] {, }) which follow the escaped '\['
    ch15/15.10/15.10.2/15.10.2.9/S15.10.2.9_A1_T4 in non-strict mode   // KNOWN: invalid backreference '\2', RegExp only has one capture; in E5.1 this is a SyntaxError
    ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-574 in non-strict mode   // KNOWN: Duktape provides property name as a (intended non-standard) second parameter to setter, this testcase tests that no extra parameter is given so it breaks
    ch15/15.5/15.5.4/15.5.4.7/S15.5.4.7_A1_T11 in non-strict mode   // KNOWN: test case relies on locale specific Date format, Duktape uses ISO 8601 for Date toString()
    ch15/15.9/15.9.3/S15.9.3.1_A5_T1 in non-strict mode   // KNOWN: apparently test case bug
    ch15/15.9/15.9.3/S15.9.3.1_A5_T2 in non-strict mode   // KNOWN: apparently test case bug
    ch15/15.9/15.9.3/S15.9.3.1_A5_T3 in non-strict mode   // KNOWN: apparently test case bug
    ch15/15.9/15.9.3/S15.9.3.1_A5_T4 in non-strict mode   // KNOWN: apparently test case bug
    ch15/15.9/15.9.3/S15.9.3.1_A5_T5 in non-strict mode   // KNOWN: apparently test case bug
    ch15/15.9/15.9.3/S15.9.3.1_A5_T6 in non-strict mode   // KNOWN: apparently test case bug
    ch12/12.6/12.6.1/S12.6.1_A4_T5 in non-strict mode   // diagnosed: INVALID opcode (0)
    ch12/12.6/12.6.2/S12.6.2_A4_T5 in non-strict mode   // diagnosed: INVALID opcode (0)
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
