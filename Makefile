#
#  Makefile for the Duktape development repo.
#
#  This Makefile is intended for ONLY internal Duktape development
#  on Linux (or other UNIX-like operating systems), and covers:
#
#    - Building the Duktape source distributable.
#    - Running some basic test cases.
#    - Building the duktape.org website.
#
#  The Makefile now also works in a very limited fashion with Cygwin,
#  you can 'make dist' as long as you have enough software installed.
#
#  The source distributable has more platform neutral example Makefiles
#  for end user projects (though an end user should really just use their
#  own Makefile).
#
#  YOU SHOULD NOT COMPILE DUKTAPE WITH THIS MAKEFILE IN YOUR PROJECT!
#
#  When creating actual distributables, always use a clean checkout.
#
#  External projects are downloaded on-the-fly.  Clone git repos shallowly
#  (--depth 1) whenever possible.  With build-critical resources, use a
#  specific version instead of "trunk".
#
#  The makefile should work with -jN except for the 'clean' target, use:
#
#    $ make clean; make -j4
#

# A few commands which may need to be edited.  NodeJS is sometimes found
# as 'nodejs', sometimes as 'node'; sometimes 'node' is unrelated to NodeJS
# so check 'nodejs' first.
GIT := $(shell command -v git 2>/dev/null)
NODE := $(shell { command -v nodejs || command -v node; } 2>/dev/null)
WGET := $(shell command -v wget 2>/dev/null)
JAVA := $(shell command -v java 2>/dev/null)
VALGRIND := $(shell command -v valgrind 2>/dev/null)
PYTHON := $(shell { command -v python2 || command -v python; } 2>/dev/null)

# Scrape version from the public header; convert from e.g. 10203 -> '1.2.3'
DUK_VERSION := $(shell cat src-input/duktape.h.in | grep 'define ' | grep DUK_VERSION | tr -s ' ' ' ' | cut -d ' ' -f 3 | tr -d 'L')
DUK_MAJOR := $(shell echo "$(DUK_VERSION) / 10000" | bc)
DUK_MINOR := $(shell echo "$(DUK_VERSION) % 10000 / 100" | bc)
DUK_PATCH := $(shell echo "$(DUK_VERSION) % 100" | bc)
DUK_VERSION_FORMATTED := $(DUK_MAJOR).$(DUK_MINOR).$(DUK_PATCH)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
GIT_DESCRIBE := $(shell git describe --always --dirty)
ifeq ($(GIT_BRANCH),master)
GIT_INFO := $(GIT_DESCRIBE)
else
GIT_INFO := $(GIT_DESCRIBE)-$(GIT_BRANCH)
endif
BUILD_DATETIME := $(shell date +%Y%m%d%H%M%S)

# Source lists.
DUKTAPE_CMDLINE_SOURCES = \
	examples/cmdline/duk_cmdline.c \
	examples/alloc-logging/duk_alloc_logging.c \
	examples/alloc-torture/duk_alloc_torture.c \
	examples/alloc-hybrid/duk_alloc_hybrid.c \
	extras/print-alert/duk_print_alert.c \
	extras/console/duk_console.c \
	extras/logging/duk_logging.c \
	extras/module-duktape/duk_module_duktape.c
ifdef SYSTEMROOT  # Windows
DUKTAPE_CMDLINE_SOURCES += examples/debug-trans-socket/duk_trans_socket_windows.c
else
DUKTAPE_CMDLINE_SOURCES += examples/debug-trans-socket/duk_trans_socket_unix.c
endif
ifdef SYSTEMROOT  # Windows
LINENOISE_SOURCES =
else
LINENOISE_SOURCES = linenoise/linenoise.c
endif

# Configure.py options for a few configuration profiles needed.
CONFIGOPTS_NONDEBUG = --option-file util/makeduk_base.yaml
CONFIGOPTS_NONDEBUG_SCANBUILD = --option-file util/makeduk_base.yaml --option-file util/makeduk_scanbuild.yaml
CONFIGOPTS_NONDEBUG_PERF = --option-file config/examples/performance_sensitive.yaml
CONFIGOPTS_NONDEBUG_SIZE = --option-file config/examples/low_memory.yaml
CONFIGOPTS_NONDEBUG_ROM = --rom-support --rom-auto-lightfunc --option-file util/makeduk_base.yaml -DDUK_USE_ROM_STRINGS -DDUK_USE_ROM_OBJECTS -DDUK_USE_ROM_GLOBAL_INHERIT -UDUK_USE_HSTRING_ARRIDX
CONFIGOPTS_NONDEBUG_DUKLOW = --option-file config/examples/low_memory.yaml --option-file util/makeduk_duklow.yaml --fixup-file util/makeduk_duklow_fixup.h
CONFIGOPTS_DEBUG_DUKLOW = $(CONFIGOPTS_NONDEBUG_DUKLOW) -DDUK_USE_ASSERTIONS -DDUK_USE_SELF_TESTS
CONFIGOPTS_NONDEBUG_DUKLOW_ROM = --rom-support --rom-auto-lightfunc --option-file config/examples/low_memory.yaml --option-file util/makeduk_duklow.yaml --fixup-file util/makeduk_duklow_fixup.h --builtin-file util/example_user_builtins1.yaml --builtin-file util/example_user_builtins2.yaml -DDUK_USE_ROM_STRINGS -DDUK_USE_ROM_OBJECTS -DDUK_USE_ROM_GLOBAL_INHERIT -UDUK_USE_HSTRING_ARRIDX -UDUK_USE_DEBUG
CONFIGOPTS_DEBUG_DUKLOW_ROM = $(CONFIGOPTS_NONDEBUG_DUKLOW_ROM) -DDUK_USE_ASSERTIONS -DDUK_USE_SELF_TESTS
CONFIGOPTS_NONDEBUG_DUKLOW_NOREFC = --option-file config/examples/low_memory.yaml --option-file util/makeduk_duklow.yaml --fixup-file util/makeduk_duklow_fixup.h -UDUK_USE_REFERENCE_COUNTING -UDUK_USE_DOUBLE_LINKED_HEAP
CONFIGOPTS_DEBUG_DUKLOW_NOREFC = $(CONFIGOPTS_NONDEBUG_DUKLOW_NOREFC) -DDUK_USE_ASSERTIONS -DDUK_USE_SELF_TESTS
CONFIGOPTS_DEBUG = --option-file util/makeduk_base.yaml --option-file util/makeduk_debug.yaml
CONFIGOPTS_DEBUG_SCANBUILD = --option-file util/makeduk_base.yaml --option-file util/makeduk_debug.yaml --option-file util/makeduk_scanbuild.yaml
CONFIGOPTS_DEBUG_ROM = --rom-support --rom-auto-lightfunc --option-file util/makeduk_base.yaml --option-file util/makeduk_debug.yaml -DDUK_USE_ROM_STRINGS -DDUK_USE_ROM_OBJECTS -DDUK_USE_ROM_GLOBAL_INHERIT -UDUK_USE_HSTRING_ARRIDX
CONFIGOPTS_EMDUK = -UDUK_USE_FASTINT -UDUK_USE_PACKED_TVAL
CONFIGOPTS_DUKWEB = --option-file util/dukweb_base.yaml --fixup-file util/dukweb_fixup.h

# Profile guided optimization test set.
PGO_TEST_SET = \
	tests/ecmascript/test-dev-mandel2-func.js \
	tests/ecmascript/test-dev-totp.js \
	tests/perf/test-fib.js \
	tests/ecmascript/test-regexp-ipv6-regexp.js

# Compiler setup for Linux.
CC = gcc
GXX = g++

CCOPTS_SHARED =
CCOPTS_SHARED += -DDUK_CMDLINE_PRINTALERT_SUPPORT
CCOPTS_SHARED += -DDUK_CMDLINE_CONSOLE_SUPPORT
CCOPTS_SHARED += -DDUK_CMDLINE_LOGGING_SUPPORT
CCOPTS_SHARED += -DDUK_CMDLINE_MODULE_SUPPORT
ifdef SYSTEMROOT  # Windows
# Skip fancy (linenoise)
else
CCOPTS_SHARED += -DDUK_CMDLINE_FANCY
#CCOPTS_SHARED += -DDUK_CMDLINE_PTHREAD_STACK_CHECK
endif
CCOPTS_SHARED += -DDUK_CMDLINE_ALLOC_LOGGING
CCOPTS_SHARED += -DDUK_CMDLINE_ALLOC_TORTURE
CCOPTS_SHARED += -DDUK_CMDLINE_ALLOC_HYBRID
CCOPTS_SHARED += -DDUK_CMDLINE_DEBUGGER_SUPPORT
CCOPTS_SHARED += -DDUK_CMDLINE_FILEIO

CCOPTS_SHARED += -D_POSIX_C_SOURCE=200809L  # to avoid linenoise strdup() warnings
CCOPTS_SHARED += -pedantic -ansi -std=c99 -fstrict-aliasing
# -Wextra is very picky but catches e.g. signed/unsigned comparisons
CCOPTS_SHARED += -Wall -Wextra -Wunused-result -Wdeclaration-after-statement -Wunused-function
CCOPTS_SHARED += -Wcast-qual
CCOPTS_SHARED += -Wcast-align
CCOPTS_SHARED += -Wshadow
CCOPTS_SHARED += -Wunreachable-code  # on some compilers unreachable code is an error
CCOPTS_SHARED += -Wmissing-prototypes
CCOPTS_SHARED += -Wfloat-equal
CCOPTS_SHARED += -Wsign-conversion
CCOPTS_SHARED += -Wsuggest-attribute=noreturn
CCOPTS_SHARED += -fmax-errors=3  # prevent floods of errors if e.g. parenthesis missing
CCOPTS_SHARED += -I./linenoise
CCOPTS_SHARED += -I./examples/cmdline
CCOPTS_SHARED += -I./examples/alloc-logging
CCOPTS_SHARED += -I./examples/alloc-torture
CCOPTS_SHARED += -I./examples/alloc-hybrid
CCOPTS_SHARED += -I./examples/debug-trans-socket
CCOPTS_SHARED += -I./extras/print-alert
CCOPTS_SHARED += -I./extras/console
CCOPTS_SHARED += -I./extras/logging
CCOPTS_SHARED += -I./extras/module-duktape
#CCOPTS_SHARED += -m32   # force 32-bit compilation on a 64-bit host
#CCOPTS_SHARED += -mx32  # force X32 compilation on a 64-bit host
#CCOPTS_SHARED += -fstack-usage  # enable manually, then e.g. $ make clean duk; python util/pretty_stack_usage.py duktape.su

CCOPTS_NONDEBUG = $(CCOPTS_SHARED) $(CCOPTS_FEATURES)
CCOPTS_NONDEBUG += -Os -fomit-frame-pointer -fno-stack-protector
CCOPTS_NONDEBUG += -g -ggdb
#CCOPTS_NONDEBUG += -malign-double

CCOPTS_DEBUG = $(CCOPTS_SHARED) $(CCOPTS_FEATURES)
CCOPTS_DEBUG += -O0
CCOPTS_DEBUG += -g -ggdb

CLANG_CCOPTS_NONDEBUG = $(CCOPTS_NONDEBUG)
CLANG_CCOPTS_NONDEBUG += -Wshorten-64-to-32
CLANG_CCOPTS_NONDEBUG += -Wcomma
#CLANG_CCOPTS_NONDEBUG += -fsanitize=undefined

GXXOPTS_SHARED = -pedantic -ansi -std=c++11 -fstrict-aliasing -Wall -Wextra -Wunused-result -Wunused-function
GXXOPTS_SHARED += -DDUK_CMDLINE_PRINTALERT_SUPPORT
GXXOPTS_NONDEBUG = $(GXXOPTS_SHARED) -Os -fomit-frame-pointer
GXXOPTS_NONDEBUG += -I./examples/alloc-logging -I./examples/alloc-torture -I./examples/alloc-hybrid -I./extras/print-alert -I./extras/console -I./extras/logging -I./extras/module-duktape
GXXOPTS_DEBUG = $(GXXOPTS_SHARED) -O0 -g -ggdb
GXXOPTS_DEBUG += -I./examples/alloc-logging -I./examples/alloc-torture -I./examples/alloc-hybrid -I./extras/print-alert -I./extras/console -I./extras/logging -I./extras/module-duktape

CCOPTS_DUKLOW = -m32
CCOPTS_DUKLOW += -flto -fno-asynchronous-unwind-tables -ffunction-sections -Wl,--gc-sections
#CCOPTS_DUKLOW += '-fpack-struct=1'
CCOPTS_DUKLOW += -Wno-unused-parameter -Wno-pedantic -Wno-sign-compare -Wno-missing-field-initializers -Wno-unused-result
CCOPTS_DUKLOW += -UDUK_CMDLINE_FANCY -DDUK_CMDLINE_LOWMEM -D_POSIX_C_SOURCE=200809L
CCOPTS_DUKLOW += -UDUK_CMDLINE_LOGGING_SUPPORT  # extras/logger init writes to Duktape.Logger, problem with ROM build
CCOPTS_DUKLOW += -UDUK_CMDLINE_MODULE_SUPPORT  # extras/module-duktape init writes to Duktape.Logger, problem with ROM build
CCOPTS_DUKLOW += -UDUK_CMDLINE_CONSOLE_SUPPORT
CCOPTS_DUKLOW += -UDUK_CMDLINE_ALLOC_LOGGING
CCOPTS_DUKLOW += -UDUK_CMDLINE_ALLOC_TORTURE
CCOPTS_DUKLOW += -UDUK_CMDLINE_ALLOC_HYBRID
CCOPTS_DUKLOW += -UDUK_CMDLINE_DEBUGGER_SUPPORT
CCOPTS_DUKLOW += -UDUK_CMDLINE_FILEIO
#CCOPTS_DUKLOW += -DDUK_ALLOC_POOL_DEBUG
CCOPTS_DUKLOW += -DDUK_ALLOC_POOL_TRACK_WASTE  # quite fast, but not free so disable for performance comparison
#CCOPTS_DUKLOW += -DDUK_ALLOC_POOL_TRACK_HIGHWATER  # VERY SLOW, just for manual testing

ifdef SYSTEMROOT  # Windows
CCLIBS = -lm -lws2_32
else
#CCLIBS = -lm -lpthread
CCLIBS = -lm
endif

# Rely on an external, configured 'emcc' command.  See docker/ for Docker
# images for an example of building a working 'emcc' environment.  See
# doc/emscripten-status.rst for the Emscripten options used.
#
# Reducing the TOTAL_MEMORY and TOTAL_STACK values is useful if you run
# Duktape cmdline with resource limits (i.e. "duk -r test.js").
EMCC = emcc
#EMCCOPTS = -s TOTAL_MEMORY=2097152 -s TOTAL_STACK=524288 --memory-init-file 0
EMCCOPTS = -O2 -std=c99 -Wall --memory-init-file 0 -s WASM=0 -s POLYFILL_OLD_MATH_FUNCTIONS
EMCCOPTS_DUKVM = -O2 -std=c99 -Wall --memory-init-file 0 -DEMSCRIPTEN -s WASM=0
EMCCOPTS_DUKWEB_EXPORT = -s EXPORTED_FUNCTIONS='["_main","_dukweb_is_open", "_dukweb_open","_dukweb_close","_dukweb_eval"]' -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["ccall","cwrap"]'
EMDUKOPTS = -s TOTAL_MEMORY=268435456 -DDUK_CMDLINE_PRINTALERT_SUPPORT
EMDUKOPTS += -DEMSCRIPTEN  # enable stdin workaround in duk_cmdline.c

# Mandelbrot test, base-64 encoded.
MAND_BASE64 = dyA9IDgwOyBoID0gNDA7IGl0ZXIgPSAxMDA7IGZvciAoaSA9IDA7IGkgLSBoOyBpICs9IDEpIHsgeTAgPSAoaSAvIGgpICogNC4wIC0gMi4wOyByZXMgPSBbXTsgZm9yIChqID0gMDsgaiAtIHc7IGogKz0gMSkgeyB4MCA9IChqIC8gdykgKiA0LjAgLSAyLjA7IHh4ID0gMDsgeXkgPSAwOyBjID0gIiMiOyBmb3IgKGsgPSAwOyBrIC0gaXRlcjsgayArPSAxKSB7IHh4MiA9IHh4Knh4OyB5eTIgPSB5eSp5eTsgaWYgKE1hdGgubWF4KDAsIDQuMCAtICh4eDIgKyB5eTIpKSkgeyB5eSA9IDIqeHgqeXkgKyB5MDsgeHggPSB4eDIgLSB5eTIgKyB4MDsgfSBlbHNlIHsgYyA9ICIuIjsgYnJlYWs7IH0gfSByZXNbcmVzLmxlbmd0aF0gPSBjOyB9IHByaW50KHJlcy5qb2luKCIiKSk7IH0K

# Options for runtests.js.
RUNTESTSOPTS = --prep-test-path util/prep_test.py --minify-uglifyjs2 UglifyJS2/bin/uglifyjs --util-include-path tests/ecmascript --known-issues doc/testcase-known-issues.yaml

# Compile 'duk' only by default
.PHONY: all
all: duk

# Clean targets: 'cleanall' also deletes downloaded third party packages
# which we don't want to delete by default with 'clean'.
.PHONY: clean
clean:
	@rm -f *.gcda
	@rm -f *.su
	@rm -rf dist/
	@rm -rf prep/
	@rm -rf site/
	@rm -f duk duk-rom dukd dukd-rom duk.O2 duk.O3 duk.O4
	@rm -f duk-pgo duk-pgo.O2
	@rm -f duk-perf duk-perf.O2 duk-perf.O3 duk-perf.O4
	@rm -f duk-perf-pgo duk-perf-pgo.O2 duk-perf-pgo.O3 duk-perf-pgo.O4
	@rm -f duk-size
	@rm -f duk-rom dukd-rom
	@rm -f duk-clang duk-perf-clang duk-sanitize-clang
	@rm -f duk-g++ dukd-g++ duk-perf-g++
	@rm -f duk-low duk-low-norefc duk-low-rom
	@rm -f emduk emduk.js
	@rm -f libduktape*.so*
	@rm -f duktape-*.tar.*
	@rm -f duktape-*.iso
	@rm -f docker-input.zip docker-output.zip
	@rm -f doc/*.html
	@rm -f src-input/*.pyc tools/*.pyc util/*.pyc
	@rm -rf massif.out.* ms_print.tmp.*
	@rm -rf cachegrind.out.*
	@rm -rf callgrind.out.*
	@rm -rf oprofile_data/
	@rm -f /tmp/duk_sizes.html
	@rm -f /tmp/duk-test-eval-file-temp.js  # used by tests/api/test-eval-file.js
	@rm -rf /tmp/duktape-regfuzz/
	@rm -f /tmp/duk-test.log /tmp/duk-api-test.log
	@rm -f /tmp/duk-test262.log /tmp/duk-test262-filtered.log
	@rm -f /tmp/duk-emcc-test*
	@rm -f /tmp/duk-emcc-luatest*
	@rm -f /tmp/duk-emcc-duktest*
	@rm -f /tmp/duk-jsint-test*
	@rm -f /tmp/duk-luajs-mandel.js /tmp/duk-luajs-test.js
	@rm -f /tmp/duk-closure-test*
	@rm -f /tmp/duk-bluebird-test*
	@rm -f a.out
	@rm -rf test262-*
	@rm -rf lua-5.2.3
	@rm -rf luajs
	@rm -f dukweb.js
	@rm -rf /tmp/dukweb-test/
	@rm -f massif-*.out
	@rm -f literal_intern_test

.PHONY: cleanall
cleanall: clean
	# Don't delete these in 'clean' to avoid re-downloading them over and over
	@rm -rf duktape-releases
	@rm -f regfuzz-*.tar.gz
	@rm -rf linenoise
	@rm -rf UglifyJS
	@rm -rf UglifyJS2
	@rm -rf closure-compiler
	@rm -rf underscore
	@rm -rf lodash
	@rm -rf cbor-js
	@rm -f d067d2f0ca30.tar.bz2
	@rm -rf JS-Interpreter
	@rm -f compiler-latest.zip
	@rm -f compiler.jar
	@rm -f cloc-1.60.pl
	@rm -f lua-5.2.3.tar.gz
	@rm -f luajs.zip
	@rm -f bluebird.js
	@rm -f jquery-1.11.*.js
	@rm -rf coffee-script
	@rm -rf LiveScript
	@rm -rf coco
	@rm -rf sax-js
	@rm -rf xmldoc
	@rm -rf FlameGraph
	@rm -rf dtrace4linux
	@rm -rf flow
	@rm -rf lz-string
	@rm -rf 3883a2e9063b0a5f2705bdac3263577a03913c94.zip
	@rm -rf es5-tests.zip
	@rm -f v1.3.5.tar.gz
	@rm -f "references/ECMA-262 5th edition December 2009.pdf"
	@rm -f "references/ECMA-262 5.1 edition June 2011.pdf"
	@rm -f "references/ECMA-262.pdf"
	@rm -f citylots.json

# Targets for preparing different Duktape configurations.
prep:
	@mkdir prep
prep/nondebug: prep
	@rm -rf ./prep/nondebug
	$(PYTHON) tools/configure.py --output-directory ./prep/nondebug --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG) --line-directives
prep/nondebug-scanbuild: prep
	@rm -rf ./prep/nondebug-scanbuild
	$(PYTHON) tools/configure.py --output-directory ./prep/nondebug-scanbuild --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_SCANBUILD) --separate-sources --line-directives
prep/nondebug-perf: prep
	@rm -rf ./prep/nondebug-perf
	$(PYTHON) tools/configure.py --output-directory ./prep/nondebug-perf --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_PERF) --line-directives
prep/nondebug-size: prep
	@rm -rf ./prep/nondebug-size
	$(PYTHON) tools/configure.py --output-directory ./prep/nondebug-size --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_SIZE) --line-directives
prep/nondebug-rom: prep
	@rm -rf ./prep/nondebug-rom
	$(PYTHON) tools/configure.py --output-directory ./prep/nondebug-rom --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_ROM) --line-directives
prep/debug: prep
	@rm -rf ./prep/debug
	$(PYTHON) tools/configure.py --output-directory ./prep/debug --source-directory src-input --config-metadata config $(CONFIGOPTS_DEBUG) --line-directives
prep/debug-scanbuild: prep
	@rm -rf ./prep/debug-scanbuild
	$(PYTHON) tools/configure.py --output-directory ./prep/debug-scanbuild --source-directory src-input --config-metadata config $(CONFIGOPTS_DEBUG_SCANBUILD) --separate-sources --line-directives
prep/debug-rom: prep
	@rm -rf ./prep/debug-rom
	$(PYTHON) tools/configure.py --output-directory ./prep/debug-rom --source-directory src-input --config-metadata config $(CONFIGOPTS_DEBUG_ROM) --line-directives
prep/emduk: prep
	@rm -rf ./prep/emduk
	$(PYTHON) tools/configure.py --output-directory ./prep/emduk --source-directory src-input --config-metadata config $(CONFIGOPTS_EMDUK) --line-directives
prep/dukweb: prep
	@rm -rf ./prep/dukweb
	$(PYTHON) tools/configure.py --output-directory ./prep/dukweb --source-directory src-input --config-metadata config $(CONFIGOPTS_DUKWEB) --line-directives
prep/duklow-nondebug: prep
	@rm -rf ./prep/duklow-nondebug
	$(PYTHON) tools/configure.py --output-directory ./prep/duklow-nondebug --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_DUKLOW) --line-directives
prep/duklow-debug: prep
	@rm -rf ./prep/duklow-debug
	$(PYTHON) tools/configure.py --output-directory ./prep/duklow-debug --source-directory src-input --config-metadata config $(CONFIGOPTS_DEBUG_DUKLOW) --line-directives
prep/duklow-nondebug-rom: prep
	@rm -rf ./prep/duklow-nondebug-rom
	$(PYTHON) tools/configure.py --output-directory ./prep/duklow-nondebug-rom --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_DUKLOW_ROM) --line-directives
prep/duklow-debug-rom: prep
	@rm -rf ./prep/duklow-debug-rom
	$(PYTHON) tools/configure.py --output-directory ./prep/duklow-debug-rom --source-directory src-input --config-metadata config $(CONFIGOPTS_DEBUG_DUKLOW_ROM) --line-directives
prep/duklow-nondebug-norefc: prep
	@rm -rf ./prep/duklow-nondebug-norefc
	$(PYTHON) tools/configure.py --output-directory ./prep/duklow-nondebug-norefc --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_DUKLOW_NOREFC) --line-directives
prep/duklow-debug-norefc: prep
	@rm -rf ./prep/duklow-debug-norefc
	$(PYTHON) tools/configure.py --output-directory ./prep/duklow-debug-norefc --source-directory src-input --config-metadata config $(CONFIGOPTS_DEBUG_DUKLOW_NOREFC) --line-directives

# Library targets.
libduktape.so.1.0.0: prep/nondebug
	rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC -I./prep/nondebug $(CCOPTS_NONDEBUG) prep/nondebug/duktape.c $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)
libduktaped.so.1.0.0: prep/debug
	rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC -I./prep/debug $(CCOPTS_DEBUG) prep/debug/duktape.c $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

# Various 'duk' command line tool targets.
duk: linenoise prep/nondebug
	$(CC) -o $@ -Iprep/nondebug $(CCOPTS_NONDEBUG) prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-pgo:
	@echo "Compiling with -fprofile-generate..."
	@rm -f *.gcda
	$(CC) -o $@ -Iprep/nondebug $(CCOPTS_NONDEBUG) -fprofile-generate prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
	@echo "Generating profile..."
	@echo "XXX: profile is pretty dummy now, some benchmark run would be much better"
	./$@ $(PGO_TEST_SET)
	@rm -f $@
	$(CC) -o $@ -Iprep/nondebug $(CCOPTS_NONDEBUG) -fprofile-use prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@echo "Recompiling with -fprofile-use..."
duk-perf: linenoise prep/nondebug-perf
	$(CC) -o $@ -Iprep/nondebug-perf $(CCOPTS_NONDEBUG) prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-size: linenoise prep/nondebug-size
	$(CC) -o $@ -Iprep/nondebug-size $(CCOPTS_NONDEBUG) prep/nondebug-size/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-rom: linenoise prep/nondebug-rom
	$(CC) -o $@ -Iprep/nondebug-rom $(CCOPTS_NONDEBUG) prep/nondebug-rom/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
dukd: linenoise prep/debug
	$(CC) -o $@ -Iprep/debug $(CCOPTS_DEBUG) prep/debug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
dukd-rom: linenoise prep/debug-rom
	$(CC) -o $@ -Iprep/debug-rom $(CCOPTS_DEBUG) prep/debug-rom/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk.O2: linenoise prep/nondebug
	$(CC) -o $@ -Iprep/nondebug $(CCOPTS_NONDEBUG) -O2 prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-pgo.O2: linenoise prep/nondebug
	@echo "Compiling with -fprofile-generate..."
	@rm -f *.gcda
	$(CC) -o $@ -Iprep/nondebug $(CCOPTS_NONDEBUG) -O2 -fprofile-generate prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
	@echo "Generating profile..."
	@echo "XXX: profile is pretty dummy now, some benchmark run would be much better"
	./$@ $(PGO_TEST_SET)
	@rm -f $@
	@echo "Recompiling with -fprofile-use..."
	$(CC) -o $@ -Iprep/nondebug $(CCOPTS_NONDEBUG) -O2 -fprofile-use prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-perf.O2: linenoise prep/nondebug-perf
	$(CC) -o $@ -Iprep/nondebug-perf $(CCOPTS_NONDEBUG) -O2 prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-perf-pgo.O2: linenoise prep/nondebug-perf
	@echo "Compiling with -fprofile-generate..."
	@rm -f *.gcda
	$(CC) -o $@ -Iprep/nondebug-perf $(CCOPTS_NONDEBUG) -O2 -fprofile-generate prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
	@echo "Generating profile..."
	@echo "XXX: profile is pretty dummy now, some benchmark run would be much better"
	./$@ $(PGO_TEST_SET)
	@rm -f $@
	@echo "Recompiling with -fprofile-use..."
	$(CC) -o $@ -Iprep/nondebug-perf $(CCOPTS_NONDEBUG) -O2 -fprofile-use prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
duk.O3: linenoise prep/nondebug
	$(CC) -o $@ -Iprep/nondebug $(CCOPTS_NONDEBUG) -O3 prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-perf.O3: linenoise prep/nondebug-perf
	$(CC) -o $@ -Iprep/nondebug-perf $(CCOPTS_NONDEBUG) -O3 prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk.O4: linenoise prep/nondebug
	$(CC) -o $@ -Iprep/nondebug $(CCOPTS_NONDEBUG) -O4 prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-perf.O4: linenoise prep/nondebug-perf
	$(CC) -o $@ -Iprep/nondebug-perf $(CCOPTS_NONDEBUG) -O4 prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-clang: linenoise prep/nondebug
	# Use -Wcast-align to trigger issues like: https://github.com/svaarala/duktape/issues/270
	# Use -Wshift-sign-overflow to trigger issues like: https://github.com/svaarala/duktape/issues/812
	# -Weverything
	clang -o $@ -Wcast-align -Wshift-sign-overflow -Iprep/nondebug $(CLANG_CCOPTS_NONDEBUG) prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-sanitize-clang: linenoise prep/nondebug
	clang -o $@ -Wcast-align -Wshift-sign-overflow -fsanitize=undefined -Iprep/nondebug $(CLANG_CCOPTS_NONDEBUG) prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-perf-clang: linenoise prep/nondebug-perf
	clang -o $@ -Wcast-align -Wshift-sign-overflow -Iprep/nondebug-perf $(CLANG_CCOPTS_NONDEBUG) prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-g++: linenoise prep/nondebug
	$(GXX) -o $@ -Iprep/nondebug $(GXXOPTS_NONDEBUG) prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-perf-g++: linenoise prep/nondebug-perf
	$(GXX) -o $@ -Iprep/nondebug-perf $(GXXOPTS_NONDEBUG) prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
dukd-g++: linenoise prep/debug
	$(GXX) -o $@ -Iprep/debug $(GXXOPTS_DEBUG) prep/debug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
.PHONY: dukscanbuild
dukscanbuild: prep/nondebug-scanbuild
	scan-build gcc -o/tmp/duk.scanbuild -Iprep/nondebug-scanbuild $(CCOPTS_NONDEBUG) prep/nondebug-scanbuild/*.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
.PHONY: dukdscanbuild
dukdscanbuild: prep/debug-scanbuild
	scan-build gcc -o/tmp/dukd.scanbuild -Iprep/debug-scanbuild $(CCOPTS_DEBUG) prep/debug-scanbuild/*.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
# Command line with a simple pool allocator, for low memory testing.
# The pool sizes only make sense with -m32, so force that.  This forces
# us to use barebones cmdline too.
duk-low: linenoise prep/duklow-nondebug
	$(CC) -o $@ \
		-Iextras/alloc-pool/ -Iprep/duklow-nondebug \
		$(CCOPTS_NONDEBUG) $(CCOPTS_DUKLOW) \
		prep/duklow-nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) \
		examples/cmdline/duk_cmdline_lowmem.c \
		extras/alloc-pool/duk_alloc_pool.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l $@
	-@size $@
dukd-low: linenoise prep/duklow-debug
	$(CC) -o $@ \
		-Iextras/alloc-pool/ -Iprep/duklow-debug \
		$(CCOPTS_DEBUG) $(CCOPTS_DUKLOW) \
		prep/duklow-debug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) \
		examples/cmdline/duk_cmdline_lowmem.c \
		extras/alloc-pool/duk_alloc_pool.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l $@
	-@size $@
duk-low-rom: linenoise prep/duklow-nondebug-rom
	$(CC) -o $@ \
		-Iextras/alloc-pool/ -Iprep/duklow-nondebug-rom \
		$(CCOPTS_NONDEBUG) $(CCOPTS_DUKLOW) \
		prep/duklow-nondebug-rom/duktape.c $(DUKTAPE_CMDLINE_SOURCES) \
		examples/cmdline/duk_cmdline_lowmem.c \
		extras/alloc-pool/duk_alloc_pool.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l $@
	-@size $@
dukd-low-rom: linenoise prep/duklow-debug-rom
	$(CC) -o $@ \
		-Iextras/alloc-pool/ -Iprep/duklow-debug-rom \
		$(CCOPTS_DEBUG) $(CCOPTS_DUKLOW) \
		prep/duklow-debug-rom/duktape.c $(DUKTAPE_CMDLINE_SOURCES) \
		examples/cmdline/duk_cmdline_lowmem.c \
		extras/alloc-pool/duk_alloc_pool.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l $@
	-@size $@
duk-low-norefc: linenoise prep/duklow-nondebug-norefc
	$(CC) -o $@ \
		-Iextras/alloc-pool/ -Iprep/duklow-nondebug-norefc \
		$(CCOPTS_NONDEBUG) $(CCOPTS_DUKLOW) \
		prep/duklow-nondebug-norefc/duktape.c $(DUKTAPE_CMDLINE_SOURCES) \
		examples/cmdline/duk_cmdline_lowmem.c \
		extras/alloc-pool/duk_alloc_pool.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l $@
	-@size $@
dukd-low-norefc: linenoise prep/duklow-debug-norefc
	$(CC) -o $@ \
		-Iextras/alloc-pool/ -Iprep/duklow-debug-norefc \
		$(CCOPTS_DEBUG) $(CCOPTS_DUKLOW) \
		prep/duklow-debug-norefc/duktape.c $(DUKTAPE_CMDLINE_SOURCES) \
		examples/cmdline/duk_cmdline_lowmem.c \
		extras/alloc-pool/duk_alloc_pool.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l $@
	-@size $@
# util/fix_emscripten.py is used so that emduk.js can also be executed using
# Duktape itself (though you can't currently pass arguments/files to it).
# No Emscripten fixes are needed in practice since Duktape 1.5.0.
emduk: emduk.js
	cat util/emduk_wrapper.sh | sed "s|WORKDIR|$(shell pwd)|" > $@
	chmod ugo+x $@
emduk.js: prep/emduk
	$(EMCC) $(EMCCOPTS) -Iprep/emduk -Iexamples/cmdline -Iextras/print-alert \
		$(EMDUKOPTS) \
		prep/emduk/duktape.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c \
		-o /tmp/duk-emduk.js
	cat /tmp/duk-emduk.js | $(PYTHON) util/fix_emscripten.py > $@
	@ls -l $@
# This is a prototype of running Duktape in a web environment with Emscripten,
# and providing an eval() facility from both sides.  This is a placeholder now
# and doesn't do anything useful yet.
dukweb.js: prep/dukweb
	$(EMCC) $(EMCCOPTS_DUKVM) $(EMCCOPTS_DUKWEB_EXPORT) --post-js dukweb/dukweb_extra.js \
		-Iprep/dukweb prep/dukweb/duktape.c dukweb/dukweb.c -o dukweb.js
	@wc dukweb.js
literal_intern_test: prep/nondebug misc/literal_intern_test.c
	$(CC) -o $@ -std=c99 -O2 -fstrict-aliasing -Wall -Wextra \
		-Iprep/nondebug prep/nondebug/duktape.c misc/literal_intern_test.c -lm
literalinterntest: literal_intern_test
	bash -c 'for i in 0 1 2 3 10 11 12 13 20 21 22 23; do echo; echo "*** $$i ***"; echo; for j in 1 2 3 4 5; do time ./literal_intern_test $$i; sleep 10; done; done'

# Miscellaneous dumps.
.PHONY: dump-public
dump-public: duk
	@(objdump -t $< | grep ' g' | grep .text | grep -v .hidden | tr -s ' ' | cut -d ' ' -f 5 | sort > /tmp/duk-public.txt ; true)
	@echo "Symbol dump in /tmp/duk-public.txt"
	@(grep duk__ /tmp/duk-public.txt ; true)  # check for leaked file local symbols (does not cover internal, but not public symbols)
.PHONY: duksizes
duksizes: duk
	$(PYTHON) util/genexesizereport.py $< > /tmp/duk_sizes.html
.PHONY: issuecount
issuecount:
	@echo "FIXME:     `grep FIXME: src-input/*.c src-input/*.h src-input/*.in | wc -l | tr -d ' '`"
	@echo "XXX:       `grep XXX: src-input/*.c src-input/*.h src-input/*.in | wc -l | tr -d ' '`"
	@echo "TODO:      `grep TODO: src-input/*.c src-input/*.h src-input/*.in | wc -l | tr -d ' '`"
	@echo "NOTE:      `grep NOTE: src-input/*.c src-input/*.h src-input/*.in | wc -l | tr -d ' '`"
	@echo "SCANBUILD: `grep SCANBUILD: src-input/*.c src-input/*.h src-input/*.in | wc -l | tr -d ' '`"
cloc: dist cloc-1.60.pl
	@echo "CLOC report on combined duktape.c source file"
	@perl cloc-1.60.pl --quiet dist/src/duktape.c
.PHONY: gccpredefs
gccpredefs:
	gcc -dM -E - < /dev/null
.PHONY: clangpredefs
clangpredefs:
	clang -dM -E - < /dev/null
.PHONY: big-git-files
big-git-files:
	util/find_big_git_files.sh
.PHONY: checkalign
checkalign:
	@echo "checkalign for: `uname -a`"
	gcc -o /tmp/check_align -Wall -Wextra util/check_align.c
	@cp util/check_align.sh /tmp
	@cd /tmp; sh check_align.sh

# Overall quick test target.
.PHONY: test
test: apitest ecmatest
	@echo ""
	@echo "### Tests successful!"

# Set of miscellaneous tests for release.
.PHONY: releasetest
releasetest: configuretest xmldoctest closuretest bluebirdtest luajstest jsinterpretertest lodashtest underscoretest emscriptenluatest emscriptenduktest emscriptenmandeltest emscriptentest errorinjecttest
	@echo ""
	@echo "### Release tests successful!"  # These tests now have output checks.

# Runtests-based ECMAScript and API tests.
.PHONY: runtestsdeps
runtestsdeps: runtests/node_modules UglifyJS2
runtests/node_modules:
	@echo "Installing required NodeJS modules for runtests"
	@cd runtests; npm install
.PHONY: ecmatest
ecmatest: runtestsdeps duk
	@echo "### ecmatest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/duk --num-threads 4 --log-file=/tmp/duk-test.log tests/ecmascript/
.PHONY: qecmatest
qecmatest: ecmatest
.PHONY: ecmatest-comparison
ecmatest-comparison: runtestsdeps duk
	@echo "### ecmatest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/duk --report-diff-to-other --run-nodejs --run-rhino --num-threads 4 --log-file=/tmp/duk-test.log tests/ecmascript/
.PHONY: apiprep
apiprep: runtestsdeps libduktape.so.1.0.0
.PHONY: apitest
apitest: apiprep
	@echo "### apitest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --num-threads 1 --log-file=/tmp/duk-api-test.log tests/api/

# Configure.py test.
configuretest:
	@echo "### configuretest"
	bash tests/configure/test_minimal.sh

# Dukweb.js test.
.PHONY: dukwebtest
dukwebtest: dukweb.js jquery-1.11.2.js
	@echo "### dukwebtest"
	@rm -rf /tmp/dukweb-test/
	mkdir /tmp/dukweb-test/
	cp dukweb.js jquery-1.11.2.js dukweb/dukweb.html dukweb/dukweb.css /tmp/dukweb-test/
	@echo "Now point your browser to: file:///tmp/dukweb-test/dukweb.html"

# Third party tests.
.PHONY: underscoretest
underscoretest: underscore duk
	@echo "### underscoretest"
	@echo "Run underscore tests with underscore-test-shim.js"
	-util/underscore_test.sh ./duk underscore/test/arrays.js
	-util/underscore_test.sh ./duk underscore/test/chaining.js
	-util/underscore_test.sh ./duk underscore/test/collections.js
	-util/underscore_test.sh ./duk underscore/test/functions.js
	-util/underscore_test.sh ./duk underscore/test/objects.js
	# speed test disabled, requires JSLitmus
	#-util/underscore_test.sh ./duk underscore/test/speed.js
	-util/underscore_test.sh ./duk underscore/test/utility.js
.PHONY: regfuzztest
regfuzztest: regfuzz-0.1.tar.gz duk
	@echo "### regfuzztest"
	# Spidermonkey test is pretty close, just lacks 'arguments'
	# Should run with assertions enabled in 'duk'
	rm -rf /tmp/duktape-regfuzz; mkdir -p /tmp/duktape-regfuzz
	cp regfuzz-0.1.tar.gz duk /tmp/duktape-regfuzz
	tar -C /tmp/duktape-regfuzz -x -v -z -f regfuzz-0.1.tar.gz
	echo "arguments = [ 0xdeadbeef ];" > /tmp/duktape-regfuzz/regfuzz-test.js
	cat /tmp/duktape-regfuzz/regfuzz-0.1/examples/spidermonkey/regexfuzz.js >> /tmp/duktape-regfuzz/regfuzz-test.js
	cd /tmp/duktape-regfuzz; ./duk regfuzz-test.js
# Lodash test.js assumes require() etc.  Placeholder test for now, no
# expect string etc.
.PHONY: lodashtest
lodashtest: lodash duk
	./duk lodash/lodash.js tests/lodash/basic.js | tee /tmp/duk-lodash-test.out
	if [ `md5sum /tmp/duk-lodash-test.out | cut -f 1 -d ' '` != "318977a4e39deb7c97c87b9b55ea9a80" ]; then false; fi
.PHONY: test262test
test262test: test262-es5-tests duk
	@echo "### test262test"
	# http://wiki.ecmascript.org/doku.php?id=test262:command
	rm -f /tmp/duk-test262.log /tmp/duk-test262-filtered.log
	-cd $<; $(PYTHON) tools/packaging/test262.py --command "../duk {{path}}" --summary >/tmp/duk-test262.log
	cat /tmp/duk-test262.log | $(PYTHON) util/filter_test262_log.py doc/test262-known-issues.yaml > /tmp/duk-test262-filtered.log
	cat /tmp/duk-test262-filtered.log
# Awkward helper to write out a testcase, the awkwardness is that it
# reads command line arguments and complains about missing targets etc:
# http://stackoverflow.com/questions/6273608/how-to-pass-argument-to-makefile-from-command-line
.PHONY: test262cat
test262cat: test262-es5-tests
	@echo "NOTE: this Makefile target will print a 'No rule...' error, ignore it" >&2
	-@cd $<; $(PYTHON) tools/packaging/test262.py --command "../duk {{path}}" --cat $(filter-out $@,$(MAKECMDGOALS))
.PHONY: emscriptentest
emscriptentest: duk
	@echo "### emscriptentest"
	@rm -f /tmp/duk-emcc-test*
	$(EMCC) $(EMCCOPTS) tests/emscripten/helloworld.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	#./duk /tmp/duk-emcc-test-fixed.js
	./duk /tmp/duk-emcc-test.js | tee /tmp/duk-emcc-test.out
	if [ `md5sum /tmp/duk-emcc-test.out | cut -f 1 -d ' '` != "59ca0efa9f5633cb0371bbc0355478d8" ]; then false; fi
.PHONY: emscriptenmandeltest
emscriptenmandeltest: duk
	@echo "### emscriptenmandeltest"
	@rm -f /tmp/duk-emcc-test*
	$(EMCC) $(EMCCOPTS) tests/emscripten/mandelbrot.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	#./duk /tmp/duk-emcc-test-fixed.js
	./duk /tmp/duk-emcc-test.js | tee /tmp/duk-emcc-test.out
	if [ `md5sum /tmp/duk-emcc-test.out | cut -f 1 -d ' '` != "a0b2daf2e979e192d9838d976920f213" ]; then false; fi
# Compile Duktape and hello.c using Emscripten and execute the result with
# Duktape.
.PHONY: emscripteninceptiontest
emscripteninceptiontest: prep/nondebug duk
	@echo "### emscripteninceptiontest"
	@rm -f /tmp/duk-emcc-test*
	$(EMCC) $(EMCCOPTS) -Iprep/nondebug prep/nondebug/duktape.c examples/hello/hello.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	#./duk /tmp/duk-emcc-test-fixed.js
	./duk /tmp/duk-emcc-test.js | tee /tmp/duk-emcc-test.out
	if [ `md5sum /tmp/duk-emcc-test.out | cut -f 1 -d ' '` != "8521f9d969cdc0a2fa26661a151cef04" ]; then false; fi
# Compile Duktape with Emscripten and execute it with NodeJS.
.PHONY: emscriptenduktest
emscriptenduktest: prep/emduk
	@echo "### emscriptenduktest"
	@rm -f /tmp/duk-emcc-duktest.js
	$(EMCC) $(EMCCOPTS_DUKVM) -Iprep/emduk prep/emduk/duktape.c examples/eval/eval.c -o /tmp/duk-emcc-duktest.js
	"$(NODE)" /tmp/duk-emcc-duktest.js \
		'print("Hello from Duktape running inside Emscripten/NodeJS");' \
		'for(i=0;i++<100;)print((i%3?"":"Fizz")+(i%5?"":"Buzz")||i)' | tee /tmp/duk-emcc-duktest-1.out
	if [ `md5sum /tmp/duk-emcc-duktest-1.out | cut -f 1 -d ' '` != "3c22acb0ec822d4c85f5d427e42826dc" ]; then false; fi
	"$(NODE)" /tmp/duk-emcc-duktest.js "eval(new Buffer(Duktape.dec('base64', '$(MAND_BASE64)')).toString())" | tee /tmp/duk-emcc-duktest-2.out
	if [ `md5sum /tmp/duk-emcc-duktest-2.out | cut -f 1 -d ' '` != "c78521c68b60065e6ed0652bebd7af0b" ]; then false; fi
LUASRC=	lapi.c lauxlib.c lbaselib.c lbitlib.c lcode.c lcorolib.c lctype.c \
	ldblib.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c liolib.c \
	llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c loslib.c \
	lparser.c lstate.c lstring.c lstrlib.c ltable.c ltablib.c ltm.c \
	lua.c lundump.c lvm.c lzio.c
# Compile Lua 5.2.3 with Emscripten and run it with Duktape.
.PHONY: emscriptenluatest
emscriptenluatest: duk lua-5.2.3
	@echo "### emscriptenluatest"
	@rm -f /tmp/duk-emcc-luatest*
	$(EMCC) $(EMCCOPTS) -Ilua-5.2.3/src/ $(patsubst %,lua-5.2.3/src/%,$(LUASRC)) -o /tmp/duk-emcc-luatest.js
	cat /tmp/duk-emcc-luatest.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-luatest-fixed.js
	@ls -l /tmp/duk-emcc-luatest*
	#./duk /tmp/duk-emcc-luatest-fixed.js
	./duk /tmp/duk-emcc-luatest.js | tee /tmp/duk-emcc-luatest.out
	if [ `md5sum /tmp/duk-emcc-luatest.out | cut -f 1 -d ' '` != "280db36b7805a00f887d559c1ba8285d" ]; then false; fi
.PHONY: jsinterpretertest
jsinterpretertest: JS-Interpreter duk
	@echo "### jsinterpretertest"
	@rm -f /tmp/duk-jsint-test*
	echo "window = {};" > /tmp/duk-jsint-test.js
	cat JS-Interpreter/acorn.js JS-Interpreter/interpreter.js >> /tmp/duk-jsint-test.js
	cat tests/jsinterpreter/addition.js >> /tmp/duk-jsint-test.js
	./duk /tmp/duk-jsint-test.js | tee /tmp/duk-jsint-test.out
	if [ `md5sum /tmp/duk-jsint-test.out | cut -f 1 -d ' '` != "9ae0ea9e3c9c6e1b9b6252c8395efdc1" ]; then false; fi
.PHONY: luajstest
luajstest: luajs duk
	@rm -f /tmp/duk-luajs-mandel.js /tmp/duk-luajs-test.js
	luajs/lua2js tests/luajs/mandel.lua /tmp/duk-luajs-mandel.js
	echo "console = { log: function() { print(Array.prototype.join.call(arguments, ' ')); } };" > /tmp/duk-luajs-test.js
	cat luajs/lua.js /tmp/duk-luajs-mandel.js >> /tmp/duk-luajs-test.js
	./duk /tmp/duk-luajs-test.js | tee /tmp/duk-luajs-test.out
	if [ `md5sum /tmp/duk-luajs-test.out | cut -f 1 -d ' '` != "a0b2daf2e979e192d9838d976920f213" ]; then false; fi
.PHONY: bluebirdtest
bluebirdtest: bluebird.js duk
	@rm -f /tmp/duk-bluebird-test.js
	cat util/bluebird-test-shim.js bluebird.js > /tmp/duk-bluebird-test.js
	echo "var myPromise = new Promise(function(resolve, reject) { setTimeout(function () { resolve('resolved 123') }, 1000); });" >> /tmp/duk-bluebird-test.js
	echo "myPromise.then(function (v) { print('then:', v); });" >> /tmp/duk-bluebird-test.js
	echo "fakeEventLoop();" >> /tmp/duk-bluebird-test.js
	./duk /tmp/duk-bluebird-test.js | tee /tmp/duk-bluebird-test.out
	if [ `md5sum /tmp/duk-bluebird-test.out | cut -f 1 -d ' '` != "6edf907604d970db7f6f4ca6991127db" ]; then false; fi
.PHONY: closuretest
closuretest: compiler.jar duk
	@echo "### closuretest"
	@rm -f /tmp/duk-closure-test*
	$(JAVA) -jar compiler.jar tests/ecmascript/test-dev-mandel2-func.js > /tmp/duk-closure-test.js
	./duk /tmp/duk-closure-test.js | tee /tmp/duk-closure-test.out
	if [ `md5sum /tmp/duk-closure-test.out | cut -f 1 -d ' '` != "a0b2daf2e979e192d9838d976920f213" ]; then false; fi
.PHONY: xmldoctest
xmldoctest: sax-js xmldoc duk
	@echo "### xmldoctest"
	@rm -f /tmp/duk-xmldoc-test*
	cat sax-js/lib/sax.js > /tmp/duk-xmldoc-test.js
	echo ";" >> /tmp/duk-xmldoc-test.js  # missing end semicolon causes automatic semicolon problem
	cat xmldoc/lib/xmldoc.js >> /tmp/duk-xmldoc-test.js
	echo ";" >> /tmp/duk-xmldoc-test.js  # missing end semicolon causes automatic semicolon problem
	cat tests/xmldoc/basic.js >> /tmp/duk-xmldoc-test.js
	./duk /tmp/duk-xmldoc-test.js | tee /tmp/duk-xmldoc-test.out
	if [ `md5sum /tmp/duk-xmldoc-test.out | cut -f 1 -d ' '` != "798cab55f8c62f3cf24f277a8192518a" ]; then false; fi
.PHONY: errorinjecttest
errorinjecttest:
	bash util/error_inject_test.sh
.PHONY: checklisttest
checklisttest:
	bash util/checklist_compile_test.sh

# Third party download/unpack targets, libraries etc.
linenoise:
	# git clone https://github.com/antirez/linenoise.git
	# Use forked repo to get compile warnings fixed.
	git clone -b fix-compile-warnings-duktape https://github.com/svaarala/linenoise.git
regfuzz-0.1.tar.gz:
	# https://code.google.com/p/regfuzz/
	# SHA1: 774be8e3dda75d095225ba699ac59969d92ac970
	$(WGET) https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/regfuzz/regfuzz-0.1.tar.gz -O $@
underscore:
	# http://underscorejs.org/
	# https://github.com/jashkenas/underscore
	# Master is OK because not a critical dependency
	$(GIT) clone --depth 1 https://github.com/jashkenas/underscore.git
lodash:
	# http://lodash.com/
	# https://github.com/lodash
	# Use pre-built .js file.
	mkdir lodash
	cd lodash && wget https://raw.githubusercontent.com/lodash/lodash/4.17.10-npm/lodash.js -O lodash.js
	#$(GIT) clone --depth 1 https://github.com/lodash/lodash.git
cbor-js:
	$(GIT) clone --depth 1 https://github.com/paroga/cbor-js.git
3883a2e9063b0a5f2705bdac3263577a03913c94.zip:
	# http://test262.ecmascript.org/
	# https://github.com/tc39/test262
	# HG repo seems to have migrated to https://github.com/tc39/test262
	#$(WGET) http://hg.ecmascript.org/tests/test262/archive/d067d2f0ca30.tar.bz2 -O $@
	#$(WGET) https://github.com/tc39/test262/archive/595a36b252ee97110724e6fa89fc92c9aa9a206a.zip -O $@
	# This is a snapshot from the master, and seems to have test case bugs
	$(WGET) https://github.com/tc39/test262/archive/3883a2e9063b0a5f2705bdac3263577a03913c94.zip -O $@
test262-3883a2e9063b0a5f2705bdac3263577a03913c94: 3883a2e9063b0a5f2705bdac3263577a03913c94.zip
	unzip -q $<
	touch $@
es5-tests.zip:
	# https://github.com/tc39/test262/tree/es5-tests
	# This is a stable branch for ES5 tests
	$(WGET) https://github.com/tc39/test262/archive/es5-tests.zip -O $@
test262-es5-tests: es5-tests.zip
	unzip -q $<
	touch $@
jquery-1.11.2.js:
	$(WGET) http://code.jquery.com/jquery-1.11.2.js -O $@
lua-5.2.3.tar.gz:
	$(WGET) http://www.lua.org/ftp/lua-5.2.3.tar.gz -O $@
lua-5.2.3: lua-5.2.3.tar.gz
	tar xfz lua-5.2.3.tar.gz
luajs.zip:
	# https://github.com/mherkender/lua.js
	$(WGET) https://github.com/mherkender/lua.js/raw/precompiled2/luajs.zip -O $@
luajs: luajs.zip
	@rm -rf luajs/
	mkdir luajs
	cd luajs; unzip ../luajs.zip
JS-Interpreter:
	# https://github.com/NeilFraser/JS-Interpreter
	# Master is OK because not a critical dependency
	$(GIT) clone --depth 1 https://github.com/NeilFraser/JS-Interpreter.git
bluebird.js:
	$(WGET) https://cdn.jsdelivr.net/bluebird/latest/bluebird.js -O $@
compiler-latest.zip:
	# Closure
	# Prebuilt latest version; this is not good as a build dependency
	# because closure changes may break minified initjs code and make
	# old builds unreliable.
	# https://code.google.com/p/closure-compiler/
	$(WGET) http://dl.google.com/closure-compiler/compiler-latest.zip -O $@
closure-compiler:
	# https://github.com/google/closure-compiler
	@rm -f v20140814.tar.gz
	$(WGET) https://github.com/google/closure-compiler/archive/v20140814.tar.gz -O v20140814.tar.gz
	tar xfz v20140814.tar.gz
	mv closure-compiler-20140814 closure-compiler
	@rm -f v20140814.tar.gz
closure-compiler/build/compiler.jar: closure-compiler
	cd closure-compiler; ant
compiler.jar: closure-compiler/build/compiler.jar
	cp closure-compiler/build/compiler.jar $@
	touch $@  # ensure date is newer than compiler-latest.zip
UglifyJS:
	# https://github.com/mishoo/UglifyJS
	# Use a specific release because UglifyJS is used in building Duktape
	# Don't use this because it's a moving critical dependency
	#$(GIT) clone --depth 1 https://github.com/mishoo/UglifyJS.git
	@rm -f v1.3.5.tar.gz
	$(WGET) https://github.com/mishoo/UglifyJS/archive/v1.3.5.tar.gz -O v1.3.5.tar.gz
	tar xfz v1.3.5.tar.gz
	mv UglifyJS-1.3.5 UglifyJS
	@rm -f v1.3.5.tar.gz
UglifyJS2:
	# https://github.com/mishoo/UglifyJS2
	# Use a specific release because UglifyJS2 is used in building Duktape
	# (This is now a bit futile because UglifyJS2 requires an 'npm install',
	# the NodeJS dependencies need to be controlled for this to really work.)
	# Don't use this because it's a moving critical dependency
	#$(GIT) clone --depth 1 https://github.com/mishoo/UglifyJS2.git
	@rm -f v2.4.12.tar.gz
	$(WGET) https://github.com/mishoo/UglifyJS2/archive/v2.4.12.tar.gz -O v2.4.12.tar.gz
	tar xfz v2.4.12.tar.gz
	mv UglifyJS2-2.4.12 UglifyJS2
	@rm -f v2.4.12.tar.gz
	cd UglifyJS2; npm install; cd -
cloc-1.60.pl:
	# http://cloc.sourceforge.net/
	$(WGET) http://downloads.sourceforge.net/project/cloc/cloc/v1.60/cloc-1.60.pl -O $@
coffee-script:
	# http://coffeescript.org/
	# https://github.com/jashkenas/coffee-script
	$(GIT) clone --depth 1 https://github.com/jashkenas/coffee-script.git
LiveScript:
	# http://livescript.net/
	# https://github.com/gkz/LiveScript
	$(GIT) clone --depth 1 https://github.com/gkz/LiveScript.git
coco:
	# https://github.com/satyr/coco
	$(GIT) clone --depth 1 https://github.com/satyr/coco
sax-js:
	# https://github.com/isaacs/sax-js
	$(GIT) clone --depth 1 https://github.com/isaacs/sax-js.git
xmldoc:
	# https://github.com/nfarina/xmldoc
	# http://nfarina.com/post/34302964969/a-lightweight-xml-document-class-for-nodejs-javascript
	$(GIT) clone --depth 1 https://github.com/nfarina/xmldoc.git 
FlameGraph:
	# http://www.brendangregg.com/FlameGraphs/cpuflamegraphs.html
	# https://github.com/brendangregg/FlameGraph
	$(GIT) clone --depth 1 https://github.com/brendangregg/FlameGraph.git
dtrace4linux:
	# https://github.com/dtrace4linux/linux
	# http://crtags.blogspot.fi/
	$(GIT) clone --depth 1 https://github.com/dtrace4linux/linux.git dtrace4linux
flow:
	# https://github.com/facebook/flow
	$(GIT) clone --depth 1 https://github.com/facebook/flow.git
lz-string:
	# https://github.com/pieroxy/lz-string.git
	$(GIT) clone --depth 1 https://github.com/pieroxy/lz-string.git
citylots.json:
	$(WGET) https://github.com/zemirco/sf-city-lots-json/raw/master/citylots.json -O $@

# Duktape binary releases are in a separate repo.
duktape-releases:
	$(GIT) clone https://github.com/svaarala/duktape-releases.git

# Reference documents.
references/ECMA-262\ 5th\ edition\ December\ 2009.pdf:
	$(WGET) "http://www.ecma-international.org/publications/files/ECMA-ST-ARCH/ECMA-262%205th%20edition%20December%202009.pdf" -O "$@"
references/ECMA-262\ 5.1\ edition\ June\ 2011.pdf:
	$(WGET) "http://www.ecma-international.org/publications/files/ECMA-ST-ARCH/ECMA-262%205.1%20edition%20June%202011.pdf" -O "$@"
references/ECMA-262.pdf:
	$(WGET) "http://www.ecma-international.org/ecma-262/6.0/ECMA-262.pdf" -O "$@"
.PHONY: refs
refs: references/ECMA-262\ 5th\ edition\ December\ 2009.pdf \
	references/ECMA-262\ 5.1\ edition\ June\ 2011.pdf \
	references/ECMA-262.pdf

# Documentation.
.PHONY: doc
doc: $(patsubst %.txt,%.html,$(wildcard doc/*.txt))
doc/%.html: doc/%.txt
	rst2html $< $@

# Source distributable for end users.
dist:
	@make codepolicycheck
	$(PYTHON) util/dist.py
.PHONY: dist-src
dist-src: dist
	rm -rf duktape-$(DUK_VERSION_FORMATTED)
	rm -rf duktape-$(DUK_VERSION_FORMATTED).tar*
	mkdir duktape-$(DUK_VERSION_FORMATTED)
	cp -r dist/* duktape-$(DUK_VERSION_FORMATTED)/
	tar cvfz duktape-$(DUK_VERSION_FORMATTED).tar.gz duktape-$(DUK_VERSION_FORMATTED)/
	tar cvf duktape-$(DUK_VERSION_FORMATTED).tar duktape-$(DUK_VERSION_FORMATTED)/
	xz -z -e -9 duktape-$(DUK_VERSION_FORMATTED).tar
	cp duktape-$(DUK_VERSION_FORMATTED).tar.gz duktape-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).tar.gz
	cp duktape-$(DUK_VERSION_FORMATTED).tar.xz duktape-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).tar.xz
	rm -rf duktape-$(DUK_VERSION_FORMATTED)
# ISO target is useful with some system emulators with no network access.
.PHONY: dist-iso
dist-iso: dist-src
	mkisofs -input-charset utf-8 -o duktape-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).iso duktape-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).tar.gz

# Website targets.
.PHONY: tidy-site
tidy-site:
	for i in website/*/*.html; do echo "*** Checking $$i"; tidy -q -e -xml $$i; done
site: duktape-releases dukweb.js jquery-1.11.2.js lz-string
	rm -rf site
	mkdir site
	-cd duktape-releases/; git pull --rebase  # get binaries up-to-date, but allow errors for offline use
	cd website/; $(PYTHON) buildsite.py ../site/
	for i in site/*.html; do echo "tidy checking $$i"; tidy -q -e -xml -utf8 $$i; done
	@rm -rf /tmp/site/
	cp -r site /tmp/  # useful for quick preview
.PHONY: dist-site
dist-site: tidy-site site
	# When doing a final dist build, run html tidy
	# Also pull binaries up-to-date
	cd duktape-releases/; git pull --rebase  # get binaries up-to-date
	rm -rf duktape-site-$(DUK_VERSION_FORMATTED)
	rm -rf duktape-site-$(DUK_VERSION_FORMATTED).tar*
	mkdir duktape-site-$(DUK_VERSION_FORMATTED)
	cp -r site/* duktape-site-$(DUK_VERSION_FORMATTED)/
	tar cvf duktape-site-$(DUK_VERSION_FORMATTED).tar duktape-site-$(DUK_VERSION_FORMATTED)/
	xz -z -e -9 duktape-site-$(DUK_VERSION_FORMATTED).tar
	cp duktape-site-$(DUK_VERSION_FORMATTED).tar.xz duktape-site-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).tar.xz
	rm -rf duktape-site-$(DUK_VERSION_FORMATTED)

# Code policy check.
ifeq ($(TRAVIS),1)
CODEPOLICYOPTS=--fail-on-errors
else
CODEPOLICYOPTS=
endif
.PHONY: codepolicycheck
codepolicycheck:
	@echo Code policy check
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-debug-log-calls \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-no-symbol-visibility \
		--check-rejected-identifiers \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--check-cpp-comment \
		--check-float-compare \
		--check-ifdef-ifndef \
		--check-longlong-constants \
		--dump-vim-commands \
		src-input/*.c src-input/*.h src-input/*.h.in tests/api/*.c
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-tab-indent \
		--dump-vim-commands \
		src-input/*.py tools/*.py util/*.py debugger/*/*.py examples/*/*.py testrunner/*.py tests/perf/*.py
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-debug-log-calls \
		--check-carriage-returns \
		--check-fixme \
		--check-no-symbol-visibility \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-tab-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		tests/ecmascript/*.js
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--check-cpp-comment \
		--check-ifdef-ifndef \
		--dump-vim-commands \
		examples/*/*.c examples/*/*.h \
		extras/*/*.c extras/*/*.h
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--check-ifdef-ifndef \
		--dump-vim-commands \
		config/architectures/* config/compilers/* config/platforms/* \
		config/feature-options/*.yaml \
		config/examples/* config/header-snippets/* config/helper-snippets/* \
		config/*.yaml
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		config/config-options/*.yaml
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		debugger/*.yaml
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		website/api/*.yaml website/api/*.html
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--dump-vim-commands \
		doc/*.rst
.PHONY: codepolicycheckvim
codepolicycheckvim:
	-$(PYTHON) util/check_code_policy.py --dump-vim-commands src-input/*.c src-input/*.h src-input/*.h.in tests/api/*.c

# Simple heap graph and peak usage using valgrind --tool=massif, for quick
# and dirty baseline comparison.  Say e.g. 'make massif-test-dev-hello-world'.
# The target name is intentionally not 'massif-%.out' so that the rule is never
# satisfied and can be executed multiple times without cleaning.
# Grep/sed hacks from:
# http://stackoverflow.com/questions/774556/peak-memory-usage-of-a-linux-unix-process
massif-%: tests/ecmascript/%.js duk
	@rm -f $(@).out
	valgrind --tool=massif --peak-inaccuracy=0.0 --massif-out-file=$(@).out ./duk $< >/dev/null 2>/dev/null
	@ms_print $(@).out | head -35
	@echo "[... clipped... ]"
	@echo ""
	@echo -n "MAXIMUM: "
	@cat $(@).out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | sort -g | tail -n 1
# Convenience targets
massif-helloworld: massif-test-dev-hello-world
massif-deepmerge: massif-test-dev-deepmerge
massif-arcfour: massif-test-dev-arcfour

# Docker targets for building images and running specific targets in a
# docker container for easier reproducibility.  Creating the images
# initially takes a long time.
.PHONY: docker-prepare
docker-prepare:
	cd docker && for subdir in duktape-*; do \
		if [ -f ~/.gitconfig ]; then cp ~/.gitconfig $$subdir/gitconfig; else touch docker/$$subdir/gitconfig; fi; \
		cp prepare_repo.sh $$subdir/; \
	done

.PHONY: docker-images-x64
docker-images-x64: docker-prepare
	docker build --build-arg UID=$(shell id -u) --build-arg GID=$(shell id -g) -t duktape-base-ubuntu-18.04-x64 docker/duktape-base-ubuntu-18.04-x64
	docker build -t duktape-dist-ubuntu-18.04-x64 docker/duktape-dist-ubuntu-18.04-x64
	docker build -t duktape-site-ubuntu-18.04-x64 docker/duktape-site-ubuntu-18.04-x64
	docker build -t duktape-duk-ubuntu-18.04-x64 docker/duktape-duk-ubuntu-18.04-x64
	docker build -t duktape-shell-ubuntu-18.04-x64 docker/duktape-shell-ubuntu-18.04-x64
	docker build -t duktape-release-1-ubuntu-18.04-x64 docker/duktape-release-1-ubuntu-18.04-x64

.PHONY: docker-images-s390x
docker-images-s390x: docker-prepare
	docker build --build-arg UID=$(shell id -u) --build-arg GID=$(shell id -g) -t duktape-base-ubuntu-18.04-s390x docker/duktape-base-ubuntu-18.04-s390x
	docker build -t duktape-shell-ubuntu-18.04-s390x docker/duktape-shell-ubuntu-18.04-s390x

.PHONY: docker-images
docker-images: docker-images-x64

.PHONY: docker-clean
docker-clean:
	-rm -f docker/*/gitconfig docker/*/prepare_repo.sh
	-docker rmi duktape-release-1-ubuntu-18.04-x64:latest
	-docker rmi duktape-shell-ubuntu-18.04-x64:latest
	-docker rmi duktape-duk-ubuntu-18.04-x64:latest
	-docker rmi duktape-site-ubuntu-18.04-x64:latest
	-docker rmi duktape-dist-ubuntu-18.04-x64:latest
	-docker rmi duktape-base-ubuntu-18.04-x64:latest
	-docker rmi duktape-shell-ubuntu-18.04-s390x:latest
	-docker rmi duktape-base-ubuntu-18.04-s390x:latest
	@echo ""
	@echo "Now run 'docker system prune' to free disk space."

.PHONY: docker-dist-src-master
docker-dist-src-master:
	rm -f docker-input.zip docker-output.zip
	docker run --rm -i duktape-dist-ubuntu-18.04-x64 > docker-output.zip
	unzip -t docker-output.zip ; true  # avoid failure due to leading garbage

.PHONY: docker-dist-src-wd
docker-dist-src-wd:
	rm -f docker-input.zip docker-output.zip
	#git archive --format zip --output docker-input.zip HEAD
	zip -1 -q -r docker-input.zip .
	docker run --rm -i -e STDIN_ZIP=1 duktape-dist-ubuntu-18.04-x64 < docker-input.zip > docker-output.zip
	unzip -t docker-output.zip ; true  # avoid failure due to leading garbage

.PHONY: docker-dist-site-master
docker-dist-site-master:
	rm -f docker-input.zip docker-output.zip
	docker run --rm -i duktape-site-ubuntu-18.04-x64 > docker-output.zip
	unzip -t docker-output.zip ; true  # avoid failure due to leading garbage

.PHONY: docker-dist-site-wd
docker-dist-site-wd:
	rm -f docker-input.zip docker-output.zip
	#git archive --format zip --output docker-input.zip HEAD
	zip -1 -q -r docker-input.zip .
	docker run --rm -i -e STDIN_ZIP=1 duktape-site-ubuntu-18.04-x64 < docker-input.zip > docker-output.zip
	unzip -t docker-output.zip ; true  # avoid failure due to leading garbage

.PHONY: docker-duk-wd
docker-duk-wd:
	rm -f docker-input.zip docker-output.zip
	#git archive --format zip --output docker-input.zip HEAD
	zip -1 -q -r docker-input.zip .
	docker run --rm -i -e STDIN_ZIP=1 duktape-duk-ubuntu-18.04-x64 < docker-input.zip > docker-output.zip
	unzip -t docker-output.zip ; true  # avoid failure due to leading garbage
	unzip -o docker-output.zip ; true

.PHONY: docker-duk-master
docker-duk-master:
	rm -f docker-input.zip docker-output.zip
	docker run --rm -i duktape-duk-ubuntu-18.04-x64 > docker-output.zip
	unzip -t docker-output.zip ; true  # avoid failure due to leading garbage
	unzip -o docker-output.zip ; true

.PHONY: docker-shell-master
docker-shell-master:
	docker run --rm -ti duktape-shell-ubuntu-18.04-x64

.PHONY: docker-shell-wd
docker-shell-wd:
	docker run -v $(shell pwd):/work/duktape-host --rm -ti duktape-shell-ubuntu-18.04-x64

.PHONY: docker-shell-wdmount
docker-shell-wdmount:
	docker run -v $(shell pwd):/work/duktape --rm -ti duktape-shell-ubuntu-18.04-x64

.PHONY: docker-release-1-wd
docker-release-1-wd:
	rm -f docker-input.zip docker-output.zip
	#git archive --format zip --output docker-input.zip HEAD
	zip -1 -q -r docker-input.zip .
	docker run --rm -i -e STDIN_ZIP=1 duktape-release-1-ubuntu-18.04-x64 < docker-input.zip
