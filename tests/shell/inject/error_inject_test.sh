#!/bin/bash
#
#  Some error injection tests
#

set -x

for i in 1 2 3 4; do
	echo "Heap alloc inject: $i"
	mkdir -p tmp/error-inject && rm -rf tmp/error-inject/duk tmp/error-inject/prep
	python3 tools/configure.py --output-directory tmp/error-inject/prep --source-directory ./src-input --config-metadata config --line-directives -DDUK_USE_INJECT_HEAP_ALLOC_ERROR=$i -DDUK_USE_ASSERTIONS
	gcc -o tmp/error-inject/duk -std=c99 -Wall -g -ggdb -DDUK_CMDLINE_PRINTALERT_SUPPORT -Itmp/error-inject/prep -Iextras/print-alert tmp/error-inject/prep/duktape.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c -lm
	valgrind --leak-check=full --error-exitcode=123 tmp/error-inject/duk
	EXITCODE=$?
	echo "Exit code is $EXITCODE"
	if [ $EXITCODE -eq 1 ]; then echo "OK: 'duk' returned error, but no fatal error";
	elif [ $EXITCODE -eq 134 ]; then echo "FAILED: ABORTED, fatal error, should not happen"; exit 1;
	elif [ $EXITCODE -eq 123 ]; then echo "FAILED: valgrind reported error, probably a leak, should not happen"; exit 1;
	else echo "Unexpected exit code $EXITCODE, should not happen"; exit 1;
	fi
done

# XXX: object resize
