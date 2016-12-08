#!/bin/sh

MERGECOUNT=100

rm -rf /tmp/duk-graph
mkdir /tmp/duk-graph
cd /tmp/duk-graph || exit 1
echo "Working in: `pwd`"
git clone https://github.com/svaarala/duktape.git
cd /tmp/duk-graph/duktape || exit 1

for sha in `git log -n $MERGECOUNT --merges --oneline --decorate=no | cut -f 1 -d ' '`; do
	cd /tmp/duk-graph/duktape || exit 1
	echo $sha
	git describe
	git reset --hard
	git clean --force
	git checkout $sha
	
	make clean
	rm -rf ./prep; mkdir ./prep
	python tools/configure.py --output ./prep --option-file config/examples/low_memory.yaml

	rm -f ./prep/hello
	gcc -o ./prep/hello -std=c99 -Wall -Os -fomit-frame-pointer -flto -fno-asynchronous-unwind-tables -ffunction-sections -Wl,--gc-sections -I./prep ./prep/duktape.c examples/hello/hello.c -lm
	size ./prep/hello | grep prep/hello | awk '/.*/ { print $4 }' >> /tmp/duk-graph/sizes-x64.txt

	rm -f ./prep/hello
	gcc -o ./prep/hello -std=c99 -Wall -Os -fomit-frame-pointer -flto -fno-asynchronous-unwind-tables -ffunction-sections -Wl,--gc-sections -m32 -I./prep ./prep/duktape.c examples/hello/hello.c -lm
	size ./prep/hello | grep prep/hello | awk '/.*/ { print $4 }' >> /tmp/duk-graph/sizes-x86.txt

	rm -f ./prep/hello
	gcc -o ./prep/hello -std=c99 -Wall -Os -fomit-frame-pointer -flto -fno-asynchronous-unwind-tables -ffunction-sections -Wl,--gc-sections -mx32 -I./prep ./prep/duktape.c examples/hello/hello.c -lm
	size ./prep/hello | grep prep/hello | awk '/.*/ { print $4 }' >> /tmp/duk-graph/sizes-x32.txt

	rm -rf ./prep
done

cat /tmp/duk-graph/sizes-x64.txt | tac | gnuplot -p -e 'set term png; set output "/tmp/duk-graph-size-x64.png"; set xlabel "merge"; set ylabel "bytes"; plot "/dev/stdin" with lines'
cat /tmp/duk-graph/sizes-x86.txt | tac | gnuplot -p -e 'set term png; set output "/tmp/duk-graph-size-x86.png"; set xlabel "merge"; set ylabel "bytes"; plot "/dev/stdin" with lines'
cat /tmp/duk-graph/sizes-x32.txt | tac | gnuplot -p -e 'set term png; set output "/tmp/duk-graph-size-x32.png"; set xlabel "merge"; set ylabel "bytes"; plot "/dev/stdin" with lines'

ls -l /tmp/duk-graph-*.png
