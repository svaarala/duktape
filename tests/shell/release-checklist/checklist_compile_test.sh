#!/bin/bash
#
#  Helper script for release checklist compile test.
#

set -x

mkdir -p tmp/checklist-compile

cat >tmp/checklist-compile/opts1.yaml <<EOF
DUK_USE_SELF_TESTS: true
EOF

cat >tmp/checklist-compile/opts2.yaml <<EOF
DUK_USE_SELF_TESTS: true
DUK_USE_ASSERTIONS: true
DUK_USE_DEBUG: true
DUK_USE_DEBUG_LEVEL: 0
DUK_USE_DEBUG_WRITE:
  verbatim: "#define DUK_USE_DEBUG_WRITE(level,file,line,func,msg) do {fprintf(stdout, \\"D%ld %s:%ld (%s): %s\\\\n\\", (long) (level), (file), (long) (line), (func), (msg));} while (0)"
EOF

TESTCODE="for (var arr=[]; arr.length < 1e6; arr.push(1)); Duktape.compact(arr); var estimate = Duktape.info(arr).pbytes / arr.length; print('duk_tval size approximation:', estimate);"

cleanup() {
	rm -rf tmp/checklist-compile/duk tmp/checklist-compile/prep
}

for compiler in gcc clang; do
	for archopt in -m64 -m32; do
		echo "*** $compiler $archopt"

		echo "- single source, normal options"
		cleanup
		python3 tools/configure.py --output-directory tmp/checklist-compile/prep --source-directory ./src-input --config-metadata config --option-file tmp/checklist-compile/opts1.yaml
		$compiler -o tmp/checklist-compile/duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -DDUK_CMDLINE_PRINTALERT_SUPPORT -Itmp/checklist-compile/prep -Iextras/print-alert tmp/checklist-compile/prep/duktape.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
		tmp/checklist-compile/duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- single source, debug options"
		cleanup
		python3 tools/configure.py --output-directory tmp/checklist-compile/prep --source-directory ./src-input --config-metadata config --option-file tmp/checklist-compile/opts2.yaml
		$compiler -o tmp/checklist-compile/duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -DDUK_CMDLINE_PRINTALERT_SUPPORT -Itmp/checklist-compile/prep -Iextras/print-alert tmp/checklist-compile/prep/duktape.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
		tmp/checklist-compile/duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- separate sources, normal options"
		cleanup
		python3 tools/configure.py --output-directory tmp/checklist-compile/prep --source-directory ./src-input --config-metadata config --separate-sources --option-file tmp/checklist-compile/opts1.yaml
		$compiler -o tmp/checklist-compile/duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -DDUK_CMDLINE_PRINTALERT_SUPPORT -Itmp/checklist-compile/prep -Iextras/print-alert tmp/checklist-compile/prep/*.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
		tmp/checklist-compile/duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js

		echo "- separate sources, debug options"
		cleanup
		python3 tools/configure.py --output-directory tmp/checklist-compile/prep --source-directory ./src-input --config-metadata config --separate-sources --option-file tmp/checklist-compile/opts2.yaml
		$compiler -o tmp/checklist-compile/duk $archopt -Os -pedantic -std=c99 -Wall -fstrict-aliasing -fomit-frame-pointer -DDUK_CMDLINE_PRINTALERT_SUPPORT -Itmp/checklist-compile/prep -Iextras/print-alert tmp/checklist-compile/prep/*.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
		tmp/checklist-compile/duk -e 'print(Duktape.env)' -e "$TESTCODE" mandel.js
	done
done
