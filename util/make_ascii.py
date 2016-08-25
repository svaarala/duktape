#!/usr/bin/env python2
#
#  Paranoia escape input file to be printable ASCII.
#

import os, sys

inp = sys.stdin.read().decode('utf-8')
for c in inp:
    if (ord(c) >= 0x20 and ord(c) <= 0x7e) or (c in '\x0a'):
        sys.stdout.write(c)
    else:
        sys.stdout.write('\\u%04x' % ord(c))
