#!/usr/bin/python
#
#  Check various C source code policy rules and issue warnings for offenders
#
#  Usage:
#
#    $ python check_code_policy.py src/*.c
#

import os
import sys
import re
import optparse

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
re_identifier = re.compile(r'[A-Za-z0-9_]+')

# These identifiers are wrapped in duk_features.h.in, and should only be used
# through the wrappers elsewhere.
rejected_plain_identifiers_list = [
	# math classification
	'fpclassify',
	'signbit',
	'isfinite',
	'isnan',
	'isinf',
	'FP_NAN',
	'FP_INFINITE',
	'FP_ZERO',
	'FP_SUBNORMAL',
	'FP_NORMAL',

	# math functions
	'fabs',
	'fmin',
	'fmax',
	'floor',
	'ceil',
	'fmod',
	'pow',
	'acos',
	'asin',
	'atan',
	'atan2',
	'sin',
	'cos',
	'tan',
	'exp',
	'log',
	'sqrt',

	# memory functions
	'malloc',
	'realloc',
	'calloc',
	'free',
	'memcpy',
	'memmove',
	'memcmp',
	'memset',

	# string functions
	'strlen',
	'strcmp',
	'strncmp',
	'printf',
	'fprintf',
	'sprintf',
	'_snprintf',
	'snprintf',
	'vsprintf',
	'_vsnprintf',
	'vsnprintf',
	'sscanf',
	'vsscanf',

	# streams
	'stdout',
	'stderr',
	'stdin',

	# file ops
	'fopen',
	'fclose',
	'fread',
	'fwrite',
	'fseek',
	'ftell',
	'fflush',
	'fputc',

	# misc
	'abort',
	'exit',
	'setjmp',
	'longjmp'
]
rejected_plain_identifiers = {}
for id in rejected_plain_identifiers_list:
	rejected_plain_identifiers[id] = True

debuglog_wrappers = {
	'DUK_DPRINT': 'DUK_D',
	'DUK_DDPRINT': 'DUK_DD',
	'DUK_DDDPRINT': 'DUK_DDD'
}

problems = []

re_repl_c_comments = re.compile(r'/\*.*?\*/', re.DOTALL)
re_repl_cpp_comments = re.compile(r'//.*?\n', re.DOTALL)
re_repl_string_literals_dquot = re.compile(r'''\"(?:\\\"|[^\"])+\"''')
re_repl_string_literals_squot = re.compile(r'''\'(?:\\\'|[^\'])+\'''')
re_repl_expect_strings = re.compile(r'/\*===.*?===*?\*/', re.DOTALL)
re_not_newline = re.compile(r'[^\n]+', re.DOTALL)

def removeCommentsAndLiterals(data):
	def repl_c(m):
		tmp = re.sub(re_not_newline, '', m.group(0))
		if tmp == '':
			tmp = ' '  # avoid /**/
		return '/*' + tmp + '*/'
	def repl_cpp(m):
		return '// removed\n'
	def repl_dquot(m):
		return '"' + ('.' * (len(m.group(0)) - 2)) + '"'
	def repl_squot(m):
		return "'" + ('.' * (len(m.group(0)) - 2)) + "'"

	data = re.sub(re_repl_c_comments, repl_c, data)
	data = re.sub(re_repl_cpp_comments, repl_cpp, data)
	data = re.sub(re_repl_string_literals_dquot, repl_dquot, data)
	data = re.sub(re_repl_string_literals_squot, repl_squot, data)
	return data

def removeExpectStrings(data):
	def repl(m):
		tmp = re.sub(re_not_newline, '', m.group(0))
		if tmp == '':
			tmp = ' '  # avoid /*======*/
		return '/*===' + tmp + '===*/'

	data = re.sub(re_repl_expect_strings, repl, data)
	return data

def checkDebugLogCalls(line, filename):
	# Allowed debug log forms:
	#
	#     DUK_D(DUK_DPRINT(...))
	#     DUK_DD(DUK_DDPRINT(...))
	#     DUK_DDD(DUK_DDDPRINT(...))
	#
	# The calls may span multiple lines, but the wrapper (DUK_D)
	# and the log macro (DUK_DPRINT) must be on the same line.

	if 'DPRINT' not in line:
		return

	m = re_debuglog_callsite.match(line)
	if m is None:
		return

	log_macro = m.group(1)
	log_wrapper = debuglog_wrappers[log_macro]
	if log_wrapper + '(' in line:
		return

	# exclude '#define DUK_DPRINT...' macros in duk_debug.h
	if len(line) >= 1 and line[0] == '#':
		return

	# exclude a few comment lines in duk_debug.h
	if len(line) >= 3 and line[0:3] == ' * ':
		return

	raise Exception('invalid debug log call form')

def checkTrailingWhitespace(line, filename):
	if len(line) > 0 and line[-1] == '\n':
		line = line[:-1]

	m = re_trailing_ws.match(line)
	if m is None:
		return

	raise Exception('trailing whitespace')

def checkCarriageReturns(line, filename):
	if not '\x0d' in line:
		return

	raise Exception('carriage return')

def checkMixedIndent(line, filename):
	if not '\x20\x09' in line:
		return

	# Mixed tab/space are only allowed after non-whitespace characters
	idx = line.index('\x20\x09')
	tmp = line[0:idx]
	m = re_only_ws.match(tmp)
	if m is None:
		return

	raise Exception('mixed space/tab indent (idx %d)' % idx)

def checkFixme(line, filename):
	if not 'FIXME' in line:
		return

	raise Exception('FIXME on line')

def checkIdentifiers(line, filename):
	bn = os.path.basename(filename)
	excludePlain = (bn == 'duk_features.h.in' or \
	                bn[0:5] == 'test-')

	for m in re.finditer(re_identifier, line):
		if rejected_plain_identifiers.has_key(m.group(0)):
			if not excludePlain:
				raise Exception('invalid identifier %r (perhaps plain)' % m.group(0))

def processFile(filename, checkersRaw, checkersNoComments, checkersNoExpectStrings):
	f = open(filename, 'rb')
	dataRaw = f.read()
	f.close()

	dataNoComments = removeCommentsAndLiterals(dataRaw)   # no c/javascript comments, literals removed
	dataNoExpectStrings = removeExpectStrings(dataRaw)    # no testcase expect strings

	linesRaw = dataRaw.split('\n')
	linesNoComments = dataNoComments.split('\n')
	linesNoExpectStrings = dataNoExpectStrings.split('\n')

	def f(lines, checkers):
		linenumber = 0
		for line in lines:
			linenumber += 1
			for fun in checkers:
				try:
					fun(line, filename)
				except Exception as e:
					problems.append(Problem(filename, linenumber, line, str(e)))

	f(linesRaw, checkersRaw)
	f(linesNoComments, checkersNoComments)
	f(linesNoExpectStrings, checkersNoExpectStrings)

	# Last line should have a newline, and there should not be an empty line.
	# The 'split' result will have one empty string as its last item in the
	# expected case.  For a single line file there will be two split results
	# (the line itself, and an empty string).

	if len(linesRaw) == 0 or \
	   len(linesRaw) == 1 and linesRaw[-1] != '' or \
	   len(linesRaw) >= 2 and linesRaw[-1] != '' or \
	   len(linesRaw) >= 2 and linesRaw[-1] == '' and linesRaw[-2] == '':
		problems.append(Problem(filename, 0, '(no line)', 'No newline on last line or empty line at end of file'))

def main():
	parser = optparse.OptionParser()
	parser.add_option('--dump-vim-commands', dest='dump_vim_commands', default=False, help='Dump oneline vim command')
	(opts, args) = parser.parse_args()

	checkersRaw = []
	checkersRaw.append(checkDebugLogCalls)
	checkersRaw.append(checkCarriageReturns)
	checkersRaw.append(checkFixme)

	checkersNoComments = []
	checkersNoComments.append(checkIdentifiers)

	checkersNoExpectStrings = []
	checkersNoExpectStrings.append(checkTrailingWhitespace)
	checkersNoExpectStrings.append(checkMixedIndent)

	for filename in args:
		processFile(filename, checkersRaw, checkersNoComments, checkersNoExpectStrings)

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

		if opts.dump_vim_commands:
			cmds = []
			for i in problems:
				cmds.append('vim +' + str(i.linenumber) + ' "' + i.filename + '"')
			print ''
			print('; '.join(cmds))

		sys.exit(1)

	sys.exit(0)

if __name__ == '__main__':
	main()
