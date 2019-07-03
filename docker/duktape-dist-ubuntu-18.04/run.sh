#!/bin/bash

set -e

/work/prepare_repo.sh

source emsdk/emsdk_env.sh

cd duktape
make clean dist-src
ls -l duktape-*.tar.*
zip /tmp/out.zip duktape-*.tar.*
cat /tmp/out.zip
