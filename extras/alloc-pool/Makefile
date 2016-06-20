# For manual testing; say 'make' in extras/alloc-pool and run ./test.

DEFS='-DDUK_OPT_DECLARE=extern void my_fatal(const char *msg);'
DEFS+='-DDUK_OPT_FATAL_HANDLER(udata,msg)=my_fatal((msg))'
#DEFS+='-DDUK_ALLOC_POOL_DEBUG'

.PHONY: test
test:
	gcc -std=c99 -Wall -Wextra -m32 -Os -otest \
		-I../../src ../../src/duktape.c \
		$(DEFS) \
		duk_alloc_pool.c test.c \
		-lm
	./test 'print("foo", "bar", 1, 2, 3)'
	./test 'alert("foo", "bar", 1, 2, 3)'

.PHONY: ptrcomptest
ptrcomptest:
	tar -x -v -z -f ../../config/genconfig_metadata.tar.gz examples/low_memory.yaml
	python ../../config/genconfig.py \
		--metadata ../../config/genconfig_metadata.tar.gz \
		--output ./duk_config.h \
		--option-file examples/low_memory.yaml \
		--option-file ptrcomp.yaml \
		--fixup-file ptrcomp_fixup.h \
		duk-config-header
	cp ../../src/duktape.c .
	cp ../../src/duktape.h .
	gcc -std=c99 -Wall -Wextra -m32 -Os -optrcomptest \
		-I. ./duktape.c \
		$(DEFS) \
		duk_alloc_pool.c test.c \
		-lm
	./ptrcomptest 'print("foo", "bar", 1, 2, 3)'
	./ptrcomptest 'alert("foo", "bar", 1, 2, 3)'
