===========================
Status of test262 testcases
===========================

Overview
========

Test262 provides testcases for various Ecmascript features.  It also includes
features and behavior beyond E5/E5.1 standard (for instance the tests related
to the ``Intl`` module and E6).

This document summarizes the currently failing testcases and why they fail.
The test run was executed against::

  595a36b252ee97110724e6fa89fc92c9aa9a206a.zip

A full list of known bugs is documented in::

  test262-known-issues.json

This file describes a subset of test cases whose reasons for failure require
a longer explanation.

Summary of failure reasons
==========================

In addition to unfixed bugs, the following reasons cause some test262 test
cases to fail:

* Anything under ``intl402`` fails as Duktape does not provide the ``Intl``
  object which is not part of E5/E5.1.  Same for ``es6``, which tests for
  E6 features.

* Duktape has internal limitations for arrays exceeding 2G or 4G entries
  (even sparse ones, the limitations related to the indices).  These cause
  some array tests to fail.

* Duktape does not provice ``RegExp.prototype.compile`` which is not part
  of E5/E5.1.

* Duktape follows the E5.1 regexp syntax strictly (except for allowing the
  ``\$`` identity escape).  Some things that fail in test cases:

  - invalid backreferences (e.g. ``/\1/``)

  - invalid identity escapes (e.g. ``/\a/``)

  - invalid decimal escapes in character classes (e.g. ``[\12-\14]``)

  - special characters appearing literally without escape (e.g. ``]``)

* Duktape has a conservative limit on the C recursion required to execute
  regexps.  This limit can cause several test cases to fail.

* When an empty quantifier is being matched with a quantifier such as ``+``,
  Duktape may now get stuck and match the empty quantified over and over
  (it should match the quantified a minimum number of times and then continue).
  To protect against infinite loop, Duktape eventually bails out with a
  RangeError.

* Duktape does not support specific locales, which affect e.g. case conversion
  and locale sensitive string comparison.  ``String.prototype.localeCompare()``
  is the same as an ordinary compare which breaks e.g.
  ch15/15.5/15.5.4/15.5.4.9/15.5.4.9_CE: ``"\u006f\u0308"`` is considered different
  from ``"\u00f6"`` (precomposed).

* Duktape allows octal syntax.  There is a test case which requires that
  ``parseInt()`` should not accept octal syntax; this test case fails.

* An enumeration corner case test (ch12/12.6/12.6.4/12.6.4-2) currently fails,
  see ``test-bug-enum-shadow-nonenumerable.js``.

* There seem to be several bugs in the Date testcases of test262 (see
  detailed error description).

* Duktape now allows non-standard function declaration outside Program or
  FunctionBody top level (such statements are technically not part of E5/E5.1).
  Unfortunately the semantics for these differ from engine to engine; Duktape
  uses the V8 semantics of "hoisting" the definition so that the function has
  only access to the top level variable scope.  Although test262 test cases
  do have non-standard function declarations (outside top level), they seem
  to be compatible with the V8 semantics and no known issues remain.

Notes on individual errors
==========================

Some notes on individual errors.  This list is not exhaustive.

annexB/B.RegExp.prototype.compile
---------------------------------

Same failure in strict and non-strict modes::

  === annexB/B.RegExp.prototype.compile failed in non-strict mode ===
  --- errors ---
  TypeError: invalid base reference for property read
          duk_hobject_props.c:1694
          testcase /tmp/test262-T1pW1o.js:2217
          runTestCase /tmp/test262-T1pW1o.js:901
          global /tmp/test262-T1pW1o.js:2059 preventsyield
  ===

The E5/E5.1 specification does not include a ``RegExp.prototype.compile()``,
so this testcase should actually fail.

ch07/7.8/7.8.5/S7.8.5_A1.4_T1
-----------------------------

Same failure in strict and non-strict modes::

  === ch07/7.8/7.8.5/S7.8.5_A1.4_T1 failed in non-strict mode ===
  --- errors ---
  SyntaxError: invalid backreference(s) (line 2216)
          duk_regexp_compiler.c:889
  ===

The test case uses a RegExp of the form ``/\1/``.  Based on E5.1 Section
15.10.2.9 this form is invalid (V8 and Rhino allow these broken regexps
though):

  NOTE
  An escape sequence of the form \ followed by a nonzero decimal number n
  matches the result of the nth set of capturing parentheses (see 15.10.2.11).
  It is an error if the regular expression has fewer than n capturing parentheses.
  If the regular expression has n or more capturing parentheses but the nth one
  is undefined because it has not captured anything, then the backreference
  always succeeds.

If you comment out the offending regexp, the test case then fails with the
following in response to an invalid RegExp ``/\a/`` (again, accepted by V8
and Rhino)::

  SyntaxError: invalid regexp escape (line 2221)
          duk_lexer.c:1551

This RegExp is invalid because "a" is not allowed as an identity escape.
E5.1 Section 15.10.1::

  IdentityEscape ::
    SourceCharacter but not IdentifierPart
    <ZWJ>
    <ZWNJ>

Because "a" belongs to IdentifierPart, it is an invalid identity escape.
Because it doesn't match any other alternatives for an AtomEscape either,
it should cause a SyntaxError.

Commenting out the ``/\a/`` regexp, the test case finishes.

This test case is a bit dubious anyway, because it asserts that a RegExp
``source`` property should have a specific form.  E5.1 Section 15.10.4.1:

  Let S be a String in the form of a Pattern equivalent to P, in which
  certain characters are escaped as described below. S may or may not be
  identical to P or pattern; however, the internal procedure that would
  result from evaluating S as a Pattern must behave identically to the
  internal procedure given by the constructed object's [[Match]] internal
  property.

So, for instance, it would be compliant to have a regexp ``/x/`` with
a ``source`` property of either ``x`` or ``\u0078`` or even ``(?:\u0078){1}``.

ch07/7.8/7.8.5/S7.8.5_A1.4_T2
-----------------------------

Same failure in strict and non-strict modes::

  === ch07/7.8/7.8.5/S7.8.5_A1.4_T2 failed in non-strict mode ===
  --- errors ---
  Test262 Error: #0031
  ===

This is caused by trying to eval the regexp ``/\1/``, which contains a
SyntaxError (invalid back-reference, see above).

ch12/12.6/12.6.4/12.6.4-2
-------------------------

Enumeration corner case issue, see ``test-bug-enum-shadow-nonenumerable.js``.

ch15/15.1/15.1.2/15.1.2.2/S15.1.2.2_A5.1_T1
-------------------------------------------

::

  === ch15/15.1/15.1.2/15.1.2.2/S15.1.2.2_A5.1_T1 failed in non-strict mode ===
  --- errors ---
  Test262 Error: parseInt should no longer accept octal
  ===

Duktape ``parseInt()`` accepts octal::

  duk> parseInt('077')
  = 63

This matches Rhino and V8 behavior.

ch15/15.10/15.10.2/S15.10.2_A1_T1
---------------------------------

::

  === ch15/15.10/15.10.2/S15.10.2_A1_T1 failed in non-strict mode ===
  --- errors ---
  Test262 Error: #4: XML Shallow Parsing with Regular Expression: [^]]*]([^]]+])*]+
  ===

First error happens with index 4 into the regexp set, the precise error is::

  SyntaxError: invalid regexp character
          duk_lexer.c:1598
          RegExp (null) native strict construct preventsyield
          global /tmp/foo.js:2285 preventsyield

The character class ``[^]]`` contains an unescaped ``]`` (probably ``[^\]]``
was intended, so it gets parsed as a character class ``[^]`` followed by a
literal, unescaped ``]`` which is a SyntaxError.  There are two other instances
like this in the test case.

ch15/15.10/15.10.2/15.10.2.5/S15.10.2.5_A1_T5
---------------------------------------------

::

  === ch15/15.10/15.10.2/15.10.2.5/S15.10.2.5_A1_T5 failed in non-strict mode ===
  --- errors ---
  RangeError: regexp executor recursion limit
          duk_regexp_executor.c:145
          exec (null) native strict preventsyield
          global /tmp/test262-yJCwFh.js:2215 preventsyield
  ===

Duktape bug: matching ``/(a*)b\1+/`` against ``"baaaac"`` first matches an
empty string to capture group 1, then matches a "b", and finally ends up
matching the empty string with a ``+`` quantifier.  Duktape doesn't currently
always handle empty quantified expressions correctly, so it gets stuck and
bails out eventually with a RangeError.  See test-regexp-empty-quantified.js.

ch15/15.10/15.10.2/15.10.2.9/S15.10.2.9_A1_T5
---------------------------------------------

Same cause as: ch15/15.10/15.10.2/15.10.2.5/S15.10.2.5_A1_T5.

ch15/15.10/15.10.2/15.10.2.10/S15.10.2.10_A2.1_T3
-------------------------------------------------

::

  === ch15/15.10/15.10.2/15.10.2.10/S15.10.2.10_A2.1_T3 failed in non-strict mode ===
  --- errors ---
  SyntaxError: invalid regexp control escape
          duk_lexer.c:1492
          RegExp (null) native strict construct preventsyield
          global /tmp/test262-heB_na.js:2219 preventsyield
  ===

This test case does e.g.::

  for (alpha = 0x0410; alpha <= 0x042F; alpha++) {
    str = String.fromCharCode(alpha % 32);
    arr = (new RegExp("\\c" + String.fromCharCode(alpha))).exec(str);
    // ...
  }

The syntax error comes from parsing a RegExp ``\cX`` where ``X`` is a non-ASCII
character (e.g. U+0410 and onwards).  This is clearly not allowed by the RegExp
syntax in E5.1 Section 15.10.1 (see CharacterEscape and ControlLetter productions).

ch15/15.10/15.10.2/15.10.2.10/S15.10.2.10_A5.1_T1
-------------------------------------------------

::

  === ch15/15.10/15.10.2/15.10.2.10/S15.10.2.10_A5.1_T1 failed in non-strict mode ===
  --- errors ---
  SyntaxError: decode error
          duk_lexer.c:404
          RegExp (null) native strict construct preventsyield
          global /tmp/test262-4ZVGcj.js:2220 preventsyield
  ===

There seems to be a test case error::

  var non_ident = "~`!@#$%^&*()-+={[}]|\\:;'<,>./?" + '"';
  var k = -1;
  do {
     k++;
     print("\\" + non_ident[k], "g")
     arr = new RegExp("\\" + non_ident[k], "g").exec(non_ident);
  } while ((arr !== null) && (arr[0] === non_ident[k]))

The loop works correctly until ``k`` points outside the ``non_ident``
array.  The loop then tries to create a regexp with::

  new RegExp("\\" + undefined, "g");

The RegExp input will be ``\undefined`` which contains an invalid Unicode
escape, causing the SyntaxError from Duktape.  There is no valid way of
parsing ``\u`` in a regexp.  Note that ``\u`` is not allowed as an identity
escape (IdentityEscape explicitly rejects IdentifierPart characters), and
there are no other rules allowing it either.

ch15/15.10/15.10.2/15.10.2.13/S15.10.2.13_A1_T16
------------------------------------------------

::

  === ch15/15.10/15.10.2/15.10.2.13/S15.10.2.13_A1_T16 failed in non-strict mode ===
  --- errors ---
  SyntaxError: invalid decimal escape (line 2215)
          duk_lexer.c:1786
  ===

The SyntaxError is caused by::

  __executed = /[\d][\12-\14]{1,}[^\d]/.exec("line1\n\n\n\n\nline2");

Here, a ``\12`` DecimalEscape occurs inside a character class.  The DecimalEscape
evaluates to the integer 12 (see E5.1 Section 15.10.2.11, step 3).  Then, the
ClassEscape throws a SyntaxError; see E5.1 Section 15.10.2.19 steps 1-2::

  1. Evaluate DecimalEscape to obtain an EscapeValue E.

  2. If E is not a character then throw a SyntaxError exception.

ch15/15.10/15.10.2/15.10.2.6/S15.10.2.6_A4_T7
---------------------------------------------

A SyntaxError occurs with the RegExp::

  __executed = /\B\[^z]{4}\B/.test("devil arise\tforzzx\nevils");

The ``\[`` is accepted as an identity escape, which then leads to SyntaxError
because none of ``^``, ``]``, ``{``, or ``}`` are accepted unescaped by E5.1
(see PatternCharacter production).

The point of the testcase is probably to test that ``\[`` is not evaluated as
``[``.  If the escape is removed, the RegExp matches with the result ``"il a"``
with both Duktape and Rhino.  This causes a test case failure, the test case
is expected not to match.

If the invalid characters are escaped, the test case passes::

  __executed = /\B\[\^z\]\{4\}\B/.test("devil arise\tforzzx\nevils");

ch15/15.4/15.4.4/15.4.4.10/S15.4.4.10_A3_T3
-------------------------------------------

::

  === ch15/15.4/15.4.4/15.4.4.10/S15.4.4.10_A3_T3 failed in non-strict mode ===
  --- errors ---
  Test262 Error: #1: var obj = {}; obj.slice = Array.prototype.slice; obj[4294967294] = "x"; obj.length = 4294967295; var arr = obj.slice(4294967294,4294967295); arr.length === 1. Actual: 0
  ===

This bug is probably caused by C typing related to array length handling.
Arrays over 2G elements long will probably have such issues.  There are
several similar failing test cases, e.g.:

* ch15/15.4/15.4.4/15.4.4.12/S15.4.4.12_A3_T3

* ch15/15.4/15.4.4/15.4.4.14/15.4.4.14-9-9

* ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-5-12

* ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-5-16

* ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-8-9

Fortunately these don't have much real world relevance.

ch15/15.5/15.5.4/15.5.4.7/S15.5.4.7_A1_T11
------------------------------------------

::

  === ch15/15.5/15.5.4/15.5.4.7/S15.5.4.7_A1_T11 failed in non-strict mode ===
  --- errors ---
  Test262 Error: #1: __instance = new Date(0); __instance.indexOf = String.prototype.indexOf;  (__instance.getTimezoneOffset()>0 ? __instance.indexOf('31') : __instance.indexOf('01')) === 8. Actual: 5
  ===

The test case relies on the ``toString()`` coercion of a Date instance.
For instance, Rhino formats the ``__instance`` as::

  Thu Jan 01 1970 02:00:00 GMT+0200 (EET)

The index for "01" here is 8.  Note that this format is locale and platform
specific so the test case is not reliable.  Duktape uses ISO 8601 also for
``toString()``::

  1970-01-01 02:00:00.000+02:00

Here the index for "01" is 5, which causes a test case failure.
 
ch15/15.9/15.9.3/S15.9.3.1_A5_{T1,T2,T3,T4,T5,T6}
-------------------------------------------------

These tests fail with::

  === ch15/15.9/15.9.3/S15.9.3.1_A5_T1 failed in non-strict mode ===
  --- errors ---
  Test262 Error: #1: Incorrect value of Date
  ===

There seem to be incorrect comparison values for the Dates.  For example,
in T6::

  if (-2208960000001 !== new Date(1899, 11, 31, 23, 59, 59, 999).valueOf()) {
    $FAIL("#1: Incorrect value of Date");
  }

The date expression yields ``-2208996000001`` in both Rhino and V8, so the
test case is probably incorrect (there is a missing ``9`` digit and extra
``0`` digit)).  There are similar issues in test 2 and 3 too.  Test 4 also
seems incorrect::

  if (28799999 !== new Date(1969, 11, 31, 23, 59, 59, 999).valueOf()) {
    $FAIL("#4: Incorrect value of Date");
  }

Because Jan 1, 1970 is the "zero point", all dates before that will have
negative time values, so the test case is obviously incorrect.  Rhino and
V8 agree, returning ``-7200001`` for the expression.

All of these test cases also fail with Rhino, and the errors seem to be in
the comparison values of the test case.
