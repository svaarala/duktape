#!/usr/bin/python
#
#  Fix the single regexp syntax error from Emscripten output.

import os
import sys

for line in sys.stdin:
	if len(line) > 1 and line[-1] == '\n':
		line = line[:-1]

	if line == r"""  if (/<?{ ?[^}]* ?}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types""":
		print(r"""  if (/<?\{ ?[^}]* ?\}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types""")
		
	else:
		print(line)

