#!/bin/sh
#
#  Check prerequisites for using the Duktape makefile.  Exit with an error
#  and a useful error message for missing prerequisites.
#

NODE_VERSION=`node -v 2>/dev/null`
if [ "x$NODE_VERSION" = "x" ]; then
	echo "*** Missing NodeJS:"
	echo "  $ sudo apt-get install nodejs npm"
	exit 1
fi

GIT_VERSION=`git --version`
if [ "x$GIT_VERSION" = "x" ]; then
	echo "*** Missing git:"
	echo "  $ sudo apt-get install git"
	exit 1
fi

uname -a | grep -ni linux >/dev/null
RET=$?
if [ "x$RET" != "x0" ]; then
	echo "*** Based on uname, you're not running on Linux.  Duktape developer"
	echo "    makefile is intended for Linux only; YMMV on other platforms."
	echo ""
	sleep 1
fi
