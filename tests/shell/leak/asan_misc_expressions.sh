#!/bin/sh

make build/duk-clang-asan
build/duk-clang-asan tests/ecmascript/test-misc-large-expressions.js
