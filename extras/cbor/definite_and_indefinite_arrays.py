#!/usr/bin/env python2
import sys

x = '\x9f' + ( '\x01' + '\x02' + '\x03' + '\x9f\xff' + '\x04' + '\x80' + '\x80' + \
    '\x9f\x10\x11\xff' + '\x84\x04\x03\x02\x01' ) + '\xff'
sys.stdout.write(x)
