#
#  Convenience Makefile; SCons is used for the actual build
#

.PHONY: default all clean test install

default:	all

all:
	scons -s -j 8

clean:
	scons -c -s

test:
	cd testcases; python runtests.py --run-duk --run-nodejs --run-rhino --num-threads 8 --test-log=/tmp/duk-test.log --test-all --cmd-duk=$(shell pwd)/build/400/duk.400

install:
	scons -j 8 install


