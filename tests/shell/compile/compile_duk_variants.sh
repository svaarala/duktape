#!/bin/sh

for target in \
	build/duk \
	build/duk-pgo \
	build/duk-perf \
	build/duk-size \
	build/duk-rom \
	build/dukd \
	build/dukd-rom \
	build/duk.O2 \
	build/duk-pgo.O2 \
	build/duk-perf.O2 \
	build/duk-perf-pgo.O2 \
	build/duk.O3 \
	build/duk-perf.O3 \
	build/duk.O4 \
	build/duk-perf.O4 \
	build/duk-clang \
	build/duk-clang-asan \
	build/duk-clang-ubsan \
	build/duk-perf-clang \
	build/duk-fuzzilli \
	build/duk-g++ \
	build/duk-perf-g++ \
	build/dukd-g++ \
	build/dukscanbuild \
	build/dukd-low \
	build/duk-low-rom \
	build/dukd-low-rom \
	build/duk-low-norefc \
	build/dukd-low-norefc; do
	echo ""
	echo "--- ${target}"
	make cleanall
	make ${target} || exit 1
done
