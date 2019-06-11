#!/bin/bash

set -e

/build/prepare_repo.sh

source emsdk-portable/emsdk_env.sh

cd duktape
make clean dist-site
ls -l duktape-site-*.tar.*
zip /tmp/out.zip duktape-site-*.tar.*
cat /tmp/out.zip
