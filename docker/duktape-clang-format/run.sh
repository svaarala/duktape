#!/bin/bash

set -e

cat >/tmp/stdin.zip
mkdir reformat && cd reformat
unzip -q /tmp/stdin.zip

clang-format-12 -i `find src-input -type f | egrep '(.c|.h|.c.in|.c.in)'`

zip -1 -q -r /tmp/stdout.zip *
cat /tmp/stdout.zip
