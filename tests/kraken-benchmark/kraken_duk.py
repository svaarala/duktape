#!/usr/bin/env python2

import os
import sys
import subprocess

DUK_COMMAND = './duk'
LOG_FILE = '/tmp/kraken_duk.log'
HARNESS_FILE = './kraken_harness.js'

def parse_opts():
    res = []
    opt = None
    for i in sys.argv[1:]:
        if opt is not None:
            if i[0] == '-':
                raise Exception('invalid argument: ' + repr(i))
            res.append(i)
            opt = None
        else:
            if i == '-f':
                opt = i
            else:
                raise Exception('invalid option: ' + repr(i))
    return res

def main():
    logfile = open(LOG_FILE, 'ab')

    logfile.write('\n*** Running: ' + repr(sys.argv) + '\n\n')
    logfile.flush()

    files = parse_opts()
    cmd = [ DUK_COMMAND, HARNESS_FILE ] + files

    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    ret = proc.communicate(input='')
    logfile.write(ret[0])
    logfile.write(ret[1])
    logfile.flush()
    sys.stdout.write(ret[0])
    sys.stderr.write(ret[1])
    sys.stdout.flush()
    sys.stderr.flush()
    sys.exit(proc.returncode)

if __name__ == '__main__':
    main()
