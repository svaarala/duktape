===================
Tagged integer type
===================

Overview
========

Ecmascript has a single number type which is assumed in many places to be
an IEEE double.  Unfortunately in many (if not most) embedded environments
hardware floating point numbers are not available, at least for IEEE doubles,
and software floating point performance is often a significant issue.
Duktape plain types are represented by a ``duk_tval`` type which normally
has a single IEEE double type for numbers.

There are several approaches to avoid this issue:

* Replace the tagged IEEE double number type with an integer or a fixed point
  type.  This will necessarily break Ecmascript compliance to some extent, but
  it would be nice if at least number range was sufficient for 32-bit bit ops
  and to represent e.g. Dates.

* Same as above, but also reserve a few bits for one or more special values
  like NaNs, to maintain compatibility better.  For instance, NaN is used to
  signify an invalid Date, and is also used as a coercion result to signal a
  coercion error.

* Extend the tagged type to support both an IEEE double and an integer or a
  fixed point type.  Convert between the two either fully transparently (to
  maintain full Ecmascript semantics) or in selected situations, chosen for
  either convenience or performance.

This document outlines various approaches and issues with each.  Ultimately
the chosen solution will be described here.

Implementation issues
=====================

* When there is no need to represent IEEE doubles, the 8-byte tagged duk_tval
  no longer needs to conform to the IEEE double constraints (NaN space reuse).
  Instead, it can be split e.g. into an 8-bit tag and 56-bit type-specific
  value.

* When there is a need to represent both integers and IEEE doubles, the 8-byte
  duk_tval must conform to the IEEE double representation, i.e. there are 16
  bits of a special tag value and 48-bit type specific value.

* Should there be a C typedef for a Duktape number?  Currently the public
  API and Duktape internals assume numbers can be read/written as doubles.
  Changing the public API will break compilation (or at least cause warnings)
  for user code, if the integer changes are visible in the API.

* Does the integer change need to be made everywhere at once, so that all
  code (including the compiler, etc) must support the underlying integer
  type before the change is complete?

  Alternatively, Duktape could read and write numbers as doubles by default
  internally (with automatic conversion back and forth as needed) and
  integer-aware optimizations would only be applied in places where it matters,
  such as arithmetic.  In particular, there would be no need to deal with
  integer representation in the compiler as it would normally have a minimal
  impact.

* Integer representations above 32 bits would normally use a 64-bit integer
  type for arithmetic.  However, some older platforms don't have such a type
  (there are workarounds for this e.g. in ``duk_numconv.c``).  So either the
  integer arithmetic must also be implemented with 32-bit replacements, or
  the representation won't be available if 64-bit types are not available.

Representation options
======================

Double type + separate integer / fixed point type (compliant)
-------------------------------------------------------------

In this case the 8-byte tagged type must conform to the IEEE NaN space
reuse, so 16 bits are lost to the type tag and 48 bits are available
for the value.

* Double and up to 48-bit integer (sign + 47-bit range).  Integers are nice
  and intuitive, but won't fit the full 53-bit integer range supported by
  IEEE doubles, so some must fall back into the double representation (not a
  big limitation).  Date values and binary operations work.

* Double and a fixed point with up to 48 bit representation, e.g. sign +
  41.6.  To support reasonable Date values, the integer part must be at least
  41 bits.  To support bit operations without falling back to IEEE doubles,
  there must be a signed and the integer part must be at least 32 bits.
  On the other hand, binary fractions require some additional shifting to
  implement, and user code is not very likely to contain specific binary
  fractions, so they would only benefit code specifically crafted to use them.

* Double and 32-bit signed or unsigned integer: 32-bit arithmetic is nice
  but unfortunately not enough to support Ecmascript bit operations which
  require the range -0x80000000 to 0xffffffff (sign + 32 bits, a 33-bit
  representation).  This would not be a compliance issue as Duktape would
  fall back to the IEEE double for some values, but if fast bit operations
  are important matter, this is not a good option.  If bit operations don't
  matter, then this is a nice option in that it avoids the 64-bit arithmetic
  issue.

Only integer / fixed point type (non-compliant)
-----------------------------------------------

Here the 8-byte tagged type can be split e.g. into a 8-bit type and a 56-bit
value which allows more range.

* 56-bit signed integer (sign + 55 bits): covers the IEEE integer range
  (53-bit), Date values work, bit ops work.  Lack of any fractions makes
  built-in Math functions mostly useless (e.g. Math.random() will always
  return zero), and some user code is likely to break.

* Sign and 47.8 or 45.10 fixed point: provides enough fractions to be
  useful, Date values work, bit ops work.  Math functions are somewhat
  useful again.

* Sign and 41.14 fixed point: maximum number of fraction bits while keeping
  Date values (and bit ops) working.

* Sign and 32.23 fixed point: maximum number of fraction bits while keeping
  bit ops working and providing user code the reasonable and intuitive
  guarantee that 32-bit integers (signed and unsigned) work.  Date values
  won't work.

* 32-bit unsigned integer or 32-bit signed integer: closest to what's fast
  and convenient on typical embedded systems, but some bit operations stop
  working because taken together they need the -0x80000000 to 0xffffffff
  range (there are both signed and unsigned bit ops).  Date values won't
  work.

Dependencies on IEEE double or range
====================================

Specification and Duktape dependencies:

* Signed integers are quite widely required, so having no support for negative
  values is probably not an option.

* At least 32-bit unsigned integers are needed for array and string lengths.

* A sign + a 32-bit range (33-bit representation) are needed for bit ops,
  which provide both signed and unsigned 32-bit results.  The required range
  is -0x80000000 to 0xffffffff.

* The Date built-in uses an integer millisecond value for time values.  This
  representation is used both internally and in the external Date API.

  - 40 (unsigned) bits is not enough to represent the current time, it only
    represents timestamps up to November 2004.

  - 41 (unsigned) bits is enough to represent timestamps up to September
    2039.

  - The Date API never uses fractions, and in fact the specification requires
    that the internal value is integer coerced (to milliseconds), so Date
    does not require fractions to work properly.

  - The implication for using only an integer / fixed point representation
    is that the integer part must contain a sign and at least 41 bits.
    For example, for a 48-bit representation sign + 41.6 fixed point is
    enough, and would provide 1/64 fractions.

  - It would be easy to fix the internal Date representation to work with any
    fixed point representation with enough bits (e.g. sign + 32.15), but
    because the integer millisecond values are used in the public Date API
    too, this doesn't solve anything.

* Signed zero semantics (separation of negative and positive zero) are
  are required and explicitly specified, but Ecmascript itself doesn't
  really depend on being able to use a negative zero, and neither does
  Duktape.

* NaN values are used in several places as significant internal or
  external values.  Invalid Date values are represented by having a
  NaN as the Date object's internal time value.  String-to-number
  coercion relies on using a NaN to indicate a coercion error
  (``Number('foo') === NaN``).  If a NaN value is not available, the
  best replacement is probably zero.

* Infinities are used in math functions but Ecmascript itself doesn't
  rely on being able to use them, and neither does Duktape.

* Duktape packs some internal values into double representation, this is
  used at least by:

  - The compiler for declaration book-keeping.  The needed bit count is
    not large (32 bits should more than suffice, for 2**24 inner functions).

  - Error object tracedata format, which needs 32 bits + a few flags;
    40 bits should suffice.

In addition to these, user code may have some practical dependencies, such as:

* Being able to represent at least signed and unsigned 32 bits, so that all
  Ecmascript bit operations work as expected.

* Being able to represent at least some fractional values.  For instance,
  suppose a custom scheduler used second-based timestamps for timers; it
  would then require a reasonable number of fractions to work properly.
  Signed 41.6 fixed point provides a fractional increment of 0.015625;
  for the scheduler, this would mean about 15.6ms resolution, which is not
  that great.

Efficient check for double-to-fastint conversion
================================================

Criteria
--------

For an IEEE double to be representable as a fast integer, it must be:

* A whole number

* In the 48-bit range

* Not a negative zero, assuming that the integer zero is taken to represent
  a positive zero

What to optimize for
--------------------

This algorithm is needed when Duktape:

* Parses a number and checks whether to represent the number as a double or
  a fastint

* Executes internal code with no fastint handling; in this case any fastint
  inputs are first coerced to doubles and then back to fastints if the result
  fits

* Executes internal code with fastint handling, with one or more of the
  inputs not matching the fastint "fast path" but the result possibly fitting
  into a fastint

The "fast path" for fastint operations doesn't execute this algorithm because
both inputs and outputs are fastints and Duktape detects this in the fast path
preconditions.  Given this, an aggressive memory-speed tradeoff (e.g. a table
for each exponent) doesn't make sense.

The speed of this algorithm affects two scenarios:

1. Computations where the numbers involved are outside the fastint range.  Here
   it's important to quickly determine that a fastint representation is not
   possible.

2. Computations where the numbers can be represented as fastints (at least some
   of the time), but one or more operations don't have a fastint "fast path" so
   that the numbers get upgraded to an IEEE double and then need to be downgraded
   back to a fastint.

Both cases matter, but for typical embedded code the latter case matters more.
In other words, the code should be optimized for the case where a fastint fit
is possible.

Exponent and sign by cases
--------------------------

An IEEE double has a sign (1 bit), an exponent (11 bits), and a 52-bit stored
mantissa.  The mantissa has an implicit (not stored) leading '1' digit, except
for denormals, NaNs, and infinities.

Going through the possible exponent values:

* If exponent is 0:

  - The number is a fastint only if the sign bit is zero (positive) and the
    entire mantissa is all zeroes.  This corresponds to +0.

  - If the mantissa is non-zero, the number is a denormal.

* If the exponent is in the range [1, 1022] the number is not a fastint
  because the implicit mantissa bit corresponds to the number 0.5.

* If exponent is exactly 1023:

  - The number is only a fastint if the stored mantissa is all zeroes.
    This corresponds to +/- 1.

* If exponent is exactly 1024:

  - The number is only a fastint if 51 lowest bits of the mantissa are all
    zeroes.  This corresponds to the numbers +/- 2 and +/- 3.

* Generalizing, if the exponent is in the range [1023,1069], the number is
  a fastint if and only if:

  - The lowest N bits of the mantissa are zero, where N = 52 - (exp - 1023),
    with either sign.

  - N can also be expressed as: N = 1075 - exp.

* If exponent is exactly 1070:

  - The number is only a fastint if the sign bit is set (negative) and the
    stored mantissa is all zeroes.  This corresponds to -2^47.  The positive
    counterpart +2^47 does not fit into the fastint range.

* If exponent is [1071,2047] the number is never a fastint:

  - For exponents [1071,2046] the number is too large to be a fastint.

  - For exponent 2047 the number is a NaN or infinity depending on the
    mantissa contents, neither a valid fastint.

Pseudocode 1
------------

The algorithm::

    is_fastint(sgn, exp, mant):
        if exp == 0:
            return sign == 0 and mzero(mant, 52)
        else if exp < 1023:
            return false
        else if exp < 1070:
            return mzero(mant, 1075 - exp)
        else if exp == 1070:
            return sign == 1 and mzero(mant, 52)
        else:
            return false

The ``mzero`` helper predicate returns true if the mantissa given has its
lowest ``n`` bits zero.

Non-zero integers in the fastint range will fall into the case where a certain
computed number of low mantissa bits must be checked to be zero.  As discussed
above, the algorithm should be optimized for the "input fits fastint" case.

Pseudocode 2
------------

Some rewriting::

    is_fastint(sgn, exp, mant):
        nzero = 1075 - exp
        if nzero >= 52 and nzero <= 6:  // exp 1023 ... exp 1069
            // exponents 1023 to 1069: regular handling, common case
            return mzero(mant, nzero)
        else if nzero == 1075:
            // exponent 0: irregular handling, but still common (positive zero)
            return sign == 0 and mzero(mant, 52)
        else if nzero == 5:
            // exponent 1070: irregular handling, rare case
            return sign == 1 and mzero(mant, 52)
        else:
            // exponents [1,1022] and [1071,2047], rare case
            return false

C algorithm with a lookup table
-------------------------------

The common case ``nzero`` values are between [6, 52] and correspond to
mantissa masks.  Compute a mask index instead as nzero - 6 = 1069 - exp::

    duk_uint64_t mzero_masks[47] = {
        0x000000000000003fULL,  /* exp 1069, nzero 6 */
        0x000000000000007fULL,  /* exp 1068, nzero 7 */
        0x00000000000000ffULL,  /* exp 1067, nzero 8 */
        0x00000000000001ffULL,  /* exp 1066, nzero 9 */
        /* ... */
        0x0003ffffffffffffULL,  /* exp 1025, nzero 50 */
        0x0007ffffffffffffULL,  /* exp 1024, nzero 51 */
        0x000fffffffffffffULL,  /* exp 1023, nzero 52 */
    };

    int is_fastint(duk_int64_t d) {
        int exp = (d >> 52) & 0x07ff;
        int idx = 1069 - exp;

        if (idx >= 0 && idx <= 46) {  /* exponents 1069 to 1023 */
            return (mzero_masks[idx] & mant) == 0;
        } else if (idx == 1069) {  /* exponent 0 */
            return (d >= 0) && ((d & 0x000fffffffffffffULL) == 0);
        } else if (idx == -1) {  /* exponent 1070 */
            return (d < 0) && ((d & 0x000fffffffffffffULL) == 0);
        } else {
            return 0;
        }
    };

The memory cost of the mask table is 8x47 = 376 bytes.  This can be halved
e.g. by using a table of 32-bit values with separate cases for nzero >= 32
and nzero < 32.

Unfortunately the expected case (exponents 1023 to 1069) involves a mask
check with a variable mask, so it may be unsuitable for direct inlining in
the most important hot spots.

C algorithm with a computed mask
--------------------------------

Since this algorithm only runs outside the proper fastint "fast path" it
may be more sensible to avoid a memory tradeoff and compute the masks::

    int is_fastint(duk_int64_t d) {
        int exp = (d >> 52) & 0x07ff;
        int shift = exp - 1023;

        if (shift >= 0 && shift <= 46) {  /* exponents 1023 to 1069 */
            return ((0x000fffffffffffffULL >> shift) & mant) == 0;
        } else if (shift == -1023) {  /* exponent 0 */
            /* return (d >= 0) && ((d & 0x000fffffffffffffULL) == 0); */
            return (d == 0);
        } else if (shift == 47) {  /* exponent 1070 */
            return (d < 0) && ((d & 0x000fffffffffffffULL) == 0);
        } else {
            return 0;
        }
    };

C algorithm with a computed mask, unsigned
------------------------------------------

Using an unsigned 64-bit integer for the input::

    int is_fastint(duk_uint64_t d) {
        int exp = (d >> 52) & 0x07ff;
        int shift = exp - 1023;

        if (shift >= 0 && shift <= 46) {  /* exponents 1023 to 1069 */
            return ((0x000fffffffffffffULL >> shift) & mant) == 0;
        } else if (shift == -1023) {  /* exponent 0 */
            /* return ((d & 0x800fffffffffffffULL) == 0); */
            return (d == 0);
        } else if (shift == 47) {  /* exponent 1070 */
            return ((d & 0x800fffffffffffffULL) == 0x8000000000000000ULL);
        } else {
            return 0;
        }
    };

C algorithm with 32-bit operations and a computed mask
------------------------------------------------------

For middle endian machines (ARM) this algorithm first needs swapping
of the 32-bit parts.  By changing the mask checks to operate on 32-bit
parts the algorithm would work on more platforms and would also remove
the need for swapping the parts on middle endian platforms::

    int is_fastint(duk_uint32_t hi, duk_uint32_t lo) {
        int exp = (hi >> 20) & 0x07ff;
        int shift = exp - 1023;

        if (shift >= 0 && shift <= 46) {  /* exponents 1023 to 1069 */
            if (shift <= 20) {
                /* 0x000fffff'ffffffff -> 0x00000000'ffffffff */
                return (((0x000fffffUL >> shift) & hi) == 0) && (lo == 0);
            } else {
                /* 0x00000000'ffffffff -> 0x00000000'0000003f */
                return (((0xffffffffUL >> (shift - 20)) & lo) == 0);
            }
        } else if (shift == -1023) {  /* exponent 0 */
            /* return ((hi & 0x800fffffUL) == 0x00000000UL) && (lo == 0); */
            return (hi == 0) && (lo == 0);
        } else if (shift == 47) {  /* exponent 1070 */
            return ((hi & 0x800fffffUL) == 0x80000000UL) && (lo == 0);
        } else {
            return 0;
        }
    };


Future work
===========

Skipping the double-to-fastint test sometimes
---------------------------------------------

The double-to-fastint can safely err on the side of caution and decide to
represent a fastint-compatible number as a double.  This opens up the
possibility of skipping the double-to-fastint test in some cases which
may improve performance and reduce code size.

For instance, when ``Math.cos()`` pushes its result on the stack, it's
probably quite a safe bet that the number won't fit a fastint, so it could
be written as a double directly without a double-to-fastint downgrade
check.  In case it is a fastint (-1, 0, or 1) it will be represented as a
double but will be downgraded to a fastint by the first operation that
does execute the downgrade check.  To support this, there could be a macro
like ``DUK_TVAL_SET_NUMBER_NOFASTINT``.

Another option is to run the double-to-fastint check randomly or e.g. only
every Nth time it is needed (N could be quite large, e.g. the prime 17).
This should be quite OK from a performance point of view.  If a number is
incorrectly stored as a double and is involved in a lot of operations,
chances are it will get downgraded quite quickly, as long as the check
interval does not unluckily correlate with the downgrade check frequency.
This approach may not be worth it because an optimized fastint downgrade
check should have quite reasonable performance, and such an approach would
have no effect on the actual fastint fast path (inputs are fastints,
outputs are fastints).
