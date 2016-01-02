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
#  The Makefile now also works in a very limited fashion with Cygwin,
#  you can 'make dist' as long as you have enough software installed.
#  The closure compiler requires a working JDK installation (JRE is
#  not enough, JAVA_HOME must point to the JDK), and a working Apache
#  'ant' command.
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
#  The makefile should work with -jN except for the 'clean' target, use:
#
#    $ make clean; make -j4
#

# A few commands which may need to be edited.  NodeJS is sometimes found
# as 'nodejs', sometimes as 'node'; sometimes 'node' is unrelated to NodeJS
# so check 'nodejs' first.
GIT:=$(shell which git)
NODE:=$(shell which nodejs node | head -1)
WGET:=$(shell which wget)
JAVA:=$(shell which java)
VALGRIND:=$(shell which valgrind)
PYTHON:=$(shell which python)

# Scrape version from the public header; convert from e.g. 10203 -> '1.2.3'
DUK_VERSION:=$(shell cat src/duk_api_public.h.in | grep define | grep DUK_VERSION | tr -s ' ' ' ' | cut -d ' ' -f 3 | tr -d 'L')
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
	$(DISTSRCSEP)/duk_util_bufwriter.c \
	$(DISTSRCSEP)/duk_util_misc.c \
	$(DISTSRCSEP)/duk_alloc_default.c \
	$(DISTSRCSEP)/duk_debug_macros.c \
	$(DISTSRCSEP)/duk_debug_vsnprintf.c \
	$(DISTSRCSEP)/duk_debug_heap.c \
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
	$(DISTSRCSEP)/duk_hbufferobject_misc.c \
	$(DISTSRCSEP)/duk_unicode_tables.c \
	$(DISTSRCSEP)/duk_unicode_support.c \
	$(DISTSRCSEP)/duk_debugger.c \
	$(DISTSRCSEP)/duk_builtins.c \
	$(DISTSRCSEP)/duk_js_ops.c \
	$(DISTSRCSEP)/duk_js_var.c \
	$(DISTSRCSEP)/duk_numconv.c \
	$(DISTSRCSEP)/duk_api_stack.c \
	$(DISTSRCSEP)/duk_api_heap.c \
	$(DISTSRCSEP)/duk_api_call.c \
	$(DISTSRCSEP)/duk_api_compile.c \
	$(DISTSRCSEP)/duk_api_bytecode.c \
	$(DISTSRCSEP)/duk_api_codec.c \
	$(DISTSRCSEP)/duk_api_memory.c \
	$(DISTSRCSEP)/duk_api_string.c \
	$(DISTSRCSEP)/duk_api_object.c \
	$(DISTSRCSEP)/duk_api_buffer.c \
	$(DISTSRCSEP)/duk_api_var.c \
	$(DISTSRCSEP)/duk_api_logging.c \
	$(DISTSRCSEP)/duk_api_debug.c \
	$(DISTSRCSEP)/duk_lexer.c \
	$(DISTSRCSEP)/duk_tval.c \
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
	$(DISTSRCSEP)/duk_bi_date_unix.c \
	$(DISTSRCSEP)/duk_bi_date_windows.c \
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
	$(DISTSRCSEP)/duk_selftest.c \
	$(DISTSRCSEP)/duk_strings.c

# Use combined sources for testing etc.
DUKTAPE_SOURCES = $(DUKTAPE_SOURCES_COMBINED)
#DUKTAPE_SOURCES = $(DUKTAPE_SOURCES_SEPARATE)

# Duktape command line tool - example of a main() program, used
# for unit testing
DUKTAPE_CMDLINE_SOURCES = \
	$(DISTCMD)/duk_cmdline.c \
	dist/examples/alloc-logging/duk_alloc_logging.c \
	dist/examples/alloc-torture/duk_alloc_torture.c \
	dist/examples/alloc-hybrid/duk_alloc_hybrid.c \
	dist/examples/debug-trans-socket/duk_trans_socket.c

# Compiler setup for Linux
CC	= gcc
GXX	= g++

CCOPTS_FEATURES =
#CCOPTS_FEATURES += -DDUK_OPT_NO_PACKED_TVAL
#CCOPTS_FEATURES += -DDUK_OPT_FORCE_ALIGN=4
#CCOPTS_FEATURES += -DDUK_OPT_FORCE_ALIGN=8
#CCOPTS_FEATURES += -DDUK_OPT_FORCE_BYTEORDER=1      # little
#CCOPTS_FEATURES += -DDUK_OPT_FORCE_BYTEORDER=2      # middle
#CCOPTS_FEATURES += -DDUK_OPT_FORCE_BYTEORDER=3      # big
#CCOPTS_FEATURES += -DDUK_OPT_NO_REFERENCE_COUNTING
#CCOPTS_FEATURES += -DDUK_OPT_NO_MARK_AND_SWEEP
#CCOPTS_FEATURES += -DDUK_OPT_NO_VOLUNTARY_GC
CCOPTS_FEATURES += -DDUK_OPT_SEGFAULT_ON_PANIC       # segfault on panic allows valgrind to show stack trace on panic
CCOPTS_FEATURES += -DDUK_OPT_DPRINT_COLORS
#CCOPTS_FEATURES += -DDUK_OPT_NO_FILE_IO
#CCOPTS_FEATURES += '-DDUK_OPT_PANIC_HANDLER(code,msg)={printf("*** %d:%s\n",(code),(msg));abort();}'
CCOPTS_FEATURES += -DDUK_OPT_SELF_TESTS
#CCOPTS_FEATURES += -DDUK_OPT_NO_TRACEBACKS
#CCOPTS_FEATURES += -DDUK_OPT_NO_PC2LINE
#CCOPTS_FEATURES += -DDUK_OPT_NO_VERBOSE_ERRORS
#CCOPTS_FEATURES += -DDUK_OPT_PARANOID_ERRORS
#CCOPTS_FEATURES += -DDUK_OPT_NO_AUGMENT_ERRORS
#CCOPTS_FEATURES += -DDUK_OPT_GC_TORTURE
#CCOPTS_FEATURES += -DDUK_OPT_SHUFFLE_TORTURE
#CCOPTS_FEATURES += -DDUK_OPT_REFZERO_FINALIZER_TORTURE
#CCOPTS_FEATURES += -DDUK_OPT_MARKANDSWEEP_FINALIZER_TORTURE
#CCOPTS_FEATURES += -DDUK_OPT_NO_MS_RESIZE_STRINGTABLE
CCOPTS_FEATURES += -DDUK_OPT_DEBUG_BUFSIZE=512
#CCOPTS_FEATURES += -DDUK_OPT_NO_STRICT_DECL
#CCOPTS_FEATURES += -DDUK_OPT_NO_REGEXP_SUPPORT
#CCOPTS_FEATURES += -DDUK_OPT_NO_OCTAL_SUPPORT
#CCOPTS_FEATURES += -DDUK_OPT_NO_SOURCE_NONBMP
#CCOPTS_FEATURES += -DDUK_OPT_STRICT_UTF8_SOURCE
#CCOPTS_FEATURES += -DDUK_OPT_NO_BROWSER_LIKE
#CCOPTS_FEATURES += -DDUK_OPT_NO_SECTION_B
CCOPTS_FEATURES += -DDUK_OPT_INTERRUPT_COUNTER
CCOPTS_FEATURES += -DDUK_OPT_DEBUGGER_SUPPORT
CCOPTS_FEATURES += -DDUK_OPT_DEBUGGER_FWD_PRINTALERT
CCOPTS_FEATURES += -DDUK_OPT_DEBUGGER_FWD_LOGGING
CCOPTS_FEATURES += -DDUK_OPT_DEBUGGER_DUMPHEAP
#CCOPTS_FEATURES += -DDUK_OPT_NO_DEBUGGER_THROW_NOTIFY
#CCOPTS_FEATURES += -DDUK_OPT_DEBUGGER_PAUSE_UNCAUGHT
#CCOPTS_FEATURES += -DDUK_OPT_DEBUGGER_TRANSPORT_TORTURE
CCOPTS_FEATURES += -DDUK_OPT_TARGET_INFO='"duk command built from Duktape repo"'
#CCOPTS_FEATURES += -DDUK_OPT_NO_JX
#CCOPTS_FEATURES += -DDUK_OPT_NO_JC
#CCOPTS_FEATURES += -DDUK_OPT_NO_NONSTD_ACCESSOR_KEY_ARGUMENT
#CCOPTS_FEATURES += -DDUK_OPT_NO_NONSTD_FUNC_STMT
#CCOPTS_FEATURES += -DDUK_OPT_NONSTD_FUNC_CALLER_PROPERTY
#CCOPTS_FEATURES += -DDUK_OPT_NONSTD_FUNC_SOURCE_PROPERTY
#CCOPTS_FEATURES += -DDUK_OPT_NO_NONSTD_ARRAY_SPLICE_DELCOUNT
#CCOPTS_FEATURES += -DDUK_OPT_NO_NONSTD_ARRAY_CONCAT_TRAILER
#CCOPTS_FEATURES += -DDUK_OPT_NO_NONSTD_ARRAY_MAP_TRAILER
#CCOPTS_FEATURES += -DDUK_OPT_NO_NONSTD_JSON_ESC_U2028_U2029
#CCOPTS_FEATURES += -DDUK_OPT_NO_NONSTD_STRING_FROMCHARCODE_32BIT
#CCOPTS_FEATURES += -DDUK_OPT_NO_ES6_OBJECT_PROTO_PROPERTY
#CCOPTS_FEATURES += -DDUK_OPT_NO_ES6_OBJECT_SETPROTOTYPEOF
#CCOPTS_FEATURES += -DDUK_OPT_NO_ES6_PROXY
#CCOPTS_FEATURES += -DDUK_OPT_NO_ZERO_BUFFER_DATA
#CCOPTS_FEATURES += -DDUK_OPT_USER_INITJS='"this.foo = 123"'
#CCOPTS_FEATURES += -DDUK_OPT_SETJMP
#CCOPTS_FEATURES += -DDUK_OPT_UNDERSCORE_SETJMP
#CCOPTS_FEATURES += -DDUK_OPT_SIGSETJMP
#CCOPTS_FEATURES += -DDUK_OPT_CPP_EXCEPTIONS
#CCOPTS_FEATURES += -DDUK_OPT_LIGHTFUNC_BUILTINS
CCOPTS_FEATURES += -DDUK_OPT_FASTINT
#CCOPTS_FEATURES += -DDUK_OPT_REFCOUNT16
#CCOPTS_FEATURES += -DDUK_OPT_STRHASH16
#CCOPTS_FEATURES += -DDUK_OPT_STRLEN16
#CCOPTS_FEATURES += -DDUK_OPT_BUFLEN16
#CCOPTS_FEATURES += -DDUK_OPT_OBJSIZES16
#CCOPTS_FEATURES += -DDUK_OPT_STRTAB_CHAIN
#CCOPTS_FEATURES += -DDUK_OPT_STRTAB_CHAIN_SIZE=128
#CCOPTS_FEATURES += -DDUK_OPT_HEAPPTR16
#CCOPTS_FEATURES += '-DDUK_OPT_HEAPPTR_ENC16(ud,p)=XXX'
#CCOPTS_FEATURES += '-DDUK_OPT_HEAPPTR_DEC16(ud,x)=XXX'
#CCOPTS_FEATURES += '-DDUK_OPT_DECLARE=XXX'
#CCOPTS_FEATURES += -DDUK_OPT_REGEXP_CANON_WORKAROUND
CCOPTS_FEATURES += -DDUK_OPT_JSON_STRINGIFY_FASTPATH
CCOPTS_FEATURES += -DDUK_CMDLINE_FANCY
CCOPTS_FEATURES += -DDUK_CMDLINE_ALLOC_LOGGING
CCOPTS_FEATURES += -DDUK_CMDLINE_ALLOC_TORTURE
CCOPTS_FEATURES += -DDUK_CMDLINE_ALLOC_HYBRID
CCOPTS_FEATURES += -DDUK_CMDLINE_DEBUGGER_SUPPORT

CCOPTS_SHARED =
CCOPTS_SHARED += -pedantic -ansi -std=c99 -fstrict-aliasing
# -Wextra is very picky but catches e.g. signed/unsigned comparisons
CCOPTS_SHARED += -Wall -Wextra -Wunused-result
CCOPTS_SHARED += -Wcast-qual
# -Wfloat-equal is too picky, there's no apparent way to compare floats
# (even when you know it's safe) without triggering warnings
CCOPTS_SHARED += -I./dist/src -I./dist/examples/alloc-logging -I./dist/examples/alloc-torture -I./dist/examples/alloc-hybrid -I./dist/examples/debug-trans-socket
#CCOPTS_SHARED += -I./dist/src-separate
#CCOPTS_SHARED += -m32                             # force 32-bit compilation on a 64-bit host
#CCOPTS_SHARED += -mx32                            # force X32 compilation on a 64-bit host

CCOPTS_NONDEBUG = $(CCOPTS_SHARED) $(CCOPTS_FEATURES)
CCOPTS_NONDEBUG += -Os -fomit-frame-pointer
CCOPTS_NONDEBUG += -g -ggdb
#CCOPTS_NONDEBUG += -DDUK_OPT_ASSERTIONS

CCOPTS_DEBUG = $(CCOPTS_SHARED) $(CCOPTS_FEATURES)
CCOPTS_DEBUG += -O0
CCOPTS_DEBUG += -g -ggdb
CCOPTS_DEBUG += -DDUK_OPT_DEBUG
CCOPTS_DEBUG += -DDUK_OPT_DPRINT
#CCOPTS_DEBUG += -DDUK_OPT_DDPRINT
#CCOPTS_DEBUG += -DDUK_OPT_DDDPRINT
CCOPTS_DEBUG += -DDUK_OPT_ASSERTIONS

GXXOPTS_NONDEBUG = -pedantic -ansi -std=c++11 -fstrict-aliasing -Wall -Wextra -Wunused-result -Os -fomit-frame-pointer
GXXOPTS_NONDEBUG += -I./dist/src -I./dist/examples/alloc-logging -I./dist/examples/alloc-torture -I./dist/examples/alloc-hybrid
GXXOPTS_NONDEBUG += -DDUK_OPT_DEBUGGER_SUPPORT -DDUK_OPT_INTERRUPT_COUNTER
GXXOPTS_DEBUG = -pedantic -ansi -std=c++11 -fstrict-aliasing -Wall -Wextra -Wunused-result -O0 -g -ggdb
GXXOPTS_DEBUG += -I./dist/src -I./dist/examples/alloc-logging -I./dist/examples/alloc-torture -I./dist/examples/alloc-hybrid
GXXOPTS_DEBUG += -DDUK_OPT_DEBUG -DDUK_OPT_DPRINT -DDUK_OPT_ASSERTIONS -DDUK_OPT_SELF_TESTS
#GXXOPTS_DEBUG += -DDUK_OPT_DDPRINT -DDUK_OPT_DDDPRINT

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
	@rm -rf dist/
	@rm -rf site/
	@rm -f duk.raw dukd.raw duk.vg dukd.vg duk dukd duk.O2 duk.O3 duk.O4
	@rm -f duk-g++ dukd-g++
	@rm -f duk-clang
	@rm -f ajduk ajdukd
	@rm -f emduk emduk.js
	@rm -f libduktape*.so*
	@rm -f duktape-*.tar.*
	@rm -f duktape-*.iso
	@rm -f doc/*.html
	@rm -f src/*.pyc
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

libduktape.so.1.0.0: dist
	rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

libduktaped.so.1.0.0: dist
	rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

duk.raw: dist
	$(CC) -o $@ $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	-@size $@

duk-clang: dist
	# Use -Wcast-align to trigger issues like: https://github.com/svaarala/duktape/issues/270
	clang -o $@ -Wcast-align $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	-@size $@

duk.O2: dist
	$(CC) -o $@ $(CCOPTS_NONDEBUG) -O2 $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	-@size $@

duk.O3: dist
	$(CC) -o $@ $(CCOPTS_NONDEBUG) -O3 $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	-@size $@

duk.O4: dist
	$(CC) -o $@ $(CCOPTS_NONDEBUG) -O4 $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	-@size $@

# Test target for g++ compile
duk-g++: dist
	$(GXX) -o $@ $(GXXOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	-@size $@
dukd-g++: dist
	$(GXX) -o $@ $(GXXOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	-@size $@

dump-public: duk.raw
	@(objdump -t $< | grep ' g' | grep .text | grep -v .hidden | tr -s ' ' | cut -d ' ' -f 5 | sort > /tmp/duk-public.txt ; true)
	@echo "Symbol dump in /tmp/duk-public.txt"
	@(grep duk__ /tmp/duk-public.txt ; true)  # check for leaked file local symbols (does not cover internal, but not public symbols)

duk.vg: duk.raw
	@rm -f $@
	@echo '#!/bin/sh' > $@
	@echo 'valgrind "$(shell pwd)/$<" "$$@"' >> $@
	@chmod ugo+rx $@

duk: duk.raw duk.vg
	@rm -f $@
ifeq ($(VALGRIND_WRAP),1)
	@echo "Using valgrind wrapped $@"
	@cp duk.vg $@
else
	@cp duk.raw $@
endif

dukd.raw: dist
	$(CC) -o $@ $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
	-@size $@

dukd.vg: dukd.raw
	@rm -f $@
	@echo '#!/bin/sh' > $@
	@echo 'valgrind "$(shell pwd)/$<" "$$@"' >> $@
	@chmod ugo+rx $@

dukd: dukd.raw dukd.vg
	@rm -f dukd
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

.PHONY: dukscanbuild
dukscanbuild: dist
	scan-build gcc -o/tmp/duk.scanbuild -Idist/src-separate/ $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES_SEPARATE) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

.PHONY: dukdscanbuild
dukdscanbuild: dist
	scan-build gcc -o/tmp/duk.scanbuild -Idist/src-separate/ $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES_SEPARATE) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

.PHONY: test
test: qecmatest apitest regfuzztest underscoretest lodashtest emscriptentest emscripteninceptiontest test262test

RUNTESTSOPTS=--prep-test-path util/prep_test.py --minify-uglifyjs2 UglifyJS2/bin/uglifyjs --util-include-path tests/ecmascript --known-issues doc/testcase-known-issues.yaml

.PHONY:	ecmatest
ecmatest: runtestsdeps duk
ifeq ($(VALGRIND_WRAP),1)
	@echo "### ecmatest (valgrind)"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/duk.raw --report-diff-to-other --valgrind --run-nodejs --run-rhino --num-threads 1 --log-file=/tmp/duk-test.log tests/ecmascript/
else
	@echo "### ecmatest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/duk --report-diff-to-other --run-nodejs --run-rhino --num-threads 4 --log-file=/tmp/duk-test.log tests/ecmascript/
endif

.PHONY:	ecmatestd
ecmatestd: runtestsdeps dukd
ifeq ($(VALGRIND_WRAP),1)
	@echo "### ecmatestd (valgrind)"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/dukd.raw --report-diff-to-other --valgrind --run-nodejs --run-rhino --num-threads 1 --log-file=/tmp/duk-test.log tests/ecmascript/
else
	@echo "### ecmatestd"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/dukd --report-diff-to-other --run-nodejs --run-rhino --num-threads 4 --log-file=/tmp/duk-test.log tests/ecmascript/
endif

.PHONY:	qecmatest
qecmatest: runtestsdeps duk
ifeq ($(VALGRIND_WRAP),1)
	@echo "### qecmatest (valgrind)"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/duk.raw --valgrind --num-threads 1 --log-file=/tmp/duk-test.log tests/ecmascript/
else
	@echo "### qecmatest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/duk --num-threads 4  --log-file=/tmp/duk-test.log tests/ecmascript/
endif

.PHONY:	qecmatestd
qecmatestd: runtestsdeps dukd
ifeq ($(VALGRIND_WRAP),1)
	@echo "### qecmatestd (valgrind)"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/dukd.raw --valgrind --num-threads 1 --log-file=/tmp/duk-test.log tests/ecmascript/
else
	@echo "### qecmatestd"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --run-duk --cmd-duk=$(shell pwd)/dukd --num-threads 4 --log-file=/tmp/duk-test.log tests/ecmascript/
endif

# Separate target because it's also convenient to run manually.
.PHONY: apiprep
apiprep: runtestsdeps libduktape.so.1.0.0

.PHONY:	apitest
apitest: apiprep
ifeq ($(VALGRIND_WRAP),1)
	@echo "### apitest (valgrind)"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --num-threads 1 --valgrind --log-file=/tmp/duk-api-test.log tests/api/
else
	@echo "### apitest"
	"$(NODE)" runtests/runtests.js $(RUNTESTSOPTS) --num-threads 1 --log-file=/tmp/duk-api-test.log tests/api/
endif

.PHONY: matrix10
matrix10: dist
	cd dist; python ../util/matrix_compile.py --count=10
.PHONY: matrix100
matrix100: dist
	cd dist; python ../util/matrix_compile.py --count=100
.PHONY: matrix1000
matrix1000: dist
	cd dist; python ../util/matrix_compile.py --count=1000
.PHONY: matrix10000
matrix10000: dist
	cd dist; python ../util/matrix_compile.py --count=10000

regfuzz-0.1.tar.gz:
	# https://code.google.com/p/regfuzz/
	# SHA1: 774be8e3dda75d095225ba699ac59969d92ac970
	$(WGET) https://regfuzz.googlecode.com/files/regfuzz-0.1.tar.gz -O $@

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

lodash:
	# http://lodash.com/
	# https://github.com/lodash
	# Master is OK because not a critical dependency
	$(GIT) clone --depth 1 https://github.com/lodash/lodash.git

# Lodash test.js assumes require() etc.  Placeholder test for now, no
# expect string etc.
.PHONY: lodashtest
lodashtest: lodash duk
	./duk lodash/lodash.js tests/lodash/basic.js

3883a2e9063b0a5f2705bdac3263577a03913c94.zip:
	# http://test262.ecmascript.org/
	# https://github.com/tc39/test262
	# HG repo seems to have migrated to https://github.com/tc39/test262
	#$(WGET) http://hg.ecmascript.org/tests/test262/archive/d067d2f0ca30.tar.bz2 -O $@
	#$(WGET) https://github.com/tc39/test262/archive/595a36b252ee97110724e6fa89fc92c9aa9a206a.zip -O $@
	# This is a snapshot from the master, and seems to have test case bugs
	$(WGET) https://github.com/tc39/test262/archive/3883a2e9063b0a5f2705bdac3263577a03913c94.zip -O $@
test262-3883a2e9063b0a5f2705bdac3263577a03913c94: 3883a2e9063b0a5f2705bdac3263577a03913c94.zip
	unzip $<
	touch $@

es5-tests.zip:
	# https://github.com/tc39/test262/tree/es5-tests
	# This is a stable branch for ES5 tests
	$(WGET) https://github.com/tc39/test262/archive/es5-tests.zip -O $@
test262-es5-tests: es5-tests.zip
	unzip $<
	touch $@

.PHONY: test262test
test262test: test262-es5-tests duk
	@echo "### test262test"
	# http://wiki.ecmascript.org/doku.php?id=test262:command
	rm -f /tmp/duk-test262.log /tmp/duk-test262-filtered.log
	-cd $<; $(PYTHON) tools/packaging/test262.py --command "../duk {{path}}" --summary >/tmp/duk-test262.log
	cat /tmp/duk-test262.log | $(PYTHON) util/filter_test262_log.py doc/test262-known-issues.yaml > /tmp/duk-test262-filtered.log
	cat /tmp/duk-test262-filtered.log

# Unholy helper to write out a testcase, the unholiness is that it reads
# command line arguments and complains about missing targets etc:
# http://stackoverflow.com/questions/6273608/how-to-pass-argument-to-makefile-from-command-line
.PHONY: test262cat
test262cat: test262-es5-tests
	@echo "NOTE: this Makefile target will print a 'No rule...' error, ignore it" >&2
	-@cd $<; $(PYTHON) tools/packaging/test262.py --command "../duk {{path}}" --cat $(filter-out $@,$(MAKECMDGOALS))

emscripten:
	# https://github.com/kripken/emscripten
	# Master is OK because not a critical dependency
	# Setup is complicated because needs matching fastcomp which
	# you must provide yourself and add to ~/.emscripten:
	# http://kripken.github.io/emscripten-site/docs/building_from_source/building_fastcomp_manually_from_source.html
	$(GIT) clone --depth 1 https://github.com/kripken/emscripten.git
	cd emscripten; ./emconfigure

# Reducing the TOTAL_MEMORY and TOTAL_STACK values is useful if you run
# Duktape cmdline with resource limits (i.e. "duk -r test.js").
#EMCCOPTS=-s TOTAL_MEMORY=2097152 -s TOTAL_STACK=524288 --memory-init-file 0
EMCCOPTS=-O2 -std=c99 -Wall --memory-init-file 0

EMDUKOPTS=-s TOTAL_MEMORY=268435456 -DDUK_OPT_NO_FASTINT -DDUK_OPT_NO_PACKED_TVAL
EMDUKOPTS+=-DEMSCRIPTEN  # enable stdin workaround in duk_cmdline.c
emduk: emduk.js
	cat util/emduk_wrapper.sh | sed "s|WORKDIR|$(shell pwd)|" > $@
	chmod ugo+x $@

# util/fix_emscripten.py is used so that emduk.js can also be executed using
# Duktape itself (though you can't currently pass arguments/files to it).
emduk.js: dist emscripten
	emscripten/emcc $(EMCCOPTS) -Idist/src -Idist/examples/cmdline \
		$(EMDUKOPTS) \
		dist/src/duktape.c dist/examples/cmdline/duk_cmdline.c \
		-o /tmp/duk-emduk.js
	cat /tmp/duk-emduk.js | $(PYTHON) util/fix_emscripten.py > $@
	@ls -l $@

.PHONY: emscriptentest
emscriptentest: emscripten duk
	@echo "### emscriptentest"
	@rm -f /tmp/duk-emcc-test*
	@echo "NOTE: this emscripten test is incomplete (compiles helloworld.c and tries to run it, no checks yet)"
	emscripten/emcc $(EMCCOPTS) tests/emscripten/helloworld.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	./duk /tmp/duk-emcc-test-fixed.js
	#./duk /tmp/duk-emcc-test.js

.PHONY: emscriptenmandeltest
emscriptenmandeltest: emscripten duk
	@echo "### emscriptenmandeltest"
	@rm -f /tmp/duk-emcc-test*
	@echo "NOTE: this emscripten test is incomplete (compiles mandelbrot.c and tries to run it, no checks yet)"
	emscripten/emcc $(EMCCOPTS) tests/emscripten/mandelbrot.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	./duk /tmp/duk-emcc-test-fixed.js
	#./duk /tmp/duk-emcc-test.js

# Compile Duktape and hello.c using Emscripten and execute the result with
# Duktape.
.PHONY: emscripteninceptiontest
emscripteninceptiontest: emscripten dist duk
	@echo "### emscripteniceptiontest"
	@rm -f /tmp/duk-emcc-test*
	@echo "NOTE: this emscripten test is incomplete (compiles Duktape and hello.c and tries to run it, no checks yet)"
	emscripten/emcc $(EMCCOPTS) -Idist/src dist/src/duktape.c dist/examples/hello/hello.c -o /tmp/duk-emcc-test.js
	cat /tmp/duk-emcc-test.js | $(PYTHON) util/fix_emscripten.py > /tmp/duk-emcc-test-fixed.js
	@ls -l /tmp/duk-emcc-test*
	./duk /tmp/duk-emcc-test-fixed.js
	#./duk /tmp/duk-emcc-test.js

# Compile Duktape with Emscripten and execute it with NodeJS:
#   - --memory-init-file 0 to avoid a separate memory init file (this is
#     not mandatory but keeps the result in a single file)
#   - -DEMSCRIPTEN needed by Duktape for feature detection
# https://github.com/kripken/emscripten/wiki/Optimizing-Code
# http://mozakai.blogspot.fi/2013/08/outlining-workaround-for-jits-and-big.html
EMCCOPTS_DUKVM=-O2 -std=c99 -Wall --memory-init-file 0 -DEMSCRIPTEN

MAND_BASE64=dyA9IDgwOyBoID0gNDA7IGl0ZXIgPSAxMDA7IGZvciAoaSA9IDA7IGkgLSBoOyBpICs9IDEpIHsgeTAgPSAoaSAvIGgpICogNC4wIC0gMi4wOyByZXMgPSBbXTsgZm9yIChqID0gMDsgaiAtIHc7IGogKz0gMSkgeyB4MCA9IChqIC8gdykgKiA0LjAgLSAyLjA7IHh4ID0gMDsgeXkgPSAwOyBjID0gIiMiOyBmb3IgKGsgPSAwOyBrIC0gaXRlcjsgayArPSAxKSB7IHh4MiA9IHh4Knh4OyB5eTIgPSB5eSp5eTsgaWYgKE1hdGgubWF4KDAsIDQuMCAtICh4eDIgKyB5eTIpKSkgeyB5eSA9IDIqeHgqeXkgKyB5MDsgeHggPSB4eDIgLSB5eTIgKyB4MDsgfSBlbHNlIHsgYyA9ICIuIjsgYnJlYWs7IH0gfSByZXNbcmVzLmxlbmd0aF0gPSBjOyB9IHByaW50KHJlcy5qb2luKCIiKSk7IH0K

.PHONY: emscriptenduktest
emscriptenduktest: emscripten dist
	@echo "### emscriptenduktest"
	@rm -f /tmp/duk-emcc-duktest.js
	emscripten/emcc $(EMCCOPTS_DUKVM) -DDUK_OPT_ASSERTIONS -DDUK_OPT_SELF_TESTS -Idist/src/ dist/src/duktape.c dist/examples/eval/eval.c -o /tmp/duk-emcc-duktest.js
	"$(NODE)" /tmp/duk-emcc-duktest.js \
		'print("Hello from Duktape running inside Emscripten/NodeJS");' \
		'print(Duktape.version, Duktape.env);' \
		'for(i=0;i++<100;)print((i%3?"":"Fizz")+(i%5?"":"Buzz")||i)'
	"$(NODE)" /tmp/duk-emcc-duktest.js "eval(''+Duktape.dec('base64', '$(MAND_BASE64)'))"

# This is a prototype of running Duktape in a web environment with Emscripten,
# and providing an eval() facility from both sides.  This is a placeholder now
# and doesn't do anything useful yet.
EMCCOPTS_DUKWEB_EXPORT=-s EXPORTED_FUNCTIONS='["_dukweb_is_open", "_dukweb_open","_dukweb_close","_dukweb_eval"]'
EMCCOPTS_DUKWEB_DEFINES='-DDUK_OPT_DECLARE=extern void dukweb_panic_handler(int code, const char *msg);' '-DDUK_OPT_PANIC_HANDLER(code,msg)={dukweb_panic_handler((code),(msg));abort();}'
#EMCCOPTS_DUKWEB_DEFINES+=-DDUK_OPT_ASSERTIONS
EMCCOPTS_DUKWEB_DEFINES+=-DDUK_OPT_SELF_TESTS

dukweb.js: emscripten dist
	emscripten/emcc $(EMCCOPTS_DUKVM) $(EMCCOPTS_DUKWEB_EXPORT) $(EMCCOPTS_DUKWEB_DEFINES) \
		-Idist/src/ dist/src/duktape.c dukweb/dukweb.c -o dukweb.js
	cat dukweb/dukweb_extra.js >> dukweb.js
	@wc dukweb.js

.PHONY: dukwebtest
dukwebtest: dukweb.js jquery-1.11.0.js
	@echo "### dukwebtest"
	@rm -rf /tmp/dukweb-test/
	mkdir /tmp/dukweb-test/
	cp dukweb.js jquery-1.11.0.js dukweb/dukweb.html dukweb/dukweb.css /tmp/dukweb-test/
	@echo "Now point your browser to: file:///tmp/dukweb-test/dukweb.html"

jquery-1.11.0.js:
	$(WGET) http://code.jquery.com/jquery-1.11.0.js -O $@

lua-5.2.3.tar.gz:
	$(WGET) http://www.lua.org/ftp/lua-5.2.3.tar.gz -O $@

lua-5.2.3: lua-5.2.3.tar.gz
	tar xfz lua-5.2.3.tar.gz

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
	./duk /tmp/duk-emcc-luatest-fixed.js

JS-Interpreter:
	# https://github.com/NeilFraser/JS-Interpreter
	# Master is OK because not a critical dependency
	$(GIT) clone --depth 1 https://github.com/NeilFraser/JS-Interpreter.git

.PHONY: jsinterpretertest
jsinterpretertest: JS-Interpreter duk
	@echo "### jsinterpretertest"
	@rm -f /tmp/duk-jsint-test*
	echo "window = {};" > /tmp/duk-jsint-test.js
	cat JS-Interpreter/acorn.js JS-Interpreter/interpreter.js >> /tmp/duk-jsint-test.js
	cat tests/jsinterpreter/addition.js >> /tmp/duk-jsint-test.js
	./duk /tmp/duk-jsint-test.js

luajs.zip:
	# https://github.com/mherkender/lua.js
	$(WGET) https://github.com/mherkender/lua.js/raw/precompiled2/luajs.zip -O $@

luajs: luajs.zip
	@rm -rf luajs/
	mkdir luajs
	cd luajs; unzip ../luajs.zip

.PHONY: luajstest
luajstest: luajs duk
	@rm -f /tmp/duk-luajs-mandel.js /tmp/duk-luajs-test.js
	luajs/lua2js tests/luajs/mandel.lua /tmp/duk-luajs-mandel.js
	echo "console = { log: function() { print(Array.prototype.join.call(arguments, ' ')); } };" > /tmp/duk-luajs-test.js
	cat luajs/lua.js /tmp/duk-luajs-mandel.js >> /tmp/duk-luajs-test.js
	./duk /tmp/duk-luajs-test.js

bluebird.js:
	$(WGET) https://cdn.jsdelivr.net/bluebird/latest/bluebird.js -O $@

.PHONY: bluebirdtest
bluebirdtest: bluebird.js duk
	@rm -f /tmp/duk-bluebird-test.js
	cat util/bluebird-test-shim.js bluebird.js > /tmp/duk-bluebird-test.js
	echo "var myPromise = new Promise(function(resolve, reject) { setTimeout(function () { resolve('resolved 123') }, 1000); });" >> /tmp/duk-bluebird-test.js
	echo "myPromise.then(function (v) { print('then:', v); });" >> /tmp/duk-bluebird-test.js
	echo "fakeEventLoop();" >> /tmp/duk-bluebird-test.js
	./duk /tmp/duk-bluebird-test.js

# Closure
compiler-latest.zip:
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

.PHONY: closuretest
closuretest: compiler.jar duk
	@echo "### closuretest"
	@rm -f /tmp/duk-closure-test*
	$(JAVA) -jar compiler.jar tests/ecmascript/test-dev-mandel2-func.js > /tmp/duk-closure-test.js
	./duk /tmp/duk-closure-test.js

UglifyJS:
	# https://github.com/mishoo/UglifyJS
	# Use a specific release because UglifyJS is used in building Duktape
	@rm -f v1.3.5.tar.gz
	$(WGET) https://github.com/mishoo/UglifyJS/archive/v1.3.5.tar.gz -O v1.3.5.tar.gz
	tar xfz v1.3.5.tar.gz
	mv UglifyJS-1.3.5 UglifyJS
	@rm -f v1.3.5.tar.gz

	# Don't use this because it's a moving critical dependency
	#$(GIT) clone --depth 1 https://github.com/mishoo/UglifyJS.git

UglifyJS2:
	# https://github.com/mishoo/UglifyJS2
	# Use a specific release because UglifyJS2 is used in building Duktape
	# (This is now a bit futile because UglifyJS2 requires an 'npm install',
	# the NodeJS dependencies need to be controlled for this to really work.)
	@rm -f v2.4.12.tar.gz
	$(WGET) https://github.com/mishoo/UglifyJS2/archive/v2.4.12.tar.gz -O v2.4.12.tar.gz
	tar xfz v2.4.12.tar.gz
	mv UglifyJS2-2.4.12 UglifyJS2
	@rm -f v2.4.12.tar.gz

	# Don't use this because it's a moving critical dependency
	#$(GIT) clone --depth 1 https://github.com/mishoo/UglifyJS2.git

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

xmldoctest: sax-js xmldoc duk
	@echo "### xmldoctest"
	@rm -f /tmp/duk-xmldoc-test*
	cat sax-js/lib/sax.js > /tmp/duk-xmldoc-test.js
	echo ";" >> /tmp/duk-xmldoc-test.js  # missing end semicolon causes automatic semicolon problem
	cat xmldoc/lib/xmldoc.js >> /tmp/duk-xmldoc-test.js
	echo ";" >> /tmp/duk-xmldoc-test.js  # missing end semicolon causes automatic semicolon problem
	cat tests/xmldoc/basic.js >> /tmp/duk-xmldoc-test.js
	./duk /tmp/duk-xmldoc-test.js

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

CCOPTS_AJDUK = -m32
#CCOPTS_AJDUK += '-fpack-struct=1'
CCOPTS_AJDUK += -Wno-unused-parameter -Wno-pedantic -Wno-sign-compare -Wno-missing-field-initializers -Wno-unused-result
CCOPTS_AJDUK += -UDUK_CMDLINE_FANCY -DDUK_CMDLINE_AJSHEAP -D_POSIX_C_SOURCE=200809L
CCOPTS_AJDUK += -DDUK_OPT_FORCE_ALIGN=4
CCOPTS_AJDUK += -DDUK_OPT_ASSERTIONS
CCOPTS_AJDUK += -DDUK_OPT_LIGHTFUNC_BUILTINS
CCOPTS_AJDUK += -DDUK_OPT_REFCOUNT16
CCOPTS_AJDUK += -DDUK_OPT_STRHASH16
CCOPTS_AJDUK += -DDUK_OPT_STRLEN16
CCOPTS_AJDUK += -DDUK_OPT_BUFLEN16
CCOPTS_AJDUK += -DDUK_OPT_OBJSIZES16
CCOPTS_AJDUK += -DDUK_OPT_STRTAB_CHAIN
CCOPTS_AJDUK += -DDUK_OPT_STRTAB_CHAIN_SIZE=128
CCOPTS_AJDUK += -DDUK_OPT_HEAPPTR16
CCOPTS_AJDUK += '-DDUK_OPT_HEAPPTR_ENC16(ud,p)=ajsheap_enc16((ud),(p))'
CCOPTS_AJDUK += '-DDUK_OPT_HEAPPTR_DEC16(ud,x)=ajsheap_dec16((ud),(x))'
CCOPTS_AJDUK += -DDUK_OPT_EXTERNAL_STRINGS
#CCOPTS_AJDUK += '-DDUK_OPT_EXTSTR_INTERN_CHECK(ud,ptr,len)=ajsheap_extstr_check_1((ptr),(len))'
#CCOPTS_AJDUK += '-DDUK_OPT_EXTSTR_FREE(ud,ptr)=ajsheap_extstr_free_1((ptr))'
CCOPTS_AJDUK += '-DDUK_OPT_EXTSTR_INTERN_CHECK(ud,ptr,len)=ajsheap_extstr_check_2((ptr),(len))'
CCOPTS_AJDUK += '-DDUK_OPT_EXTSTR_FREE(ud,ptr)=ajsheap_extstr_free_2((ptr))'
#CCOPTS_AJDUK += '-DDUK_OPT_EXTSTR_INTERN_CHECK(ud,ptr,len)=ajsheap_extstr_check_3((ptr),(len))'
#CCOPTS_AJDUK += '-DDUK_OPT_EXTSTR_FREE(ud,ptr)=ajsheap_extstr_free_3((ptr))'
CCOPTS_AJDUK += '-DDUK_OPT_EXEC_TIMEOUT_CHECK(udata)=ajsheap_exec_timeout_check(udata)'
CCOPTS_AJDUK += '-DDUK_OPT_DECLARE=extern uint8_t *ajsheap_ram; extern duk_uint16_t ajsheap_enc16(void *ud, void *p); extern void *ajsheap_dec16(void *ud, duk_uint16_t x); extern const void *ajsheap_extstr_check_1(const void *ptr, duk_size_t len); extern const void *ajsheap_extstr_check_2(const void *ptr, duk_size_t len); extern const void *ajsheap_extstr_check_3(const void *ptr, duk_size_t len); extern void ajsheap_extstr_free_1(const void *ptr); extern void ajsheap_extstr_free_2(const void *ptr); extern void ajsheap_extstr_free_3(const void *ptr); extern duk_bool_t ajsheap_exec_timeout_check(void *udata);'
#CCOPTS_AJDUK += -DDUK_OPT_DEBUG -DDUK_OPT_DPRINT
#CCOPTS_AJDUK += -DDUK_OPT_DEBUG -DDUK_OPT_DPRINT -DDUK_OPT_DDPRINT -DDUK_OPT_DDDPRINT

# Command line with Alljoyn.js pool allocator, for low memory testing.
# The pool sizes only make sense with -m32, so force that.  This forces
# us to use barebones cmdline too.
ajduk: alljoyn-js ajtcl dist
	$(CC) -o $@ \
		-Ialljoyn-js/src -Iajtcl/inc/ -Iajtcl/src/target/linux/ \
		$(CCOPTS_NONDEBUG) \
		$(CCOPTS_AJDUK) \
		$(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) \
		dist/examples/cmdline/duk_cmdline_ajduk.c \
		alljoyn-js/src/ajs_heap.c ajtcl/src/aj_debug.c ajtcl/src/target/linux/aj_target_util.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l ajduk
ajdukd: alljoyn-js ajtcl dist
	$(CC) -o $@ \
		-Ialljoyn-js/src -Iajtcl/inc/ -Iajtcl/src/target/linux/ \
		$(CCOPTS_DEBUG) \
		$(CCOPTS_AJDUK) \
		$(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) \
		dist/examples/cmdline/duk_cmdline_ajduk.c \
		alljoyn-js/src/ajs_heap.c ajtcl/src/aj_debug.c ajtcl/src/target/linux/aj_target_util.c \
		-lm -lpthread
	@echo "*** SUCCESS:"
	@ls -l ajdukd

.PHONY: gccpredefs
gccpredefs:
	gcc -dM -E - < /dev/null

.PHONY: clangpredefs
clangpredefs:
	clang -dM -E - < /dev/null

.PHONY:	runtestsdeps
runtestsdeps:	runtests/node_modules UglifyJS2

runtests/node_modules:
	echo "Installing required NodeJS modules for runtests"
	cd runtests; npm install

.PHONY:	doc
doc:	$(patsubst %.txt,%.html,$(wildcard doc/*.txt))

doc/%.html: doc/%.txt
	rst2html $< $@

cloc:	dist cloc-1.60.pl
	@echo "CLOC report on combined duktape.c source file"
	@perl cloc-1.60.pl --quiet dist/src/duktape.c

# Source distributable for end users
# XXX: want to run codepolicycheck when dist gets built, but don't want to depend on it.
# XXX: make prints a harmless warning related to the sub-make.
dist:
	@make codepolicycheck
	if [ -f compiler.jar ]; then sh util/make_dist.sh --minify closure --create-spdx; else sh util/make_dist.sh --minify none --create-spdx; fi

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

# Require closure compiler for official build
.PHONY: dist-src-official
dist-src-official:	compiler.jar dist-src

# ISO target is useful with some system emulators with no network access
.PHONY: dist-iso
dist-iso:	dist-src
	mkisofs -input-charset utf-8 -o duktape-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).iso duktape-$(DUK_VERSION_FORMATTED)-$(BUILD_DATETIME)-$(GIT_INFO).tar.gz

.PHONY: tidy-site
tidy-site:
	for i in website/*/*.html; do echo "*** Checking $$i"; tidy -q -e -xml $$i; done

# Duktape binary releases are in a separate repo
duktape-releases:
	$(GIT) clone https://github.com/svaarala/duktape-releases.git

# Website
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

ifeq ($(TRAVIS),1)
CODEPOLICYOPTS=--fail-on-errors
else
CODEPOLICYOPTS=
endif

.PHONY: codepolicycheck
codepolicycheck:
	python util/check_code_policy.py \
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
		src/*.c src/*.h src/*.h.in tests/api/*.c
	python util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-debug-log-calls \
		--check-carriage-returns \
		--check-fixme \
		--check-no-symbol-visibility \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		tests/ecmascript/*.js
	python util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--check-cpp-comment \
		--dump-vim-commands \
		examples/*/*.c examples/*/*.h
	# XXX: not yet FIXME pure
	python util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		config/config-options/*.yaml config/feature-options/*.yaml config/*.yaml
	python util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		debugger/*.yaml
	python util/check_code_policy.py \
		$(CODEPOLICYOPTS) \
		--check-carriage-returns \
		--check-fixme \
		--check-non-ascii \
		--check-trailing-whitespace \
		--check-mixed-indent \
		--check-nonleading-tab \
		--dump-vim-commands \
		website/api/*.yaml website/api/*.html

.PHONY: codepolicycheckvim
codepolicycheckvim:
	-python util/check_code_policy.py --dump-vim-commands src/*.c src/*.h src/*.h.in tests/api/*.c

.PHONY: big-git-files
big-git-files:
	util/find_big_git_files.sh

# Alignment check
.PHONY: checkalign
checkalign:
	@echo "checkalign for: `uname -a`"
	gcc -o /tmp/check_align -Wall -Wextra util/check_align.c
	@cp util/check_align.sh /tmp
	@cd /tmp; sh check_align.sh

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

#TIME=python util/time_multi.py --count 1 --sleep 0 --sleep-factor 0.8 --mode min # Take minimum time of N
#TIME=python util/time_multi.py --count 3 --sleep 0 --sleep-factor 0.8 --mode min # Take minimum time of N
TIME=python util/time_multi.py --count 5 --sleep 0 --sleep-factor 0.8 --mode min # Take minimum time of N

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
		printf ' duk.O2.130 %5s' "`$(TIME) ./duk.O2.130 $$i`"; \
		printf ' duk.O2.124 %5s' "`$(TIME) ./duk.O2.124 $$i`"; \
		printf ' duk.O2.113 %5s' "`$(TIME) ./duk.O2.113 $$i`"; \
		printf ' duk.O2.102 %5s' "`$(TIME) ./duk.O2.102 $$i`"; \
		printf ' |'; \
		printf ' mujs %5s' "`$(TIME) mujs $$i`"; \
		printf ' lua %5s' "`$(TIME) lua $${i%%.js}.lua`"; \
		printf ' python %5s' "`$(TIME) python $${i%%.js}.py`"; \
		printf ' perl %5s' "`$(TIME) perl $${i%%.js}.pl`"; \
		printf ' ruby %5s' "`$(TIME) ruby $${i%%.js}.rb`"; \
		printf ' |'; \
		printf ' rhino %5s' "`$(TIME) rhino $$i`"; \
		printf ' node %5s' "`$(TIME) node $$i`"; \
		printf ' luajit %5s' "`$(TIME) luajit $${i%%.js}.lua`"; \
		printf '\n'; \
	done
perftestduk: duk duk.O2
	for i in tests/perf/*.js; do \
		printf '%-36s:' "`basename $$i`"; \
		printf ' duk.Os %5s' "`$(TIME) ./duk $$i`"; \
		printf ' duk.O2 %5s' "`$(TIME) ./duk.O2 $$i`"; \
		printf ' |'; \
		printf ' duk.O2.130 %5s' "`$(TIME) ./duk.O2.130 $$i`"; \
		printf ' duk.O2.124 %5s' "`$(TIME) ./duk.O2.124 $$i`"; \
		printf ' duk.O2.113 %5s' "`$(TIME) ./duk.O2.113 $$i`"; \
		printf ' duk.O2.102 %5s' "`$(TIME) ./duk.O2.102 $$i`"; \
		printf '\n'; \
	done
perftestduk3: duk.O2
	for i in tests/perf/*.js; do \
		printf '%-36s:' "`basename $$i`"; \
		printf ' duk.O2'; \
		printf ' %5s' "`$(TIME) ./duk.O2 $$i`"; \
		printf ' %5s' "`$(TIME) ./duk.O2 $$i`"; \
		printf ' %5s' "`$(TIME) ./duk.O2 $$i`"; \
		printf '\n'; \
	done
