#!/bin/sh
#
#  Check prerequisites for using the Duktape makefile.  Exit with an error
#  and a useful error message for missing prerequisites.
#

ERRORS=0
WARNINGS=0

uname -a | grep -ni linux >/dev/null
RET=$?
if [ "x$RET" != "x0" ]; then
	echo "*** Based on uname, you're not running on Linux.  Duktape developer"
	echo "    makefile is intended for Linux only; YMMV on other platforms."
	echo ""
	sleep 1
fi

python -c 'import yaml' 2>/dev/null
if [ $? != 0 ]; then
	echo "*** Missing Python yaml:"
	echo "  $ sudo apt-get install python-yaml"
	echo ""
	ERRORS=1
fi

NODEJS_VERSION=`nodejs -v 2>/dev/null`
if [ $? != 0 ]; then
	echo "*** Missing NodeJS:"
	echo "  $ sudo apt-get install nodejs nodejs-legacy npm  # may also be 'node'"
	echo ""
	ERRORS=1
fi
#echo "NodeJS version: $NODEJS_VERSION"

# some tools like uglifyjs require 'node', not 'nodejs'
NODE_VERSION=`node -v 2>/dev/null`
if [ $? != 0 ]; then
	echo "*** Missing NodeJS legacy ('node' command):"
	echo "  $ sudo apt-get install nodejs-legacy"
	echo ""
	ERRORS=1
fi
#echo "NodeJS 'node' version: $NODE_VERSION"

GIT_VERSION=`git --version 2>/dev/null`
if [ $? != 0 ]; then
	echo "*** Missing git:"
	echo "  $ sudo apt-get install git"
	echo ""
	ERRORS=1
fi
#echo "Git version: $GIT_VERSION"

UNZIP_VERSION=`unzip -v 2>/dev/null`
if [ $? != 0 ]; then
	echo "*** Missing unzip:"
	echo "  $ sudo apt-get install unzip"
	echo ""
	ERRORS=1
fi
#echo "UNZIP_VERSION: $UNZIP_VERSION"

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

CLANG_VERSION=`clang -v 2>&1`
if [ $? != 0 ]; then
	echo "*** Missing clang (affects emscripten tests):"
	echo "  $ sudo apt-get install clang"
	echo ""
	WARNINGS=1
fi

LLVM_LINK_VERSION=`llvm-link --version 2>&1`  # exit code will be 1
which llvm-link 2>/dev/null >/dev/null
if [ $? != 0 ]; then
	echo "*** Missing llvm (affects emscripten tests):"
	echo "  $ sudo apt-get install llvm"
	echo ""
	WARNINGS=1
fi

python -c 'from bs4 import BeautifulSoup, Tag' 2>/dev/null
if [ $? != 0 ]; then
	echo "*** Missing BeautifulSoup (affects website build)"
	echo "  $ sudo apt-get install python-bs4"
	echo ""
	WARNINGS=1
fi

SOURCE_HIGHLIGHT_VERSION=`source-highlight --version`
if [ $? != 0 ]; then
	echo "*** Missing source-highlight (affects website build)"
	echo "  $ sudo apt-get install source-highlight"
	echo ""
	WARNINGS=1
fi

if [ "x$ERRORS" != "x0" ]; then
	echo "*** Errors found in system setup, see error messages above!"
	exit 1
fi

if [ "x$WARNINGS" != "x0" ]; then
	echo "*** Warnings found in system setup, see warnings above"
	exit 0
fi

# 'tidy' is intentionally not checked as it only relates to website development
# and is not mandatory to website build.

exit 0
