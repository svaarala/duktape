#!/usr/bin/env python2
#
#  Print out a few IEEE double representations related to the Duktape fastint
#  number model.
#

import struct
import math

def isFastint(x):
    if math.floor(x) == x and \
       x >= -(2**47) and \
       x < (2**47) and \
       (x != 0 or math.copysign(1.0, x) == 1.0):  # Negative zero is a bit tricky
        return True
    return False

def stringRep(x):
    tmp = struct.pack('>d', x)
    tmphex = tmp.encode('hex')

    sgnexp = (ord(tmp[0]) << 8) + ord(tmp[1])
    sgn = (sgnexp) >> 15
    exp = (sgnexp & 0x7ff0) >> 4
    manthex = tmphex[3:]

    return '%s sgn=%d exp=%d sgnexp=%x manthex=%s' % (tmphex, sgn, exp, sgnexp, manthex)

def main():
    for i in [ -(2**47) - 1,
               -(2**47),
               -(2**47) + 1,
               -(2**32) - 1,
               -(2**32),
               -(2**32) + 1,
               -(long(0xdeadbeef)),
               -9,
               -8,
               -8,
               -7,
               -6,
               -5,
               -4,
               -3,
               -2,
               -1,
               -0.0,  # must use float to get neg zero
               0,
               1,
               2,
               3,
               4,
               5,
               6,
               7,
               8,
               9,
               long(0xdeadbeef),
               (2**32) - 1,
               (2**32),
               (2**32) + 1,
               (2**47) - 1,
               (2**47)
    ]:
        print('%f %x (fastint=%s): %s' % (float(i), int(i), str(isFastint(i)), stringRep(i)))

if __name__ == '__main__':
    main()
