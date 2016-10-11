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
GIT:=$(shell command -v git 2>/dev/null)
NODE:=$(shell { command -v nodejs || command -v node; } 2>/dev/null)
WGET:=$(shell command -v wget 2>/dev/null)
JAVA:=$(shell command -v java 2>/dev/null)
VALGRIND:=$(shell command -v valgrind 2>/dev/null)
PYTHON:=$(shell { command -v python2 || command -v python; } 2>/dev/null)

# Scrape version from the public header; convert from e.g. 10203 -> '1.2.3'
DUK_VERSION:=$(shell cat src-input/duk_api_public.h.in | grep define | grep DUK_VERSION | tr -s ' ' ' ' | cut -d ' ' -f 3 | tr -d 'L')
DUK_MAJOR:=$(shell echo "$(DUK_VERSION) / 10000" | bc)
DUK_MINOR:=$(shell echo "$(DUK_VERSION) % 10000 / 100" | bc)
DUK_PATCH:=$(shell echo "$(DUK_VERSION) % 100" | bc)
DUK_VERSION_FORMATTED:=$(DUK_MAJOR).$(DUK_MINOR).$(DUK_PATCH)
GIT_BRANCH:=$(shell git rev-parse --abbrev-ref HEAD)
GIT_DESCRIBE:=$(shell git describe --always --dirty)
ifeq ($(GIT_BRANCH),master)
GIT_INFO:=$(GIT_DESCRIBE)
else
GIT_INFO:=$(GIT_DESCRIBE)-$(GIT_BRANCH)
endif
BUILD_DATETIME:=$(shell date +%Y%m%d%H%M%S)

# Source lists.
DUKTAPE_CMDLINE_SOURCES = \
	examples/cmdline/duk_cmdline.c \
	examples/alloc-logging/duk_alloc_logging.c \
	examples/alloc-torture/duk_alloc_torture.c \
	examples/alloc-hybrid/duk_alloc_hybrid.c \
	examples/debug-trans-socket/duk_trans_socket_unix.c \
	extras/print-alert/duk_print_alert.c \
	extras/console/duk_console.c \
	extras/logging/duk_logging.c \
	extras/module-duktape/duk_module_duktape.c
LINENOISE_SOURCES = \
	linenoise/linenoise.c

# Configure.py options for a few configuration profiles needed.
CONFIGOPTS_NONDEBUG=--option-file profiles/config/makeduk_base.yaml
CONFIGOPTS_NONDEBUG_SCANBUILD=--option-file profiles/config/makeduk_base.yaml --option-file profiles/config/makeduk_scanbuild.yaml
CONFIGOPTS_NONDEBUG_PERF=--option-file config/examples/performance_sensitive.yaml
CONFIGOPTS_NONDEBUG_SIZE=--option-file config/examples/low_memory.yaml
CONFIGOPTS_NONDEBUG_AJDUK=--option-file profiles/config/makeduk_base.yaml --option-file profiles/config/makeduk_ajduk.yaml --fixup-file profiles/config/makeduk_ajduk_fixup.h
CONFIGOPTS_NONDEBUG_ROM=--rom-support --rom-auto-lightfunc --option-file profiles/config/makeduk_base.yaml -DDUK_USE_ROM_STRINGS -DDUK_USE_ROM_OBJECTS -DDUK_USE_ROM_GLOBAL_INHERIT
CONFIGOPTS_NONDEBUG_AJDUK_ROM=--rom-support --rom-auto-lightfunc --builtin-file profiles/builtins/example_user_builtins1.yaml --builtin-file profiles/builtins/example_user_builtins2.yaml -DDUK_USE_ROM_STRINGS -DDUK_USE_ROM_OBJECTS -DDUK_USE_ROM_GLOBAL_INHERIT -DDUK_USE_ASSERTIONS -UDUK_USE_DEBUG
CONFIGOPTS_DEBUG=--option-file profiles/config/makeduk_base.yaml --option-file profiles/config/makeduk_debug.yaml
CONFIGOPTS_DEBUG_SCANBUILD=--option-file profiles/config/makeduk_base.yaml --option-file profiles/config/makeduk_debug.yaml --option-file profiles/config/makeduk_scanbuild.yaml
CONFIGOPTS_DEBUG_ROM=--rom-support --rom-auto-lightfunc --option-file profiles/config/makeduk_base.yaml --option-file profiles/config/makeduk_debug.yaml -DDUK_USE_ROM_STRINGS -DDUK_USE_ROM_OBJECTS -DDUK_USE_ROM_GLOBAL_INHERIT
CONFIGOPTS_EMDUK=-UDUK_USE_FASTINT -UDUK_USE_PACKED_TVAL
CONFIGOPTS_DUKWEB=--option-file profiles/config/dukweb_base.yaml --fixup-file profiles/config/dukweb_fixup.h

# Compiler setup for Linux.
CC	= gcc
GXX	= g++

CCOPTS_SHARED =
CCOPTS_SHARED += -DDUK_CMDLINE_PRINTALERT_SUPPORT
CCOPTS_SHARED += -DDUK_CMDLINE_CONSOLE_SUPPORT
CCOPTS_SHARED += -DDUK_CMDLINE_LOGGING_SUPPORT
CCOPTS_SHARED += -DDUK_CMDLINE_MODULE_SUPPORT
CCOPTS_SHARED += -DDUK_CMDLINE_FANCY
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
CCOPTS_SHARED += -Wshadow
CCOPTS_SHARED += -Wunreachable-code  # on some compilers unreachable code is an error
# -Wfloat-equal is too picky, there's no apparent way to compare floats
# (even when you know it's safe) without triggering warnings
CCOPTS_SHARED += -I./linenoise
CCOPTS_SHARED += -I./examples/alloc-logging
CCOPTS_SHARED += -I./examples/alloc-torture
CCOPTS_SHARED += -I./examples/alloc-hybrid
CCOPTS_SHARED += -I./examples/debug-trans-socket
CCOPTS_SHARED += -I./extras/print-alert
CCOPTS_SHARED += -I./extras/console
CCOPTS_SHARED += -I./extras/logging
CCOPTS_SHARED += -I./extras/module-duktape
#CCOPTS_SHARED += -m32                             # force 32-bit compilation on a 64-bit host
#CCOPTS_SHARED += -mx32                            # force X32 compilation on a 64-bit host

CCOPTS_NONDEBUG = $(CCOPTS_SHARED) $(CCOPTS_FEATURES)
CCOPTS_NONDEBUG += -Os -fomit-frame-pointer
CCOPTS_NONDEBUG += -g -ggdb

CCOPTS_DEBUG = $(CCOPTS_SHARED) $(CCOPTS_FEATURES)
CCOPTS_DEBUG += -O0
CCOPTS_DEBUG += -g -ggdb

GXXOPTS_SHARED = -pedantic -ansi -std=c++11 -fstrict-aliasing -Wall -Wextra -Wunused-result -Wunused-function
GXXOPTS_SHARED += -DDUK_CMDLINE_PRINTALERT_SUPPORT
GXXOPTS_NONDEBUG = $(GXXOPTS_SHARED) -Os -fomit-frame-pointer
GXXOPTS_NONDEBUG += -I./examples/alloc-logging -I./examples/alloc-torture -I./examples/alloc-hybrid -I./extras/print-alert -I./extras/console -I./extras/logging -I./extras/module-duktape
GXXOPTS_DEBUG = $(GXXOPTS_SHARED) -O0 -g -ggdb
GXXOPTS_DEBUG += -I./examples/alloc-logging -I./examples/alloc-torture -I./examples/alloc-hybrid -I./extras/print-alert -I./extras/console -I./extras/logging -I./extras/module-duktape

CCOPTS_AJDUK = -m32
#CCOPTS_AJDUK += '-fpack-struct=1'
CCOPTS_AJDUK += -Wno-unused-parameter -Wno-pedantic -Wno-sign-compare -Wno-missing-field-initializers -Wno-unused-result
CCOPTS_AJDUK += -UDUK_CMDLINE_FANCY -DDUK_CMDLINE_AJSHEAP -D_POSIX_C_SOURCE=200809L
CCOPTS_AJDUK += -UDUK_CMDLINE_LOGGING_SUPPORT  # extras/logger init writes to Duktape.Logger, problem with ROM build
CCOPTS_AJDUK += -UDUK_CMDLINE_MODULE_SUPPORT  # extras/module-duktape init writes to Duktape.Logger, problem with ROM build

CCLIBS	= -lm

# Emscripten options:
#   - --memory-init-file 0 to avoid a separate memory init file (this is
#     not mandatory but keeps the result in a single file)
#   - -DEMSCRIPTEN needed by Duktape for feature detection
# https://github.com/kripken/emscripten/wiki/Optimizing-Code
# http://mozakai.blogspot.fi/2013/08/outlining-workaround-for-jits-and-big.html
#
# Reducing the TOTAL_MEMORY and TOTAL_STACK values is useful if you run
# Duktape cmdline with resource limits (i.e. "duk -r test.js").
#EMCCOPTS=-s TOTAL_MEMORY=2097152 -s TOTAL_STACK=524288 --memory-init-file 0
EMCCOPTS=-O2 -std=c99 -Wall --memory-init-file 0
EMCCOPTS_DUKVM=-O2 -std=c99 -Wall --memory-init-file 0 -DEMSCRIPTEN
EMCCOPTS_DUKWEB_EXPORT=-s EXPORTED_FUNCTIONS='["_dukweb_is_open", "_dukweb_open","_dukweb_close","_dukweb_eval"]'
EMDUKOPTS=-s TOTAL_MEMORY=268435456 -DDUK_CMDLINE_PRINTALERT_SUPPORT
EMDUKOPTS+=-DEMSCRIPTEN  # enable stdin workaround in duk_cmdline.c

# Mandelbrot test, base-64 encoded.
MAND_BASE64=dyA9IDgwOyBoID0gNDA7IGl0ZXIgPSAxMDA7IGZvciAoaSA9IDA7IGkgLSBoOyBpICs9IDEpIHsgeTAgPSAoaSAvIGgpICogNC4wIC0gMi4wOyByZXMgPSBbXTsgZm9yIChqID0gMDsgaiAtIHc7IGogKz0gMSkgeyB4MCA9IChqIC8gdykgKiA0LjAgLSAyLjA7IHh4ID0gMDsgeXkgPSAwOyBjID0gIiMiOyBmb3IgKGsgPSAwOyBrIC0gaXRlcjsgayArPSAxKSB7IHh4MiA9IHh4Knh4OyB5eTIgPSB5eSp5eTsgaWYgKE1hdGgubWF4KDAsIDQuMCAtICh4eDIgKyB5eTIpKSkgeyB5eSA9IDIqeHgqeXkgKyB5MDsgeHggPSB4eDIgLSB5eTIgKyB4MDsgfSBlbHNlIHsgYyA9ICIuIjsgYnJlYWs7IH0gfSByZXNbcmVzLmxlbmd0aF0gPSBjOyB9IHByaW50KHJlcy5qb2luKCIiKSk7IH0K

# Options for runtests.js.
RUNTESTSOPTS=--prep-test-path util/prep_test.py --minify-uglifyjs2 UglifyJS2/bin/uglifyjs --util-include-path tests/ecmascript --known-issues doc/testcase-known-issues.yaml

# Compile 'duk' only by default
.PHONY:	all
all:	checksetup duk

# Check setup and warn about missing tools, libraries, etc.
.PHONY: checksetup
checksetup:
	@util/check_setup.sh

# Clean targets: 'cleanall' also deletes downloaded third party packages
# which we don't want to delete by default with 'clean'.
.PHONY:	clean
clean:
	@rm -rf dist/
	@rm -rf prep/
	@rm -rf site/
	@rm -f duk duk-rom dukd dukd-rom duk.O2 duk.O3 duk.O4
	@rm -f duk-clang duk-g++ dukd-g++
	@rm -f ajduk ajduk-rom ajdukd
	@rm -f emduk emduk.js
	@rm -f libduktape*.so*
	@rm -f duktape-*.tar.*
	@rm -f duktape-*.iso
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
	@rm -f d067d2f0ca30.tar.bz2
	@rm -rf emscripten
	@rm -rf JS-Interpreter
	@rm -f compiler-latest.zip
	@rm -f compiler.jar
	@rm -f cloc-1.60.pl
	@rm -f lua-5.2.3.tar.gz
	@rm -f luajs.zip
	@rm -f bluebird.js
	@rm -f jquery-1.11.0.js
	@rm -rf coffee-script
	@rm -rf LiveScript
	@rm -rf coco
	@rm -rf sax-js
	@rm -rf xmldoc
	@rm -rf FlameGraph
	@rm -rf dtrace4linux
	@rm -rf flow
	@rm -rf 3883a2e9063b0a5f2705bdac3263577a03913c94.zip
	@rm -rf es5-tests.zip
	@rm -rf alljoyn-js ajtcl
	@rm -f v1.3.5.tar.gz
	@rm -f "references/ECMA-262 5th edition December 2009.pdf"
	@rm -f "references/ECMA-262 5.1 edition June 2011.pdf"
	@rm -f "references/ECMA-262.pdf"

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
prep/ajduk-nondebug: prep
	@rm -rf ./prep/ajduk-nondebug
	$(PYTHON) tools/configure.py --output-directory ./prep/ajduk-nondebug --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_AJDUK) --line-directives
prep/ajduk-nondebug-rom: prep
	@rm -rf ./prep/ajduk-nondebug-rom
	$(PYTHON) tools/configure.py --output-directory ./prep/ajduk-nondebug-rom --source-directory src-input --config-metadata config $(CONFIGOPTS_NONDEBUG_AJDUK_ROM) --line-directives

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
duk-perf.O2: linenoise prep/nondebug-perf
	$(CC) -o $@ -Iprep/nondebug-perf $(CCOPTS_NONDEBUG) -O2 prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
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
	clang -o $@ -Wcast-align -Wshift-sign-overflow -Iprep/nondebug $(CCOPTS_NONDEBUG) prep/nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
	@ls -l $@
	-@size $@
duk-perf-clang: linenoise prep/nondebug-perf
	clang -o $@ -Wcast-align -Wshift-sign-overflow -Iprep/nondebug-perf $(CCOPTS_NONDEBUG) prep/nondebug-perf/duktape.c $(DUKTAPE_CMDLINE_SOURCES) $(LINENOISE_SOURCES) $(CCLIBS)
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
# Command line with Alljoyn.js pool allocator, for low memory testing.
# The pool sizes only make sense with -m32, so force that.  This forces
# us to use barebones cmdline too.
ajduk: alljoyn-js ajtcl linenoise prep/ajduk-nondebug
	$(CC) -o $@ \
		-Ialljoyn-js/src -Iajtcl/inc/ -Iajtcl/src/target/linux/ -Iprep/ajduk-nondebug \
		$(CCOPTS_NONDEBUG) $(CCOPTS_AJDUK) \
		prep/ajduk-nondebug/duktape.c $(DUKTAPE_CMDLINE_SOURCES) \
		examples/cmdline/duk_cmdline_ajduk.c \
		alljoyn-js/src/ajs_heap.c ajtcl/src/aj_debug.c ajtcl/src/target/linux/aj_target_util.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l $@
	-@size $@
ajduk-rom: alljoyn-js ajtcl linenoise prep/ajduk-nondebug-rom
	$(CC) -o $@ \
		-Ialljoyn-js/src -Iajtcl/inc/ -Iajtcl/src/target/linux/ -Iprep/ajduk-nondebug-rom \
		$(CCOPTS_NONDEBUG) $(CCOPTS_AJDUK) \
		prep/ajduk-nondebug-rom/duktape.c $(DUKTAPE_CMDLINE_SOURCES) \
		examples/cmdline/duk_cmdline_ajduk.c \
		alljoyn-js/src/ajs_heap.c ajtcl/src/aj_debug.c ajtcl/src/target/linux/aj_target_util.c \
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
emduk.js: emscripten prep/emduk
	emscripten/emcc $(EMCCOPTS) -Iprep/emduk -Iexamples/cmdline -Iextras/print-alert \
		$(EMDUKOPTS) \
		prep/emduk/duktape.c examples/cmdline/duk_cmdline.c extras/print-alert/duk_print_alert.c \
		-o /tmp/duk-emduk.js
	cat /tmp/duk-emduk.js | $(PYTHON) util/fix_emscripten.py > $@
	@ls -l $@
# This is a prototype of running Duktape in a web environment with Emscripten,
# and providing an eval() facility from both sides.  This is a placeholder now
# and doesn't do anything useful yet.
dukweb.js: emscripten prep/dukweb
	emscripten/emcc $(EMCCOPTS_DUKVM) $(EMCCOPTS_DUKWEB_EXPORT) \
		-Iprep/dukweb prep/dukweb/duktape.c dukweb/dukweb.c -o dukweb.js
	cat dukweb/dukweb_extra.js >> dukweb.js
	@wc dukweb.js

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
cloc:	dist cloc-1.60.pl
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

# Overall test target, not very useful.
.PHONY: test
test: qecmatest apitest regfuzztest underscoretest lodashtest emscriptentest emscripteninceptiontest test262test

# Runtests-based Ecmascript and API tests.
.PHONY:	runtestsdeps
runtestsdeps:	runtests/node_modules UglifyJS2
runtests/node_modules:
	echo "Installing required NodeJS modules for runtests"
	cd runtests; npm install
.PHONY:	ecmatest
ecmatest: runtestsdeps duk
	@echo "### ecmatest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/duk --report-diff-to-other --run-nodejs --run-rhino --num-threads 4 --log-file=/tmp/duk-test.log tests/ecmascript/
.PHONY:	ecmatestd
ecmatestd: runtestsdeps dukd
	@echo "### ecmatestd"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/dukd --report-diff-to-other --run-nodejs --run-rhino --num-threads 4 --log-file=/tmp/duk-test.log tests/ecmascript/
.PHONY:	qecmatest
qecmatest: runtestsdeps duk
	@echo "### qecmatest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/duk --num-threads 4  --log-file=/tmp/duk-test.log tests/ecmascript/
.PHONY:	qecmatestd
qecmatestd: runtestsdeps dukd
	@echo "### qecmatestd"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/dukd --num-threads 4 --log-file=/tmp/duk-test.log tests/ecmascript/
.PHONY: apiprep
apiprep: runtestsdeps libduktape.so.1.0.0
.PHONY:	apitest
apitest: apiprep
	@echo "### apitest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --num-threads 1 --log-file=/tmp/duk-api-test.log tests/api/

# Matrix tests.
.PHONY: matrix10
matrix10: dist
	cd dist; $(PYTHON) ../util/matrix_compile.py --count=10
.PHONY: matrix100
matrix100: dist
	cd dist; $(PYTHON) ../util/matrix_compile.py --count=100
.PHONY: matrix1000
matrix1000: dist
	cd dist; $(PYTHON) ../util/matrix_compile.py --count=1000
.PHONY: matrix10000
matrix10000: dist
	cd dist; $(PYTHON) ../util/matrix_compile.py --count=10000

# Dukweb.js test.
.PHONY: dukwebtest
dukwebtest: dukweb.js jquery-1.11.0.js
	@echo "### dukwebtest"
	@rm -rf /tmp/dukweb-test/
	mkdir /tmp/dukweb-test/
	cp dukweb.js jquery-1.11.0.js dukweb/dukweb.html dukweb/dukweb.css /tmp/dukweb-test/
	@echo "Now point your browser to: file:///tmp/dukweb-test/dukweb.html"

# Third party tests.
.PHONY: underscoretest
underscoretest:	underscore duk
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
.PHONY:	regfuzztest
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
	./duk lodash/lodash.js tests/lodash/basic.js
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
emscriptentest: emscripten duk
	@echo "### emscriptentest"
	@rm -f /tmp/duk-emcc-test*
	@echo "NOTE: this emscripten test is incomplete (compiles helloworld.c and runs it, no checks yet)"
	emscripten/emcc $(EMCCOPTS) tests/emscripten/helloworld.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	#./duk /tmp/duk-emcc-test-fixed.js
	./duk /tmp/duk-emcc-test.js
.PHONY: emscriptenmandeltest
emscriptenmandeltest: emscripten duk
	@echo "### emscriptenmandeltest"
	@rm -f /tmp/duk-emcc-test*
	@echo "NOTE: this emscripten test is incomplete (compiles mandelbrot.c and runs it, no checks yet)"
	emscripten/emcc $(EMCCOPTS) tests/emscripten/mandelbrot.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	#./duk /tmp/duk-emcc-test-fixed.js
	./duk /tmp/duk-emcc-test.js
# Compile Duktape and hello.c using Emscripten and execute the result with
# Duktape.
.PHONY: emscripteninceptiontest
emscripteninceptiontest: emscripten prep/nondebug duk
	@echo "### emscripteniceptiontest"
	@rm -f /tmp/duk-emcc-test*
	@echo "NOTE: this emscripten test is incomplete (compiles Duktape and hello.c and runs it, no checks yet)"
	emscripten/emcc $(EMCCOPTS) -Iprep/nondebug prep/nondebug/duktape.c examples/hello/hello.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	#./duk /tmp/duk-emcc-test-fixed.js
	./duk /tmp/duk-emcc-test.js
# Compile Duktape with Emscripten and execute it with NodeJS.
.PHONY: emscriptenduktest
emscriptenduktest: emscripten prep/emduk
	@echo "### emscriptenduktest"
	@rm -f /tmp/duk-emcc-duktest.js
	emscripten/emcc $(EMCCOPTS_DUKVM) -Iprep/emduk prep/emduk/duktape.c examples/eval/eval.c -o /tmp/duk-emcc-duktest.js
	"$(NODE)" /tmp/duk-emcc-duktest.js \
		'print("Hello from Duktape running inside Emscripten/NodeJS");' \
		'print(Duktape.version, Duktape.env);' \
		'for(i=0;i++<100;)print((i%3?"":"Fizz")+(i%5?"":"Buzz")||i)'
	"$(NODE)" /tmp/duk-emcc-duktest.js "eval(String.fromBuffer(Duktape.dec('base64', '$(MAND_BASE64)')))"
LUASRC=	lapi.c lauxlib.c lbaselib.c lbitlib.c lcode.c lcorolib.c lctype.c \
	ldblib.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c liolib.c \
	llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c loslib.c \
	lparser.c lstate.c lstring.c lstrlib.c ltable.c ltablib.c ltm.c \
	lua.c lundump.c lvm.c lzio.c
# Compile Lua 5.2.3 with Emscripten and run it with Duktape.
.PHONY: emscriptenluatest
emscriptenluatest: emscripten duk lua-5.2.3
	@echo "### emscriptenluatest"
	@rm -f /tmp/duk-emcc-luatest*
	emscripten/emcc $(EMCCOPTS) -Ilua-5.2.3/src/ $(patsubst %,lua-5.2.3/src/%,$(LUASRC)) -o /tmp/duk-emcc-luatest.js
	cat /tmp/duk-emcc-luatest.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-luatest-fixed.js
	@ls -l /tmp/duk-emcc-luatest*
	#./duk /tmp/duk-emcc-luatest-fixed.js
	./duk /tmp/duk-emcc-luatest.js
.PHONY: jsinterpretertest
jsinterpretertest: JS-Interpreter duk
	@echo "### jsinterpretertest"
	@rm -f /tmp/duk-jsint-test*
	echo "window = {};" > /tmp/duk-jsint-test.js
	cat JS-Interpreter/acorn.js JS-Interpreter/interpreter.js >> /tmp/duk-jsint-test.js
	cat tests/jsinterpreter/addition.js >> /tmp/duk-jsint-test.js
	./duk /tmp/duk-jsint-test.js
.PHONY: luajstest
luajstest: luajs duk
	@rm -f /tmp/duk-luajs-mandel.js /tmp/duk-luajs-test.js
	luajs/lua2js tests/luajs/mandel.lua /tmp/duk-luajs-mandel.js
	echo "console = { log: function() { print(Array.prototype.join.call(arguments, ' ')); } };" > /tmp/duk-luajs-test.js
	cat luajs/lua.js /tmp/duk-luajs-mandel.js >> /tmp/duk-luajs-test.js
	./duk /tmp/duk-luajs-test.js
.PHONY: bluebirdtest
bluebirdtest: bluebird.js duk
	@rm -f /tmp/duk-bluebird-test.js
	cat util/bluebird-test-shim.js bluebird.js > /tmp/duk-bluebird-test.js
	echo "var myPromise = new Promise(function(resolve, reject) { setTimeout(function () { resolve('resolved 123') }, 1000); });" >> /tmp/duk-bluebird-test.js
	echo "myPromise.then(function (v) { print('then:', v); });" >> /tmp/duk-bluebird-test.js
	echo "fakeEventLoop();" >> /tmp/duk-bluebird-test.js
	./duk /tmp/duk-bluebird-test.js
.PHONY: closuretest
closuretest: compiler.jar duk
	@echo "### closuretest"
	@rm -f /tmp/duk-closure-test*
	$(JAVA) -jar compiler.jar tests/ecmascript/test-dev-mandel2-func.js > /tmp/duk-closure-test.js
	./duk /tmp/duk-closure-test.js
xmldoctest: sax-js xmldoc duk
	@echo "### xmldoctest"
	@rm -f /tmp/duk-xmldoc-test*
	cat sax-js/lib/sax.js > /tmp/duk-xmldoc-test.js
	echo ";" >> /tmp/duk-xmldoc-test.js  # missing end semicolon causes automatic semicolon problem
	cat xmldoc/lib/xmldoc.js >> /tmp/duk-xmldoc-test.js
	echo ";" >> /tmp/duk-xmldoc-test.js  # missing end semicolon causes automatic semicolon problem
	cat tests/xmldoc/basic.js >> /tmp/duk-xmldoc-test.js
	./duk /tmp/duk-xmldoc-test.js

# Third party download/unpack targets, libraries etc.
linenoise:
	# git clone https://github.com/antirez/linenoise.git
	# Use forked repo to get compile warnings fixed (not yet fixed in
	# linenoise master).
	git clone -b fix-compile-warnings https://github.com/svaarala/linenoise.git
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
	# Master is OK because not a critical dependency
	$(GIT) clone --depth 1 https://github.com/lodash/lodash.git
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
emscripten:
	# https://github.com/kripken/emscripten
	# Master is OK because not a critical dependency
	# Setup is complicated because needs matching fastcomp which
	# you must provide yourself and add to ~/.emscripten:
	# http://kripken.github.io/emscripten-site/docs/building_from_source/building_fastcomp_manually_from_source.html
	$(GIT) clone --depth 1 https://github.com/kripken/emscripten.git
	cd emscripten; ./emconfigure
jquery-1.11.0.js:
	$(WGET) http://code.jquery.com/jquery-1.11.0.js -O $@
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
alljoyn-js:
	# https://git.allseenalliance.org/cgit/core/alljoyn-js.git/
	# no --depth 1 ("dumb http transport does not support --depth")
	$(GIT) clone https://git.allseenalliance.org/gerrit/core/alljoyn-js.git
ajtcl:
	# https://git.allseenalliance.org/cgit/core/ajtcl.git/
	# no --depth 1 ("dumb http transport does not support --depth")
	$(GIT) clone https://git.allseenalliance.org/gerrit/core/ajtcl.git/
	ln -s . ajtcl/inc/ajtcl  # workaround for #include <ajtcl/xxx.h>
	ln -s . ajtcl/src/target/linux/ajtcl
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
refs:	references/ECMA-262\ 5th\ edition\ December\ 2009.pdf \
	references/ECMA-262\ 5.1\ edition\ June\ 2011.pdf \
	references/ECMA-262.pdf

# Documentation.
.PHONY:	doc
doc:	$(patsubst %.txt,%.html,$(wildcard doc/*.txt))
doc/%.html: doc/%.txt
	rst2html $< $@

# Source distributable for end users.
dist:
	@make codepolicycheck
	$(PYTHON) util/dist.py --create-spdx
.PHONY:	dist-src
dist-src:	dist
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
dist-iso:	dist-src
	mkisofs -input-charset utf-8 -o duktape-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).iso duktape-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).tar.gz

# Website targets.
.PHONY: tidy-site
tidy-site:
	for i in website/*/*.html; do echo "*** Checking $$i"; tidy -q -e -xml $$i; done
site: duktape-releases dukweb.js jquery-1.11.0.js
	rm -rf site
	mkdir site
	-cd duktape-releases/; git pull --rebase  # get binaries up-to-date, but allow errors for offline use
	cd website/; $(PYTHON) buildsite.py ../site/
	@rm -rf /tmp/site/
	cp -r site /tmp/  # useful for quick preview
.PHONY:	dist-site
dist-site:	tidy-site site
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
		src-input/*.py tools/*.py util/*.py debugger/*/*.py examples/*/*.py testrunner/*/*.py tests/perf/*.py
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
		--dump-vim-commands \
		config/architectures/* config/compilers/* config/platforms/* \
		config/feature-options/*.yaml \
		profiles/config/* config/header-snippets/* config/helper-snippets/* \
		config/*.yaml
	# XXX: config files not yet FIXME pure
	@$(PYTHON) util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		config/config-options/*.yaml config/other-defines/*.yaml
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

# Simple performance test, minimum time for N runs
# - Duktape is interpreted and uses reference counting
# - Python and Perl are interpreted and also use reference counting
# - Ruby and Lua are interpreted but don't use reference counting
# - Mujs is interpreted but doesn't use reference counting
# - Rhino compiles to Java bytecode and is ultimately JITed
# - Node.js (V8) is JITed
# - Luajit is JITed

#TIME=$(PYTHON) util/time_multi.py --count 1 --sleep 0 --sleep-factor 2.0 --mode min # Take minimum time of N
#TIME=$(PYTHON) util/time_multi.py --count 3 --sleep 0 --sleep-factor 2.0 --mode min # Take minimum time of N
TIME=$(PYTHON) util/time_multi.py --count 5 --sleep 0 --sleep-factor 2.0 --mode min # Take minimum time of N

# Blocks: optimization variants, previous versions, other interpreting engines,
# other JIT engines.
perftest: duk duk.O2 duk.O3 duk.O4
	for i in tests/perf/*.js; do \
		printf '%-36s:' "`basename $$i`"; \
		printf ' duk.Os %5s' "`$(TIME) ./duk $$i`"; \
		printf ' duk.O2 %5s' "`$(TIME) ./duk.O2 $$i`"; \
		printf ' duk.O3 %5s' "`$(TIME) ./duk.O3 $$i`"; \
		printf ' duk.O4 %5s' "`$(TIME) ./duk.O4 $$i`"; \
		printf ' |'; \
		printf ' duk.O2.150 %5s' "`$(TIME) ./duk.O2.150 $$i`"; \
		printf ' duk.O2.140 %5s' "`$(TIME) ./duk.O2.140 $$i`"; \
		printf ' duk.O2.130 %5s' "`$(TIME) ./duk.O2.130 $$i`"; \
		printf ' duk.O2.124 %5s' "`$(TIME) ./duk.O2.124 $$i`"; \
		printf ' duk.O2.113 %5s' "`$(TIME) ./duk.O2.113 $$i`"; \
		printf ' duk.O2.102 %5s' "`$(TIME) ./duk.O2.102 $$i`"; \
		printf ' |'; \
		printf ' mujs %5s' "`$(TIME) mujs $$i`"; \
		printf ' jerry %5s' "`$(TIME) jerry $$i`"; \
		printf ' lua %5s' "`$(TIME) lua $${i%%.js}.lua`"; \
		printf ' python %5s' "`$(TIME) $(PYTHON) $${i%%.js}.py`"; \
		printf ' perl %5s' "`$(TIME) perl $${i%%.js}.pl`"; \
		printf ' ruby %5s' "`$(TIME) ruby $${i%%.js}.rb`"; \
		printf ' |'; \
		printf ' rhino %5s' "`$(TIME) rhino $$i`"; \
		printf ' node %5s' "`$(TIME) node $$i`"; \
		printf ' luajit %5s' "`$(TIME) luajit $${i%%.js}.lua`"; \
		printf '\n'; \
	done
perftestduk: duk.O2 duk-perf.O2
	for i in tests/perf/*.js; do \
		printf '%-36s:' "`basename $$i`"; \
		printf ' duk-perf.O2 %5s' "`$(TIME) ./duk-perf.O2 $$i`"; \
		printf ' duk.O2 %5s' "`$(TIME) ./duk.O2 $$i`"; \
		printf ' |'; \
		printf ' mujs %5s' "`$(TIME) mujs $$i`"; \
		printf ' jerry %5s' "`$(TIME) jerry $$i`"; \
		printf ' duk.O2.master %5s' "`$(TIME) ./duk.O2.master $$i`"; \
		printf ' duk.O2.150 %5s' "`$(TIME) ./duk.O2.150 $$i`"; \
		printf '\n'; \
	done
