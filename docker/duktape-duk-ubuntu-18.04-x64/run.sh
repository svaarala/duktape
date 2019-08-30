#!/bin/bash

set -e

/work/prepare_repo.sh

source emsdk/emsdk_env.sh

cd duktape
make clean duk duk.O2 dukd
zip /tmp/out.zip duk duk.O2 dukd
cat /tmp/out.zip
