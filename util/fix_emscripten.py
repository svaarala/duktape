#!/usr/bin/python
#
#  Fix a few errors from Emscripten output (stopgap until emscripten mainline
#  is updated).

import os
import sys

fix_count = 0

for line in sys.stdin:
	if len(line) > 1 and line[-1] == '\n':
		line = line[:-1]

	if line == r"""  if (/<?{ ?[^}]* ?}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types""":
		# RegExp fix, now fixed in the Emscripten repository and should no longer
		# be necessary.
		# https://github.com/kripken/emscripten/commit/277ac5239057721ebe3c6e7813dc478eeab2cea0
		print(r"""  if (/<?\{ ?[^}]* ?\}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types""")
		fix_count += 1
	elif line == r"""  var sourceRegex = /^function\s\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/;""":
		# GH-11: Another RegExp escaping fix.
		print(r"""  var sourceRegex = /^function\s\(([^)]*)\)\s*\{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?\}$/;""")
		fix_count += 1
	elif line == r"""    var parsed = jsfunc.toString().match(sourceRegex).slice(1);""":
		# GH-11: Attempt to parse a function's toString() output with a RegExp.
		# The RegExp makes invalid assumptions and won't parse Duktape's function
		# toString output ("function empty() {/* source code*/)}").
		# This stopgap will prevent a 'TypeError: invalid base reference for property read'
		# and allows at least a hello world to run.
		print(r"""    var parsed = (jsfunc.toString().match(sourceRegex) || []).slice(1);""")

	else:
		print(line)

if fix_count > 0:
	sys.stderr.write('Emscripten fixes needed (fix_emscripten.py): fix_count=%d' % fix_count)
	sys.stderr.flush()
