#!/usr/bin/python
#
#  Fix the single regexp syntax error from Emscripten output.
#
#  This has been fixed in the Emscripten repository and should no longer
#  be necessary.
#
#  (https://github.com/kripken/emscripten/commit/277ac5239057721ebe3c6e7813dc478eeab2cea0)
#

import os
import sys

for line in sys.stdin:
	if len(line) > 1 and line[-1] == '\n':
		line = line[:-1]

	if line == r"""  if (/<?{ ?[^}]* ?}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types""":
		print(r"""  if (/<?\{ ?[^}]* ?\}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types""")
		
	else:
		print(line)

