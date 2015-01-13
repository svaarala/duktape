#!/usr/bin/python
#
#  UnicodeData.txt may contain ranges in addition to individual characters.
#  Unpack the ranges into individual characters for the other scripts to use.
#

import os
import sys

def main():
	f_in = open(sys.argv[1], 'rb')
	f_out = open(sys.argv[2], 'wb')
	while True:
		line = f_in.readline()
		if line == '' or line == '\n':
			break
		parts = line.split(';')  # keep newline
		if parts[1].endswith('First>'):
			line2 = f_in.readline()
			parts2 = line2.split(';')
			if not parts2[1].endswith('Last>'):
				raise Exception('cannot parse range')
			cp1 = long(parts[0], 16)
			cp2 = long(parts2[0], 16)

			for i in xrange(cp1, cp2 + 1):  # inclusive
				parts[0] = '%04X' % i
				f_out.write(';'.join(parts))
		else:
			f_out.write(line)

	f_in.close()
	f_out.flush()
	f_out.close()

if __name__ == '__main__':
	main()
