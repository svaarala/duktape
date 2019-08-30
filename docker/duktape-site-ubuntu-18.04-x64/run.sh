#!/bin/bash

set -e

/work/prepare_repo.sh

source emsdk/emsdk_env.sh

cd duktape
make clean dist-site
ls -l duktape-site-*.tar.*
zip /tmp/out.zip duktape-site-*.tar.*
cat /tmp/out.zip
