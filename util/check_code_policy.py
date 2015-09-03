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
re_nonleading_tab = re.compile(r'^.*?[^\t]\t.*?$')  # tabs are only used for indent
re_identifier = re.compile(r'[A-Za-z0-9_]+')
re_nonascii = re.compile(r'^.*?[\x80-\xff].*?$')
re_func_decl_or_def = re.compile(r'^(\w+)\s+(?:\w+\s+)*(\w+)\(.*?.*?$')  # may not finish on same line
re_cpp_comment = re.compile(r'^.*?//.*?$')

# These identifiers are wrapped in duk_config.h, and should only be used
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

allowed_visibility_macros = [
	'DUK_EXTERNAL_DECL',
	'DUK_EXTERNAL',
	'DUK_INTERNAL_DECL',
	'DUK_INTERNAL',
	'DUK_LOCAL_DECL',
	'DUK_LOCAL'
]

problems = []

re_repl_c_comments = re.compile(r'/\*.*?\*/', re.DOTALL)
re_repl_cpp_comments = re.compile(r'//.*?\n', re.DOTALL)
re_repl_string_literals_dquot = re.compile(r'''\"(?:\\\"|[^\"])*\"''')
re_repl_string_literals_squot = re.compile(r'''\'(?:\\\'|[^\'])*\'''')
re_repl_expect_strings = re.compile(r'/\*===.*?===*?\*/', re.DOTALL)
re_not_newline = re.compile(r'[^\n]+', re.DOTALL)

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

def removeLiterals(data):
	data = re.sub(re_repl_string_literals_dquot, repl_dquot, data)
	data = re.sub(re_repl_string_literals_squot, repl_squot, data)
	return data

def removeCCommentsAndLiterals(data):
	data = re.sub(re_repl_c_comments, repl_c, data)
	data = re.sub(re_repl_string_literals_dquot, repl_dquot, data)
	data = re.sub(re_repl_string_literals_squot, repl_squot, data)
	return data

def removeAnyCommentsAndLiterals(data):
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

def checkDebugLogCalls(lines, idx, filename):
	# Allowed debug log forms:
	#
	#     DUK_D(DUK_DPRINT(...))
	#     DUK_DD(DUK_DDPRINT(...))
	#     DUK_DDD(DUK_DDDPRINT(...))
	#
	# The calls may span multiple lines, but the wrapper (DUK_D)
	# and the log macro (DUK_DPRINT) must be on the same line.

	line = lines[idx]
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

def checkTrailingWhitespace(lines, idx, filename):
	line = lines[idx]
	if len(line) > 0 and line[-1] == '\n':
		line = line[:-1]

	m = re_trailing_ws.match(line)
	if m is None:
		return

	raise Exception('trailing whitespace')

def checkCarriageReturns(lines, idx, filename):
	line = lines[idx]
	if not '\x0d' in line:
		return

	raise Exception('carriage return')

def checkMixedIndent(lines, idx, filename):
	line = lines[idx]
	if not '\x20\x09' in line:
		return

	# Mixed tab/space are only allowed after non-whitespace characters
	idx = line.index('\x20\x09')
	tmp = line[0:idx]
	m = re_only_ws.match(tmp)
	if m is None:
		return

	raise Exception('mixed space/tab indent (idx %d)' % idx)

def checkNonLeadingTab(lines, idx, filename):
	line = lines[idx]
	m = re_nonleading_tab.match(line)
	if m is None:
		return

	raise Exception('non-leading tab (idx %d)' % idx)

def checkFixme(lines, idx, filename):
	line = lines[idx]
	if not 'FIXME' in line:
		return

	raise Exception('FIXME on line')

def checkIdentifiers(lines, idx, filename):
	line = lines[idx]
	# XXX: this now executes for every line which is pointless
	bn = os.path.basename(filename)
	excludePlain = (bn[0:5] == 'test-')

	for m in re.finditer(re_identifier, line):
		if rejected_plain_identifiers.has_key(m.group(0)):
			if not excludePlain:
				raise Exception('invalid identifier %r (perhaps plain)' % m.group(0))

def checkNonAscii(lines, idx, filename):
	line = lines[idx]
	m = re_nonascii.match(line)
	if m is None:
		return

	bn = os.path.basename(filename)
	if bn == 'test-lex-utf8.js':
		# this specific file is intentionally exempt
		pass
	else:
		raise Exception('non-ascii character')

def checkNoSymbolVisibility(lines, idx, filename):
	line = lines[idx]

	# Workaround for DUK_ALWAYS_INLINE preceding a declaration
	# (e.g. "DUK_ALWAYS_INLINE DUK_LOCAL ...")
	if line.startswith('DUK_ALWAYS_INLINE '):
		line = line[18:]

	m = re_func_decl_or_def.match(line)
	if m is None:
		return

	bn = os.path.basename(filename)
	if not ((bn[-2:] == '.c' or bn[-2:] == '.h' or bn[-5:] == '.h.in') and bn[0:5] != 'test-'):
		# Apply to only specific files in src/
		return

	if m.group(1) in allowed_visibility_macros and \
	   not ((m.group(1) != 'DUK_LOCAL' and m.group(1) != 'DUK_LOCAL_DECL') and 'duk__' in m.group(2)) and \
	   not ((m.group(1) == 'DUK_LOCAL' or m.group(1) == 'DUK_LOCAL_DECL') and 'duk__' not in m.group(2)):
		return

	# Previous line may contain the declaration (alone)
	if idx > 0 and lines[idx - 1].strip() in allowed_visibility_macros:
		return

	# Special exceptions
	# (None now)

	raise Exception('missing symbol visibility macro')

def checkCppComment(lines, idx, filename):
	line = lines[idx]
	m = re_cpp_comment.match(line)
	if m is None:
		return

	raise Exception('c++ comment')

def processFile(filename, checkersRaw, checkersNoCommentsOrLiterals, checkersNoCCommentsOrLiterals, checkersNoExpectStrings):
	global problems
	f = open(filename, 'rb')
	dataRaw = f.read()
	f.close()

	dataNoCommentsOrLiterals = removeAnyCommentsAndLiterals(dataRaw)   # no C/javascript comments, literals removed
	dataNoCCommentsOrLiterals = removeCCommentsAndLiterals(dataRaw)    # no C comments, literals removed
	dataNoExpectStrings = removeExpectStrings(dataRaw)                 # no testcase expect strings

	linesRaw = dataRaw.split('\n')
	linesNoCommentsOrLiterals = dataNoCommentsOrLiterals.split('\n')
	linesNoCCommentsOrLiterals = dataNoCCommentsOrLiterals.split('\n')
	linesNoExpectStrings = dataNoExpectStrings.split('\n')

	def f(lines, checkers):
		for linenumber in xrange(len(lines)):
			for fun in checkers:
				try:
					fun(lines, linenumber, filename)  # linenumber is zero-based here
				except Exception as e:
					problems.append(Problem(filename, linenumber + 1, lines[linenumber], str(e)))

	f(linesRaw, checkersRaw)
	f(linesNoCommentsOrLiterals, checkersNoCommentsOrLiterals)
	f(linesNoCCommentsOrLiterals, checkersNoCCommentsOrLiterals)
	f(linesNoExpectStrings, checkersNoExpectStrings)

	# Last line should have a newline, and there should not be an empty line.
	# The 'split' result will have one empty string as its last item in the
	# expected case.  For a single line file there will be two split results
	# (the line itself, and an empty string).

	if len(linesRaw) == 0 or \
	   len(linesRaw) == 1 and linesRaw[-1] != '' or \
	   len(linesRaw) >= 2 and linesRaw[-1] != '' or \
	   len(linesRaw) >= 2 and linesRaw[-1] == '' and linesRaw[-2] == '':
		problems.append(Problem(filename, len(linesRaw), '(no line)', 'No newline on last line or empty line at end of file'))

	# First line should not be empty (unless it's the only line, len(linesRaw)==2)
	if len(linesRaw) > 2 and linesRaw[0] == '':
		problems.append(Problem(filename, 1, '(no line)', 'First line is empty'))

def asciiOnly(x):
	return re.sub(r'[\x80-\xff]', '#', x)

def main():
	global problems
	parser = optparse.OptionParser()
	parser.add_option('--dump-vim-commands', dest='dump_vim_commands', default=False, help='Dump oneline vim command')
	parser.add_option('--check-debug-log-calls', dest='check_debug_log_calls', default=False, help='Check debug log call consistency')
	parser.add_option('--check-carriage-returns', dest='check_carriage_returns', default=False, help='Check carriage returns')
	parser.add_option('--check-fixme', dest='check_fixme', default=False, help='Check FIXME tags')
	parser.add_option('--check-non-ascii', dest='check_non_ascii', default=False, help='Check non-ASCII characters')
	parser.add_option('--check-no-symbol-visibility', dest='check_no_symbol_visibility', default=False, help='Check for missing symbol visibility macros')
	parser.add_option('--check-rejected-identifiers', dest='check_rejected_identifiers', default=False, help='Check for rejected identifiers like plain "printf()" calls')
	parser.add_option('--check-trailing-whitespace', dest='check_trailing_whitespace', default=False, help='Check for trailing whitespace')
	parser.add_option('--check-mixed-indent', dest='check_mixed_indent', default=False, help='Check for mixed indent (space and tabs)')
	parser.add_option('--check-nonleading-tab', dest='check_nonleading_tab', default=False, help='Check for non-leading tab characters')
	parser.add_option('--check-cpp-comment', dest='check_cpp_comment', default=False, help='Check for c++ comments ("// ...")')

	parser.add_option("-v", dest='verbose', action='store_true', default=False, help='Enable Verbose Output.')
	parser.add_option('-d',  default=[], action='append', dest='file_path', type='string', help='Specify Folder to Test Files')
	parser.add_option('--dir',  default=[], action='append', dest='file_path', type='string', help='Specify Folder to Test Files')
	parser.add_option('--ext',  default=[], action='append', dest='file_extensions', type='string', help='Specify File Extensions')
	(opts, args) = parser.parse_args()

	checkersRaw = []
	if opts.check_debug_log_calls:
		checkersRaw.append(checkDebugLogCalls)
	if opts.check_carriage_returns:
		checkersRaw.append(checkCarriageReturns)
	if opts.check_fixme:
		checkersRaw.append(checkFixme)
	if opts.check_non_ascii:
		checkersRaw.append(checkNonAscii)
	if opts.check_no_symbol_visibility:
		checkersRaw.append(checkNoSymbolVisibility)

	checkersNoCCommentsOrLiterals = []
	if opts.check_cpp_comment:
		checkersNoCCommentsOrLiterals.append(checkCppComment)

	checkersNoCommentsOrLiterals = []
	if opts.check_rejected_identifiers:
		checkersNoCommentsOrLiterals.append(checkIdentifiers)

	checkersNoExpectStrings = []
	if opts.check_trailing_whitespace:
		checkersNoExpectStrings.append(checkTrailingWhitespace)
	if opts.check_mixed_indent:
		checkersNoExpectStrings.append(checkMixedIndent)
	if opts.check_nonleading_tab:
		checkersNoExpectStrings.append(checkNonLeadingTab)
	if(0 != len(opts.file_path) and 0 != len(opts.file_extensions) ):
		totalProblems = 0;
		for filepath in opts.file_path:
			mFilePath = os.path.join('', filepath)
			for root, dirs, files in os.walk(filepath):
				for file in files:
					filename = os.path.join(root, file)
					for Ext in opts.file_extensions:
						if file.endswith(Ext):
							processFile(filename, checkersRaw, checkersNoCommentsOrLiterals, checkersNoCCommentsOrLiterals, checkersNoExpectStrings)
							if len(problems) > 0:
								totalProblems += len(problems)
								if opts.verbose:
									for i in problems:
										tmp = 'vim +' + str(i.linenumber)
										while len(tmp) < 10:
											tmp = tmp + ' '
										tmp += ' ' + str(i.filename) + ' : ' + str(i.reason)
										while len(tmp) < 80:
											tmp = tmp + ' '
										tmp += ' - ' + asciiOnly(i.line.strip())
										print(tmp)
								print '*** Total: %d problems for \"%s\"' % (len(problems),filename)
								if opts.dump_vim_commands and opts.verbose:
									cmds = []
									for i in problems:
										cmds.append('vim +' + str(i.linenumber) + ' "' + i.filename + '"')
									print ''
									print('; '.join(cmds))
								del problems[:] # Empty out the problems Array
								problems = []
		if(totalProblems):
			print '*** Total: %d problems' % totalProblems
			sys.exit(1)
		sys.exit(0)
	else:
		for filename in args:
			processFile(filename, checkersRaw, checkersNoCommentsOrLiterals, checkersNoCCommentsOrLiterals, checkersNoExpectStrings)

		if len(problems) > 0:
			for i in problems:
				tmp = 'vim +' + str(i.linenumber)
				while len(tmp) < 10:
					tmp = tmp + ' '
				tmp += ' ' + str(i.filename) + ' : ' + str(i.reason)
				while len(tmp) < 80:
					tmp = tmp + ' '
				tmp += ' - ' + asciiOnly(i.line.strip())
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
