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
	$(DISTSRCSEP)/duk_builtin_pointer.c \
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
CCOPTS_SHARED = -pedantic -ansi -std=c99 -Wall -fstrict-aliasing
CCOPTS_SHARED += -I./dist/src
#CCOPTS_SHARED += -I./dist/src-separate
#CCOPTS_SHARED += -m32                             # force 32-bit compilation on a 64-bit host
CCOPTS_SHARED += -DDUK_OPT_SEGFAULT_ON_PANIC       # segfault on panic allows valgrind to show stack trace on panic
CCOPTS_SHARED += -DDUK_OPT_DPRINT_COLORS
#CCOPTS_SHARED += -DDUK_OPT_NO_FILE_IO
#CCOPTS_SHARED += '-DDUK_PANIC_HANDLER(code,msg)={printf("*** %d:%s\n",(code),(msg));abort();}'
CCOPTS_SHARED += -DDUK_OPT_SELF_TESTS
#CCOPTS_SHARED += -DDUK_OPT_NO_TRACEBACKS
#CCOPTS_SHARED += -DDUK_OPT_NO_VERBOSE_ERRORS
CCOPTS_SHARED += -DDUK_OPT_NO_MS_RESIZE_STRINGTABLE
CCOPTS_SHARED += -DDUK_OPT_DEBUG_BUFSIZE=80
CCOPTS_NONDEBUG = $(CCOPTS_SHARED) -Os -fomit-frame-pointer
CCOPTS_NONDEBUG += -g -ggdb
CCOPTS_DEBUG = $(CCOPTS_SHARED) -O0 -g -ggdb
CCOPTS_DEBUG += -DDUK_OPT_DEBUG
CCLIBS	= -lm
CCLIBS += -lreadline
CCLIBS += -lncurses  # on some systems -lreadline also requires -lncurses (e.g. RHEL)
.PHONY: default all clean test install

all:	duk \
	dukd \
	libduktape.so.1.0.0 \
	libduktaped.so.1.0.0

clean:
	-@rm -rf dist/
	-@rm -rf full/
	-@rm -rf site/
	-@rm -f duk dukd
	-@rm -f libduktape*.so*
	-@rm -f doc/*.html
	-@rm -f src/*.pyc

libduktape.so.1.0.0:	dist
	-rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

libduktaped.so.1.0.0:	dist
	-rm -f $(subst .so.1.0.0,.so.1,$@) $(subst .so.1.0.0,.so.1.0.0,$@) $(subst .so.1.0.0,.so,$@)
	$(CC) -o $@ -shared -Wl,-soname,$(subst .so.1.0.0,.so.1,$@) -fPIC $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(CCLIBS)
	ln -s $@ $(subst .so.1.0.0,.so.1,$@)
	ln -s $@ $(subst .so.1.0.0,.so,$@)

duk:	dist
	$(CC) -o $@ $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

dukd:	dist
	$(CC) -o $@ $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

test:	npminst duk
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk --run-nodejs --run-rhino --num-threads 8 --log-file=/tmp/duk-test.log ecmascript-testcases/

testd:	npminst dukd
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/dukd --run-nodejs --run-rhino --num-threads 8 --log-file=/tmp/duk-test.log ecmascript-testcases/

qtest:	npminst duk
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk --num-threads 16 --log-file=/tmp/duk-test.log ecmascript-testcases/

qtestd:	npminst dukd
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/dukd --num-threads 16 --log-file=/tmp/duk-test.log ecmascript-testcases/

vgtest:	npminst duk
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk --num-threads 1 --test-sleep 30  --log-file=/tmp/duk-vgtest.log --valgrind --verbose ecmascript-testcases/

apitest:	npminst libduktape.so.1.0.0
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

