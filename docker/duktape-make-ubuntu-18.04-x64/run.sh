#!/bin/bash

set -e

/work/prepare_repo.sh

source emsdk/emsdk_env.sh

cd duktape
make "$@"
zip -r /tmp/out.zip build dist tmp
cat /tmp/out.zip
