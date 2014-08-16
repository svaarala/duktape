=================================================
Number-to-string and string-to-number conversions
=================================================

Overview
========

Accurate number-to-string and string-to-number conversion is a non-trivial
problem.  Duktape incorporates built-in algorithms for these operations to
avoid dependence on platform primitives.  This is important for several
reasons, including: (1) platform primitives may not behave consistently
across platforms; (2) they almost never provide enough functionality to
fulfill Ecmascript requirements which include, for instance, formatting
fractional values in arbitrary radix in the range 2 to 36; (3) and they
may have a large code or memory footprint which is a specific concern in
Duktape.

The current number conversion primitives have been implemented in
``duk_numconv.c``, with the header ``duk_numconv.h`` providing flags and
constants.  The script ``gennumdigits.py`` is used to generate some tables
needed by the implementation.

The implementation is based on the Dragon4 number-to-string algorithm, with
some modifications for handling Ecmascript requirements.  Dragon4 requires
the use of big integers, so a limited functionality bigint implementation
is included in ``duk_numconv.c``; the bignum implementation uses fixed size
stack buffers to avoid dynamic memory allocation.  Dragon4 is also currently
used, rather awkwardly, for string-to-number conversion.

The current number-to-string approach should produce optimal shortest form
(free form) strings, but may not produce optimal fixed format strings.  String
parsing may not produce optimal results either.  These limitations should be
fixed later.  Known bugs are documented in failing bug testcases.

Ecmascript number conversions
=============================

Ecmascript requires number-to-string conversion (or more specifically, IEEE
double to string conversion) in the following places:

* ``ToString()`` coercion of numbers, E5.1 Section 9.8.1.  ToString() only
  uses decimal conversion (radix 10).

* ``Number.prototype.toString([radix])`` allows conversion to arbitary radix
  in the range 2 to 36.  E5.1 Section 15.7.4.2 states that the algorithm used
  should be a generalization of E5.1 Section 9.8.1.

* ``Number.prototype.toFixed(fractionDigits)`` converts a number to a string
  with a specified number of fraction digits (0 to 20), radix 10 only.  If
  the absolute value of the input is >= 10^21, ``ToString()`` is used instead.

* ``Number.prototype.toExponential(fractionDigits)`` converts a number to a
  string in exponential notation with a specified number of fraction digits
  following the single lead digit and the decimal point, radix 10 only.  If
  ``fractionDigits`` is not given, outputs the shortest 

* ``Number.prototype.toPrecision(precision)`` converts a number to a string
  with a specified number of digits, radix 10 only.  The N-digit representation
  is rounded if necessary, and exponent notation is used if certain conditions
  are triggered (the specifics are a bit complicated and discussed below).

* ``JSON.stringify()`` serializes numbers using ``ToString()``.

String-to-number conversion (or more specifically, string to IEEE double
conversion) is required in the following places:

* Ecmascript compilation uses a ``NumericLiteral`` production, whose parsing
  semantics are given in E5.1 Section 7.8.3.  Radix 10 only.

* ``ToNumber()`` coercion of strings, E5.1 Section 9.3.1.  Radix 10 only.

* Global object ``parseInt(string,radix)``, E5.1 Section 15.1.2.2.  Parses
  only integer values, in any radix, and stops parsing when a non-digit is
  encountered (e.g. "1.234" is parsed as "1").

* Global object ``parseFloat(string)``, E5.1 Section 15.1.2.3.  Radix 10 only.

* ``JSON.parse()`` parses JSON data as a restricted form of Ecmascript syntax.
  JSON number literals match the production ``JSONNumber`` which is a subset
  of ``NumericLiteral``.  Notably, ``JSONNumber`` does not allow hex literals,
  does not allow fractions without a leading integer part (e.g. ".123" is
  rejected), and only allows an optional negative sign (not a plus sign).

The specific requirements for each of these primitives is discussed below
(not exhaustively though).

At a high level, the functionality needed for number-to-string conversion
includes:

* Conversion of an IEEE double into a string in an arbitrary radix in the
  range 2 to 36, supporting fractional digits even for non-decimal radix
  values.

* Shortest form (free form) and fixed form formatting.  Fixed form formatting
  needs to support both relative precision (number of digits) as well as
  absolute precision (formatting to a certain number of fractions).

* Plain and exponential format.  Exponential format for non-base-10 is not 
  explicitly required.

* Special handling of NaN and +/- Infinity (output "NaN", "Infinity", or
  "-Infinity").

* The primitive should produce the shortest possible string which converts
  back exactly to the original number.  However, this is not actually required
  (just nice to have).

* The specification does *not* require that ``ToNumber(ToString(x)) === x``
  (except for -0, which loses its sign in the process).  However, this
  property is very desirable.

For string-to-number conversion, the high level functionality includes:

* Conversion of an arbitrary decimal number into an IEEE double.  Support
  for parsing arbitrary numbers in radix values other than 10 is not required.

* Conversion of an arbitrary integer in any radix in the range 2 to 36 into
  an IEEE double.

* Supporting a variety of small lexical differences in the Ecmascript "call
  sites": recognizing "0x"/"0X" hex notation and leading zero octal notation,
  allowing or rejecting leading and trailing whitespace, allowing or rejecting
  trailing garbage, treating the empty string as zero (vs. NaN), etc.

* In some cases ``Infinity`` (and ``-Infinity``) need to be recognized.
  ``NaN`` is not recognized but some primitives produce a NaN for any number
  which cannot be parsed correctly (e.g. both "NaN" and "foobar" would
  produce a NaN).

Note that although it is possible to format an arbitrary number into any
radix in the range 2 to 36 (even fractions), there is no primitive to parse
non-integer numbers back in any other radix than 10.

Notes on Ecmascript number-to-string conversion
===============================================

ToString() and Number.prototype.toString()
------------------------------------------

The algorithm in E5.1 Section 9.8.1 has specific rules when to fall back to
exponent notation.

In case the final digit is not well defined (two digits are equally acceptable)
ToString() doesn't strictly require that either one be chosen.  However, the
specification recommends that an even last digit be favored over an odd last
digit.  (E5.1 Section 9.8.1, NOTE 2.)

When the radix is not 10, E5.1 does not specify exact requirements but suggests
that something analogous to the decimal conversion algorithm be used.  The
specification leaves open, for instance, what to do with exponential notation
when radix is not 10.  The Dragon4 paper formats the exponent value in the
target radix (B); another reasonable choice is to format the exponent always
in base 10.  Regardless, the exponent separator character ('e') becomes
difficult to parse when radix is 15 or above, and the digit 'e' is also used
for the digits.  Consider, for instance, the base 16 value::

  1.faecee+1c == 1.faece * 16^(0x1c)

Let's look at the format selection process; from E5.1 Section 9.8.1:

* Step 5: Otherwise, let n, k, and s be integers such that k >= 1,
  10^(k-1) <= s < 10^k, the Number value for s x 10^(n-k) is m, and k is
  as small as possible.  Note that k is the number of digits in the decimal
  representation of s, that s is not divisible by 10, and that the least
  significant digit of s is not necessarily uniquely determined by these
  criteria.

* Step 6: If k <= n < 21, return the String consisting of the k digits of the
  decimal representation of s (in order, with no leading zeroes), followed by
  n-k occurrences of the character '0'.

* Step 7: If 0 < n <= 21, return the String consisting of the most significant
  n digits of the decimal representation of s, followed by a decimal point '.',
  followed by the remaining k-n digits of the decimal representation of s.

* Step 8: If -6 < n <= 0, return the String consisting of the character '0',
  followed by a decimal point '.', followed by -n occurrences of the character
  '0', followed by the k digits of the decimal representation of s.

* Step 9: Otherwise, if k = 1, return the String consisting of the single digit
  of s, followed by lowercase character 'e', followed by a plus sign '+' or a
  minus sign '-' according to whether n-1 is positive or negative, followed by
  the decimal representation of the integer abs(n-1) (with no leading zeroes).

First, examples of the selection of n, k, and s::

  1.2345  --> s = 12345, k = 5, n = 1
          --> s x 10^(n-k) = 12345 * 10^(1-5) = 12345 * 10^(-4)
                           = 1.2345

Note that the naming of the variables differs from that used e.g. in the
Burger-Dybvig paper:

* ``s`` is the integer representation of digits (minimal length); in
  Burger-Dybvig this is named ``f``.

* ``k`` is the digit length of ``s``.

* ``n`` indicates the position of the leading digit of ``s``, with n=0
  being the first fraction (0.X), n=1 being the least significant integer
  position (X.0), n=2 being the "tens" position (X0.0) etc.  In Burger-Dybvig
  this is named ``k`` (!).

toFixed()
---------

If the absolute value of the input is 1e21 or above, behaves like ToString().
Otherwise outputs the number in decimal notation with fractionDigits
trailing the decimal point.  If no fractionDigits is given, behaves as if the
value was zero, in which case no decimal point and no fractional digits are
output.

Example:

* (123).toFixed(3) -> "123.000"

* (0.1).toFixed(0) -> "0"

* (0.9).toFixed(0) -> "1"  (rounds up)

* (1e21).toFixed(10) -> "1e+21"  (falls back to ToString())

toExponential()
---------------

If 0 digits are requested, the decimal period is omitted:

* (123).toExponential(0) -> "1e+2"

If > 0 digits (but less than 21; fractionDigits must be in range [0,20])
are requested, a single leading digit (0-9) followed by a decimal point
and fractionDigits are output:

* (12345).toExponential(2) -> "1.23e+4"

If fractionDigits is ``undefined``, the shortest form which ensures that
the number parses back appropriately ("free form") is used:

* (12345).toExponential() -> "1.2345e+4"
* (0.1).toExponential() -> "1e-1"

toPrecision()
-------------

If N digits are requested and the digits end before the decimal period
or if the topmost (most significant) digit has an exponent of -7 or less
(in other words, it is the seventh or later digit after the decimal point),
toPrecision() uses an exponent notation.  Examples:

* (1234).toPrecision(4) -> "1234"

* (1234).toPrecision(3) -> "1.23e+3"

* (9876).toPrecision(3) -> "9.88e+3" (rounding up is necessary)

* (9999).toPrecision(3) -> "1.00e+4" (rounding up and carrying over the
  leading digit is necessary)

* (0.000001).toPrecision(2) -> "0.0000010"

* (0.0000001).toPrecision(2) -> "1.0e-7"

Note that leading fractional zeroes are prepended if necessary.  Trailing
zeroes are not appended to reach the decimal point from above.

Notes on Ecmascript string-to-number conversion
===============================================

Lexical trivia differences in call sites
----------------------------------------

The following table summarizes the lexical trivia differences between the
variants appearing in the specification:

+----------------------+----------------+------------+------------+--------------+--------------+
| Feature              | NumericLiteral | ToNumber() | parseInt() | parseFloat() | JSON.parse() |
+======================+================+============+============+==============+==============+
| Leading whitespace   | no [1a]        | yes        | yes        | yes          | no [5a]      |
+----------------------+----------------+------------+------------+--------------+--------------+
| Trailing whitespace  | no [1a]        | yes        | yes        | yes          | no [5a]      |
+----------------------+----------------+------------+------------+--------------+--------------+
| Trailing garbage     | no [1a]        | no         | yes [3a]   | yes [4a]     | no [5a]      |
+----------------------+----------------+------------+------------+--------------+--------------+
| Leading zeroes       | no             | yes        | yes [3b]   | yes          | no           |
+----------------------+----------------+------------+------------+--------------+--------------+
| Allow plus sign      | no [1b]        | yes        | yes        | yes          | no           |
+----------------------+----------------+------------+------------+--------------+--------------+
| Allow minus sign     | no [1b]        | yes        | yes        | yes          | yes          |
+----------------------+----------------+------------+------------+--------------+--------------+
| Allow fractions      | yes (decimal)  | yes        | no         | yes          | yes          |
+----------------------+----------------+------------+------------+--------------+--------------+
| Allow fraction w/o   | yes            | yes        | n/a        | yes          | no           |
| leading integer      |                |            | (= NaN)    |              |              |
| (e.g. ".123")        |                |            |            |              |              |
+----------------------+----------------+------------+------------+--------------+--------------+
| Allow fraction w/o   | yes            | yes        | yes [3c]   | yes          | no           |
| fraction digits      |                |            |            |              |              |
| (e.g. "123.")        |                |            |            |              |              |
+----------------------+----------------+------------+------------+--------------+--------------+
| Allow hex (integer)  | yes            | yes        | yes        | no           | no           |
+----------------------+----------------+------------+------------+--------------+--------------+
| 0x/0X hex (integer)  | yes            | yes        | yes [3d]   | no           | no           |
+----------------------+----------------+------------+------------+--------------+--------------+
| Empty == zero        | no             | yes        | no         | no           | no           |
|                      |                |            | (= NaN)    | (= NaN)      |              |
+----------------------+----------------+------------+------------+--------------+--------------+
| Allow arbitrary      | no             | no         | yes        | no           | no           |
| radix                |                |            |            |              |              |
+----------------------+----------------+------------+------------+--------------+--------------+
| Parse Infinity       | no [1c]        | yes        | no         | yes          | no           |
|                      |                |            | (= NaN)    |              |              |
+----------------------+----------------+------------+------------+--------------+--------------+
| Parse +Infinity      | no [1c]        | yes        | no         | yes          | no           |
|                      |                |            | (= NaN)    |              |              |
+----------------------+----------------+------------+------------+--------------+--------------+
| Parse -Infinity      | no [1c]        | yes        | no         | yes          | no           |
|                      |                |            | (= NaN)    |              |              |
+----------------------+----------------+------------+------------+--------------+--------------+
| Parse NaN            | no [1c]        | no [2a]    | no         | no [4b]      | no           |
|                      |                | (= NaN)    | (= NaN)    | (= NaN)      |              |
+----------------------+----------------+------------+------------+--------------+--------------+

Notes:

* [1a]: Lexer will eat whitespace and terminate numeric literal at unexpected
  characters, e.g. "   1+2" parses as the tokens "1", "+", "2".  The literal
  must not be followed immediately by a DecimalDigit or IdentifierStart (e.g.
  "3in" is a SyntaxError, and is not parsed as "3" followed by "in").

* [1b]: An explicit sign is parsed as an unary plus/minus operator, e.g.
  "+123" is parsed as the tokens "+", "123".

* [1c]: "NaN" and "Infinity" are value properties of the global object, so
  the expressions "Infinity, "+Infinity", "-Infinity", "NaN" will evaluate
  to the expected numeric values.  However, these expressions are not handled
  through number parsing but through identifier resolution.  For instance,
  "-Infinity" parses as "-" (unary minus) and identifier reference "Infinity".

* [2a]: "NaN" is not included in the StringNumericLiteral production, but any
  non-parseable number will parse back as a NaN.  For instance, both "NaN"
  and "foobar" will parse back as NaN.

* [3a]: Allows trailing whitespace, because parsing tolerates trailing non-digit
  garbage.  Also a decimal point is interpreted as garbage, e.g. "1.23" is parsed
  as "1".

* [3b]: Leading zeroes may trigger automatical octal mode in some implementations.
  E.g. in V8, parseInt("0009") returns 0 because V8 switches to octal mode, and
  treats '9' as garbage; parseInt("0009", 10) returns the correct value 9.

* [3c]: Decimal point is interpreted as a garbage digit and terminates literal,
  so "123." is interpreted as "123", so it gets the right numeric value even
  though a decimal point is not explicitly allowed (same as e.g. "123@").

* [3d]: Interprets leading "0x" and "0X" specially if radix not given or radix
  is 16.

* [4a]: Allows trailing garbage; the algorithm in E5.1 Section 15.1.2.3 finds the
  longest prefix which matches ``StrDecimalLiteral`` (the same production used
  by string ``ToNumber()``) and thus essentially chops off trailing garbage.

* [4b]: "NaN" is not included in StrDecimalLiteral, but all non-parseable values
  parse as NaN.

* [5a]: JSON parser will eat whitespace.

White space
-----------

* ToNumber() accepts white space StrWhiteSpaceChar::

    StrWhiteSpaceChar::
      WhiteSpace
      LineTerminator

    WhiteSpace::
      <TAB> | <VT> | <FF> | <SP> | <NBSP> | <BOM>
      <USP>   (Other category "Zs")

    LineTerminator::
      <LF> | <CR> | <LS> | <PS>

  StrWhiteSpaceChar matches the characters that String.prototype.trim()
  considers white space (E5.1 Section 15.5.4.20).

* parseInt() and parseFloat() strip using StrWhiteSpaceChar.

* NumericLiteral and JSONNumber do not accept white space (it's not
  necessary because the Ecmascript/JSON parser will deal with whitespace
  on its own)

Infinity
--------

The string "Infinity" is parsed as an infinity-value in some contexts.
In other contexts, it may be a valid number value, e.g.::

  > parseFloat('Infinity')
  Infinity
  > parseInt('Infinity', 36)
  1461559270678

Zero
----

Zero sign must be respected, e.g.::

  > 1/JSON.parse('0')
  Infinity
  > 1/JSON.parse('-0')
  -Infinity

NumericLiteral notes
--------------------

Decimal numbers can have fractions and an exponent part.  Hexadecimal values
are prefixed with "0x" or "0X" and can only be integers.

Octal values are optional to support and begin with a leading zero.
Implementations have varying behavior for dealing with inputs like "0779".

The specification explicitly allows ignoring decimal digits beyond the 20th digit
and allows the 20th digit to be rounded upwards.  This makes it easier to parse
numbers with extremely large mantissa values, e.g. "1<million zeros>e-1000000"
which has the numeric value 1.  The parser can parse the first 20 digits ('1'
followed by 19 '0' digits), and ignore the rest of the digits (999981 zero digits),
keeping track of their count.  The exponent part is then adjusted by the number of
ignored digits, yielding "10000000000000000000" as the mantissa and
-1000000 + 999981 = -19 as the exponent; in other words, the number is treated the
same as "10000000000000000000e-19".  This is easier to process and ensures that
there is an upper bound to the size of the internal big integers representing
intermediate values.

Similar mantissa chopping limits can be established for non-decimal inputs.
See ``gennumdigits.py``.

ToNumber()
----------

Trailing garbage produces a NaN::

  > +"   123"
  123
  > +"   123foo"
  NaN

parseInt() notes
----------------

None.

parseFloat() notes
------------------

None.

JSON.parse() notes
------------------

A leading plus sign is not allowed for the significand::

  1.23    // allowed
  -1.23   // allowed
  +1.23   // rejected

However, the exponent part uses the ``ExponentPart`` production which
allows all of the following::

  1.23e1
  1.23e+1
  1.23e-1

Octal support
-------------

Section B.1.1 of the E5.1 specification includes octal syntax for parsing
literal numbers; there is no official octal syntax for numbers converted
with ToNumber() or its equivalents.  However, practical implementations
will parse octal also in such contexts; as an example, V8 and Rhino::

  > parseInt('077')
  63

Octal syntax is similar to automatic hex syntax, in that (1) it is detected
based on a prefix (a leading zero followed by at least one octal digit),
and (2) it is only applied to integers.

Both Rhino and V8 also have a feature that if a number begins with an
octal prefix but turns out to contain decimal digits other than octal
digits (i.e. '8' and '9'), the number is parsed as a decimal integer
(this behavior requires multiple passes or back-tracking)::

  js> eval('077')
  63
  js> eval('088')
  88
  js> eval('099')
  99

However, this is not the case in contexts which allow trailing garbage
to end number parsing.  Behavior also differs; V8 stops parsing at the
offending digit and emitting the result of the valid prefix::

  > parseInt('077')
  63
  > parseInt('088')
  0
  > parseInt('099')
  0
  > parseInt('0789')    // parsed as '07'
  7
  > parseInt('07789')   // parsed as '077'
  63


Rhino will return a NaN if the offending digit follows the leading octal
zero immediately, but otherwise behaves like V8::

  js> parseInt('077')
  63
  js> parseInt('088')
  NaN
  js> parseInt('099')
  NaN
  js> parseInt('0789')
  7
  js> parseInt('07789')
  63

Literature
==========

Number-to-string ("output problem")
-----------------------------------

Number-to-string conversion is a well researched problem, with a lot of
solutions.  Dragon4 is an old but well established algorithm which requires
big integer arithmetic for ensuring correct and minimal length output.
It is described in:

* Guy L. Steele Jr., Jon L. White: "How to Print Floating-Point Numbers
  Accurately", 1990.

Many improvements on the basic algorithm exist.  For instance, Burger and
Dybvig optimize one aspect of the algorithm (scaling) using a logarithm
estimate (this paper is also the basis for the current implementation):

* Robert G. Burger, R. Kent Dybvig: "Printing Floating-Point Numbers
  Quickly and Accurately", 1996.

Gay discusses many practical optimizations and other implementation issues,
and also discusses the reverse problem of number parsing:

* David M. Gay: "Correctly Rounded Binary-Decimal and Decimal-Binary
  Conversions", 1990.

* This (and ``dtoa``) is also referred to in the E5.1 specification, see
  Section 9.8.1.

Gay's observations have been incorporated in the ``dtoa`` implementation:

* http://www.netlib.org/fp/dtoa.c

Grisu3 is a quite recent hybrid algorithm which handles about 99.5% of input
numbers very quickly, using a fixed-size software floating point approach
(with a mantissa of 64 bits); the remaining 0.5% of inputs need to fall back
to a traditional approach (e.g. Dragon4).

* Florian Loitsch: "Printing Floating-Point Numbers Quickly and Accurately
  With Integers", 2010.  http://www.sengupta.net/musings/2012/07/grisu/

Grisu3 is the basis of number conversion in Google V8, and has been
encapsulated in the following library:

* https://code.google.com/p/double-conversion/

This library has (comparatively) a very large memory footprint, as it
incorporates two libraries and uses large lookup tables.

String-to-number ("input problem")
----------------------------------

Superficially string-to-number conversion is similar to number-to-string
conversion: in both cases, a number is converted from one radix to another.
However, the problems are actually different, which is also reflected in the
algorithms:

* A string-to-number conversion may result in an overflow (infinity) or an
  underflow (zero) even when the input is not infinity/zero.

* A string-to-number conversion may need to deal with arbitrarily large
  mantissa values and exponent values, even when the number represented is
  finite.  For instance, 123 can be represented as "123000e-3" or equivalently
  as "123<million zeroes>e-1000000".  For number-to-string conversion, the
  mantissa and exponent are always in strict, unique format.

* A string-to-number conversion converts from a representation without a
  fixed accuracy limit (decimal digits of arbitrary length) to a representation
  with a fixed accuracy limit (IEEE double).  In number-to-string conversion
  the roles are reversed: conversion is from a limited accuracy representation
  to an unlimited accuracy representation.

The input problem is also well researched.  One important paper is:

* William D. Clinger: "How to Read Floating Point Numbers Accurately", 1990.

Notes on existing algorithms
----------------------------

There don't seem to be any accurate algorithm which doesn't need bigints for
at least some input values.

Some conversion algorithms prefer speed over code size; for instance, Grisu3
suggests using 8 kilobytes of precomputed powers of 10.  This is unacceptable
for Duktape, considering that the entire regular expression engine is about
8 kilobytes in code footprint.

It's important to optimize for typical cases, but simultaneously correctness
needs to be preserved for all inputs.  Many different shortcuts have been
incorporated into practical conversion algorithms.  For embedded use, printing
small integers should be very fast (and can easily bypass the generic hard
case algorithm).

Current solution
================

The current algorithm is a variant of Dragon4, based on the unoptimized
(basic) algorithm in Figure 1 the Burger-Dybvig paper for free-format
output.  Fixed format output has been implemented on top of the free-format
algorithm by working in options to generate additional digits, and then
rounding explicitly (instead of generating the correct result directly).
String-to-number conversion uses the same basic algorithm with minor
tweaks.  The basic algorithm allows input and output bases to be arbitrary
to support both conversion directions.

The current solution should be correct for free-form output but there are
some fixed-format corner cases which don't work correctly now (all known
cases should have bug testcases illustrating the problem).

The implementation uses a bigint implementation which has an upper limit
on integer size, and the buffers needed are stack allocated.  This is good
in general and also improves cache coherence.  However, the bigint code is
pure, portable C, and inefficient compared to an assembler implementation.

There is a fast path for 32-bit integers (the range [-2**32-1,2**32-1]).
Embedded software is likely to work a lot with small integers, and is also
likely to print out many integers.  Other Dragon4 optimizations have not
been included in the implementation, in an attempt to keep code footprint
as small as possible.

Implementation notes
====================

Bigint operations and size limit
--------------------------------

Dragon4 requires >= 1050-bit integer arithmetic for IEEE doubles.  Operations
needed include: add, subtract, compare, multiply, divide by radix, divide one
bigint by another with the result known to be in the range 0...radix-1 (allowing
some special case code).  1050 bits rounds up to 33 x 32-bit integers, i.e.
132 bytes.  Allocating, say, 4 such slots from the stack should not be an issue.

Typical number-to-string conversion requires much fewer bits, so the
arithmetic should be tuned to small numbers.

The current implementation has bignum size limits larger than this to
accommodate string-to-number conversion in addition to number-to-string
conversion.  See ``BI_MAX_PARTS`` in ``duk_numconv.c``.

Precomputed tables
------------------

Having 10^k tabulated for 326 values would take too much memory: each value
would be a big integer.  One could use a more sparse table, e.g. for every
Nth power (10^10, 10^20, 10^30) and multiply the remaining 0-9 steps
normally.  One could also store binary powers of 10 (10^1, 10^2, 10^4, 10^8,
10^16, 10^32, 10^64, 10^128, and 10^256; a total of 9 values), and use
"binary exponentiation" for faster computation::

  10^365 = 10^(1*256 + 0*128 + 1*64 + 1*32 + 0*16 + 1*8 + 1*4 + 0*2 + 1*1)
         = 10^1 * 10^4 * 10^8 * 10^32 * 10^64 * 10^256

Given that the current bigint implementation requires about 144 bytes per
bigint value, this means a table of about 1.3 kilobytes.  By optimizing the
memory layout (requiring some ugly C casting) this can be reduced considerably.

One can also create the exponents on the fly, i.e. compute 10^(2n) from 10^n
as 10^n * 10^n = 10^(2n).  This technique requires no precomputations and
works in every base, and is used by the current implementation for exponentiation.

Fixed-format output
-------------------

The current approach to fixed-format output is a shortcut: we generate an
extra digit and use simple rounding to fix up the digit before that.  This
may require a carry, which is propagated as needed.  If the carry propagates
up to the first digit, an extra '1' digit is prepended and 'k' is updated.

Simple case, 4-digit output of 8.88888888::

    8 8 8 8 8      generate one extra digit; k = 1
    8 8 8 9 #      round and carry (last digit is irrelevant afterwards)
    `-----'

  4-digit result is "8.889"

Complex case, for 4-digit output of 9.99999999::

    9 9 9 9 9      generate one extra digit; k = 1
  1 0 0 0 0 #      round and carry (last digit is irrelevant afterwards)
  `-----'          carry goes beyond first -> k++ -> k = 2

  4-digit result is "10.00"

.. note:: The current implementation probably does not implement the
   Number.prototype.toPrecision() semantics exactly correctly.  In
   particular, E5.1 Section 15.7.4.7 step 10.a specifies a specific
   rounding tie-breaker which we may not follow properly.

Stripping and Unicode
---------------------

Actual number parsing only supports ASCII characters, and will consider
any non-ASCII characters garbage.  Since the number productions which
allow whitespace include non-ASCII characters, whitespace is always
trimmed first with a Unicode-aware process.  The resulting string can
then be processed in pure ASCII.

Future work
===========

* Improve fixed-format output to be more robust (perhaps adopt an actual,
  documented algorithm).  Currently the fixed-format output approach has
  several problems.

* In very constrained environments it may be a reasonable tradeoff to use
  ANSI C number formatting and parsing (and drop a bunch of features, such
  as arbitrary radix support, some of the precision modes etc), even if it
  is not fully compatible with Ecmascript semantics.  The impact of custom
  number formatting is about 8-9 kilobytes of code footprint at the moment.
