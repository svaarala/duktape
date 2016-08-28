#
#  Configuration profile compilation commands for Linux
#

CC=gcc
CCOPTSSHARED=-fomit-frame-pointer -flto -fno-asynchronous-unwind-tables \
	-ffunction-sections -Wl,--gc-sections -m32
CCOPTSMEM=-Os $(CCOPTSSHARED)
CCOPTSPERF=-O2 $(CCOPTSSHARED)

# FIXME: srcdir
TEMPDIR=./duk-profile-temp
CONFIGURE=tools/configure.py
SRCINPUT=src-input

# FIXME: sub-python CWD dependency is ugly
# FIXME: path handling is awkward..
# FIXME: makeduk etc

.PHONY: all
all: duk-default \
	duk-lowmem duk-lowmem-ptrcomp duk-lowmem-ptrcomp-rom \
	duk-stripped duk-stripped-ptrcomp duk-stripped-ptrcomp-rom \
	duk-perf duk-perf-norefcount
	size $^

.PHONY: clean
clean:
	@rm -rf $(TEMPDIR)
	@rm -f duk-*

duk-default:
	@rm -rf $(TEMPDIR); mkdir $(TEMPDIR)
	cd ..; python2 $(CONFIGURE) \
		--output-directory profiles/$(TEMPDIR)
	$(CC) $(CCOPTSMEM) -o $@ \
		-I$(TEMPDIR) -I../examples/eval \
		$(TEMPDIR)/duktape.c ../examples/eval/eval.c \
		-lm
	size $@

duk-lowmem:
	@rm -rf $(TEMPDIR); mkdir $(TEMPDIR)
	cd ..; python2 $(CONFIGURE) \
		--output-directory profiles/$(TEMPDIR) \
		--option-file profiles/config/low_memory.yaml
	$(CC) $(CCOPTSMEM) -o $@ \
		-I$(TEMPDIR) -I../examples/eval \
		$(TEMPDIR)/duktape.c ../examples/eval/eval.c \
		-lm
	size $@

duk-lowmem-ptrcomp:

duk-lowmem-ptrcomp-rom:

duk-stripped:
	@rm -rf $(TEMPDIR); mkdir $(TEMPDIR)
	cd ..; python2 $(CONFIGURE) \
		--output-directory profiles/$(TEMPDIR) \
		--option-file profiles/config/low_memory.yaml \
		--option-file profiles/config/low_memory_strip.yaml \
		--builtin-file profiles/builtins/remove_math_properties.yaml \
		--builtin-file profiles/builtins/remove_bufferobject_properties.yaml \
		--unicode-data $(SRCINPUT)/UnicodeData-8bit.txt \
		--special-casing $(SRCINPUT)/SpecialCasing-8bit.txt
	$(CC) $(CCOPTSMEM) -o $@ \
		-I$(TEMPDIR) -I../examples/eval \
		$(TEMPDIR)/duktape.c ../examples/eval/eval.c \
		-lm
	size $@

duk-stripped-ptrcomp:

duk-stripped-ptrcomp-rom:

duk-perf:

duk-perf-norefcount:
