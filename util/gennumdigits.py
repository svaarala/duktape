#!/usr/bin/env python2
#
#  Generate a table indicating how many digits should be considered
#  significant for each radix (2 to 36) when doing string-to-number
#  conversion.
#
#  For decimal, the E5/E5.1 specification indicates that anything
#  after the 20th digit can be ignored (treated as zero) and the
#  20th digit can be rounded upwards.  We estimate the significant
#  bits of precision from this and compute similar values for other
#  radix values.
#
#  Also generate a table of minimum and maximum radix-specific
#  exponent values above and below which a number is guaranteed
#  to overflow to Infinity or underflow to zero.  This allows the
#  C code to quick reject such exponent values, and to keep bigint
#  values bounded.  The exponent limit is relative to an integer
#  significand padded to the precision-related digit count (e.g.
#  20 for decimal).
#

import math

digits_table = []
limits_table = []

for radix in xrange(2, 36+1):
    bits_per_digit = math.log(radix, 2)

    if radix == 10:
        prec_digits = 20
    else:
        target_bits = math.ceil(math.log(10, 2) * 20) + 2  # +2 is extra, just in case
        prec_digits = int(math.ceil(target_bits / bits_per_digit))
    digits_table.append(prec_digits)

    # these are conservative (details are off by one etc); +/- 2 is the extra
    overflow_limit = int(math.ceil(1024.0 / bits_per_digit)) + 2 - prec_digits
    underflow_limit = int(math.floor((-1024.0 - 52.0) / bits_per_digit)) - 2 - prec_digits

    limits_table.append(overflow_limit)
    limits_table.append(underflow_limit)

print repr(digits_table)
print repr(limits_table)
