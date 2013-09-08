#!/bin/sh
#
#  Create full distributable.  Full distributable needs the ability to make
#  an end-user distribute because e.g. testcases need to be run against such
#  a distributable source.  This awkwardness can be removed once a public
#  repo exists.
#

FULL=`pwd`/full

rm -rf $FULL
mkdir $FULL
mkdir $FULL/src
mkdir $FULL/doc
mkdir $FULL/runtests
mkdir $FULL/examples
mkdir $FULL/examples/cmdline
mkdir $FULL/examples/coffee
mkdir $FULL/examples/hello
mkdir $FULL/ecmascript-testcases
mkdir $FULL/api-testcases

for i in src/*.c src/*.h src/*.py src/*.txt; do
	cp --parents $i $FULL/
done

for i in \
	doc/json.txt \
	doc/datetime.txt \
	doc/number_conversion.txt \
	doc/regexp.txt \
	doc/sorting.txt \
	; do
	cp --parents $i $FULL/
done

for i in \
	runtests/runtests.js \
	runtests/package.json \
	runtests/api_testcase_main.c \
	; do
	cp --parents $i $FULL/
done

for i in \
	examples/test.c \
	examples/cmdline/duk_cmdline.c \
	examples/cmdline/duk_ncurses.c \
	examples/cmdline/duk_socket.c \
	examples/cmdline/duk_fileio.c \
	examples/coffee/mandel.js \
	examples/coffee/hello.js \
	examples/coffee/globals.js \
	examples/coffee/mandel.coffee \
	examples/coffee/hello.coffee \
	examples/coffee/globals.coffee \
	examples/Makefile.cmdline \
	examples/Makefile.example \
	examples/hello/hello.c \
	; do
	cp --parents $i $FULL/
done

for i in ecmascript-testcases/*.js; do
	cp $i $FULL/ecmascript-testcases/
done

for i in api-testcases/*.c; do
	cp $i $FULL/api-testcases/
done

for i in \
	README.txt.dist \
	README.txt.full \
	LICENSE.txt \
	Makefile \
	make_dist.sh \
	combine_src.py \
	; do
	cp $i $FULL/
done

