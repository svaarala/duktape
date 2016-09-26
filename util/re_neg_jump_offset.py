#!/usr/bin/env python2

def xutf8len(x):
    if x < 0x80: return 1
    if x < 0x800: return 2
    if x < 0x10000: return 3
    if x < 0x200000: return 4
    if x < 0x4000000: return 5
    if x < 0x80000000: return 6
    return 7

def enci32(x):
    if x >= 0: return x
    return (-x) * 2 + 1

def skipadjust(x):
    if x >= 0: return 0
    t = xutf8len(enci32(x))
    t = xutf8len(enci32(x - t))  # byte length of encoded, adjusted skip
    return t

def adjusted(skip):
    return skip - skipadjust(skip)

# Binary search for lowest negative skip offset which, when adjusted for
# encoding length, encodes to numbytes.
def binsearch(numbytes):
    a = -1
    b = -0xffffffff

    while a - b > 1:
        c = (a + b) / 2
        n = skipadjust(c)
        #print(a, b, c, n, numbytes)
        if n > numbytes:
            b = c
        else:
            a = c
    if skipadjust(a) == numbytes:
        return a
    else:
        return b

def closed1(skip):
    if skip >= 0: return skip
    if skip >= -0x3e: skip -= 1
    elif skip >= -0x3fd: skip -= 2
    elif skip >= -0x7ffc: skip -= 3
    elif skip >= -0xffffb: skip -= 4
    elif skip >= -0x1fffffa: skip -= 5
    elif skip >= -0x3ffffff9: skip -= 6
    else: skip -= 7
    return skip

def closed2(skip):
    if skip >= 0: return skip
    skip -= 1
    if skip < -0x3f: skip -= 1
    if skip < -0x3ff: skip -= 1
    if skip < -0x7fff: skip -= 1
    if skip < -0xfffff: skip -= 1
    if skip < -0x1ffffff: skip -= 1
    if skip < -0x3fffffff: skip -= 1
    return skip

def main():
    def validate(skip):
        print('validate: skip %d -> adjusted %d, closed1 %d, closed2 %d' % (skip, adjusted(skip), closed1(skip), closed2(skip)))
        assert(adjusted(skip) == closed1(skip))
        assert(adjusted(skip) == closed2(skip))

    for i in xrange(1, 7):
        n = binsearch(i)
        print('lowest unadjusted skip offset for %d byte encoding: 0x%x' % (i, n))
        for j in xrange(-1, 2):
            validate(n + j)

    print('spot checks')
    validate(-0x7fffffff)
    validate(-0x80000000)

if __name__ == '__main__':
    main()
