#!/bin/sh
#
#  Create a distributable Duktape package into 'dist' directory.  The contents
#  of this directory can then be packaged into a source distributable.
#
#  The distributed source files contain all profiles and variants in one.
#  A developer should be able to use the distributes source as follows:
#
#    1. Add the Duktape source files to their project, whichever build
#       tool they use (make, scons, etc)
#
#    2. Add the Duktape header files to their include path.
#
#    3. Optionally define some DUK_OPT_xxx feature options.
#
#    4. Compile their program (which uses Duktape API).
#
#  In addition to sources, documentation, example programs, and some
#  example Makefiles are packaged into the dist package.
#

DIST=`pwd`/dist
DISTSRCSEP=$DIST/src-separate
DISTSRCCOM=$DIST/src

# DUK_VERSION is grepped from duk_api_public.h.in: it is needed for the
# public API and we want to avoid defining it in two places.
DUK_VERSION=`cat src/duk_api_public.h.in | grep define | grep DUK_VERSION | tr -s ' ' ' ' | cut -d ' ' -f 3 | tr -d 'L'`
DUK_MAJOR=`echo "$DUK_VERSION / 10000" | bc`
DUK_MINOR=`echo "$DUK_VERSION % 10000 / 100" | bc`
DUK_PATCH=`echo "$DUK_VERSION % 100" | bc`
DUK_VERSION_FORMATTED=$DUK_MAJOR.$DUK_MINOR.$DUK_PATCH
GIT_COMMIT=`git rev-parse HEAD`
GIT_DESCRIBE=`git describe`

echo "DUK_VERSION: $DUK_VERSION"
echo "GIT_COMMIT: $GIT_COMMIT"
echo "GIT_DESCRIBE: $GIT_DESCRIBE"
echo "Creating distributable sources to: $DIST"

# Create dist directory structure

rm -rf $DIST
mkdir $DIST
mkdir $DIST/src-separate
mkdir $DIST/src
mkdir $DIST/polyfills
#mkdir $DIST/doc
mkdir $DIST/licenses
mkdir $DIST/examples
mkdir $DIST/examples/hello
mkdir $DIST/examples/eval
mkdir $DIST/examples/cmdline
mkdir $DIST/examples/eventloop
mkdir $DIST/examples/guide
mkdir $DIST/examples/coffee

# Copy most files directly

for i in \
	duk_alloc_default.c	\
	duk_alloc_torture.c	\
	duk_api_internal.h	\
	duk_api_buffer.c	\
	duk_api.c		\
	duk_api_call.c		\
	duk_api_codec.c		\
	duk_api_compile.c	\
	duk_api_memory.c	\
	duk_api_object.c	\
	duk_api_string.c	\
	duk_api_var.c		\
	duk_api_logging.c	\
	duk_api_debug.c		\
	duk_bi_array.c		\
	duk_bi_boolean.c	\
	duk_bi_buffer.c		\
	duk_bi_date.c		\
	duk_bi_duktape.c	\
	duk_bi_error.c		\
	duk_bi_function.c	\
	duk_bi_global.c		\
	duk_bi_json.c		\
	duk_bi_math.c		\
	duk_bi_number.c		\
	duk_bi_object.c		\
	duk_bi_pointer.c	\
	duk_bi_logger.c		\
	duk_bi_protos.h		\
	duk_bi_regexp.c		\
	duk_bi_string.c		\
	duk_bi_proxy.c		\
	duk_bi_thread.c		\
	duk_bi_thrower.c	\
	duk_debug_fixedbuffer.c	\
	duk_debug.h		\
	duk_debug_heap.c	\
	duk_debug_hobject.c	\
	duk_debug_macros.c	\
	duk_debug_vsnprintf.c	\
	duk_error_augment.c	\
	duk_error.h		\
	duk_error_longjmp.c	\
	duk_error_macros.c	\
	duk_error_misc.c	\
	duk_error_throw.c	\
	duk_forwdecl.h		\
	duk_hbuffer_alloc.c	\
	duk_hbuffer.h		\
	duk_hbuffer_ops.c	\
	duk_hcompiledfunction.h	\
	duk_heap_alloc.c	\
	duk_heap.h		\
	duk_heap_hashstring.c	\
	duk_heaphdr.h		\
	duk_heap_markandsweep.c	\
	duk_heap_memory.c	\
	duk_heap_misc.c		\
	duk_heap_refcount.c	\
	duk_heap_stringcache.c	\
	duk_heap_stringtable.c	\
	duk_hnativefunction.h	\
	duk_hobject_alloc.c	\
	duk_hobject_class.c	\
	duk_hobject_enum.c	\
	duk_hobject_finalizer.c	\
	duk_hobject.h		\
	duk_hobject_misc.c	\
	duk_hobject_pc2line.c	\
	duk_hobject_props.c	\
	duk_hstring.h		\
	duk_hstring_misc.c	\
	duk_hthread_alloc.c	\
	duk_hthread_builtins.c	\
	duk_hthread.h		\
	duk_hthread_misc.c	\
	duk_hthread_stacks.c	\
	duk_internal.h		\
	duk_jmpbuf.h		\
	duk_js_bytecode.h	\
	duk_js_call.c		\
	duk_js_compiler.c	\
	duk_js_compiler.h	\
	duk_js_executor.c	\
	duk_js.h		\
	duk_json.h		\
	duk_js_ops.c		\
	duk_js_var.c		\
	duk_lexer.c		\
	duk_lexer.h		\
	duk_numconv.c		\
	duk_numconv.h		\
	duk_regexp_compiler.c	\
	duk_regexp_executor.c	\
	duk_regexp.h		\
	duk_tval.h		\
	duk_unicode.h		\
	duk_unicode_support.c	\
	duk_unicode_tables.c	\
	duk_util_bitdecoder.c	\
	duk_util_bitencoder.c	\
	duk_util.h		\
	duk_util_hashbytes.c	\
	duk_util_hashprime.c	\
	duk_util_misc.c		\
	duk_util_tinyrandom.c	\
	duk_selftest.c		\
	duk_selftest.h		\
	duk_strings.c		\
	duk_strings.h		\
	duk_replacements.c      \
	duk_replacements.h      \
	; do
	cp src/$i $DISTSRCSEP/
done

for i in \
	console-minimal.js \
	object-prototype-definegetter.js \
	object-prototype-definesetter.js \
	; do
	cp polyfills/$i $DIST/polyfills/
done


for i in \
	README.txt \
	Makefile \
	duk_cmdline.c \
	; do
	cp examples/cmdline/$i $DIST/examples/cmdline/
done

for i in \
	README.txt \
	Makefile \
	c_eventloop.c \
	c_eventloop.js \
	ecma_eventloop.js \
	main.c \
	poll.c \
	ncurses.c \
	socket.c \
	fileio.c \
	curses-timers.js \
	basic-test.js \
	server-socket-test.js \
	client-socket-test.js \
	; do
	cp examples/eventloop/$i $DIST/examples/eventloop/
done

for i in \
	README.txt \
	Makefile \
	hello.c \
	; do
	cp examples/hello/$i $DIST/examples/hello/
done

for i in \
	README.txt \
	Makefile \
	eval.c \
	; do
	cp examples/eval/$i $DIST/examples/eval/
done

for i in \
	README.txt \
	fib.js \
	process.js \
	processlines.c \
	prime.js \
	primecheck.c \
	uppercase.c \
	; do
	cp examples/guide/$i $DIST/examples/guide/
done

for i in \
	README.txt \
	Makefile \
	globals.coffee \
	hello.coffee \
	mandel.coffee \
	; do
	cp examples/coffee/$i $DIST/examples/coffee/
done

for i in \
	Makefile.example \
	Makefile.cmdline \
	; do
	cp examples/$i $DIST/
done

cat README.txt.dist | sed \
	-e "s/@DUK_VERSION_FORMATTED@/$DUK_VERSION_FORMATTED/" \
	-e "s/@GIT_COMMIT@/$GIT_COMMIT/" \
	-e "s/@GIT_DESCRIBE@/$GIT_DESCRIBE/" \
	> $DIST/README.txt
cp LICENSE.txt $DIST/LICENSE.txt
cp AUTHORS.txt $DIST/AUTHORS.txt
cp RELEASES.txt $DIST/RELEASES.txt

for i in \
	murmurhash2.txt \
	; do
	cp licenses/$i $DIST/licenses/
done

# Build temp versions of LICENSE.txt and AUTHORS.txt for embedding into
# autogenerated C/H files.

echo '/*' > $DIST/LICENSE.txt.tmp
cat LICENSE.txt | python util/make_ascii.py | sed -e 's/^/ \*  /' >> $DIST/LICENSE.txt.tmp
echo ' */' >> $DIST/LICENSE.txt.tmp

echo '/*' > $DIST/AUTHORS.txt.tmp
cat AUTHORS.txt | python util/make_ascii.py | sed -e 's/^/ \*  /' >> $DIST/AUTHORS.txt.tmp
echo ' */' >> $DIST/AUTHORS.txt.tmp

# Build duktape.h from parts, with some git-related replacements.

cat src/duktape.h.in | sed -e '
/^@LICENSE_TXT@$/ {
    r dist/LICENSE.txt.tmp
    d
}
/^@AUTHORS_TXT@$/ {
    r dist/AUTHORS.txt.tmp
    d
}
/^@DUK_FEATURES_H@$/ {
    r src/duk_features.h.in
    d
}
/^@DUK_API_PUBLIC_H@$/ {
    r src/duk_api_public.h.in
    d
}
/^@DUK_FEATURES_SANITY_H@$/ {
    r src/duk_features_sanity.h.in
    d
}
/^@DUK_DBLUNION_H@$/ {
    r src/duk_dblunion.h.in
    d
}' | sed \
	-e "s/@DUK_VERSION_FORMATTED@/$DUK_VERSION_FORMATTED/" \
	-e "s/@GIT_COMMIT@/$GIT_COMMIT/" \
	-e "s/@GIT_DESCRIBE@/$GIT_DESCRIBE/" \
	> $DISTSRCSEP/duktape.h

# Initjs code: built-in Ecmascript code snippets which are evaluated when
# a new global context is created.  There are multiple minifiers, closure
# seems to be doing the best job so only that is enabled now.  Obfuscating
# the code is not a goal, although that happens as an unwanted side effect.
#
# Closure compiler --compilation_level ADVANCED_OPTIMIZATIONS breaks some
# of the existing code, so don't use it.

#cat src/duk_initjs.js \
#	| UglifyJS/bin/uglifyjs --ascii --no-dead-code --no-copyright \
#	> $DISTSRCSEP/duk_initjs_uglify.js.tmp
#if [ $? -ne 0 ]; then
#	echo "UglifyJS initjs step failed"
#	exit 1
#fi

#UglifyJS2/bin/uglifyjs \
#	src/duk_initjs.js \
#	--screw-ie8 \
#	--compress warnings=false \
#	> $DISTSRCSEP/duk_initjs_uglify2.js.tmp
#if [ $? -ne 0 ]; then
#	echo "UglifyJS2 initjs step failed"
#	exit 1
#fi

java -jar compiler.jar \
	--warning_level QUIET \
	--language_in ECMASCRIPT5 \
	--compilation_level SIMPLE_OPTIMIZATIONS \
	src/duk_initjs.js \
	> $DISTSRCSEP/duk_initjs_closure.js.tmp
if [ $? -ne 0 ]; then
	echo "Closure initjs step failed"
	exit 1
fi

INITJS_ORIG=`wc -c < src/duk_initjs.js`
INITJS_UGLIFY=DISABLED
INITJS_UGLIFY2=DISABLED
#INITJS_UGLIFY=`wc -c < $DISTSRCSEP/duk_initjs_uglify.js.tmp`
#INITJS_UGLIFY2=`wc -c < $DISTSRCSEP/duk_initjs_uglify2.js.tmp`
INITJS_CLOSURE=`wc -c < $DISTSRCSEP/duk_initjs_closure.js.tmp`
echo "Minified initjs size: original=$INITJS_ORIG, UglifyJS=$INITJS_UGLIFY, UglifyJS2=$INITJS_UGLIFY2, closure=$INITJS_CLOSURE"

# choose which minifier to use
#echo "Using UglifyJS minified version"; cp $DISTSRCSEP/duk_initjs_uglify.js.tmp $DISTSRCSEP/duk_initjs_min.js
#echo "Using UglifyJS2 minified version"; cp $DISTSRCSEP/duk_initjs_uglify2.js.tmp $DISTSRCSEP/duk_initjs_min.js
echo "Using closure minified version"; cp $DISTSRCSEP/duk_initjs_closure.js.tmp $DISTSRCSEP/duk_initjs_min.js

# Autogenerated strings and built-in files
#
# There are currently no profile specific variants of strings/builtins, but
# this will probably change when functions are added/removed based on profile.

python src/genbuildparams.py \
	--version=$DUK_VERSION \
	--out-json=$DISTSRCSEP/buildparams.json.tmp \
	--out-header=$DISTSRCSEP/duk_buildparams.h.tmp

python src/genbuiltins.py \
	--buildinfo=$DISTSRCSEP/buildparams.json.tmp \
	--initjs-data=$DISTSRCSEP/duk_initjs_min.js \
	--out-header=$DISTSRCSEP/duk_builtins.h \
	--out-source=$DISTSRCSEP/duk_builtins.c \

# Autogenerated Unicode files
#
# Note: not all of the generated headers are used.  For instance, the
# match table for "WhiteSpace-Z" is not used, because a custom piece
# of code handles that particular match.
#
# For IDPART:
#   UnicodeCombiningMark -> categories Mn, Mc
#   UnicodeDigit -> categories Nd
#   UnicodeConnectorPunctuation -> categories Pc

# Whitespace (unused now)
WHITESPACE_INCL='Zs'  # USP = Any other Unicode space separator
WHITESPACE_EXCL='NONE'

# Unicode letter (unused now)
LETTER_INCL='Lu,Ll,Lt,Lm,Lo'
LETTER_EXCL='NONE'
LETTER_NOA_INCL='Lu,Ll,Lt,Lm,Lo'
LETTER_NOA_EXCL='ASCII'
LETTER_NOABMP_INCL=$LETTER_NOA_INCL
LETTER_NOABMP_EXCL='ASCII,NONBMP'

# Identifier start
# E5 Section 7.6
IDSTART_INCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F'
IDSTART_EXCL='NONE'
IDSTART_NOA_INCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F'
IDSTART_NOA_EXCL='ASCII'
IDSTART_NOABMP_INCL=$IDSTART_NOA_INCL
IDSTART_NOABMP_EXCL='ASCII,NONBMP'

# Identifier start - Letter: allows matching of (rarely needed) 'Letter'
# production space efficiently with the help of IdentifierStart.  The
# 'Letter' production is only needed in case conversion of Greek final
# sigma.
IDSTART_MINUS_LETTER_INCL=$IDSTART_NOA_INCL
IDSTART_MINUS_LETTER_EXCL='Lu,Ll,Lt,Lm,Lo'
IDSTART_MINUS_LETTER_NOA_INCL=$IDSTART_NOA_INCL
IDSTART_MINUS_LETTER_NOA_EXCL='Lu,Ll,Lt,Lm,Lo,ASCII'
IDSTART_MINUS_LETTER_NOABMP_INCL=$IDSTART_NOA_INCL
IDSTART_MINUS_LETTER_NOABMP_EXCL='Lu,Ll,Lt,Lm,Lo,ASCII,NONBMP'

# Identifier start - Identifier part
# E5 Section 7.6: IdentifierPart, but remove IdentifierStart (already above)
IDPART_MINUS_IDSTART_INCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,Mn,Mc,Nd,Pc,200C,200D'
IDPART_MINUS_IDSTART_EXCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F'
IDPART_MINUS_IDSTART_NOA_INCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,Mn,Mc,Nd,Pc,200C,200D'
IDPART_MINUS_IDSTART_NOA_EXCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,ASCII'
IDPART_MINUS_IDSTART_NOABMP_INCL=$IDPART_MINUS_IDSTART_NOA_INCL
IDPART_MINUS_IDSTART_NOABMP_EXCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,ASCII,NONBMP'

extract_chars() {
	python src/extract_chars.py \
		--unicode-data=src/UnicodeData.txt \
		--include-categories="$1" \
		--exclude-categories="$2" \
		--out-source=$DISTSRCSEP/duk_unicode_$3.c.tmp \
		--out-header=$DISTSRCSEP/duk_unicode_$3.h.tmp \
		--table-name=duk_unicode_$3 \
		> $DISTSRCSEP/$3.txt
}

extract_caseconv() {
	python src/extract_caseconv.py \
		--unicode-data=src/UnicodeData.txt \
		--special-casing=src/SpecialCasing.txt \
		--out-source=$DISTSRCSEP/duk_unicode_caseconv.c.tmp \
		--out-header=$DISTSRCSEP/duk_unicode_caseconv.h.tmp \
		--table-name-lc=duk_unicode_caseconv_lc \
		--table-name-uc=duk_unicode_caseconv_uc \
		> $DISTSRCSEP/caseconv.txt
}

extract_chars $WHITESPACE_INCL $WHITESPACE_EXCL ws
extract_chars $LETTER_INCL $LETTER_EXCL let
extract_chars $LETTER_NOA_INCL $LETTER_NOA_EXCL let_noa
extract_chars $LETTER_NOABMP_INCL $LETTER_NOABMP_EXCL let_noabmp
extract_chars $IDSTART_INCL $IDSTART_EXCL ids
extract_chars $IDSTART_NOA_INCL $IDSTART_NOA_EXCL ids_noa
extract_chars $IDSTART_NOABMP_INCL $IDSTART_NOABMP_EXCL ids_noabmp
extract_chars $IDSTART_MINUS_LETTER_INCL $IDSTART_MINUS_LETTER_EXCL ids_m_let
extract_chars $IDSTART_MINUS_LETTER_NOA_INCL $IDSTART_MINUS_LETTER_NOA_EXCL ids_m_let_noa
extract_chars $IDSTART_MINUS_LETTER_NOABMP_INCL $IDSTART_MINUS_LETTER_NOABMP_EXCL ids_m_let_noabmp
extract_chars $IDPART_MINUS_IDSTART_INCL $IDPART_MINUS_IDSTART_EXCL idp_m_ids
extract_chars $IDPART_MINUS_IDSTART_NOA_INCL $IDPART_MINUS_IDSTART_NOA_EXCL idp_m_ids_noa
extract_chars $IDPART_MINUS_IDSTART_NOABMP_INCL $IDPART_MINUS_IDSTART_NOABMP_EXCL idp_m_ids_noabmp
extract_caseconv

# Inject autogenerated files into source and header files so that they are
# usable (for all profiles and define cases) directly.
#
# The injection points use a standard C preprocessor #include syntax
# (earlier these were actual includes).

cat > $DISTSRCSEP/sed.tmp <<EOF
/#include "duk_unicode_ids_noa.h"/ {
	r $DISTSRCSEP/duk_unicode_ids_noa.h.tmp
	d
}
/#include "duk_unicode_ids_noabmp.h"/ {
	r $DISTSRCSEP/duk_unicode_ids_noabmp.h.tmp
	d
}
/#include "duk_unicode_ids_m_let_noa.h"/ {
	r $DISTSRCSEP/duk_unicode_ids_m_let_noa.h.tmp
	d
}
/#include "duk_unicode_ids_m_let_noabmp.h"/ {
	r $DISTSRCSEP/duk_unicode_ids_m_let_noabmp.h.tmp
	d
}
/#include "duk_unicode_idp_m_ids_noa.h"/ {
	r $DISTSRCSEP/duk_unicode_idp_m_ids_noa.h.tmp
	d
}
/#include "duk_unicode_idp_m_ids_noabmp.h"/ {
	r $DISTSRCSEP/duk_unicode_idp_m_ids_noabmp.h.tmp
	d
}
/#include "duk_unicode_caseconv.h"/ {
	r $DISTSRCSEP/duk_unicode_caseconv.h.tmp
	d
}
EOF

mv $DISTSRCSEP/duk_unicode.h $DISTSRCSEP/duk_unicode.h.tmp
sed -f $DISTSRCSEP/sed.tmp $DISTSRCSEP/duk_unicode.h.tmp > $DISTSRCSEP/duk_unicode.h
rm $DISTSRCSEP/sed.tmp
rm $DISTSRCSEP/duk_unicode.h.tmp

cat > $DISTSRCSEP/sed.tmp <<EOF
/#include "duk_unicode_ids_noa.c"/ {
	r $DISTSRCSEP/duk_unicode_ids_noa.c.tmp
	d
}
/#include "duk_unicode_ids_noabmp.c"/ {
	r $DISTSRCSEP/duk_unicode_ids_noabmp.c.tmp
	d
}
/#include "duk_unicode_ids_m_let_noa.c"/ {
	r $DISTSRCSEP/duk_unicode_ids_m_let_noa.c.tmp
	d
}
/#include "duk_unicode_ids_m_let_noabmp.c"/ {
	r $DISTSRCSEP/duk_unicode_ids_m_let_noabmp.c.tmp
	d
}
/#include "duk_unicode_idp_m_ids_noa.c"/ {
	r $DISTSRCSEP/duk_unicode_idp_m_ids_noa.c.tmp
	d
}
/#include "duk_unicode_idp_m_ids_noabmp.c"/ {
	r $DISTSRCSEP/duk_unicode_idp_m_ids_noabmp.c.tmp
	d
}
/#include "duk_unicode_caseconv.c"/ {
	r $DISTSRCSEP/duk_unicode_caseconv.c.tmp
	d
}
EOF

mv $DISTSRCSEP/duk_unicode_tables.c $DISTSRCSEP/duk_unicode_tables.c.tmp
sed -f $DISTSRCSEP/sed.tmp $DISTSRCSEP/duk_unicode_tables.c.tmp > $DISTSRCSEP/duk_unicode_tables.c
rm $DISTSRCSEP/sed.tmp
rm $DISTSRCSEP/duk_unicode_tables.c.tmp

# Clean up sources

rm $DISTSRCSEP/*.tmp
for i in \
	ws \
	let let_noa let_noabmp \
	ids ids_noa ids_noabmp \
	ids_m_let ids_m_let_noa ids_m_let_noabmp \
	idp_m_ids idp_m_ids_noa idp_m_ids_noabmp; do
	rm $DISTSRCSEP/$i.txt
done
rm $DISTSRCSEP/caseconv.txt

# Create a combined source file, duktape.c, into a separate combined source
# directory.  This allows user to just include "duktape.c" and "duktape.h"
# into a project and maximizes inlining and size optimization opportunities
# even with older compilers.  Because some projects include these files into
# their repository, the result should be deterministic and diffable.  Also,
# it must retain __FILE__/__LINE__ behavior through preprocessor directives.
# Whitespace and comments can be stripped as long as the other requirements
# are met.

python util/combine_src.py $DISTSRCSEP $DISTSRCCOM/duktape.c \
	"$DUK_VERSION" "$GIT_COMMIT" "$GIT_DESCRIBE" \
	$DIST/LICENSE.txt.tmp $DIST/AUTHORS.txt.tmp
echo "CLOC report on combined duktape.c source file"
perl cloc-1.60.pl --quiet $DISTSRCCOM/duktape.c

cp $DISTSRCSEP/duktape.h $DISTSRCCOM/duktape.h

# Clean up remaining files

rm $DIST/*.tmp
