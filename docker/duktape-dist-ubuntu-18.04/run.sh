#!/bin/bash

set -e

/build/prepare_repo.sh

source emsdk-portable/emsdk_env.sh

cd duktape
make clean dist-src
ls -l duktape-*.tar.*
zip /tmp/out.zip duktape-*.tar.*
cat /tmp/out.zip
