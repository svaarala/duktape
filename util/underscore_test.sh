#!/bin/sh
#
#  Executed with 'cwd' in Duktape checkout top.
#

if [ ! -f LICENSE.txt ]; then
	echo "CWD must be Duktape checkout top."
	exit 1
fi

if [ "$1" = "valgrind" ]; then
	VALGRIND=1
	CMD=$2
	TEST=$3
else
	VALGRIND=0
	CMD=$1
	TEST=$2
fi

cat util/underscore-test-shim.js \
    underscore/test/vendor/qunit.js \
    underscore/underscore.js \
    $TEST \
    > /tmp/duk-underscore-test.js

if [ x"$VALGRIND" = "x1" ]; then
	valgrind $CMD /tmp/duk-underscore-test.js
else
	$CMD /tmp/duk-underscore-test.js
fi
