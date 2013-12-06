#
#  Makefile for the Duktape development
#
#  Duktape command line tools are built by first creating a source
#  dist directory, and then using the sources from the dist directory
#  for compilation.  This is as close as possible to sources used
#  by a developer, at the risk of polluting the dist directory
#  (accidentally; we try to avoid that of course).
#
#  When creating actual distributables, always clean first.
#

# FIXME: stripping and size reporting

VERSION=0.8.0

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
	$(DISTSRCSEP)/duk_error_fatal.c \
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
	$(DISTSRCSEP)/duk_api.c \
	$(DISTSRCSEP)/duk_lexer.c \
	$(DISTSRCSEP)/duk_js_call.c \
	$(DISTSRCSEP)/duk_js_executor.c \
	$(DISTSRCSEP)/duk_js_compiler.c \
	$(DISTSRCSEP)/duk_regexp_compiler.c \
	$(DISTSRCSEP)/duk_regexp_executor.c \
	$(DISTSRCSEP)/duk_builtin_duk.c \
	$(DISTSRCSEP)/duk_builtin_thread.c \
	$(DISTSRCSEP)/duk_builtin_thrower.c \
	$(DISTSRCSEP)/duk_builtin_array.c \
	$(DISTSRCSEP)/duk_builtin_boolean.c \
	$(DISTSRCSEP)/duk_builtin_date.c \
	$(DISTSRCSEP)/duk_builtin_error.c \
	$(DISTSRCSEP)/duk_builtin_function.c \
	$(DISTSRCSEP)/duk_builtin_global.c \
	$(DISTSRCSEP)/duk_builtin_json.c \
	$(DISTSRCSEP)/duk_builtin_math.c \
	$(DISTSRCSEP)/duk_builtin_number.c \
	$(DISTSRCSEP)/duk_builtin_object.c \
	$(DISTSRCSEP)/duk_builtin_regexp.c \
	$(DISTSRCSEP)/duk_builtin_string.c \
	$(DISTSRCSEP)/duk_builtin_buffer.c \
	$(DISTSRCSEP)/duk_builtin_pointer.c

# Use combined sources for testing etc.
DUKTAPE_SOURCES = $(DUKTAPE_SOURCES_COMBINED)

# Duktape command line tool - example of a main() program, used
# for unit testing
DUKTAPE_CMDLINE_SOURCES = \
	$(DISTCMD)/duk_cmdline.c

DUK_SHARED_LIBS_NONDEBUG = \
	libduktape100.so.1.0.0 libduktape101.so.1.0.0 \
	libduktape200.so.1.0.0 libduktape201.so.1.0.0 \
	libduktape300.so.1.0.0 libduktape301.so.1.0.0 \
	libduktape400.so.1.0.0 libduktape401.so.1.0.0 \
	libduktape500.so.1.0.0 libduktape501.so.1.0.0

DUK_SHARED_LIBS_DEBUG = \
	libduktape100d.so.1.0.0 libduktape101d.so.1.0.0 \
	libduktape200d.so.1.0.0 libduktape201d.so.1.0.0 \
	libduktape300d.so.1.0.0 libduktape301d.so.1.0.0 \
	libduktape400d.so.1.0.0 libduktape401d.so.1.0.0 \
	libduktape500d.so.1.0.0 libduktape501d.so.1.0.0

DUK_CMDLINE_TOOLS_NONDEBUG = \
	duk.100 duk.101 \
	duk.200 duk.201 \
	duk.300 duk.301 \
	duk.400 duk.401 \
	duk.500 duk.501

DUK_CMDLINE_TOOLS_DEBUG = \
	duk.100d duk.101d \
	duk.200d duk.201d \
	duk.300d duk.301d \
	duk.400d duk.401d \
	duk.500d duk.501d

# Compiler setup for Linux
CC	= gcc
CCOPTS_SHARED = -pedantic -ansi -std=c99 -Wall -fstrict-aliasing
CCOPTS_SHARED += -I./dist/src
#CCOPTS_SHARED += -I./dist/src-separate
#CCOPTS_SHARED += -m32                             # force 32-bit compilation on a 64-bit host
CCOPTS_SHARED += -DDUK_OPT_SEGFAULT_ON_PANIC       # segfault on panic allows valgrind to show stack trace on panic
CCOPTS_NONDEBUG = $(CCOPTS_SHARED) -Os -fomit-frame-pointer
CCOPTS_DEBUG = $(CCOPTS_SHARED) -O0 -g -ggdb
CCLIBS	= -lm
CCLIBS += -lreadline
CCLIBS += -lncurses  # on some systems -lreadline also requires -lncurses (e.g. RHEL)
.PHONY: default all clean test install

default:	all64

all32:	$(DUK_CMDLINE_TOOLS_NONDEBUG) \
	$(DUK_CMDLINE_TOOLS_DEBUG) \
	$(DUK_SHARED_LIBS_NONDEBUG) \
	$(DUK_SHARED_LIBS_DEBUG)

all64:	duk.400 duk.401 \
	duk.400d duk.401d \
	duk.500 duk.501 \
	duk.500d duk.501d \
	libduktape400.so.1.0.0 libduktape401.so.1.0.0 \
	libduktape400d.so.1.0.0 libduktape401d.so.1.0.0 \
	libduktape500.so.1.0.0 libduktape501.so.1.0.0 \
	libduktape500d.so.1.0.0 libduktape501d.so.1.0.0

clean:
	-@rm -rf dist/
	-@rm -rf full/
	-@rm -rf site/
	-@rm -f $(DUK_CMDLINE_TOOLS_NONDEBUG)
	-@rm -f $(DUK_CMDLINE_TOOLS_DEBUG)
	-@rm -f $(DUK_SHARED_LIBS_NONDEBUG)
	-@rm -f $(DUK_SHARED_LIBS_DEBUG)
	-@rm -f libduktape*.so*
	-@rm -f doc/*.html
	-@rm -f src/*.pyc

$(DUK_SHARED_LIBS_NONDEBUG): dist
	-rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC -DDUK_PROFILE=$(subst d,,$(subst .so.1.0.0,,$(subst libduktape,,$@))) $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

$(DUK_SHARED_LIBS_DEBUG): dist
	-rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC -DDUK_PROFILE=$(subst d,,$(subst .so.1.0.0,,$(subst libduktape,,$@))) $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

$(DUK_CMDLINE_TOOLS_NONDEBUG): dist
	$(CC) -o $@ -DDUK_PROFILE=$(subst d,,$(subst duk.,,$@)) $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

$(DUK_CMDLINE_TOOLS_DEBUG): dist
	$(CC) -o $@ -DDUK_PROFILE=$(subst d,,$(subst duk.,,$@)) $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

test:	npminst duk.400
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk.400 --run-nodejs --run-rhino --num-threads 8 --log-file=/tmp/duk-test.log ecmascript-testcases/

qtest:	npminst duk.400
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk.400 --num-threads 16 --log-file=/tmp/duk-test.log ecmascript-testcases/

vgtest:	npminst duk.400
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk.400 --num-threads 1 --test-sleep 30  --log-file=/tmp/duk-vgtest.log --valgrind --verbose ecmascript-testcases/

apitest:	npminst libduktape400.so.1.0.0
	node runtests/runtests.js --num-threads 1 --log-file=/tmp/duk-api-test.log api-testcases/

# FIXME: torturetest; torture + valgrind

.PHONY:	npminst
npminst:	runtests/node_modules

runtests/node_modules:
	echo "Installing required NodeJS modules for runtests"
	cd runtests; npm install

.PHONY:	doc
doc:	$(patsubst %.txt,%.html,$(wildcard doc/*.txt))

doc/%.html: doc/%.txt
	rst2html $< $@

# Simulate end user distribution, creates dist/ directory
dist:
	sh make_dist.sh

