#!/bin/sh

make build/duk-clang-asan
ASAN_OPTIONS=fast_unwind_on_malloc=0 build/duk-clang-asan tests/ecmascript/test-misc-large-expressions.js
