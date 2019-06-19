#!/bin/bash

set -e

echo "WD: `pwd`"
if [ -d duktape ]; then
	echo "duktape/ already exists, use as is"
	cd duktape
elif [ $STDIN_ZIP ]; then
	echo "STDIN_ZIP set, unzip from STDIN"
	cat > /tmp/duktape.zip
	mkdir duktape
	cd duktape
	unzip -q /tmp/duktape.zip
else
	echo "STDIN_ZIP not set, use master"
	cp -r duktape-prep duktape
	cd duktape
	git pull
	cd duktape-releases
	git pull
	cd ..
fi
