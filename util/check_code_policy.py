#!/usr/bin/python
#
#  Check various C source code policy rules and issue warnings for offenders
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

class Problem:
	filename = None
	linenumber = None
	line = None
	reason = None

	def __init__(self, filename, linenumber, line, reason):
		self.filename = filename
		self.linenumber = linenumber
		self.line = line
		self.reason = reason


re_debuglog_callsite = re.compile(r'^.*?(DUK_D+PRINT).*?$')
re_trailing_ws = re.compile(r'^.*?\s$')
re_only_ws = re.compile(r'^\s*$')

debuglog_wrappers = {
	'DUK_DPRINT': 'DUK_D',
	'DUK_DDPRINT': 'DUK_DD',
	'DUK_DDDPRINT': 'DUK_DDD'
}

problems = []

def checkDebugLogCalls(line):
	if 'DPRINT' not in line:
		return

	m = re_debuglog_callsite.match(line)
	if m is None:
		return

	log_macro = m.group(1)
	log_wrapper = debuglog_wrappers[log_macro]
	if log_wrapper + '(' in line:
		return

	raise Exception('invalid debug log call form')

def checkTrailingWhitespace(line):
	if len(line) > 0 and line[-1] == '\n':
		line = line[:-1]

	m = re_trailing_ws.match(line)
	if m is None:
		return

	raise Exception('trailing whitespace')

def checkCarriageReturns(line):
	if not '\x0d' in line:
		return

	raise Exception('carriage return')

def checkMixedIndent(line):
	if not '\x20\x09' in line:
		return

	# Mixed tab/space are only allowed after non-whitespace characters
	idx = line.index('\x20\x09')
	tmp = line[0:idx]
	m = re_only_ws.match(tmp)
	if m is None:
		return

	raise Exception('mixed space/tab indent (idx %d)' % idx)

def processFile(filename, checkers):
	f = open(filename, 'rb')

	linenumber = 0
	for line in f:
		linenumber += 1
		for fun in checkers:
			try:
				fun(line)
			except Exception as e:
				problems.append(Problem(filename, linenumber, line, str(e)))
	f.close()

def main():
	checkers = []
	checkers.append(checkDebugLogCalls)
	checkers.append(checkTrailingWhitespace)
	checkers.append(checkCarriageReturns)
	checkers.append(checkMixedIndent)

	for filename in sys.argv[1:]:
		processFile(filename, checkers)

	if len(problems) > 0:
		for i in problems:
			tmp = 'vim +' + str(i.linenumber)
			while len(tmp) < 10:
				tmp = tmp + ' '
			tmp += ' ' + str(i.filename) + ' : ' + str(i.reason)
			while len(tmp) < 80:
				tmp = tmp + ' '
			tmp += ' - ' + i.line.strip()
			print(tmp)

		print '*** Total: %d problems' % len(problems)
		sys.exit(1)

	sys.exit(0)

if __name__ == '__main__':
	main()
