#!/usr/bin/python
#
#  Prepare an Ecmascript or API testcase file for execution.
#
#  For Ecmascript testcases:
#
#    - Lift a 'use global' statement to top of file if present
#    - Add a prologue which handles engine differences
#    - Resolve include files
#
#  The prologue and includes are minified to a one-liner so that the line
#  numbers in the original test file are kept.
#
#  For C testcases:
#
#    - Add a prologue with testing utilities, includes, etc.
#

import os
import sys
import re
import tempfile
import subprocess
import optparse

re_include = re.compile(r'^/\*@include\s(.*?)\s*@\*/$')

def readFile(fn):
	f = open(fn, 'rb')
	data = f.read()
	f.close()
	return data

def writeFile(fn, data):
	f = open(fn, 'wb')
	f.write(data)
	f.close()

def stripTrailingNewlines(data):
	while data.endswith('\n'):
		data = data[:-1]
	return data

class TestcasePreparer:
	def __init__(self,
	             util_include_path=None,
	             minify_provider=None,
	             closure_jar_path=None,
	             uglifyjs_exe_path=None,
	             uglifyjs2_exe_path=None):
		self.util_include_path = util_include_path
		self.minify_provider = minify_provider
		self.closure_jar_path = closure_jar_path
		self.uglifyjs_exe_path = uglifyjs_exe_path
		self.uglifyjs2_exe_path = uglifyjs2_exe_path

	def prepApiTest(self, fn, data):
		# FIXME: implement API testcase prepping
		return data

	def minifyClosure(self, fn):
		fh, absFn = tempfile.mkstemp(suffix='prep_temp')
		os.close(fh)

		rc = subprocess.call(['java', '-jar', self.closure_jar_path, '--js_output_file', absFn, fn ])
		if rc != 0:
			raise Exception('closure minify failed')

		res = readFile(absFn)
		os.unlink(absFn)
		return res

	def minifyUglifyJS(self, fn):
		fh, absFn = tempfile.mkstemp(suffix='prep_temp')
		os.close(fh)

		rc = subprocess.call([self.uglifyjs_exe_path, '-o', absFn, fn])
		if rc != 0:
			raise Exception('uglifyjs minify failed')

		res = readFile(absFn)
		os.unlink(absFn)
		return res

	def minifyUglifyJS2(self, fn):
		fh, absFn = tempfile.mkstemp(suffix='prep_temp')
		os.close(fh)

		rc = subprocess.call([self.uglifyjs2_exe_path, '-o', absFn, fn])
		if rc != 0:
			raise Exception('uglifyjs2 minify failed')

		res = readFile(absFn)
		os.unlink(absFn)
		return res

	def minifyOneLine(self, fn):
		# Closure is very slow to start so it's not ideal for test case use.
		# The only thing we really need is to make Ecmascript a one-liner.

		if self.minify_provider == 'closure':
			return self.minifyClosure(fn)
		elif self.minify_provider == 'uglifyjs':
			return self.minifyUglifyJS(fn)
		elif self.minify_provider == 'uglifyjs2':
			return self.minifyUglifyJS2(fn)
		else:
			raise Exception('no minifier')

	def prepEcmaPrologue(self, fn):
		return stripTrailingNewlines(self.minifyOneLine(fn))

	def prepEcmaInclude(self, fn):
		absFn = os.path.join(self.util_include_path, fn)
		return '/* INCLUDE: ' + fn + ' */ ' + stripTrailingNewlines(self.minifyOneLine(absFn))

	def prepEcmaTest(self, fn_in, fn_prologue, data):
		is_strict = False

		lines = []
		for line in data.split('\n'):
			if line.startswith('/'):
				m = re_include.match(line)
				if m is not None:
					lines.append(self.prepEcmaInclude(m.group(1)))
					continue
			elif line.startswith('"use strict"') or line.startswith("'use strict'"):
				# This is very approximate, but correct for current tests.
				is_strict = True

			lines.append(line)

		if fn_prologue is not None:
			# Prepend prologue to first line; if the program is strict
			# duplicate the 'use strict' declaration.
			lines[0] = self.prepEcmaPrologue(fn_prologue) + ' /*...*/ ' + lines[0]
			if is_strict:
				lines[0] = "'use strict'; " + lines[0]

		return '\n'.join(lines)

	def prepareTestcase(self, fn_in, fn_out, fn_prologue):
		data = readFile(fn_in)

		if fn_in.endswith('.c'):
			res = self.prepApiTest(fn_in, fn_prologue, data)
		elif fn_in.endswith('.js'):
			res = self.prepEcmaTest(fn_in, fn_prologue, data)
		else:
			raise Exception('invalid file (not .c or .js)')

		writeFile(fn_out, res)

def main():
	parser = optparse.OptionParser()
	parser.add_option('--input', dest='input', default=None)
	parser.add_option('--output', dest='output', default=None)
	parser.add_option('--prologue', dest='prologue', default=None)
	parser.add_option('--util-include-path', dest='util_include_path', default=None)
	parser.add_option('--minify-closure', dest='minify_closure', default=None)   # point to compiler.jar
	parser.add_option('--minify-uglifyjs', dest='minify_uglifyjs', default=None)  # point to uglifyjs exe
	parser.add_option('--minify-uglifyjs2', dest='minify_uglifyjs2', default=None)  # point to uglifyjs exe
	(opts, args) = parser.parse_args()

	if opts.input is None or opts.output is None:
		raise Exception('filename argument(s) missing (--input and/or --output)')
	if opts.util_include_path is None:
		raise Exception('missing util include path (--util-include-path)')

	fn_in = opts.input
	fn_out = opts.output
	fn_prologue = opts.prologue

	minify_provider = None
	if opts.minify_closure is not None:
		minify_provider = 'closure'
	elif opts.minify_uglifyjs is not None:
		minify_provider = 'uglifyjs'
	elif opts.minify_uglifyjs2 is not None:
		minify_provider = 'uglifyjs2'
	else:
		raise Exception('must provide a minifier (include files must be converted to one-liners)')

	preparer = TestcasePreparer(util_include_path=opts.util_include_path,
	                            minify_provider=minify_provider,
	                            closure_jar_path=opts.minify_closure,
	                            uglifyjs_exe_path=opts.minify_uglifyjs,
	                            uglifyjs2_exe_path=opts.minify_uglifyjs2)

	preparer.prepareTestcase(fn_in, fn_out, fn_prologue)

if __name__ == '__main__':
	main()
