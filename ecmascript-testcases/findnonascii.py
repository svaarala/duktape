#!/usr/bin/python
#
#  $ for i in *.js; do python findnonascii.py $i; done


import os, sys

f = open(sys.argv[1], 'rb')
data = f.read()
f.close()

for linenum, linedata in enumerate(data.split('\n')):
	non_ascii = False
	for i in xrange(len(linedata)):
		x = ord(linedata[i])
		if x >= 0x80:
			print '%s: non-ascii data on line %d, char index %d, value %d (0x%02x)' % \
				(sys.argv[1], linenum + 1, i + 1, x, x)
			non_ascii = True
