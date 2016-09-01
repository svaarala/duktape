#!/usr/bin/env python2
#
#  Extract function call names from source C/H files.
#
#  Useful for e.g. hunting non-Duktape library calls.  Function calls can
#  also be determined from object files, but some function calls are
#  compilation option specific, so it's better to find them from source
#  files.
#
#  Example run:
#
#    $ python util/find_func_calls.py src-input/*.c src-input/*.h | \
#      grep -v -i -P ^duk_ | grep -v -P '^(sizeof|va_start|va_end|va_arg)' | \
#      sort | uniq | less
#

import os
import sys
import re

re_linecont = re.compile(r'\\\n')
re_comment = re.compile(r'/\*.*?\*/', re.DOTALL)
re_func_call = re.compile(r'([A-Za-z_][A-Za-z0-9_]+)\(')
re_string = re.compile(r'"(\\"|[^"])*"')

def stripLineContinuations(x):
    res = re.sub(re_linecont, ' ', x)
    #print(res)
    return res

def stripComments(x):
    res = re.sub(re_comment, '/*omit*/', x)
    #print(res)
    return res

def stripStrings(x):
    res = re.sub(re_string, '"..."', x)
    #print(res)
    return res

def findFuncCalls(d, fn):
    res = []
    for line in d.split('\n'):
        if len(line) >= 1 and line[0] == '#':
            # Preprocessor lines contain function call like
            # syntax but are not function calls.
            continue

        for m in re_func_call.finditer(line):
            res.append({
                'name': m.group(1),
                'filename': fn
            })
    return res

def main():
    # Duktape code does not have a space between a function name and
    # an open parenthesis.  If the regexp includes an optional space,
    # it will provide a lot of false matches.

    for fn in sys.argv[1:]:
        f = open(fn, 'rb')
        d = f.read()
        f.close()

        # Strip line continuations, comments, and strings so that
        # we minimize false matches.

        d = stripLineContinuations(d)
        d = stripComments(d)
        d = stripStrings(d)

        # Find function calls (close enough).

        for i in findFuncCalls(d, fn):
            #print '%s' % i['name']
            print '%-25s%s' % (i['name'], i['filename'])

if __name__ == '__main__':
    main()
