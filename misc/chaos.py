#!/usr/bin/env python2
#
#  $ ./duk test-dev-chaos.js | python chaos.py > chaos.raw
#  $ sox -r 8k -e unsigned -b 8 -c 1 chaos.raw chaos.wav

import os
import sys

data = sys.stdin.read()
data = data.strip()
data = data.decode('hex')
sys.stdout.write(data)
