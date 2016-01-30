#!/usr/bin/python
#
#  Scan Duktape code base for references to built-in strings, i.e. for
#  strings which will need DUK_STRIDX_xxx constants and a place in the
#  thr->strs[] array.
#

import os
import sys
import re
import json

re_stridx = re.compile(r'DUK_STRIDX_(\w+)', re.MULTILINE)
re_heap = re.compile(r'DUK_HEAP_STRING_(\w+)', re.MULTILINE)
re_hthread = re.compile(r'DUK_HTHREAD_STRING_(\w+)', re.MULTILINE)

def main():
	defs = {}

	for fn in sys.argv[1:]:
		with open(fn, 'rb') as f:
			d = f.read()
			for m in re.finditer(re_stridx, d):
				defs[m.group(1)] = True
			for m in re.finditer(re_heap, d):
				defs[m.group(1)] = True
			for m in re.finditer(re_hthread, d):
				defs[m.group(1)] = True

	used = []
	doc = { 'used_stridx_defines': used }
	for k in sorted(defs.keys()):
		used.append('DUK_STRIDX_' + k)
	doc['count_used_stridx_defines'] = len(used)

	print(json.dumps(doc, indent=4))

if __name__ == '__main__':
	main()
