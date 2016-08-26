#!/usr/bin/env python2
#
#  Size report of (stripped) object and source files.
#

import os
import sys

def getsize(fname):
    return os.stat(fname).st_size

def getlines(fname):
    f = None
    try:
        f = open(fname, 'rb')
        lines = f.read().split('\n')
        return len(lines)
    finally:
        if f is not None:
            f.close()
            f = None

def process(srcfile, objfile):
    srcsize = getsize(srcfile)
    srclines = getlines(srcfile)
    srcbpl = float(srcsize) / float(srclines)
    objsize = getsize(objfile)
    objbpl = float(objsize) / float(srclines)

    return objsize, objbpl, srcsize, srclines, srcbpl

def main():
    tot_srcsize = 0
    tot_srclines = 0
    tot_objsize = 0

    tmp = []
    for i in sys.argv[1:]:
        objfile = i
        if i.endswith('.strip'):
            objname = i[:-6]
        else:
            objname = i
        base, ext = os.path.splitext(objname)
        srcfile = base + '.c'

        objsize, objbpl, srcsize, srclines, srcbpl = process(srcfile, objfile)
        srcbase = os.path.basename(srcfile)
        objbase = os.path.basename(objname)  # foo.o.strip -> present as foo.o
        tot_srcsize += srcsize
        tot_srclines += srclines
        tot_objsize += objsize
        tmp.append((srcbase, srcsize, srclines, srcbpl, objbase, objsize, objbpl))

    def mycmp(a,b):
        return cmp(a[5], b[5])

    tmp.sort(cmp=mycmp, reverse=True)    # sort by object size
    fmt = '%-20s size=%-7d lines=%-6d bpl=%-6.3f  -->  %-20s size=%-7d bpl=%-6.3f'
    for srcfile, srcsize, srclines, srcbpl, objfile, objsize, objbpl in tmp:
        print(fmt % (srcfile, srcsize, srclines, srcbpl, objfile, objsize, objbpl))

    print('========================================================================')
    print(fmt % ('TOTAL', tot_srcsize, tot_srclines, float(tot_srcsize) / float(tot_srclines),
                 '', tot_objsize, float(tot_objsize) / float(tot_srclines)))

if __name__ == '__main__':
    # Usage:
    #
    #   $ strip *.o
    #   $ python genobjsizereport.py *.o

    main()
