#!/bin/sh
#
#  Manual script to get index page footprint/RAM figures.
#

set -e
#set -x

ARCHOPT=-m32
#ARCHOPT="-mthumb -march=armv7-a"

echo ""
echo "***"
echo "*** default"
echo "***"
echo ""

rm -rf /tmp/duk-index-tmp
rm -f /tmp/hello
python2 tools/configure.py \
	--source-directory src-input \
	--output-directory /tmp/duk-index-tmp \
	--config-metadata config
gcc -o/tmp/hello $ARCHOPT -std=c99 -Wall -Os \
	-fomit-frame-pointer -flto \
	-fno-asynchronous-unwind-tables \
	-ffunction-sections -Wl,--gc-sections \
	-fno-stack-protector \
	-I/tmp/duk-index-tmp \
	/tmp/duk-index-tmp/duktape.c examples/hello/hello.c -lm
size /tmp/hello
valgrind --tool=massif --massif-out-file=/tmp/duk-massif.out /tmp/hello
ms_print /tmp/duk-massif.out | head -20

echo ""
echo "***"
echo "*** lowmem"
echo "***"
echo ""

rm -rf /tmp/duk-index-tmp
rm -f /tmp/hello
python2 tools/configure.py \
	--source-directory src-input \
	--output-directory /tmp/duk-index-tmp \
	--config-metadata config \
	--option-file config/examples/low_memory.yaml
gcc -o/tmp/hello $ARCHOPT -std=c99 -Wall -Os \
	-fomit-frame-pointer -flto \
	-fno-asynchronous-unwind-tables \
	-ffunction-sections -Wl,--gc-sections \
	-fno-stack-protector \
	-I/tmp/duk-index-tmp \
	/tmp/duk-index-tmp/duktape.c examples/hello/hello.c -lm
size /tmp/hello
valgrind --tool=massif --massif-out-file=/tmp/duk-massif.out /tmp/hello
ms_print /tmp/duk-massif.out | head -20

echo ""
echo "***"
echo "*** full lowmem"
echo "***"
echo ""

cat >/tmp/duk-lowmem.yaml <<EOF
DUK_USE_ROM_OBJECTS: true
DUK_USE_ROM_STRINGS: true
DUK_USE_ROM_GLOBAL_INHERIT: true

DUK_USE_REFCOUNT16: true
DUK_USE_REFCOUNT32: false
DUK_USE_STRHASH16: true
DUK_USE_STRLEN16: true
DUK_USE_BUFLEN16: true
DUK_USE_OBJSIZES16: true
DUK_USE_HSTRING_CLEN: false
DUK_USE_HSTRING_LAZY_CLEN: true  # must be lazy when clen field dropped
DUK_USE_HOBJECT_HASH_PART: false

DUK_USE_HEAPPTR16: true
DUK_USE_HEAPPTR_ENC16:
  verbatim: "#define DUK_USE_HEAPPTR_ENC16(ud,p) duk_alloc_pool_enc16((p))"
DUK_USE_HEAPPTR_DEC16:
  verbatim: "#define DUK_USE_HEAPPTR_DEC16(ud,x) duk_alloc_pool_dec16((x))"

DUK_USE_STRTAB_PTRCOMP: true
EOF

rm -rf /tmp/duk-index-tmp
rm -f /tmp/hello
python2 tools/configure.py \
	--rom-support --rom-auto-lightfunc \
	--source-directory src-input \
	--output-directory /tmp/duk-index-tmp \
	--config-metadata config \
	--option-file config/examples/low_memory.yaml \
	--option-file /tmp/duk-lowmem.yaml \
	--fixup-line '#include "duk_alloc_pool.h"'
gcc -o/tmp/hello $ARCHOPT -std=c99 -Wall -Os \
	-fomit-frame-pointer -flto \
	-fno-asynchronous-unwind-tables \
	-ffunction-sections -Wl,--gc-sections \
	-fno-stack-protector \
	-I/tmp/duk-index-tmp -Iextras/alloc-pool \
	-DDUK_ALLOC_POOL_TRACK_HIGHWATER -DDUK_ALLOC_POOL_TRACK_WASTE \
	/tmp/duk-index-tmp/duktape.c examples/hello/hello_ptrcomp.c extras/alloc-pool/duk_alloc_pool.c -lm
size /tmp/hello
valgrind --tool=massif --massif-out-file=/tmp/duk-massif.out /tmp/hello
ms_print /tmp/duk-massif.out | head -20
