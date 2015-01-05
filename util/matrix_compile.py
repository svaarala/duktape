#!/usr/bin/python
#
#  Compile test for a lot of option combinations
#

# XXX: rewrite as nodejs and parallelize (large indices handling need bigint)

import os
import sys
import time
import datetime
import json
import random
import optparse
import subprocess
import StringIO  # no need for cStringIO

#
#  Test matrix helper: given a specification of combinations, count the
#  total number of combinations and allow a specific combination to be
#  fetched using an index.  This avoids creating the combinations explicitly
#  and also allows random sampling of the combination space (which can be
#  very large).
#

# Select one: Select([ 1, 2, 3 ]) -> [ 1 ], [ 2 ], [ 3 ]
class Select:
	val = None

	def __init__(self, val):
		self.val = val

# Combine: Combine([ 1, 2 ], 'foo') -> [ 1 'foo' ], [ 2 'foo' ]
class Combine:
	val = None

	def __init__(self, val):
		self.val = val

# Subset: Subset([ 'foo', 'bar' ]) -> Combine([ [ '', 'foo' ], [ '', 'bar' ] ])
#      -> [ '' '' ], [ 'foo' '' ], [ '' 'bar' ], [ 'foo' 'bar' ]
class Subset:
	val = None

	def __init__(self, val):
		self.val = val

# Sequence: Sequence([ 'foo', 'bar', 'quux' ]) -> [ 'foo', 'bar', 'quux' ]
# Plain list is also interpreted as a Sequence.
class Sequence:
	val = None

	def __init__(self, val):
		self.val = val

# Prepare a combination lookup structure.
def prepcomb(val):
	if isinstance(val, (str, unicode)):
		return { 'size': 1, 'value': val, 'type': 'terminal' }
	if isinstance(val, Sequence):
		return { 'size': 1, 'value': val.val, 'type': 'sequence' }
	if isinstance(val, list):
		# interpret as Sequence
		return { 'size': 1, 'value': val, 'type': 'sequence' }
	if isinstance(val, Select):
		nodes = []
		size = 0
		for i in val.val:
			node = prepcomb(i)
			nodes.append(node)
			size += node['size']
		return { 'size': size, 'value': nodes, 'type': 'select' }
	if isinstance(val, Combine):
		nodes = []
		size = 1
		for i in val.val:
			node = prepcomb(i)
			nodes.append(node)
			size *= node['size']
		return { 'size': size, 'value': nodes, 'type': 'combine' }
	if isinstance(val, Subset):
		nodes = []
		size = 1
		for i in val.val:
			node = prepcomb(i)
			nodes.append(node)
			size *= (node['size'] + 1)   # value or not present
		return { 'size': size, 'value': nodes, 'type': 'subset' }
	raise Exception('invalid argument')

# Return number of combinations for input lists.
def countcombinations(prepped):
	return prepped['size']

# Return a combination for index, for index in [0,countcombinations(lists)[.
# This allows random selection of combinations using a PRNG.
def getcomb(prepped, index):
	if prepped['type'] == 'terminal':
		return [ prepped['value'] ], index
	if prepped['type'] == 'sequence':
		return prepped['value'], index
	if prepped['type'] == 'select':
		idx = index % prepped['size']
		index = index / prepped['size']

		for i in prepped['value']:
			if idx >= i['size']:
				idx -= i['size']
				continue
			ret, ign_index = getcomb(i, idx)
			return ret, index

		raise Exception('should not be here')
	if prepped['type'] == 'combine':
		ret = []
		for i in prepped['value']:
			idx = index % i['size']
			index = index / i['size']
			tmp, tmp_index = getcomb(i, idx)
			ret.append(tmp)
		return ret, index
	if prepped['type'] == 'subset':
		ret = []
		for i in prepped['value']:
			idx = index % (i['size'] + 1)
			index = index / (i['size'] + 1)
			if idx == 0:
				# no value
				ret.append('')
			else:
				tmp, tmp_index = getcomb(i, idx - 1)
				ret.append(tmp)
		return ret, index
	raise Exception('invalid prepped value')

def flatten(v):
	if isinstance(v, (str, unicode)):
		return [ v ]
	if isinstance(v, list):
		ret = []
		for i in v:
			ret += flatten(i)
		return ret
	raise Exception('invalid value')


def getcombination(val, index):
	res, res_index = getcomb(val, index)
	if res_index != 0:
		sys.stderr.write('WARNING: index not consumed entirely, invalid index? (input index %d, output index %d)\n' % (index, res_index))

	return res

# Generate all combinations.
def getcombinations(val):
	res = []
	for i in xrange(countcombinations(val)):
		res.append(getcombination(val, i))
	return res

#
#  Test matrix
#

def create_matrix():
	gcc_language_options = Select([
		'',
		#'-std=c99',
		[ '-std=c99', '-pedantic' ]
	])
	gxx_language_options = Select([
		'',
		'-std=c++98',
		'-std=c++11',
		'-std=c++1y',
		'-std=gnu++1y'
	])

	gcc_gxx_debug_options = Select([
		'',
		[ '-g', '-ggdb' ]
	])
	gcc_gxx_warning_options = Select([
		'',
		#'-Wall',
		[ '-Wall', '-Wextra' ]  # FIXME
		# [ '-Wall', '-Wextra', '-Werror' ]
	])
	gcc_gxx_optimization_options = Select([
		#'-O0',
		#'-O1',
		#'-O2',

		# -O3 and -O4 produces spurious warnings on gcc 4.8.1, e.g. "error: assuming signed overflow does not occur when assuming that (X - c) > X is always false [-Werror=strict-overflow]"
		# Not sure what causes these, but perhaps GCC converts signed comparisons into subtractions and then runs into: https://gcc.gnu.org/wiki/FAQ#signed_overflow

		[ '-O3', '-fno-strict-overflow' ],
		#'-O3'

		#[ '-O4', '-fno-strict-overflow' ],
		#'-O4'
		'-Os'
	])

	clang_language_options = Select([
		'',
		'-std=c99'
	])
	clang_debug_options = Select([
		'',
		[ '-g', '-ggdb' ]
	])
	clang_warning_options = Select([
		'',
		[ '-Wall', '-Wextra' ]  # FIXME
		#[ '-Wall', '-Wextra', '-Werror' ]
	])
	clang_optimization_options = Select([
		#'-O0'
		#'-O1',
		#'-O2',
		'-O3',
		#'-O4',
		'-Os'
	])

	# Feature options in suitable chunks that can be subsetted arbitrarilt.

	duktape_options = Subset([
		Select([ '-DDUK_OPT_NO_REFERENCE_COUNTING',
		         '-DDUK_OPT_NO_MARK_AND_SWEEP',
		         '-DDUK_OPT_GC_TORTURE' ]),
		'-DDUK_OPT_NO_VOLUNTARY_GC',
		'-DDUK_OPT_SEGFAULT_ON_PANIC',
		'-DDUK_OPT_DPRINT_COLORS',
		'-DDUK_OPT_NO_PACKED_TVAL',
		Select([ '', '-DDUK_OPT_FORCE_ALIGN=4', '-DDUK_OPT_FORCE_ALIGN=8' ]),
		'-DDUK_OPT_DEEP_C_STACK',
		'-DDUK_OPT_NO_TRACEBACKS',
		'-DDUK_OPT_NO_PC2LINE',
		'-DDUK_OPT_NO_VERBOSE_ERRORS',
		'-DDUK_OPT_NO_MS_RESIZE_STRINGTABLE',
		'-DDUK_OPT_NO_STRICT_DECL',
		'-DDUK_OPT_NO_REGEXP_SUPPORT',
		'-DDUK_OPT_NO_OCTAL_SUPPORT',
		'-DDUK_OPT_NO_SOURCE_NONBMP',
		'-DDUK_OPT_STRICT_UTF8_SOURCE',
		#'-DDUK_OPT_NO_BROWSER_LIKE',  # FIXME: no print()
		#'-DDUK_OPT_NO_FILE_IO',       # FIXME: no print()
		'-DDUK_OPT_NO_SECTION_B',
		'-DDUK_OPT_NO_INTERRUPT_COUNTER',
		'-DDUK_OPT_NO_JX',
		'-DDUK_OPT_NO_JC',
		'-DDUK_OPT_NO_NONSTD_ACCESSOR_KEY_ARGUMENT',
		'-DDUK_OPT_NO_NONSTD_FUNC_STMT',
		'-DDUK_OPT_NONSTD_FUNC_CALLER_PROPERTY',
		'-DDUK_OPT_NONSTD_FUNC_SOURCE_PROPERTY',
		'-DDUK_OPT_NO_NONSTD_ARRAY_SPLICE_DELCOUNT',
		'-DDUK_OPT_NO_NONSTD_ARRAY_CONCAT_TRAILER',
		'-DDUK_OPT_NO_NONSTD_ARRAY_MAP_TRAILER',
		'-DDUK_OPT_NO_NONSTD_JSON_ESC_U2028_U2029',
		'-DDUK_OPT_NO_ES6_OBJECT_PROTO_PROPERTY',
		'-DDUK_OPT_NO_ES6_OBJECT_SETPROTOTYPEOF',
		'-DDUK_OPT_NO_ES6_PROXY',
		'-DDUK_OPT_NO_ZERO_BUFFER_DATA',
		'''-DDUK_OPT_USER_INITJS="Math.MEANING_OF_LIFE=42"''',
		Select([ '', '-DDUK_OPT_SETJMP', '-DDUK_OPT_UNDERSCORE_SETJMP', '-DDUK_OPT_SIGSETJMP' ]),
		'-DDUK_OPT_LIGHTFUNC_BUILTINS',
		'-DDUK_OPT_ASSERTIONS',
		[ '-DDUK_OPT_DEBUG', '-DDUK_OPT_DPRINT', '-DDUK_OPT_DDDPRINT' ],
		'-DDUK_OPT_SELF_TESTS',
		[ '-DDUK_OPT_STRTAB_CHAIN', '-DDUK_OPT_STRTAB_CHAIN_SIZE=64' ]

		# XXX: 16-bit options
	])

	# The final command is compiler specific because e.g. include path
	# and link option syntax could (in principle) differ between compilers.

	gcc_cmd_matrix = Combine([
		'gcc',
		gcc_language_options,
		gcc_gxx_debug_options,
		gcc_gxx_warning_options,
		gcc_gxx_optimization_options,
		duktape_options,
		[ '-Isrc', 'src/duktape.c', 'examples/cmdline/duk_cmdline.c', '-o', '/tmp/duk', '-lm' ]
	])

	gxx_cmd_matrix = Combine([
		'g++',
		gxx_language_options,
		gcc_gxx_debug_options,
		gcc_gxx_warning_options,
		gcc_gxx_optimization_options,
		duktape_options,
		[ '-Isrc', 'src/duktape.c', 'examples/cmdline/duk_cmdline.c', '-o', '/tmp/duk', '-lm' ]
	])

	clang_cmd_matrix = Combine([
		'clang',
		clang_language_options,
		clang_debug_options,
		clang_warning_options,
		clang_optimization_options,
		duktape_options,
		[ '-Isrc', 'src/duktape.c', 'examples/cmdline/duk_cmdline.c', '-o', '/tmp/duk', '-lm' ]
	])

	matrix = Select([ gcc_cmd_matrix, gxx_cmd_matrix, clang_cmd_matrix ])
	return matrix

#
#  Main
#

def check_unlink(filename):
	if os.path.exists(filename):
		os.unlink(filename)

def main():
	# XXX: add option for testcase(s) to run?
	parser = optparse.OptionParser()
	parser.add_option('--count', dest='count', default='1000')
	parser.add_option('--seed', dest='seed', default='default_seed_' + str(long(time.time() * 1000.0)))
	parser.add_option('--out-results-json', dest='out_results_json', default='/tmp/matrix_results.json')
	parser.add_option('--out-failed', dest='out_failed', default='/tmp/matrix_failed.txt')
        (opts, args) = parser.parse_args()

	# Avoid any optional features (like JSON or RegExps) in the test
	f = open('/tmp/test.js', 'wb')
	f.write('''print('Hello world', 1 + 2, Math.PI, JSON.stringify({ foo: 'bar' }))''')
	f.close()
	expect = 'Hello world 3 3.141592653589793 {"foo":"bar"}\n'

	print('Using seed: ' + repr(opts.seed))
	random.seed(opts.seed)
	matrix = create_matrix()
	prepped = prepcomb(matrix)
#	print(json.dumps(prepped, indent=4))
#	print(json.dumps(getcombinations(prepped), indent=4))
	numcombinations = countcombinations(prepped)

	# The number of combinations is large so do (pseudo) random
	# testing over the matrix.  Ideally we'd avoid re-testing the
	# same combination twice, but with the matrix space in billions
	# this doesn't need to be checked.

	res = []
	failed = []
	for i in xrange(long(opts.count)):
		fail = False
		idx = random.randrange(0, numcombinations)
		cmd = getcombination(prepped, idx)
		#cmd = getcombination(prepped, idx)
		compile_command = flatten(cmd)
		compile_command = [ elem for elem in compile_command if elem != '' ]  # remove empty strings

		print('%d/%d (combination %d, count %d)' % (i + 1, long(opts.count), idx, numcombinations))
		#print('%d/%d (combination %d, count %d) %s' % (i + 1, long(opts.count), idx, numcombinations, repr(compile_command)))

		check_unlink('/tmp/duk')
		compile_p = subprocess.Popen(compile_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		compile_stdout, compile_stderr = compile_p.communicate()
		compile_exitcode = compile_p.returncode

		if compile_exitcode != 0:
			fail = True
		else:
			if not os.path.exists('/tmp/duk'):
				print('*** WARNING: compile success but no /tmp/duk ***')

		check_unlink('/tmp/test.out')
		run_command = [ '/tmp/duk', '/tmp/test.js' ]
		if fail:
			run_stdout = None
			run_stderr = None
			run_exitcode = 1
		else:
			run_p = subprocess.Popen(run_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			run_stdout, run_stderr = run_p.communicate()
			run_exitcode = run_p.returncode

		if run_exitcode != 0:
			fail = True
		if run_stdout != expect:
			fail = True

		if fail:
			print('------------------------------------------------------------------------------')
			print('*** FAILED: %s' % repr(compile_command))
			print(' '.join(compile_command))
			failed.append(' '.join(compile_command))

			print('COMPILE STDOUT:')
			print(compile_stdout)
			print('COMPILE STDERR:')
			print(compile_stderr)
			print('RUN STDOUT:')
			print(run_stdout)
			print('RUN STDERR:')
			print(run_stderr)
			print('------------------------------------------------------------------------------')

		res.append({
			'compile_command': compile_command,
			'compile_stdout': compile_stdout,
			'compile_stderr': compile_stderr,
			'compile_exitcode': compile_exitcode,
			'run_command': run_command,
			'run_stdout': run_stdout,
			'run_stderr': run_stderr,
			'run_exitcode': run_exitcode,
			'run_expect': expect,
			'success': not fail
		})


	f = open(opts.out_results_json, 'wb')
	f.write(json.dumps(res, indent=4, sort_keys=True))
	f.close()

	f = open(opts.out_failed, 'wb')
	f.write('\n'.join(failed) + '\n')
	f.close()

if __name__ == '__main__':
	main()
