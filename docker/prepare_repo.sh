#!/bin/bash

set -e

echo "--- Setting up /work/duktape ---"
echo "WD: `pwd`"
if [ -d duktape ]; then
	echo "duktape/ already exists: use as is"
	cd duktape
elif [ -d duktape-host ]; then
	echo "duktape-host/ exists: make a copy"
	cp -r duktape-host duktape
	cd duktape
elif [ $STDIN_ZIP ]; then
	echo "STDIN_ZIP set: unzip from STDIN"
	cat > /tmp/duktape.zip
	mkdir duktape
	cd duktape
	unzip -q /tmp/duktape.zip
	# Reduce I/O for some common cases.
	if [ ! -d duktape-releases ]; then
		cp -r ../repo-snapshots/duktape-releases duktape-releases
		cd duktape-releases
		git pull
		cd ..
	fi
else
	echo "STDIN_ZIP not set: use master"
	cp -r duktape-prep duktape
	cd duktape
	git pull
	cd duktape-releases
	git pull
	cd ..
fi
echo "--- Done setting up /work/duktape ---"
echo ""
