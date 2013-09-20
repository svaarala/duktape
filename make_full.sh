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
mkdir $FULL/licenses
mkdir $FULL/runtests
mkdir $FULL/examples
mkdir $FULL/examples/hello
mkdir $FULL/examples/cmdline
mkdir $FULL/examples/guide
mkdir $FULL/examples/coffee
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
	doc/uri.txt \
	doc/testcases.txt \
	doc/code-issues.txt \
	; do
	cp --parents $i $FULL/
done

for i in \
	licenses/murmurhash2.txt \
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
	examples/cmdline/duk_cmdline.c \
	examples/cmdline/duk_ncurses.c \
	examples/cmdline/duk_socket.c \
	examples/cmdline/duk_fileio.c \
	examples/hello/hello.c \
	examples/guide/fib.js \
	examples/guide/process.js \
	examples/guide/processlines.c \
	examples/guide/prime.js \
	examples/guide/primecheck.c \
	examples/guide/uppercase.c \
	examples/coffee/Makefile \
	examples/coffee/mandel.coffee \
	examples/coffee/hello.coffee \
	examples/coffee/globals.coffee \
	examples/Makefile.cmdline \
	examples/Makefile.example \
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

