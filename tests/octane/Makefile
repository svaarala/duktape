# octane/mandreel.js: exceeds const limit at present.
# octane/run.js: not needed, duktape_harness.js handles
# octane/typescript-compiler.js: I/O to be stubbed
# octane/typescript-input.js: -""-
# octane/typescript.js: -""-
# octane/zlib.js: read() dependency
# octane/zlib-data.js: -""-
# octane/regexp.js: checksum error

OCTANEBASE= \
	octane/base.js
TESTS= \
	octane/box2d.js \
	octane/code-load.js \
	octane/crypto.js \
	octane/deltablue.js \
	octane/earley-boyer.js \
	octane/gbemu-part1.js \
	octane/gbemu-part2.js \
	octane/navier-stokes.js \
	octane/pdfjs.js \
	octane/raytrace.js \
	octane/richards.js \
	octane/splay.js

ENGINE=../../duk.O2
#ENGINE=jerry
#ENGINE=mujs

.PHONY: all
all: test

.PHONY: check-engine
check-engine:
	if [ ! -f $(ENGINE) ]; then echo "*** Expected $(ENGINE) to exist"; exit 1; fi

.PHONY: test
test: check-engine octane
	$(ENGINE) $(OCTANEBASE) $(TESTS) duktape_harness.js

# Useful for comparison to other engines when the set of successful sub-tests
# isn't the same; run what can be run.
.PHONY: test-individual
test-individual: octane
	for i in $(TESTS); do echo "*** $$i" ; $(ENGINE) mujs_tweaks.js $(OCTANEBASE) $$i duktape_harness.js; done

octane:
	git clone https://github.com/chromium/octane.git
	touch $@
