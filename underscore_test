#!/bin/sh

if [ "$1" = "valgrind" ]; then
	VALGRIND=1
	CMD=$2
	TEST=$3
else
	VALGRIND=0
	CMD=$1
	TEST=$2
fi

cat underscore-test-shim.js \
    underscore/underscore.js \
    $TEST \
    > /tmp/duk-underscore-test.js

if [ x"$VALGRIND" = "x1" ]; then
	valgrind $CMD /tmp/duk-underscore-test.js
else
	$CMD /tmp/duk-underscore-test.js
fi
