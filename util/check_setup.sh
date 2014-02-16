#!/bin/sh
#
#  Check prerequisites for using the Duktape makefile.  Exit with an error
#  and a useful error message for missing prerequisites.
#

ERRORS=0

uname -a | grep -ni linux >/dev/null
RET=$?
if [ "x$RET" != "x0" ]; then
	echo "*** Based on uname, you're not running on Linux.  Duktape developer"
	echo "    makefile is intended for Linux only; YMMV on other platforms."
	echo ""
	sleep 1
fi

NODEJS_VERSION=`nodejs -v 2>/dev/null`
if [ $? != 0 ]; then
	NODE_VERSION=`node -v 2>/dev/null`
	if [ $? != 0 ]; then
		echo "*** Missing NodeJS:"
		echo "  $ sudo apt-get install nodejs npm  # may also be 'node'"
		echo ""
		ERRORS=1
	fi
fi
#echo "Node version: $NODE_VERSION"

GIT_VERSION=`git --version 2>/dev/null`
if [ $? != 0 ]; then
	echo "*** Missing git:"
	echo "  $ sudo apt-get install git"
	echo ""
	ERRORS=1
fi
#echo "Git version: $GIT_VERSION"

PERL_VERSION=`perl -version 2>/dev/null`
if [ $? != 0 ]; then
	echo "*** Missing perl:"
	echo "  $ sudo apt-get install perl"
	echo ""
	ERRORS=1
fi
#echo "PERL_VERSION: $PERL_VERSION"

JAVA_VERSION=`java -version 2>&1`
if [ $? != 0 ]; then
	echo "*** Missing java:"
	echo "  $ sudo apt-get install openjdk-7-jre"
	echo ""
	ERRORS=1
fi
#echo "JAVA_VERSION: $JAVA_VERSION"

if [ "x$ERRORS" = "x0" ]; then
	exit 0
fi

echo "*** Errors found in system setup, see error messages above!"
exit 1
