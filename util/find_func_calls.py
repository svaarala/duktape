#!/usr/bin/python
#
#  Extract function call names from source C/H files.
#
#  Useful for e.g. hunting non-Duktape library calls.  Function calls can
#  also be determined from object files, but some function calls are
#  compilation option specific, so it's better to find them from source
#  files.
#

import os
import sys
import re

re_comment = re.compile(r'/\*.*?\*/', re.DOTALL)
re_func_call = re.compile(r'([A-Za-z_][A-Za-z0-9_]+)\(', re.MULTILINE)

def stripComments(d):
	res = re.sub(re_comment, '/*omit*/', d)
	#print(res)
	return res

def findFuncCalls(d):
	res = []
	for m in re_func_call.finditer(d):
		res.append(m.group(1))
	return res

def main():
	# Duktape code does not have a space between a function name and
	# an open parenthesis.  If the regexp includes an optional space,
	# it will provide a lot of false matches.

	for fn in sys.argv[1:]:
		f = open(fn, 'rb')
		d = f.read()
		f.close()

		# Strip block comments.

		d = stripComments(d)

		# Find function calls (close enough).

		for i in findFuncCalls(d):
			print(i)

if __name__ == '__main__':
	main()
