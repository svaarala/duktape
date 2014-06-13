#
#  Makefile for the Duktape development repo
#
#  This Makefile is intended for ONLY internal Duktape development
#  on Linux (or other UNIX-like operating systems), and covers:
#
#    - building the Duktape source distributable
#    - running test cases
#    - building the duktape.org website
#
#  The source distributable has more platform neutral example Makefiles
#  for end user projects (though an end user should really just use their
#  own Makefile).
#
#  YOU SHOULD NOT COMPILE DUKTAPE WITH THIS MAKEFILE IN YOUR PROJECT!
#
#  Duktape command line tools are built by first creating a source dist
#  directory, and then using the sources from the dist directory for
#  compilation.  This is as close as possible to the sources used by an
#  end user, at the risk of accidentally polluting the dist directory.
#
#  When creating actual distributables, always clean first.
#
#  External projects are downloaded on-the-fly.  Clone git repos shallowly
#  (--depth 1) whenever possible.  With build-critical resources, use a
#  specific version instead of "trunk".
#

# A few commands which may need to be edited.  NodeJS is sometimes found
# as 'nodejs', sometimes as 'node'; sometimes 'node' is unrelated to NodeJS
# so check 'nodejs' first.
GIT=$(shell which git)
NODE=$(shell which nodejs node | head -1)
WGET=$(shell which wget)
JAVA=$(shell which java)
VALGRIND=$(shell which valgrind)
PYTHON=$(shell which python)

# Scrape version from the public header; convert from e.g. 10203 -> '1.2.3'
DUK_VERSION=$(shell cat src/duk_api_public.h.in | grep define | grep DUK_VERSION | tr -s ' ' ' ' | cut -d ' ' -f 3 | tr -d 'L')
DUK_MAJOR=$(shell echo "$(DUK_VERSION) / 10000" | bc)
DUK_MINOR=$(shell echo "$(DUK_VERSION) % 10000 / 100" | bc)
DUK_PATCH=$(shell echo "$(DUK_VERSION) % 100" | bc)
DUK_VERSION_FORMATTED=$(DUK_MAJOR).$(DUK_MINOR).$(DUK_PATCH)
GIT_DESCRIBE=$(shell git describe)

# Ditz release (next release name)
DITZ_RELEASE=v0.11

DISTSRCSEP = dist/src-separate
DISTSRCCOM = dist/src
DISTCMD = dist/examples/cmdline

DUKTAPE_SOURCES_COMBINED =	\
	$(DISTSRCCOM)/duktape.c

DUKTAPE_SOURCES_SEPARATE =	\
	$(DISTSRCSEP)/duk_util_hashbytes.c \
	$(DISTSRCSEP)/duk_util_hashprime.c \
	$(DISTSRCSEP)/duk_util_bitdecoder.c \
	$(DISTSRCSEP)/duk_util_bitencoder.c \
	$(DISTSRCSEP)/duk_util_tinyrandom.c \
	$(DISTSRCSEP)/duk_util_misc.c \
	$(DISTSRCSEP)/duk_alloc_default.c \
	$(DISTSRCSEP)/duk_debug_macros.c \
	$(DISTSRCSEP)/duk_debug_vsnprintf.c \
	$(DISTSRCSEP)/duk_debug_heap.c \
	$(DISTSRCSEP)/duk_debug_hobject.c \
	$(DISTSRCSEP)/duk_debug_fixedbuffer.c \
	$(DISTSRCSEP)/duk_error_macros.c \
	$(DISTSRCSEP)/duk_error_longjmp.c \
	$(DISTSRCSEP)/duk_error_throw.c \
	$(DISTSRCSEP)/duk_error_augment.c \
	$(DISTSRCSEP)/duk_error_misc.c \
	$(DISTSRCSEP)/duk_heap_misc.c \
	$(DISTSRCSEP)/duk_heap_memory.c \
	$(DISTSRCSEP)/duk_heap_alloc.c \
	$(DISTSRCSEP)/duk_heap_refcount.c \
	$(DISTSRCSEP)/duk_heap_markandsweep.c \
	$(DISTSRCSEP)/duk_heap_hashstring.c \
	$(DISTSRCSEP)/duk_heap_stringtable.c \
	$(DISTSRCSEP)/duk_heap_stringcache.c \
	$(DISTSRCSEP)/duk_hstring_misc.c \
	$(DISTSRCSEP)/duk_hthread_misc.c \
	$(DISTSRCSEP)/duk_hthread_alloc.c \
	$(DISTSRCSEP)/duk_hthread_builtins.c \
	$(DISTSRCSEP)/duk_hthread_stacks.c \
	$(DISTSRCSEP)/duk_hobject_alloc.c \
	$(DISTSRCSEP)/duk_hobject_class.c \
	$(DISTSRCSEP)/duk_hobject_enum.c \
	$(DISTSRCSEP)/duk_hobject_props.c \
	$(DISTSRCSEP)/duk_hobject_finalizer.c \
	$(DISTSRCSEP)/duk_hobject_pc2line.c \
	$(DISTSRCSEP)/duk_hobject_misc.c \
	$(DISTSRCSEP)/duk_hbuffer_alloc.c \
	$(DISTSRCSEP)/duk_hbuffer_ops.c \
	$(DISTSRCSEP)/duk_unicode_tables.c \
	$(DISTSRCSEP)/duk_unicode_support.c \
	$(DISTSRCSEP)/duk_builtins.c \
	$(DISTSRCSEP)/duk_js_ops.c \
	$(DISTSRCSEP)/duk_js_var.c \
	$(DISTSRCSEP)/duk_numconv.c \
	$(DISTSRCSEP)/duk_api_call.c \
	$(DISTSRCSEP)/duk_api_compile.c \
	$(DISTSRCSEP)/duk_api_codec.c \
	$(DISTSRCSEP)/duk_api_memory.c \
	$(DISTSRCSEP)/duk_api_string.c \
	$(DISTSRCSEP)/duk_api_object.c \
	$(DISTSRCSEP)/duk_api_thread.c \
	$(DISTSRCSEP)/duk_api_buffer.c \
	$(DISTSRCSEP)/duk_api_var.c \
	$(DISTSRCSEP)/duk_api_logging.c \
	$(DISTSRCSEP)/duk_api_debug.c \
	$(DISTSRCSEP)/duk_api.c \
	$(DISTSRCSEP)/duk_lexer.c \
	$(DISTSRCSEP)/duk_js_call.c \
	$(DISTSRCSEP)/duk_js_executor.c \
	$(DISTSRCSEP)/duk_js_compiler.c \
	$(DISTSRCSEP)/duk_regexp_compiler.c \
	$(DISTSRCSEP)/duk_regexp_executor.c \
	$(DISTSRCSEP)/duk_bi_duktape.c \
	$(DISTSRCSEP)/duk_bi_thread.c \
	$(DISTSRCSEP)/duk_bi_thrower.c \
	$(DISTSRCSEP)/duk_bi_array.c \
	$(DISTSRCSEP)/duk_bi_boolean.c \
	$(DISTSRCSEP)/duk_bi_date.c \
	$(DISTSRCSEP)/duk_bi_error.c \
	$(DISTSRCSEP)/duk_bi_function.c \
	$(DISTSRCSEP)/duk_bi_global.c \
	$(DISTSRCSEP)/duk_bi_json.c \
	$(DISTSRCSEP)/duk_bi_math.c \
	$(DISTSRCSEP)/duk_bi_number.c \
	$(DISTSRCSEP)/duk_bi_object.c \
	$(DISTSRCSEP)/duk_bi_regexp.c \
	$(DISTSRCSEP)/duk_bi_string.c \
	$(DISTSRCSEP)/duk_bi_proxy.c \
	$(DISTSRCSEP)/duk_bi_buffer.c \
	$(DISTSRCSEP)/duk_bi_pointer.c \
	$(DISTSRCSEP)/duk_bi_logger.c \
	$(DISTSRCSEP)/duk_selftest.c

# Use combined sources for testing etc.
DUKTAPE_SOURCES = $(DUKTAPE_SOURCES_COMBINED)
#DUKTAPE_SOURCES = $(DUKTAPE_SOURCES_SEPARATE)

# Duktape command line tool - example of a main() program, used
# for unit testing
DUKTAPE_CMDLINE_SOURCES = \
	$(DISTCMD)/duk_cmdline.c

# Compiler setup for Linux
CC	= gcc
CCOPTS_SHARED =
CCOPTS_SHARED += -pedantic -ansi -std=c99
CCOPTS_SHARED += -fstrict-aliasing
CCOPTS_SHARED += -Wall
CCOPTS_SHARED += -Wextra  # very picky but catches e.g. signed/unsigned comparisons
CCOPTS_SHARED += -I./dist/src
#CCOPTS_SHARED += -I./dist/src-separate
#CCOPTS_SHARED += -m32                             # force 32-bit compilation on a 64-bit host
#CCOPTS_SHARED += -mx32                            # force X32 compilation on a 64-bit host
#CCOPTS_SHARED += -DDUK_OPT_NO_PACKED_TVAL
#CCOPTS_SHARED += -DDUK_OPT_FORCE_ALIGN=4
#CCOPTS_SHARED += -DDUK_OPT_FORCE_ALIGN=8
#CCOPTS_SHARED += -DDUK_OPT_FORCE_BYTEORDER=1      # little
#CCOPTS_SHARED += -DDUK_OPT_FORCE_BYTEORDER=2      # middle
#CCOPTS_SHARED += -DDUK_OPT_FORCE_BYTEORDER=3      # big
#CCOPTS_SHARED += -DDUK_OPT_DEEP_C_STACK
#CCOPTS_SHARED += -DDUK_OPT_NO_REFERENCE_COUNTING
#CCOPTS_SHARED += -DDUK_OPT_NO_MARK_AND_SWEEP
#CCOPTS_SHARED += -DDUK_OPT_NO_VOLUNTARY_GC
CCOPTS_SHARED += -DDUK_OPT_SEGFAULT_ON_PANIC       # segfault on panic allows valgrind to show stack trace on panic
CCOPTS_SHARED += -DDUK_OPT_DPRINT_COLORS
#CCOPTS_SHARED += -DDUK_OPT_NO_FILE_IO
#CCOPTS_SHARED += '-DDUK_OPT_PANIC_HANDLER(code,msg)={printf("*** %d:%s\n",(code),(msg));abort();}'
CCOPTS_SHARED += -DDUK_OPT_SELF_TESTS
#CCOPTS_SHARED += -DDUK_OPT_NO_TRACEBACKS
#CCOPTS_SHARED += -DDUK_OPT_NO_PC2LINE
#CCOPTS_SHARED += -DDUK_OPT_NO_VERBOSE_ERRORS
#CCOPTS_SHARED += -DDUK_OPT_GC_TORTURE
#CCOPTS_SHARED += -DDUK_OPT_NO_MS_RESIZE_STRINGTABLE
CCOPTS_SHARED += -DDUK_OPT_DEBUG_BUFSIZE=512
#CCOPTS_SHARED += -DDUK_OPT_NO_REGEXP_SUPPORT
#CCOPTS_SHARED += -DDUK_OPT_NO_OCTAL_SUPPORT
#CCOPTS_SHARED += -DDUK_OPT_NO_SOURCE_NONBMP
#CCOPTS_SHARED += -DDUK_OPT_STRICT_UTF8_SOURCE
#CCOPTS_SHARED += -DDUK_OPT_NO_BROWSER_LIKE
#CCOPTS_SHARED += -DDUK_OPT_NO_SECTION_B
#CCOPTS_SHARED += -DDUK_OPT_NO_INTERRUPT_COUNTER
#CCOPTS_SHARED += -DDUK_OPT_NO_JX
#CCOPTS_SHARED += -DDUK_OPT_NO_JC
#CCOPTS_SHARED += -DDUK_OPT_NO_NONSTD_ACCESSOR_KEY_ARGUMENT
#CCOPTS_SHARED += -DDUK_OPT_NO_NONSTD_FUNC_STMT
#CCOPTS_SHARED += -DDUK_OPT_NONSTD_FUNC_CALLER_PROPERTY
#CCOPTS_SHARED += -DDUK_OPT_NONSTD_FUNC_SOURCE_PROPERTY
#CCOPTS_SHARED += -DDUK_OPT_NO_NONSTD_ARRAY_SPLICE_DELCOUNT
#CCOPTS_SHARED += -DDUK_OPT_NO_ES6_OBJECT_PROTO_PROPERTY
#CCOPTS_SHARED += -DDUK_OPT_NO_ES6_OBJECT_SETPROTOTYPEOF
#CCOPTS_SHARED += -DDUK_OPT_NO_ES6_PROXY
#CCOPTS_SHARED += -DDUK_OPT_NO_ZERO_BUFFER_DATA
#CCOPTS_SHARED += -DDUK_OPT_USER_INITJS='"this.foo = 123"'
#CCOPTS_SHARED += -DDUK_CMDLINE_BAREBONES
CCOPTS_NONDEBUG = $(CCOPTS_SHARED) -Os -fomit-frame-pointer
CCOPTS_NONDEBUG += -g -ggdb
#CCOPTS_NONDEBUG += -DDUK_OPT_ASSERTIONS
CCOPTS_DEBUG = $(CCOPTS_SHARED) -O0 -g -ggdb
CCOPTS_DEBUG += -DDUK_OPT_DEBUG
CCOPTS_DEBUG += -DDUK_OPT_DPRINT
#CCOPTS_DEBUG += -DDUK_OPT_DDPRINT
#CCOPTS_DEBUG += -DDUK_OPT_DDDPRINT
CCOPTS_DEBUG += -DDUK_OPT_ASSERTIONS
CCLIBS	= -lm
CCLIBS += -lreadline
CCLIBS += -lncurses  # on some systems -lreadline also requires -lncurses (e.g. RHEL)

# Replace 'duk' and 'dukd' with automatic valgrind wrappers (plain commands
# will be duk.raw and dukd.raw).  Targets for runtests.js bypass the wrapper
# because runtests.js has its own valgrind handling.
#VALGRIND_WRAP=1

# Compile 'duk' only by default
.PHONY:	all
all:	checksetup duk

.PHONY: checksetup
checksetup:
	@util/check_setup.sh

.PHONY:	clean
clean:
	-@rm -rf dist/
	-@rm -rf site/
	-@rm -f duk.raw dukd.raw duk.vg dukd.vg duk dukd
	-@rm -f libduktape*.so*
	-@rm -f doc/*.html
	-@rm -f src/*.pyc
	-@rm -rf duktape-*  # covers various files and dirs
	-@rm -rf massif.out.* ms_print.tmp.*
	-@rm -rf cachegrind.out.*
	-@rm -rf callgrind.out.*
	-@rm -rf oprofile_data/
	-@rm -f /tmp/duk_sizes.html
	-@rm -f /tmp/duk-test-eval-file-temp.js  # used by api-testcase/test-eval-file.js
	-@rm -rf /tmp/duktape-regfuzz/
	-@rm -f /tmp/duk-test.log /tmp/duk-api-test.log
	-@rm -f /tmp/duk-test262.log /tmp/duk-test262-filtered.log
	-@rm -f /tmp/duk-emcc-test*
	-@rm -f /tmp/duk-emcc-luatest*
	-@rm -f /tmp/duk-emcc-duktest*
	-@rm -f /tmp/duk-jsint-test*
	-@rm -f /tmp/duk-luajs-mandel.js /tmp/duk-luajs-test.js
	-@rm -f /tmp/duk-closure-test*
	-@rm -f a.out
	-@rm -rf test262-d067d2f0ca30
	-@rm -f compiler.jar
	-@rm -rf lua-5.2.3
	-@rm -rf luajs
	-@rm -f dukweb.js
	-@rm -rf /tmp/dukweb-test/

.PHONY: cleanall
cleanall: clean
	# Don't delete these in 'clean' to avoid re-downloading them over and over
	-@rm -f regfuzz-*.tar.gz
	-@rm -rf UglifyJS
	-@rm -rf UglifyJS2
	-@rm -rf underscore
	-@rm -f d067d2f0ca30.tar.bz2
	-@rm -rf emscripten
	-@rm -rf JS-Interpreter
	-@rm -f compiler-latest.zip
	-@rm -f cloc-1.60.pl
	-@rm -f lua-5.2.3.tar.gz
	-@rm -f luajs.zip
	-@rm -f jquery-1.11.0.js
	-@rm -rf coffee-script
	-@rm -rf LiveScript
	-@rm -rf coco
	-@rm -rf sax-js
	-@rm -rf xmldoc
	-@rm -rf FlameGraph
	-@rm -rf dtrace4linux

libduktape.so.1.0.0: dist
	-rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

libduktaped.so.1.0.0: dist
	-rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

.PHONY: debuglogcheck
debuglogcheck:
	-python util/check_debuglog_calls.py src/*.c

duk.raw: dist
	$(CC) -o $@ $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

duk.vg: duk.raw
	-@rm -f $@
	@echo '#!/bin/sh' > $@
	@echo 'valgrind "$(shell pwd)/$<" "$$@"' >> $@
	@chmod ugo+rx $@

duk: duk.raw duk.vg
	-@rm -f $@
ifeq ($(VALGRIND_WRAP),1)
	@echo "Using valgrind wrapped $@"
	@cp duk.vg $@
else
	@cp duk.raw $@
endif

dukd.raw: dist
	$(CC) -o $@ $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

dukd.vg: dukd.raw
	-@rm -f $@
	@echo '#!/bin/sh' > $@
	@echo 'valgrind "$(shell pwd)/$<" "$$@"' >> $@
	@chmod ugo+rx $@

dukd: dukd.raw dukd.vg
	-@rm -f dukd
ifeq ($(VALGRIND_WRAP),1)
	@echo "Using valgrind wrapped $@"
	@cp dukd.vg $@
else
	@cp dukd.raw $@
endif

.PHONY: duksizes
duksizes: duk.raw
	$(PYTHON) src/genexesizereport.py $< > /tmp/duk_sizes.html

.PHONY: issuecount
issuecount:
	@echo "FIXME:     `grep FIXME: src/*.c src/*.h src/*.in | wc -l | tr -d ' '`"
	@echo "XXX:       `grep XXX: src/*.c src/*.h src/*.in | wc -l | tr -d ' '`"
	@echo "TODO:      `grep TODO: src/*.c src/*.h src/*.in | wc -l | tr -d ' '`"
	@echo "NOTE:      `grep NOTE: src/*.c src/*.h src/*.in | wc -l | tr -d ' '`"
	@echo "SCANBUILD: `grep SCANBUILD: src/*.c src/*.h src/*.in | wc -l | tr -d ' '`"
	@echo "Ditz ($(DITZ_RELEASE)): `ditz todo $(DITZ_RELEASE) | wc -l | tr -d ' '`"

.PHONY: dukscanbuild
dukscanbuild: dist
	scan-build gcc -o/tmp/duk.scanbuild -Idist/src-separate/ $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES_SEPARATE) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

.PHONY: dukdscanbuild
dukdscanbuild: dist
	scan-build gcc -o/tmp/duk.scanbuild -Idist/src-separate/ $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES_SEPARATE) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

.PHONY: test
test: qecmatest apitest regfuzztest underscoretest emscriptentest test262test

.PHONY:	ecmatest
ecmatest: npminst duk
ifeq ($(VALGRIND_WRAP),1)
	@echo "### ecmatest (valgrind)"
	$(NODE) runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk.raw --valgrind --num-threads 1 --log-file=/tmp/duk-test.log ecmascript-testcases/
else
	@echo "### ecmatest"
	$(NODE) runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk --run-nodejs --run-rhino --num-threads 8 --log-file=/tmp/duk-test.log ecmascript-testcases/
endif

.PHONY:	ecmatestd
ecmatestd: npminst dukd
ifeq ($(VALGRIND_WRAP),1)
	@echo "### ecmatestd (valgrind)"
	$(NODE) runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/dukd.raw --valgrind --num-threads 1 --log-file=/tmp/duk-test.log ecmascript-testcases/
else
	@echo "### ecmatestd"
	$(NODE) runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/dukd --run-nodejs --run-rhino --num-threads 8 --log-file=/tmp/duk-test.log ecmascript-testcases/
endif

.PHONY:	qecmatest
qecmatest: npminst duk
ifeq ($(VALGRIND_WRAP),1)
	@echo "### qecmatest (valgrind)"
	$(NODE) runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk.raw --valgrind --num-threads 1 --log-file=/tmp/duk-test.log ecmascript-testcases/
else
	$(NODE) runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk --num-threads 16 --log-file=/tmp/duk-test.log ecmascript-testcases/
	@echo "### qecmatest"
endif

.PHONY:	qecmatestd
qecmatestd: npminst dukd
ifeq ($(VALGRIND_WRAP),1)
	@echo "### qecmatestd (valgrind)"
	$(NODE) runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/dukd.raw --valgrind --num-threads 1 --log-file=/tmp/duk-test.log ecmascript-testcases/
else
	@echo "### qecmatestd"
	$(NODE) runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/dukd --num-threads 16 --log-file=/tmp/duk-test.log ecmascript-testcases/
endif

# Separate target because it's also convenient to run manually.
.PHONY: apiprep
apiprep: npminst libduktape.so.1.0.0

.PHONY:	apitest
apitest: apiprep
ifeq ($(VALGRIND_WRAP),1)
	@echo "### apitest (valgrind)"
	$(NODE) runtests/runtests.js --num-threads 1 --valgrind --log-file=/tmp/duk-api-test.log api-testcases/
else
	@echo "### apitest"
	$(NODE) runtests/runtests.js --num-threads 1 --log-file=/tmp/duk-api-test.log api-testcases/
endif

regfuzz-0.1.tar.gz:
	# https://code.google.com/p/regfuzz/
	# SHA1: 774be8e3dda75d095225ba699ac59969d92ac970
	$(WGET) https://regfuzz.googlecode.com/files/regfuzz-0.1.tar.gz

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

underscore:
	# http://underscorejs.org/
	# https://github.com/jashkenas/underscore
	# Master is OK because not a critical dependency
	$(GIT) clone --depth 1 https://github.com/jashkenas/underscore.git

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

d067d2f0ca30.tar.bz2:
	# http://test262.ecmascript.org/
	$(WGET) http://hg.ecmascript.org/tests/test262/archive/d067d2f0ca30.tar.bz2

test262-d067d2f0ca30: d067d2f0ca30.tar.bz2
	tar xfj d067d2f0ca30.tar.bz2

.PHONY: test262test
test262test: test262-d067d2f0ca30 duk
	@echo "### test262test"
	# http://wiki.ecmascript.org/doku.php?id=test262:command
	-rm -f /tmp/duk-test262.log /tmp/duk-test262-filtered.log
	cd test262-d067d2f0ca30; $(PYTHON) tools/packaging/test262.py --command "../duk {{path}}" --summary >/tmp/duk-test262.log
	cat /tmp/duk-test262.log | $(PYTHON) util/filter_test262_log.py doc/test262-known-issues.json > /tmp/duk-test262-filtered.log
	cat /tmp/duk-test262-filtered.log

# Unholy helper to write out a testcase, the unholiness is that it reads
# command line arguments and complains about missing targets etc:
# http://stackoverflow.com/questions/6273608/how-to-pass-argument-to-makefile-from-command-line
.PHONY: test262cat
test262cat: test262-d067d2f0ca30
	@echo "NOTE: this Makefile target will print a 'No rule...' error, ignore it" >&2
	@cd test262-d067d2f0ca30; $(PYTHON) tools/packaging/test262.py --command "../duk {{path}}" --cat $(filter-out $@,$(MAKECMDGOALS))

emscripten:
	# https://github.com/kripken/emscripten
	# Master is OK because not a critical dependency
	$(GIT) clone --depth 1 https://github.com/kripken/emscripten.git
	cd emscripten; ./emconfigure

# Reducing the TOTAL_MEMORY and TOTAL_STACK values is useful if you run
# Duktape cmdline with resource limits (i.e. "duk -r test.js").
# Recent Emscripten will assume typed array support unless EMCC_FAST_COMPILER=0
# is given through the environment:
# https://github.com/kripken/emscripten/wiki/LLVM-Backend
#EMCCOPTS=-s USE_TYPED_ARRAYS=0 -s TOTAL_MEMORY=2097152 -s TOTAL_STACK=524288
EMCCOPTS=-s USE_TYPED_ARRAYS=0

.PHONY: emscriptentest
emscriptentest: emscripten duk
	@echo "### emscriptentest"
	-@rm -f /tmp/duk-emcc-test*
	@echo "NOTE: this emscripten test is incomplete (compiles hello_world.cpp and tries to run it, no checks yet)"
	EMCC_FAST_COMPILER=0 emscripten/emcc $(EMCCOPTS) emscripten/tests/hello_world.cpp -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	./duk /tmp/duk-emcc-test-fixed.js
	#./duk /tmp/duk-emcc-test.js

# Compile Duktape with Emscripten and execute it with NodeJS.
# Current status: requires Duktape alignment fixes (alignment by 8).
# With manual alignment hacks the result works.  Emscripten options
# need to be chosen carefully:
#   - Without DUK_OPT_NO_PACKED_TVAL some asserts fail, at least when
#     -O2 (with asm.js) is used
#   - -O1 and above triggers asm.js use, which has at least the following
#     problem:
#     "too many setjmps in a function call, build with a higher value for MAX_SETJMPS"
#   - Using -O2 without asm.js for now.
# https://github.com/kripken/emscripten/wiki/Optimizing-Code
# http://mozakai.blogspot.fi/2013/08/outlining-workaround-for-jits-and-big.html
EMCCOPTS_DUKVM=-O2 -std=c99 -Wall -s OUTLINING_LIMIT=20000 -s MAX_SETJMPS=1000 -s ASM_JS=0 -DEMSCRIPTEN

MAND_BASE64=dyA9IDgwOyBoID0gNDA7IGl0ZXIgPSAxMDA7IGZvciAoaSA9IDA7IGkgLSBoOyBpICs9IDEpIHsgeTAgPSAoaSAvIGgpICogNC4wIC0gMi4wOyByZXMgPSBbXTsgZm9yIChqID0gMDsgaiAtIHc7IGogKz0gMSkgeyB4MCA9IChqIC8gdykgKiA0LjAgLSAyLjA7IHh4ID0gMDsgeXkgPSAwOyBjID0gIiMiOyBmb3IgKGsgPSAwOyBrIC0gaXRlcjsgayArPSAxKSB7IHh4MiA9IHh4Knh4OyB5eTIgPSB5eSp5eTsgaWYgKE1hdGgubWF4KDAsIDQuMCAtICh4eDIgKyB5eTIpKSkgeyB5eSA9IDIqeHgqeXkgKyB5MDsgeHggPSB4eDIgLSB5eTIgKyB4MDsgfSBlbHNlIHsgYyA9ICIuIjsgYnJlYWs7IH0gfSByZXNbcmVzLmxlbmd0aF0gPSBjOyB9IHByaW50KHJlcy5qb2luKCIiKSk7IH0K

.PHONY: emscriptenduktest
emscriptenduktest: emscripten dist
	@echo "### emscriptenduktest"
	-@rm -f /tmp/duk-emcc-duktest.js
	EMCC_FAST_COMPILER=0 emscripten/emcc $(EMCCOPTS_DUKVM) -DDUK_OPT_ASSERTIONS -DDUK_OPT_SELF_TESTS -Idist/src/ dist/src/duktape.c dist/examples/eval/eval.c -o /tmp/duk-emcc-duktest.js
	$(NODE) /tmp/duk-emcc-duktest.js \
		'print("Hello from Duktape running inside Emscripten/NodeJS");' \
		'print(Duktape.version, Duktape.env);' \
		'for(i=0;i++<100;)print((i%3?"":"Fizz")+(i%5?"":"Buzz")||i)'
	$(NODE) /tmp/duk-emcc-duktest.js "eval(''+Duktape.dec('base64', '$(MAND_BASE64)'))"

# This is a prototype of running Duktape in a web environment with Emscripten,
# and providing an eval() facility from both sides.  This is a placeholder now
# and doesn't do anything useful yet.
EMCCOPTS_DUKWEB_EXPORT=-s EXPORTED_FUNCTIONS='["_dukweb_is_open", "_dukweb_open","_dukweb_close","_dukweb_eval"]'
EMCCOPTS_DUKWEB_DEFINES=-DDUK_OPT_ASSERTIONS -DDUK_OPT_SELF_TESTS '-DDUK_OPT_DECLARE=extern void dukweb_panic_handler(int code, const char *msg);' '-DDUK_OPT_PANIC_HANDLER(code,msg)={dukweb_panic_handler((code),(msg));abort();}' 

# FIXME: need to be able to declare dukweb_panic_handler to avoid warnings
dukweb.js: emscripten dist
	EMCC_FAST_COMPILER=0 emscripten/emcc $(EMCCOPTS_DUKVM) $(EMCCOPTS_DUKWEB_EXPORT) $(EMCCOPTS_DUKWEB_DEFINES) \
		-Idist/src/ dist/src/duktape.c dukweb/dukweb.c -o dukweb.js
	cat dukweb/dukweb_extra.js >> dukweb.js
	@wc dukweb.js

.PHONY: dukwebtest
dukwebtest: dukweb.js jquery-1.11.0.js
	@echo "### dukwebtest"
	-@rm -rf /tmp/dukweb-test/
	mkdir /tmp/dukweb-test/
	cp dukweb.js jquery-1.11.0.js dukweb/dukweb.html dukweb/dukweb.css /tmp/dukweb-test/
	@echo "Now point your browser to: file:///tmp/dukweb-test/dukweb.html"

jquery-1.11.0.js:
	$(WGET) http://code.jquery.com/jquery-1.11.0.js

lua-5.2.3.tar.gz:
	$(WGET) http://www.lua.org/ftp/lua-5.2.3.tar.gz

lua-5.2.3: lua-5.2.3.tar.gz
	tar xfz lua-5.2.3.tar.gz

LUASRC=	lapi.c lauxlib.c lbaselib.c lbitlib.c lcode.c lcorolib.c lctype.c \
	ldblib.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c liolib.c \
	llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c loslib.c \
	lparser.c lstate.c lstring.c lstrlib.c ltable.c ltablib.c ltm.c \
	lua.c lundump.c lvm.c lzio.c

# This doesn't currently work: -s USE_TYPED_ARRAYS=0 causes compilation to
# fail.  Duktape also used to run out of registers trying to run the result,
# but this should no longer be the case.
.PHONY: emscriptenluatest
emscriptenluatest: emscripten duk lua-5.2.3
	@echo "### emscriptenluatest"
	-@rm -f /tmp/duk-emcc-luatest*
	EMCC_FAST_COMPILER=0 emscripten/emcc $(EMCCOPTS) -Ilua-5.2.3/src/ $(patsubst %,lua-5.2.3/src/%,$(LUASRC)) -o /tmp/duk-emcc-luatest.js
	cat /tmp/duk-emcc-luatest.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-luatest-fixed.js
	@ls -l /tmp/duk-emcc-luatest*
	./duk /tmp/duk-emcc-luatest-fixed.js

JS-Interpreter:
	# https://github.com/NeilFraser/JS-Interpreter
	# Master is OK because not a critical dependency
	$(GIT) clone --depth 1 https://github.com/NeilFraser/JS-Interpreter.git

.PHONY: jsinterpretertest
jsinterpretertest: JS-Interpreter duk
	@echo "### jsinterpretertest"
	-@rm -f /tmp/duk-jsint-test*
	echo "window = {};" > /tmp/duk-jsint-test.js
	cat JS-Interpreter/acorn.js JS-Interpreter/interpreter.js >> /tmp/duk-jsint-test.js
	cat jsinterpreter-testcases/addition.js >> /tmp/duk-jsint-test.js
	./duk /tmp/duk-jsint-test.js

luajs.zip:
	# https://github.com/mherkender/lua.js
	$(WGET) https://github.com/mherkender/lua.js/raw/precompiled2/luajs.zip

luajs: luajs.zip
	-@rm -rf luajs/
	mkdir luajs
	cd luajs; unzip ../luajs.zip

.PHONY: luajstest
luajstest: luajs duk
	-@rm -f /tmp/duk-luajs-mandel.js /tmp/duk-luajs-test.js
	luajs/lua2js luajs-testcases/mandel.lua /tmp/duk-luajs-mandel.js
	echo "console = { log: function() { print(Array.prototype.join.call(arguments, ' ')); } };" > /tmp/duk-luajs-test.js
	cat luajs/lua.js /tmp/duk-luajs-mandel.js >> /tmp/duk-luajs-test.js
	./duk /tmp/duk-luajs-test.js

# Closure
compiler-latest.zip:
	# https://code.google.com/p/closure-compiler/
	$(WGET) http://dl.google.com/closure-compiler/compiler-latest.zip

compiler.jar: compiler-latest.zip
	unzip compiler-latest.zip compiler.jar
	touch compiler.jar  # ensure date is newer than compiler-latest.zip

.PHONY: closuretest
closuretest: compiler.jar duk
	@echo "### closuretest"
	-@rm -f /tmp/duk-closure-test*
	$(JAVA) -jar compiler.jar ecmascript-testcases/test-dev-mandel2-func.js > /tmp/duk-closure-test.js
	./duk /tmp/duk-closure-test.js

UglifyJS:
	# https://github.com/mishoo/UglifyJS
	# Use a specific release because UglifyJS is used in building Duktape
	-@rm -f v1.3.5.tar.gz
	$(WGET) https://github.com/mishoo/UglifyJS/archive/v1.3.5.tar.gz
	tar xfz v1.3.5.tar.gz
	mv UglifyJS-1.3.5 UglifyJS
	-@rm -f v1.3.5.tar.gz

	# Don't use this because it's a moving critical dependency
	#$(GIT) clone --depth 1 https://github.com/mishoo/UglifyJS.git

.PHONY: UglifyJS2_setup
UglifyJS2_setup: UglifyJS2 UglifyJS2/node_modules

UglifyJS2:
	# https://github.com/mishoo/UglifyJS2
	# Use a specific release because UglifyJS2 is used in building Duktape
	# (This is now a bit futile because UglifyJS2 requires an 'npm install',
	# the NodeJS dependencies need to be controlled for this to really work.)
	-@rm -f v2.4.12.tar.gz
	$(WGET) https://github.com/mishoo/UglifyJS2/archive/v2.4.12.tar.gz
	tar xfz v2.4.12.tar.gz
	mv UglifyJS2-2.4.12 UglifyJS2
	-@rm -f v2.4.12.tar.gz

	# Don't use this because it's a moving critical dependency
	#$(GIT) clone --depth 1 https://github.com/mishoo/UglifyJS2.git

UglifyJS2/node_modules: UglifyJS2
	cd UglifyJS2; npm install; cd -

cloc-1.60.pl:
	# http://cloc.sourceforge.net/
	$(WGET) http://downloads.sourceforge.net/project/cloc/cloc/v1.60/cloc-1.60.pl

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

xmldoctest: sax-js xmldoc duk
	@echo "### xmldoctest"
	-@rm -f /tmp/duk-xmldoc-test*
	cat sax-js/lib/sax.js > /tmp/duk-xmldoc-test.js
	echo ";" >> /tmp/duk-xmldoc-test.js  # missing end semicolon causes automatic semicolon problem
	cat xmldoc/lib/xmldoc.js >> /tmp/duk-xmldoc-test.js
	echo ";" >> /tmp/duk-xmldoc-test.js  # missing end semicolon causes automatic semicolon problem
	cat xmldoc-testcases/basic.js >> /tmp/duk-xmldoc-test.js
	./duk /tmp/duk-xmldoc-test.js

FlameGraph:
	# http://www.brendangregg.com/FlameGraphs/cpuflamegraphs.html
	# https://github.com/brendangregg/FlameGraph
	$(GIT) clone --depth 1 https://github.com/brendangregg/FlameGraph.git

dtrace4linux:
	# https://github.com/dtrace4linux/linux
	# http://crtags.blogspot.fi/
	$(GIT) clone --depth 1 https://github.com/dtrace4linux/linux.git dtrace4linux

.PHONY: gccpredefs
gccpredefs:
	gcc -dM -E - < /dev/null

.PHONY: clangpredefs
clangpredefs:
	clang -dM -E - < /dev/null

.PHONY:	npminst
npminst:	runtests/node_modules

runtests/node_modules:
	echo "Installing required NodeJS modules for runtests"
	cd runtests; npm install

.PHONY:	doc
doc:	$(patsubst %.txt,%.html,$(wildcard doc/*.txt))

doc/%.html: doc/%.txt
	rst2html $< $@

# Source distributable for end users
dist:	debuglogcheck UglifyJS UglifyJS2_setup compiler.jar cloc-1.60.pl
	sh util/make_dist.sh

.PHONY:	dist-src
dist-src:	dist
	rm -rf duktape-$(DUK_VERSION_FORMATTED)
	rm -rf duktape-$(DUK_VERSION_FORMATTED).tar*
	mkdir duktape-$(DUK_VERSION_FORMATTED)
	cp -r dist/* duktape-$(DUK_VERSION_FORMATTED)/
	tar cvfz duktape-$(DUK_VERSION_FORMATTED).tar.gz duktape-$(DUK_VERSION_FORMATTED)/
	cp duktape-$(DUK_VERSION_FORMATTED).tar.gz duktape-$(DUK_VERSION_FORMATTED)-$(GIT_DESCRIBE).tar.gz
	tar cvfj duktape-$(DUK_VERSION_FORMATTED).tar.bz2 duktape-$(DUK_VERSION_FORMATTED)/
	cp duktape-$(DUK_VERSION_FORMATTED).tar.bz2 duktape-$(DUK_VERSION_FORMATTED)-$(GIT_DESCRIBE).tar.bz2
	tar cvf duktape-$(DUK_VERSION_FORMATTED).tar duktape-$(DUK_VERSION_FORMATTED)/
	xz -z -e -9 duktape-$(DUK_VERSION_FORMATTED).tar
	cp duktape-$(DUK_VERSION_FORMATTED).tar.xz duktape-$(DUK_VERSION_FORMATTED)-$(GIT_DESCRIBE).tar.xz
	zip -r duktape-$(DUK_VERSION_FORMATTED).zip duktape-$(DUK_VERSION_FORMATTED)/
	mkisofs -input-charset utf-8 -o duktape-$(DUK_VERSION_FORMATTED).iso duktape-$(DUK_VERSION_FORMATTED).tar.bz2
	cp duktape-$(DUK_VERSION_FORMATTED).iso duktape-$(DUK_VERSION_FORMATTED)-$(GIT_DESCRIBE).iso

.PHONY: tidy-site
tidy-site:
	for i in website/*/*.html; do echo "*** Checking $$i"; tidy -q -e -xml $$i; done

# Website
site: dukweb.js jquery-1.11.0.js
	rm -rf site
	mkdir site
	cd website/; $(PYTHON) buildsite.py ../site/
	-@rm -rf /tmp/site/
	cp -r site /tmp/  # FIXME

.PHONY:	dist-site
dist-site:	tidy-site site
	# When doing a final dist build, run html tidy
	rm -rf duktape-site-$(DUK_VERSION_FORMATTED)
	rm -rf duktape-site-$(DUK_VERSION_FORMATTED).tar*
	mkdir duktape-site-$(DUK_VERSION_FORMATTED)
	cp -r site/* duktape-site-$(DUK_VERSION_FORMATTED)/
	tar cvf duktape-site-$(DUK_VERSION_FORMATTED).tar duktape-site-$(DUK_VERSION_FORMATTED)/
	xz -z -e -9 duktape-site-$(DUK_VERSION_FORMATTED).tar

