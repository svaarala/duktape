#!/usr/bin/env python2
#
#  Automatically fix one-line broken debug log calls.  Adds a missing
#  wrapper for such lines, e.g. changes:
#
#    DUK_DPRINT(...);
#
#  into:
#
#    DUK_D(DUK_DPRINT(...));
#
#  Does not handle multiline log calls.
#
#  Usage:
#
#    $ python autofix_debuglog_calls.py src-input/*.c
#
#  WARNING: works in place, so commit any changes before running, then
#  check diff.
#

import os
import sys
import re

re_callsite = re.compile(r'^\s*(DUK_D+PRINT).*?;$')

wrappers = {
    'DUK_DPRINT': 'DUK_D',
    'DUK_DDPRINT': 'DUK_DD',
    'DUK_DDDPRINT': 'DUK_DDD'
}

warnings = []

def process(filename):
    f = open(filename, 'rb')
    output = []

    linenumber = 0
    fixes = 0
    for line in f:
        linenumber += 1
        if 'DPRINT' not in line:
            output.append(line)
            continue
        m = re_callsite.match(line)
        if m is None:
            output.append(line)
            continue
        log_macro = m.group(1)
        log_wrapper = wrappers[log_macro]
        line = line.replace(log_macro, log_wrapper + '(' + log_macro)  # DUK_DPRINT( -> DUK_D(DUK_DPRINT(
        line = line.replace(');', '));')                               # ...); -> ...));
        output.append(line)
        fixes += 1

    f.close()

    if fixes > 0:
        print '%s: %d fixes' % (filename, fixes)

        f = open(filename, 'wb')
        f.write(''.join(output))
        f.close()

def main():
    for filename in sys.argv[1:]:
        process(filename)

if __name__ == '__main__':
    main()
