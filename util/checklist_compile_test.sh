#!/bin/bash
#
#  Helper script for release checklist compile test.
#

set -x

cat >opts1.yaml <<EOF
DUK_USE_SELF_TESTS: true
EOF

cat >opts2.yaml <<EOF
DUK_USE_SELF_TESTS: true
DUK_USE_ASSERTIONS: true
DUK_USE_DEBUG: true
DUK_USE_DEBUG_LEVEL: 0
DUK_USE_DEBUG_WRITE:
  verbatim: "#define DUK_USE_DEBUG_WRITE(level,file,line,func,msg) do {fprintf(stdout, \\"D%ld %s:%ld (%s): %s\\\\n\\", (long) (level), (file), (long) (line), (func), (msg));} while (0)"
EOF

TESTCODE="for (var arr=[]; arr.length < 1e6; arr.push(1)); Duktape.compact(arr); var estimate = Duktape.info(arr).pbytes / arr.length; print('duk_tval size approximation:', estimate);"

for compiler in gcc clang; do
	for archopt in -m64 -m32; do
		echo "*** $compiler $archopt"

		echo "- single source, normal options"
		rm -f duk; rm -rf ./prep
		python2 tools/configure.py --output-directory ./prep --source-directory ./src-input --config-metadata config --option-file opts1.yaml
		$compiler -o duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -DDUK_CMDLINE_PRINTALERT_SUPPORT -I./prep -Iextras/print-alert prep/duktape.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
		./duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- single source, debug options"
		rm -f duk; rm -rf ./prep
		python2 tools/configure.py --output-directory ./prep --source-directory ./src-input --config-metadata config --option-file opts2.yaml
		$compiler -o duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -DDUK_CMDLINE_PRINTALERT_SUPPORT -I./prep -Iextras/print-alert prep/duktape.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
		./duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- separate sources, normal options"
		rm -f duk; rm -rf ./prep
		python2 tools/configure.py --output-directory ./prep --source-directory ./src-input --config-metadata config --separate-sources --option-file opts1.yaml
		$compiler -o duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -DDUK_CMDLINE_PRINTALERT_SUPPORT -I./prep -Iextras/print-alert prep/*.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
		./duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- separate sources, debug options"
		rm -f duk; rm -rf ./prep
		python2 tools/configure.py --output-directory ./prep --source-directory ./src-input --config-metadata config --separate-sources --option-file opts2.yaml
		$compiler -o duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -DDUK_CMDLINE_PRINTALERT_SUPPORT -I./prep -Iextras/print-alert prep/*.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
		./duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js
	done
done
