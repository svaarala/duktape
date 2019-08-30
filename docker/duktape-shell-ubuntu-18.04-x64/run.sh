#!/bin/bash

set -e

/work/prepare_repo.sh

source emsdk/emsdk_env.sh

cd duktape
/bin/bash --login
