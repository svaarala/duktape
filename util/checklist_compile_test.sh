#!/bin/bash
#
#  Helper script for release checklist compile test.
#

set -x

OPTS1="-DDUK_OPT_SELF_TESTS"
OPTS2="-DDUK_OPT_SELF_TESTS -DDUK_OPT_ASSERTIONS -DDUK_OPT_DEBUG -DDUK_OPT_DPRINT"

TESTCODE="for (var arr=[]; arr.length < 1e6; arr.push(1)); Duktape.compact(arr); var estimate = Duktape.info(arr)[4] / arr.length; print('duk_tval size approximation:', estimate);"

for compiler in gcc clang; do
	for archopt in -m64 -m32; do
		echo "*** $compiler $archopt"

		echo "- single source, normal options"
		rm -f duk
		$compiler -o duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -I./src ${OPTS1} src/duktape.c examples/cmdline/duk_cmdline.c -lm
		./duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- single source, debug options"
		rm -f duk
		$compiler -o duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -I./src ${OPTS2} src/duktape.c examples/cmdline/duk_cmdline.c -lm
		./duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- separate sources, normal options"
		rm -f duk
		$compiler -o duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -I./src-separate $OPTS1 src-separate/*.c examples/cmdline/duk_cmdline.c -lm
		./duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- separate sources, debug options"
		rm -f duk
		$compiler -o duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -I./src-separate $OPTS2 src-separate/*.c examples/cmdline/duk_cmdline.c -lm
		./duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js
	done
done
