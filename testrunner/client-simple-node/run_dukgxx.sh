#!/bin/sh

# Full name of repo, e.g. "svaarala/duktape".
REPOFULLNAME=$1

# Repo HTTPS clone URL, e.g. "https://github.com/svaarala/duktape.git".
REPOCLONEURL=$2

# Commit hash.
SHA=$3

# Context, e.g. "x64-qecmatest".
CONTEXT=$4

# Automatic temporary directory created by testclient, automatically
# deleted (recursively) by testclient.
TEMPDIR=$5

# Directory holding repo tar.gz clone snapshots for faster test init.
# Example: /tmp/repo-snapshots/svaarala/duktape.tar.gz.
REPO_TARGZS=/tmp/repo-snapshots

set -e

echo "*** Run duk-g++ test: `date`"
echo "REPOFULLNAME=$REPOFULLNAME"
echo "REPOCLONEURL=$REPOCLONEURL"
echo "SHA=$SHA"
echo "CONTEXT=$CONTEXT"

if [ "x$TEMPDIR" = "x" ]; then
	echo "Missing TEMPDIR."
	exit 1
fi
cd "$TEMPDIR"

if echo "$REPOFULLNAME" | grep -E '[[:space:]\.]'; then
	echo "Repo full name contains invalid characters"
	exit 1
fi
if [ "$REPOFULLNAME" != "svaarala/duktape" ]; then
	echo "Repo is not whitelisted"
	exit 1
fi

# This isn't great security-wise, but $REPOFULLNAME has been checked not to
# contain dots (e.g. '../').  It'd be better to resolve the absolute filename
# and ensure it begins with $REPO_TARGZS.
TARGZ=$REPO_TARGZS/$REPOFULLNAME.tar.gz
echo "Using repo tar.gz: '$TARGZ'"
if [ ! -f "$TARGZ" ]; then
	echo "Repo snapshot doesn't exist: $TARGZ."
	exit 1
fi

echo "Git preparations"
cd "$TEMPDIR"
mkdir repo
cd repo
tar -x --strip-components 1 -z -f "$TARGZ"
git clean -f
git checkout master
git pull --rebase
git clean -f
git checkout "$SHA"

echo ""
echo "Running: make clean duk-g++"
make clean duk-g++

echo ""
echo "Hello world test"
./duk-g++ -e "print('hello world!');" >hello.out
echo "hello world!" >hello.require
cat hello.out

# If stdout contains "TESTRUNNER_DESCRIPTION: xxx" it will become the Github
# status description.  Last occurrence wins.  If no such line is printed, the
# test client will use a generic "Success" or "Failure" text.
echo "TESTRUNNER_DESCRIPTION: DUMMY"
if cmp hello.out hello.require; then
	echo "match"
	echo "TESTRUNNER_DESCRIPTION: Hello world success"
	exit 0
else
	echo "no match"
	echo "TESTRUNNER_DESCRIPTION: Hello world failed"
	exit 1
fi
