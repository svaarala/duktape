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
			non_ascii = True
	if non_ascii:
		print '%s: non-ascii data on line %d' % (sys.argv[1], linenum + 1)

