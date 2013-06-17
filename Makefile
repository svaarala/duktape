#
#  Makefile for the Duktape repository (Duktape development)
#
#  This is an internal Makefile usd to create distributables like a
#  source package, documentation, or Duktape website pages.  It also
#  supports Duktape development, e.g. convenient test case runs.
#
#  Duktape command line tools are built by first creating a source
#  dist directory, and then using the sources from the dist directory
#  for compilation.  This is as close as possible to sources used
#  by a developer, at the risk of polluting the dist directory
#  (accidentally, we try to avoid that of course).
#
#  When creating an actual distributables, always clean first.
#

# FIXME: stripping and size reporting

DUKTAPE_SOURCES =	\
	dist/src/duk_util_hashbytes.c \
	dist/src/duk_util_hashprime.c \
	dist/src/duk_util_bitdecoder.c \
	dist/src/duk_util_bitencoder.c \
	dist/src/duk_util_tinyrandom.c \
	dist/src/duk_util_misc.c \
	dist/src/duk_alloc_default.c \
	dist/src/duk_debug_macros.c \
	dist/src/duk_debug_vsnprintf.c \
	dist/src/duk_debug_heap.c \
	dist/src/duk_debug_hobject.c \
	dist/src/duk_debug_fixedbuffer.c \
	dist/src/duk_error_macros.c \
	dist/src/duk_error_longjmp.c \
	dist/src/duk_error_throw.c \
	dist/src/duk_error_fatal.c \
	dist/src/duk_error_augment.c \
	dist/src/duk_error_misc.c \
	dist/src/duk_heap_misc.c \
	dist/src/duk_heap_memory.c \
	dist/src/duk_heap_alloc.c \
	dist/src/duk_heap_refcount.c \
	dist/src/duk_heap_markandsweep.c \
	dist/src/duk_heap_hashstring.c \
	dist/src/duk_heap_stringtable.c \
	dist/src/duk_heap_stringcache.c \
	dist/src/duk_hthread_misc.c \
	dist/src/duk_hthread_alloc.c \
	dist/src/duk_hthread_builtins.c \
	dist/src/duk_hthread_stacks.c \
	dist/src/duk_hobject_alloc.c \
	dist/src/duk_hobject_class.c \
	dist/src/duk_hobject_enum.c \
	dist/src/duk_hobject_props.c \
	dist/src/duk_hobject_finalizer.c \
	dist/src/duk_hobject_pc2line.c \
	dist/src/duk_hobject_misc.c \
	dist/src/duk_hbuffer_alloc.c \
	dist/src/duk_hbuffer_ops.c \
	dist/src/duk_unicode_tables.c \
	dist/src/duk_unicode_support.c \
	dist/src/duk_strings.c \
	dist/src/duk_builtins.c \
	dist/src/duk_js_ops.c \
	dist/src/duk_js_var.c \
	dist/src/duk_numconv.c \
	dist/src/duk_api_call.c \
	dist/src/duk_api_conv.c \
	dist/src/duk_api_codec.c \
	dist/src/duk_api_memory.c \
	dist/src/duk_api_string.c \
	dist/src/duk_api_object.c \
	dist/src/duk_api_thread.c \
	dist/src/duk_api_buffer.c \
	dist/src/duk_api_var.c \
	dist/src/duk_api.c \
	dist/src/duk_lexer.c \
	dist/src/duk_js_call.c \
	dist/src/duk_js_executor.c \
	dist/src/duk_js_compiler.c \
	dist/src/duk_regexp_compiler.c \
	dist/src/duk_regexp_executor.c \
	dist/src/duk_builtin_duk.c \
	dist/src/duk_builtin_thread.c \
	dist/src/duk_builtin_thrower.c \
	dist/src/duk_builtin_array.c \
	dist/src/duk_builtin_boolean.c \
	dist/src/duk_builtin_date.c \
	dist/src/duk_builtin_error.c \
	dist/src/duk_builtin_function.c \
	dist/src/duk_builtin_global.c \
	dist/src/duk_builtin_json.c \
	dist/src/duk_builtin_math.c \
	dist/src/duk_builtin_number.c \
	dist/src/duk_builtin_object.c \
	dist/src/duk_builtin_regexp.c \
	dist/src/duk_builtin_string.c

# Duktape command line tool - example of a main() program
DUKTAPE_CMDLINE_SOURCES = \
	dist/examples/cmdline/duk_cmdline.c \
	dist/examples/cmdline/duk_ncurses.c \
	dist/examples/cmdline/duk_socket.c \
	dist/examples/cmdline/duk_fileio.c

VERSION=0.6.0

CC	= gcc
CCOPTS_SHARED = -pedantic -ansi -std=c99 -Wall -fstrict-aliasing
CCOPTS_SHARED += -I./dist/src
CCOPTS_SHARED += -D_POSIX_C_SOURCE=200809L
CCOPTS_SHARED += -D_GNU_SOURCE                     # e.g. getdate_r
CCOPTS_SHARED += -D_XOPEN_SOURCE                   # e.g. strptime
CCOPTS_SHARED += -m32                              # force 32-bit compilation on a 64-bit host
CCOPTS_SHARED += -I/usr/include/ncursesw           # for cmdline tool
CCOPTS_NONDEBUG = $(CCOPTS_SHARED) -Os -fomit-frame-pointer
CCOPTS_DEBUG = $(CCOPTS_SHARED) -O0 -g -ggdb
CCLIBS	= -lm
CCLIBS += -lreadline -lncursesw                    # for cmdline tool
.PHONY: default all clean test install

default:	all

all:	duk.100 duk.101 duk.200 duk.201 duk.300 duk.301 duk.400 duk.401 duk.500 duk.501

clean:
	-@rm -rf dist/
	-@rm -f duk.100 duk.101 duk.200 duk.201 duk.300 duk.301 duk.400 duk.401 duk.500 duk.501
	-@rm -rf duktape-$(VERSION)
	-@rm -rf duktape-$(VERSION).tar*

duk.100:	dist
	$(CC) -o $@ -DDUK_PROFILE=100 $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.101:	dist
	$(CC) -o $@ -DDUK_PROFILE=101 $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.200:	dist
	$(CC) -o $@ -DDUK_PROFILE=200 $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.201:	dist
	$(CC) -o $@ -DDUK_PROFILE=201 $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.300:	dist
	$(CC) -o $@ -DDUK_PROFILE=300 $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.301:	dist
	$(CC) -o $@ -DDUK_PROFILE=301 $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.400:	dist
	$(CC) -o $@ -DDUK_PROFILE=400 $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.401:	dist
	$(CC) -o $@ -DDUK_PROFILE=401 $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.500:	dist
	$(CC) -o $@ -DDUK_PROFILE=500 $(CCOPTS_NONDEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)
duk.501:	dist
	$(CC) -o $@ -DDUK_PROFILE=501 $(CCOPTS_DEBUG) $(DUKTAPE_SOURCES) $(DUKTAPE_CMDLINE_SOURCES) $(CCLIBS)

test:	duk.400
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk.400 --run-nodejs --run-rhino --num-threads 8 --log-file=/tmp/duk-test.log testcases/

qtest:	duk.400
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk.400 --num-threads 16 --log-file=/tmp/duk-test.log testcases/

vgtest:		duk.400
	node runtests/runtests.js --run-duk --cmd-duk=$(shell pwd)/duk.400 --num-threads 1 --log-file=/tmp/duk-vgtest.log --valgrind --verbose testcases/

# FIXME: torturetest; torture + valgrind

doc/%.html: doc/%.txt
	rst2html $< $@

dist:
	sh make_dist.sh

dist-src:	dist
	rm -rf duktape-$(VERSION)
	rm -rf duktape-$(VERSION).tar*
	mkdir duktape-$(VERSION)
	cp -r dist/* duktape-$(VERSION)/
	tar cvfj duktape-$(VERSION).tar.bz2 duktape-$(VERSION)/
	tar cvf duktape-$(VERSION).tar duktape-$(VERSION)/
	xz -z -e -9 duktape-$(VERSION).tar
	mkisofs -o duktape-$(VERSION).iso duktape-$(VERSION).tar.bz2

dist-website:
	echo "FIXME"
	cp website/*.css /tmp
	cd website; python buildsite.py api/ /tmp/foo.html
