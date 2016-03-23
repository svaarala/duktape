# Say 'make' and then just run combined.js.
#
# https://v8.googlecode.com/svn/data/benchmarks/v7/run.html

.PHONY: all
all:	combined.js

base.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/base.js
richards.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/richards.js
deltablue.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/deltablue.js
crypto.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/crypto.js
	patch -p0 < crypto.diff  # needed by mujs
raytrace.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/raytrace.js
earley-boyer.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/earley-boyer.js
	patch -p0 < earley-boyer.diff  # needed by mujs
regexp.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/regexp.js
splay.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/splay.js
navier-stokes.js:
	wget -O $@ http://v8.googlecode.com/svn/data/benchmarks/v7/navier-stokes.js

combined.js: base.js richards.js deltablue.js crypto.js raytrace.js earley-boyer.js regexp.js splay.js navier-stokes.js
	cat base.js richards.js deltablue.js crypto.js raytrace.js earley-boyer.js regexp.js splay.js navier-stokes.js run_harness.js >$@
	wc $@

.PHONY:
clean:
	rm -f base.js richards.js deltablue.js crypto.js raytrace.js earley-boyer.js regexp.js splay.js navier-stokes.js
	rm -f combined.js
