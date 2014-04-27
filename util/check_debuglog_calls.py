#!/usr/bin/python
#
#  Check debug log macro calls in C code for consistency.  Allowed forms:
#
#    DUK_D(DUK_DPRINT(...))
#    DUK_DD(DUK_DDPRINT(...))
#    DUK_DDD(DUK_DDDPRINT(...))
#
#  The calls may span multiple lines, but the wrapper (DUK_D) and the log
#  macro (DUK_DPRINT) must be on the same line.
#
#  Usage:
#
#    $ python check_debuglog_calls.py src/*.c
#

import os
import sys
import re

re_callsite = re.compile(r'^.*?(DUK_D+PRINT).*?$')

wrappers = {
	'DUK_DPRINT': 'DUK_D',
	'DUK_DDPRINT': 'DUK_DD',
	'DUK_DDDPRINT': 'DUK_DDD'
}

warnings = []

def process(filename):
	f = open(filename, 'rb')

	linenumber = 0
	for line in f:
		linenumber += 1
		if 'DPRINT' not in line:
			continue
		m = re_callsite.match(line)
		if m is None:
			continue
		log_macro = m.group(1)
		log_wrapper = wrappers[log_macro]
		if log_wrapper + '(' in line:
			continue
		warnings.append(filename + ':' + str(linenumber) + ': ' + line.strip())

	f.close()

def main():
	for filename in sys.argv[1:]:
		process(filename)

	if len(warnings) > 0:
		print 'WARNING: possibly incorrect debug log statements in code:'
		for i in warnings:
			print i

		sys.exit(1)
	sys.exit(0)

if __name__ == '__main__':
	main()
