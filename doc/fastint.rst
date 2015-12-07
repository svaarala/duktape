============
Fastint type
============

Overview
========

Ecmascript has a single number type which is required to be an IEEE double.
This is a potential performance issue in some embedded environments where
hardware floating point numbers (at least IEEE doubles) are not available
and software floating point emulation performs poorly.

Duktape provides optional support for fast integers or "fastints" which
allows Duktape to represent numbers internally either as IEEE doubles or
48-bit signed integers.  Duktape will transparently upgrade integers to
doubles when necessary (e.g. when an integer operation overflows) and
downgrade doubles to integers when possible.

Because a double-to-integer downgrade check is relatively expensive, it is
only applied in specific situations.  Currently:

* All compiler constants are represented as fastints if possible.

* Unary plus performs a ToNumber() coercion and also downgrades an IEEE
  double to a fastint if possible.

* All function return values are automatically downgraded to fastints if
  possible.

* Thread yield/resume values are automatically downgraded to fastints if
  possible.

Fastints don't affect Ecmascript semantics and are completely transparent
to user C and Ecmascript code: all conversions are automatic.

To enable fastint support, simply define:

* ``DUK_OPT_FASTINT`` / ``DUK_USE_FASTINT``

You should measure the impact of enabling fastint support for your target
platform and Ecmascript code base.  Fastint support is not an automatic
performance win: while the fast path is a clear improvement for soft float
(and even some hard float) platforms, there is a run-time cost of doing
fastint downgrade checks and other book-keeping.  Very roughly:

* Code that benefits most from fastint upsides (e.g. heavy integer arithmetic
  in large loops) can run about 1000% faster on soft float platforms.

* Code that suffers most from fastint downsides can run about 10% more
  slowly.

* Executable size will increase by about 7-10kB.

This document provides tips for using fastints, and provides some background
on the approach chosen.  Some specific fastint algorithms used by Duktape are
also described in detail.

Application considerations
==========================

Because fastints are transparent to user code, the only real consideration is
to make sure performance critical sections take advantage of fastints.  Some
tips for using fastints:

* Because a double-to-fastint downgrade check is only done for specific
  operations, make sure that integer values don't accidentally become
  IEEE doubles.

  There's no easy way to check how a number is represented internally.
  However, ``Duktape.info()`` provides a way to peek into the internal
  representation.  An example algorithm is provided in
  ``polyfills/duktape-isfastint.js``.  You can use this polyfill to debug
  your code if necessary.

* When in doubt, you can use unary plus to force a number to be downgrade
  checked::

      // Result is exactly 1, but is represented internally as a double.
      var t = Math.PI / Math.PI;

      // Result is exactly 1, downgrade checked, and is represented
      // internally as a fastint.
      var t = +(Math.PI / Math.PI);

* All function return values from both Ecmascript and Duktape/C functions
  are automatically downgraded to fastints.  So, the following value can be
  trusted to be 3 and represented internally as a fastint::

      // Resulting 'three' is a fastint because Math.floor() return
      // value (double 3) is automatically downgraded to a fastint.
      var three = Math.floor(Math.PI);

  Same applies to any user functions::

      function my_max(a, b) {
          // For the call below, 'b' is 1 but is not represented as a
          // fastint here.  Only when we return is the return value 1
          // downgraded into a fastint.
          return (a >= b ? a : b);
      }

      // 't' is exactly 1, and represented internally as a fastint.
      var t = my_max(0, Math.PI / Math.PI);

* All compiler constants are automatically downgraded to fastints when
  possible.  For example, all constants below will be fastints::

      var i, n;

      for (i = 0, n = 1e6; i < n; i++) {
          // All 'i' values here will be fastints.
      }

* Note that the number syntax doesn't affect the fastint downgrade check,
  only the final value matters.  All of the following will be represented
  as fastints::

      t = 1;
      t = 1.0;
      t = 100e-2;
      t = 0.01e2;

  Similarly constant folding, when possible, will be done before doing the
  downgrade check, so the following will be represented as a fastint::

      t = 123.123 / 123.123;  // fastint 1

  But because ``Math.PI`` needs a runtime lookup, the following will not be
  a fastint::

      t = Math.PI / Math.PI;  // double 1

* Non-fastint values will "taint" fastints in operations so that the result
  will be represented as a double instead of a fastint::

      t1 = 123;            // fastint
      t2 = 0.5;            // double
      t3 = t1 + t2;        // <fastint> + <double> -> <double>
      t4 = t3 - t2;        // <double> - <double> -> <double>
      t5 = +t4;            // restore into fastint representation

  While adding and subtracting ``t2`` is a net zero change and ``t4`` would
  be fastint compatible, it will not be represented as a fastint internally
  until the next explicit downgrade check.  Here unary plus is used to get
  the result back into fastint representation.

* Negative zero cannot be represented as a fastint.  Ordinary Ecmascript
  code will very rarely deal with negative zeros.  Negative zero can "taint"
  a fastint, too::

      t1 = 123;      // fastint
      t2 = -0;       // double
      t3 = t1 + t2;  // <fastint> + <double> -> <double> (!)

  Here the result is a double even when an innocent zero value is added to
  a fastint.  When in doubt you can use unary plus to ensure the result is
  a fastint if it's fastint compatible.

* When doing Duktape API calls from C code, prefer API calls which take
  integer arguments.  Such API calls will typically have fastint support.
  For example::

      // Value pushed will be 1, represented internally as a double.
      duk_push_number(ctx, 1.0);

      // Value pushed will be 1, represented internally as a fastint.
      duk_push_int(ctx, 1);

* Because the fastint support is transparent from a semantics perspective,
  Duktape fastint fast path and downgrade behavior may change in future
  versions.  Such changes won't change outward behavior but may affect
  code performance.

  As a general rule, optimize for fastints only in code sections where it
  really matters for performance, e.g. heavy loops.

Detecting that a number is represented as a fastint internally
==============================================================

There's no explicit API for this now, but ``Duktape.info()`` provides the
necessary information (in a highly fragile manner though).  For instance,
you can use something like::

  /* Fastint tag depends on duk_tval packing */
  var fastintTag = (Duktape.info(true)[1] === 0xfff5 ?
                   0xfff2 /* tag for packed duk_tval) :
                   1 /* tag for unpacked duk_tval */ );

  function isFastint(x) {
      if (typeof x !== 'number') {
          return false;
      }
      return Duktape.info(x)[1] === fastintTag;
  }

There's an example polyfill which provides ``Duktape.isFastint()`` in:

* polyfills/duktape-isfastint.js

.. note:: This is fragile and may stop working when internal tag number
   changes are made.  Such changes are possible even in minor version
   updates.

Fastints and Duktape internals
==============================

A few notes on how fastints are used internally, what macros are used, etc.

Fastint aware vs. unware code
-----------------------------

Fastint support is optional and added between ifdefs::

  #if defined(DUK_USE_FASTINT)
  ...
  #endif

Number handling will be either:

* fastint unaware: requires no changes to existing code

* fastint aware: requires fastint detection e.g. in switch-case statements
  and then usage of fastint aware macros

Type switch cases
-----------------

The minimum change necessary is to ensure fastints are handled in type
switch-cases::

  /* ... */

      switch(DUK_TVAL_GET_TAG(tv)) {
      case DUK_TAG_UNDEFINED:
          /* ... */
  #if defined(DUK_USE_FASTINT)
      case DUK_TAG_FASTINT:
          /* no direct support, fall through */
  #endif
      default:
          /* number, double or fastint; use fastint unaware macros
           * which will automatically upgrade a fastint to a double
           * when necessary:
           */

          duk_double_t d = DUK_TVAL_GET_NUMBER(tv);  /* auto upgrade */
          /* ... */
      }

Even without this change the default clause will capture ``DUK_TAG_FASTINT``
values but it's preferable to have the fall through happen explicitly.

Fastint aware code will have specific code in the ``DUK_TAG_FASTINT`` case,
and the ``default`` case can then assume the number is represented as a
double.  The ``default`` case must be written carefully so that it also works
correctly when fastints are disabled.

Getting numbers/fastints
------------------------

Fastint unaware code uses::

  DUK_TVAL_GET_NUMBER(tv)

which will always evaluate to a double, and automatically upgrades a fastint
to a double.  The implementation with fastints enabled is something like::

  #define DUK_TVAL_GET_NUMBER(v) \
      (DUK_TVAL_IS_FASTINT(v) ? \
          (duk_double_t) DUK_TVAL_GET_FASTINT(v) : \
          DUK_TVAL_GET_DOUBLE(v))

The extra compared to a direct read has a small runtime cost, but only when
fastints are enabled.  When they're not enabled, ``DUK_TVAL_GET_NUMBER()``
will just read a double.

Fastint aware code uses the following::

  /* When 'tv' is known to be a fastint, e.g. switch DUK_TAG_FASTINT or
   * explicit check.
   */
  DUK_TVAL_GET_FASTINT(tv)  /* result is duk_int64_t */

  /* When 'tv' is known to be a fastint, and we just need the lowest 32 bits
   * as a duk_uint32_t.
   */
  DUK_TVAL_GET_FASTINT_U32(tv)  /* result is duk_uint32_t */

  /* Similarly for a duk_int32_t. */
  DUK_TVAL_GET_FASTINT_I32(tv)  /* result is duk_int32_t */

  /* When 'tv' is known to be a double, e.g. switch or explicit check. */
  DUK_TVAL_GET_DOUBLE(tv)

The ``DUK_TVAL_GET_DOUBLE(tv)`` macro is also defined when fastints are not
enabled; in that case it's simply a synonym for ``DUK_TVAL_GET_NUMBER()``
because all numbers are represented as doubles.  It should only be used when
in the fastint enabled case the number is known to be represented as a double.

This allows control structures like::

  /* Fictional ToBoolean()-like operation. */

      switch(DUK_TVAL_GET_TAG(tv)) {
      ...
  #if defined(DUK_USE_FASTINT)
      case DUK_TAG_FASTINT:
          /* Fastints enabled and 'tv' is a fastint. */
          return (DUK_TVAL_GET_FASTINT(tv) != 0 ? 1 : 0);
  #endif
      default:
          /* Fastints enabled and 'tv' is a double, or fastints disabled. */
          return (DUK_TVAL_GET_DOUBLE(tv) != 0.0 ? 1 : 0);
      }

Setting numbers/fastints
------------------------

Fastint unaware code uses::

  DUK_TVAL_SET_NUMBER(tv, d);

This sets the number always into an internal double representation, i.e.
no double-to-fastint downgrade is automatically done.  (This was one
design option, but it turns out double-to-fastint coercion test is quite
expensive and adds a considerable overhead to the fastint unaware slow
path.)

Fastint aware which wants to set a double and downgrade it automatically
into a fastint when possible uses::

  DUK_TVAL_SET_NUMBER_CHKFAST(tv, d);

This macro concretely calls into a helper function so there's a performance
penalty involved.  Downgrade checks are only added to specific places where
they provide the most benefit.

Fastint aware code which wants to set a double explicitly (with no fastint
downgrade check) uses::

  DUK_TVAL_SET_DOUBLE(tv, d);

Fastint aware code which wants to set a fastint explicitly (and has ensured
that the value is fastint compatible) uses::

  /* 'i' must be in 48-bit signed range */
  DUK_TVAL_SET_FASTINT(tv, i);  /* i is duk_int64_t */

  /* 'i' must be in 32-bit unsigned range */
  DUK_TVAL_SET_FASTINT_U32(tv, i);  /* i is duk_uint32_t */

  /* 'i' must be in 32-bit signed range */
  DUK_TVAL_SET_FASTINT_I32(tv, i);  /* i is duk_int32_t */

The following macros are available even when fastints are disabled::

  DUK_TVAL_SET_DOUBLE(tv, d);
  DUK_TVAL_SET_NUMBER_CHKFAST(tv, d);

When fastints are disabled the macros will just write a double with no
checks or additional overhead.  This is just a convenience to reduce the
number of ifdefs.

In-place double-to-fastint downgrade check
------------------------------------------

The following macro is used to perform an in-place double-to-fastint
downgrade check::

  DUK_TVAL_CHKFAST_INPLACE(tv);

The target 'tv' can have any type; the macro first checks if the value
is a double and if so, if it can be fastint coerced.

When fastint support is disabled, the macro is a no-op.

Type checks
-----------

Fastint unaware code checks for a number (either double or fastint) using::

  DUK_TVAL_IS_NUMBER(tv)

Fastint aware code uses::

  /* Number represented as a fastint */
  DUK_TVAL_IS_FASTINT(tv)

  /* Number represented as a double */
  DUK_TVAL_IS_DOUBLE(tv)

The following is defined even when fastints are disabled to support the
switch code structure described above::

  /* When fastints disabled, same as DUK_TVAL_IS_NUMBER() */
  DUK_TVAL_IS_DOUBLE(tv)

Background
==========

This section provides some background, discussion, and issues on various
approaches to integer support.  It's not up to date with the current
implementation.

Approaches to integer support
-----------------------------

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

* Extend the tagged type to support both an IEEE double and an integer or a
  fixed point type.  Extend the public API and Ecmascript environment to
  expose the new integer type explicitly.  The upside is minimal performance
  cost because there are fewer automatic conversion checks.  The downside is
  a significant API change and introduction of custom language features.

* Same as above, but expose the integer type only for user C code; keep the
  Ecmascript environment unaware of the change.

Implementation issues
---------------------

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
----------------------

Double type + separate integer / fixed point type (compliant)
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

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
  the integer part must support both signed and unsigned 32-bit values.
  Binary fractions require some additional shifting to implement, and user
  code is not very likely to contain specific binary fractions, so they would
  only benefit code specifically crafted to use them.

* Double and 32-bit signed or unsigned integer: 32-bit arithmetic is nice
  but unfortunately not enough to support Ecmascript bit operations which
  require the range -0x80000000 to 0xffffffff (sign + 32 bits, a 33-bit
  representation).  This would not be a compliance issue as Duktape would
  fall back to the IEEE double for some values, but if fast bit operations
  are important matter, this is not a good option.  If bit operations don't
  matter, then this is a nice option in that it avoids the 64-bit arithmetic
  issue.

Only integer / fixed point type (non-compliant)
:::::::::::::::::::::::::::::::::::::::::::::::

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
------------------------------------

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

Efficient check for double-to-fastint downgrade
===============================================

Overview
--------

For an IEEE double to be representable as a fast integer, it must be:

* A whole number

* In the signed 48-bit range

* Not a negative zero, assuming that the integer zero is taken to represent
  a positive zero

This algorithm is needed when Duktape does an explicit downgrade check to see
if a double value can be represented as a fastint.

The "fast path" for fastint operations doesn't execute this algorithm because
both inputs and outputs are fastints and Duktape detects this in the fast path
preconditions.  Even so the performance of the downgrade check matters for
overall performance.

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
    zeroes (with the top bit either zero or one).  This corresponds to the
    numbers +/- 2 and +/- 3.

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

Performance notes
-----------------

Coercing a double to an int64_t seems to be very slow on some platforms, so it
may be faster to get the fastint out of the IEEE double value with custom C
code.  The code doesn't need to handle denormals, NaNs, etc, so it can be much
simpler than a full coercion routine.

There's a standard trick which is based on adding a double constant that
forces the mantissa to be shifted so that the integer value can be directly
extracted.  See e.g.:

* http://stackoverflow.com/questions/17035464/a-fast-method-to-round-a-double-to-a-32-bit-int-explained

A similar trick is used in the number-to-double upgrade, see below.

Efficient check for number-to-double upgrade
============================================

Slow path code often needs to handle a number which may be either a fastint or
a double.  The code needs to read the value efficiently as a double.  To
minimize the slow path penalty, this check and conversion from a fastint to
a double (if necessary) needs to be fast.

The algorithm has two parts: (1) detecting that the value is a fastint, and
(2) converting a fastint into a double if necessary.

Checking for a fastint
----------------------

Checking for a fastint is easy:

* For packed duk_tval: if 16 highest bits are 0xfff1 (DUK_TAG_FASTINT) the
  value is a fastint.

* For unpacked duk_tval: compare tag value similarly.

Trivial fastint-to-double conversion
------------------------------------

Converting a fastint into a double could be done by:

1. Sign extending the 48-bit value into a signed 64-bit value; the sign
   extension can be achieved by two shifts.

2. Coercing the 64-bit value to a double.

Example::

  duk_int64_t tmp = du.ull[DUK_DBL_IDX_ULL0];
  tmp = (tmp << 16) >> 16;  /* sign extend */
  return (duk_double_t) tmp;

Unfortunately this is very slow, at least on some soft float platforms
where this was tested on.

Alternate fastint-to-double conversion
--------------------------------------

Because the input number range is 48-bit signed (and zero) the conversion can
be optimized a great deal.  Let's first consider a positive value [1,2^47-1]:

* Construct an IEEE double with:

  - Sign = 0

  - Exponent field = 1023 + 52 = 1075

  - Mantissa = the 52-bit fastint value aligned to the right of the field,
    i.e. padded with zero bits on the left

* Because of the implicit leading 1-bit, the value represented is 2^52 +
  fastint_value.  Floating point subtract 2^52 to yield the final result.

The C code for this could be something like::

  /* For fastint value [1,2^47-1]. */
  du.ull[DUK_DBL_IDX_ULL0] = (duk_uint64_t) fastint_value |
                             (duk_uint64_t) 0x4330000000000000ULL;
  du.d = du.d - 4503599627370496.0;  /* 1<<52 */
  return du.d;

Negative values need similar handling but the double sign bit needs to be set.
It's good to avoid sign extending the 48-bit value::

  /* For fastint value [-2^47,-1]. */
  du.ull[DUK_DBL_IDX_ULL0] = ((duk_uint64_t) (-fastint_value) &
                              (duk_uint64_t) 0x000fffffffffffffULL) |
                             (duk_uint64_t) 0xc330000000000000ULL;
  du.d = du.d + 4503599627370496.0;  /* 1<<52 */
  return du.d;

Zero fastint is simply represented as an IEEE double with all bits zero, which
unfortunately needs a separate case.

In the concrete implementation the fastint_value might include the fastint
duk_tval tag and be masked out also for the positive number case.

Future work
===========

Fastint on platforms with no 64-bit integer type
------------------------------------------------

Currently fastint support can only be used if the platform/compiler has
support for a 64-bit integer type.  This limitation could be removed by
implementing alternative fastint fast paths which only relied on 32-bit
arithmetic.

32-bit fastint
--------------

It might be worth investigating if a signed or unsigned 32-bit fastint
(instead of a signed 48-bit fastint) would be more useful.  Fast path
arithmetic would certainly be faster.

The downside would be that some bit operations won't be possible: to
fully support all bit operations both signed and unsigned 32-bit values
is needed.

Optimize upgrade and downgrade
------------------------------

These operations are very important for performance so perhaps inline
assembler optimization would be useful for specific platforms, e.g. ARM.

The current C algorithms can also be optimized further.
