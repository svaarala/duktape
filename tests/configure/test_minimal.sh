#!/bin/sh
#
#  Minimal test coverage for configure.py.
#

set -e

reinit() {
	if [ ! -d /tmp ]; then
		echo "This script expects /tmp to exist"
		exit 1
	fi

	rm -rf /tmp/duk-configure-test
	mkdir /tmp/duk-configure-test
}

# Test for command line -D with macro arguments, GH-2013 / GH-2014.
reinit
python tools/configure.py \
	'-DDUK_USE_DEBUG_WRITE(level,file,line,func,msg)=do {fprintf(stderr, "D%ld %s:%ld (%s): %s\n", (long) (level), (file), (long) (line), (func), (msg));} while(0)' \
	--output-directory /tmp/duk-configure-test
#grep DUK_USE_DEBUG_WRITE /tmp/duk-configure-test/duk_config.h
#grep DUK_USE_DEBUG_WRITE /tmp/duk-configure-test/duk_config.h | md5sum
LINE=`grep DUK_USE_DEBUG_WRITE /tmp/duk-configure-test/duk_config.h`
if [ "$LINE" != "#define DUK_USE_DEBUG_WRITE(level,file,line,func,msg) do {fprintf(stderr, \"D%ld %s:%ld (%s): %s\n\", (long) (level), (file), (long) (line), (func), (msg));} while(0)" ]; then
	echo "DUK_USE_DEBUG_WRITE not found in duk_config.h in expected form"
	echo $LINE
fi

echo "Done."
