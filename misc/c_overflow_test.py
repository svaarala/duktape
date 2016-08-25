#!/usr/bin/env python2

import math

limit = (1 << 32) - 1
for i in xrange(65536 + 10):
    if i == 0:
        continue

    temp = float(1 << 32) / float(i)
    approx1 = int(math.floor(temp) - 3)
    approx2 = int(math.floor(temp + 3))
    for j in xrange(approx1, approx2 + 1):
        if i*j >= (1 << 32):
            exact = True
        else:
            exact = False

        if i > limit / j:
            check = True
        else:
            check = False

        #print(i, j, exact, check)
        if exact != check:
            print('inexact', i, j)
