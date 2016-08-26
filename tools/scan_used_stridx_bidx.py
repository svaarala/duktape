#!/usr/bin/env python2
#
#  Scan Duktape code base for references to built-in strings and built-in
#  objects, i.e. for:
#
#  - Strings which will need DUK_STRIDX_xxx constants and a place in the
#    thr->strs[] array.
#
#  - Objects which will need DUK_BIDX_xxx constants and a place in the
#    thr->builtins[] array.
#

import os
import sys
import re
import json

re_str_stridx = re.compile(r'DUK_STRIDX_(\w+)', re.MULTILINE)
re_str_heap = re.compile(r'DUK_HEAP_STRING_(\w+)', re.MULTILINE)
re_str_hthread = re.compile(r'DUK_HTHREAD_STRING_(\w+)', re.MULTILINE)
re_obj_bidx = re.compile(r'DUK_BIDX_(\w+)', re.MULTILINE)

def main():
    str_defs = {}
    obj_defs = {}

    for fn in sys.argv[1:]:
        with open(fn, 'rb') as f:
            d = f.read()
            for m in re.finditer(re_str_stridx, d):
                str_defs[m.group(1)] = True
            for m in re.finditer(re_str_heap, d):
                str_defs[m.group(1)] = True
            for m in re.finditer(re_str_hthread, d):
                str_defs[m.group(1)] = True
            for m in re.finditer(re_obj_bidx, d):
                obj_defs[m.group(1)] = True

    str_used = []
    for k in sorted(str_defs.keys()):
        str_used.append('DUK_STRIDX_' + k)

    obj_used = []
    for k in sorted(obj_defs.keys()):
        obj_used.append('DUK_BIDX_' + k)

    doc = {
        'used_stridx_defines': str_used,
        'used_bidx_defines': obj_used,
        'count_used_stridx_defines': len(str_used),
        'count_used_bidx_defines': len(obj_used)
    }
    print(json.dumps(doc, indent=4))

if __name__ == '__main__':
    main()
