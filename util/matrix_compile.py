#!/usr/bin/python
#
#  Compile test for a lot of option combinations
#

# FIXME: proper command execution
# FIXME: rewrite as nodejs and parallelize
# FIXME: better duktape option coverage
# FIXME: 16-bit and lightfunc option matrix
# FIXME: compile coverage, add clang and g++ at least

import os
import sys
import json

debugopts = [
	'',
	'-DDUK_OPT_DEBUG',
	'-DDUK_OPT_DEBUG -DDUK_OPT_DPRINT',
	#'-DDUK_OPT_DEBUG -DDUK_OPT_DPRINT -DDUK_OPT_DDPRINT',
	'-DDUK_OPT_DEBUG -DDUK_OPT_DPRINT -DDUK_OPT_DDDPRINT'
]

assertopts = [
	'',
	'-DDUK_OPT_ASSERTIONS'
]

featureopts = [
	'',
	'-DDUK_OPT_NO_PACKED_TVAL -DDUK_OPT_SELF_TESTS -DDUK_OPT_NO_MARK_AND_SWEEP',
	'-DDUK_OPT_NO_PC2LINE'
	# XXX: more feature combinations
]

gccoptimizeopts = [
	'-O0',
	#'-O1',
	'-O2',
	#'-O3',
	'-O4',
	'-Os'
]

gccdebugopts = [
	'',
	'-g -ggdb'
]

gccstdopts = [
	'',
	#'-std=c99',
	'-std=c99 -pedantic'
]

gccwarnopts = [
	'',
	#'-Wall',
	'-Wall -Wextra -Werror'
]

# Return number of combinations for input lists.
def countsize(lists):
	ret = 1
	for lst in lists:
		ret = ret * len(lst)
	return ret

# Return a combination for index, for index in [0,countsize(lists)[.
# This allows random selection of combinations using a PRNG.
def gencommand(lists, index):
	comb = []
	for lst in lists:
		comb.append(str(lst[index % len(lst)]))
		index = index / len(lst)
	return ' '.join(comb)

# Generate all combinations.
def gencommands(lists):
	res = []
	for i in xrange(countsize(lists)):
		res.append(gencommand(lists, i))
	return res

def check_unlink(filename):
	if os.path.exists(filename):
		os.unlink(filename)

def main():
	opts = gencommands([gccstdopts, gccdebugopts, gccwarnopts, gccoptimizeopts,
	                    debugopts, assertopts, featureopts])

	f = open('/tmp/test.js', 'wb')
	f.write('''\
print('Hello world', 1 + 2, Math.PI, Duktape.enc('jx', { foo: 'bar' }))
''')
	f.close()
	expect = 'Hello world 3 3.141592653589793 {foo:"bar"}\n'

	res = []
	failed = []
	for idx,i in enumerate(opts):
		fail = False

		check_unlink('/tmp/duk')
		compile_command = 'gcc ' + i + ' -Isrc src/duktape.c examples/cmdline/duk_cmdline.c -o /tmp/duk -lm'

		print('%d/%d %s' % (idx + 1, len(opts), compile_command))
		compile_exitcode = os.system(compile_command)
		if compile_exitcode != 0:
			fail = True

		check_unlink('/tmp/test.out')
		run_exitcode = os.system('/tmp/duk /tmp/test.js > /tmp/test.out 2> /tmp/test.err')
		f = open('/tmp/test.out', 'rb')
		run_output = f.read()
		f.close()
		if run_exitcode != 0:
			fail = True
		if run_output != expect:
			fail = True

		res.append({ 'compile_command': compile_command,
		             'compile_exitcode': compile_exitcode,
		             'run_exitcode': run_exitcode,
		             'run_output': run_output,
		             'success': not fail })

		if fail:
			print('*** FAILED: %s' % compile_command)
			failed.append(compile_command)

	f = open('/tmp/matrix_results.json', 'wb')
	f.write(json.dumps(res, indent=4, sort_keys=True))
	f.close()

	f = open('/tmp/matrix_failed.txt', 'wb')
	f.write('\n'.join(failed) + '\n')
	f.close()

if __name__ == '__main__':
	main()
