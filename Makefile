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
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/build/400/duk.400 --run-nodejs --run-rhino --num-threads 8 --log-file=/tmp/duk-test.log testcases/

vgtest:
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/build/400/duk.400 --run-nodejs --run-rhino --num-threads 1 --log-file=/tmp/duk-vgtest.log --valgrind testcases/
	
install:
	scons -j 8 install


