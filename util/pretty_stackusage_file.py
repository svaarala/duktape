#!/usr/bin/env python2

import os
import sys
import re

def main():
    re_line = re.compile(r'^(.*?)\s+(.*?)\s+(.*?)$')
    with open(sys.argv[1], 'rb') as f:
        lines = []
        for line in f:
            m = re_line.match(line)
            assert(m is not None)
            t = [ int(m.group(2)), m.group(1), m.group(3) ]
            lines.append(t)

        lines.sort()
        for line in lines:
            print('%5d  %s %s' % (line[0], line[1], line[2]))

if __name__ == '__main__':
    main()
