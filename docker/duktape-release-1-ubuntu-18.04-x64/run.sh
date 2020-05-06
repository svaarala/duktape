#!/bin/bash

set -e
set -x

/work/prepare_repo.sh

source emsdk/emsdk_env.sh

cd duktape
ROOT=`pwd`

test_combined_source_compilation() {
	make clean dist
	cd dist
	cat Makefile.cmdline
	make -f Makefile.cmdline
	ls -l duk; size duk
	./duk mandel.js > /tmp/out.txt
	cat /tmp/out.txt
	if [ `md5sum /tmp/out.txt | cut -f 1 -d ' '` != "627cd86f0a4255e018c564f86c6d0ab3" ]; then
		echo "Combined source compilation failed!"
		exit 1
	fi
}

test_clang_compilation() {
	make clean duk-clang
}

test_gxx_compilation() {
	make clean duk-g++
	make clean dukd-g++
}

test_misc_compilation() {
	make clean duk-rom
	make clean duk-low
	make clean duk-low-norefc
	make clean duk-low-rom
	make clean duk-perf
	make clean duk-size
	make clean duk-pgo.O2
	make clean dukd
	make clean dukd-rom
	make clean dukd-low
	make clean dukd-low-norefc
	make clean dukd-low-rom
}

test_configure_fastint() {
	make clean dist
	cd dist
	rm -rf /tmp/out
	python tools/configure.py -DDUK_USE_FASTINT --output-directory /tmp/out
	ls -l /tmp/out
}

test_releasetest() {
	make clean
	make releasetest
}

test_checklist_compile_test() {
	make clean dist
	cd dist
	bash ../util/checklist_compile_test.sh
}

echo ""
echo "*** Test compilation from combined sources"
echo ""
cd $ROOT
test_combined_source_compilation

echo ""
echo "*** Test clang compilation (duk-clang)"
echo ""
cd $ROOT
test_clang_compilation

echo ""
echo "*** Test C++ compilation (duk-g++)"
echo ""
cd $ROOT
test_gxx_compilation

echo ""
echo "*** Test misc compilation targets (duk-low, etc)"
echo ""
cd $ROOT
test_misc_compilation

echo ""
echo "*** Test configure.py -DDUK_USE_FASTINT"
echo ""
cd $ROOT
test_configure_fastint

echo ""
echo "*** Test make releasetest"
echo ""
cd $ROOT
test_releasetest

echo ""
echo "*** Test checklist_compile_test.sh"
echo ""
cd $ROOT
test_checklist_compile_test

echo ""
echo "*** All done!"
echo ""
