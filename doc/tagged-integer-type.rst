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
