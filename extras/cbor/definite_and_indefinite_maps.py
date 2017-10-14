#!/usr/bin/env python2
import sys

x = '\xbf' + ( '\x63foo\x63bar' + '\x63bar\xa0' + '\x64quux\xbf\xff' + \
    '\x63baz\xa2\x61a\x61b\x61c\x61d' + '\x65quuux\xbf\x61x\x61y\x61z\x61w\xff' ) + \
    '\xff'
sys.stdout.write(x)
