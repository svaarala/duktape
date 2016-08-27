#!/usr/bin/env python2
#
#  Double constants, see http://en.wikipedia.org/wiki/Double-precision_floating-point_format.
#  YAML builtins metadata expressed the constants in binary form (8 bytes of
#  IEEE double data) to ensure bit exactness.
#

import struct
import mpmath

def create_double_constants_mpmath():
    # Just a helper to use manually
    # http://mpmath.googlecode.com/svn/trunk/doc/build/basics.html

    mpmath.mp.prec = 1000  # 1000 bits

    def printhex(name, x):
        # to hex string, ready for create_double()
        hex = struct.pack('>d', float(str(x))).encode('hex')
        flt = struct.unpack('>d', hex.decode('hex'))[0]
        print '%-11s -> %s  (= %.20f)' % (name, hex, flt)

    printhex('DBL_E', mpmath.mpf(mpmath.e))
    printhex('DBL_LN10', mpmath.log(10))
    printhex('DBL_LN2', mpmath.log(2))
    printhex('DBL_LOG2E', mpmath.log(mpmath.e) / mpmath.log(2))
    printhex('DBL_LOG10E', mpmath.log(mpmath.e) / mpmath.log(10))
    printhex('DBL_PI', mpmath.mpf(mpmath.pi))
    printhex('DBL_SQRT1_2', mpmath.mpf(1) / mpmath.sqrt(2))
    printhex('DBL_SQRT2', mpmath.sqrt(2))

create_double_constants_mpmath()

def create_double(x):
    return struct.unpack('>d', x.decode('hex'))[0]

DBL_NAN =                    create_double('7ff8000000000000')  # a NaN matching our "normalized NAN" definition (see duk_tval.h)
DBL_POSITIVE_INFINITY =      create_double('7ff0000000000000')  # positive infinity (unique)
DBL_NEGATIVE_INFINITY =      create_double('fff0000000000000')  # negative infinity (unique)
DBL_MAX_DOUBLE =             create_double('7fefffffffffffff')  # 'Max Double'
DBL_MIN_DOUBLE =             create_double('0000000000000001')  # 'Min subnormal positive double'
DBL_E =                      create_double('4005bf0a8b145769')  # (= 2.71828182845904509080)
DBL_LN10 =                   create_double('40026bb1bbb55516')  # (= 2.30258509299404590109)
DBL_LN2 =                    create_double('3fe62e42fefa39ef')  # (= 0.69314718055994528623)
DBL_LOG2E =                  create_double('3ff71547652b82fe')  # (= 1.44269504088896338700)
DBL_LOG10E =                 create_double('3fdbcb7b1526e50e')  # (= 0.43429448190325181667)
DBL_PI =                     create_double('400921fb54442d18')  # (= 3.14159265358979311600)
DBL_SQRT1_2 =                create_double('3fe6a09e667f3bcd')  # (= 0.70710678118654757274)
DBL_SQRT2 =                  create_double('3ff6a09e667f3bcd')  # (= 1.41421356237309514547)
