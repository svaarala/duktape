#!/usr/bin/python
#
#  Fix a few errors from Emscripten output (stopgap until emscripten mainline
#  is updated).

import os
import sys

fix_count = 0

replacements = {
	# RegExp fix, now fixed in the Emscripten repository and should no longer
	# be necessary.
	# https://github.com/kripken/emscripten/commit/277ac5239057721ebe3c6e7813dc478eeab2cea0
	r"""if (/<?{ ?[^}]* ?}>?/.test(type)) return true""":
		r"""if (/<?\{ ?[^}]* ?\}>?/.test(type)) return true""",

	# GH-11: Another RegExp escaping fix.
	r"""var sourceRegex = /^function\s\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/;""":
		r"""var sourceRegex = /^function\s\(([^)]*)\)\s*\{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?\}$/;""",
	r"""var sourceRegex = /^function\s*\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/;""":
		r"""var sourceRegex = /^function\s*\(([^)]*)\)\s*\{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?\}$/;""",

	# GH-11: Attempt to parse a function's toString() output with a RegExp.
	# The RegExp makes invalid assumptions and won't parse Duktape's function
	# toString output ("function empty() {/* source code*/)}").
	# This stopgap will prevent a 'TypeError: invalid base reference for property read'
	# and allows at least a hello world to run.
	r"""var parsed = jsfunc.toString().match(sourceRegex).slice(1);""":
		r"""var parsed = (jsfunc.toString().match(sourceRegex) || []).slice(1);""",
	r"""jsfunc.toString().match(sourceRegex).slice(1);""":
		r"""(jsfunc.toString().match(sourceRegex) || []).slice(1);""",

	# Newer emscripten has this at least with -O2
	r"""/^function\s*\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/""":
		r"""/^function\s*\(([^)]*)\)\s*\{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?\}$/""",
}

repl_keys = replacements.keys()
repl_keys.sort()

for line in sys.stdin:
	if len(line) > 1 and line[-1] == '\n':
		line = line[:-1]

		for k in repl_keys:
			line_fix = line.replace(k, replacements[k])
			if line_fix != line:
				fix_count += 1
			line = line_fix

		print(line)

if fix_count > 0:
	sys.stderr.write('Emscripten fixes needed (fix_emscripten.py): fix_count=%d\n' % fix_count)
	sys.stderr.flush()
