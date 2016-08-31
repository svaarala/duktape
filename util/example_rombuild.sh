#
#  Some compilation examples for ROM strings/objects
#

set -e

PYTHON=`which python2 python | head -1`

make clean dist

# Prepare-and-config sources manually to enable ROM support.  User builtin
# metadata can be provided through one or more YAML files (which are applied
# in sequence).  Duktape configuration can be given at the same time.
rm -rf dist/src dist/src-noline dist/src-separate
$PYTHON dist/tools/configure.py \
	--source-directory dist/src-input \
	--output-directory dist \
	--rom-support \
	--rom-auto-lightfunc \
	--user-builtin-metadata util/example_user_builtins1.yaml \
	--user-builtin-metadata util/example_user_builtins2.yaml \
	--config-metadata dist/config \
	-DDUK_USE_ROM_STRINGS \
	-DDUK_USE_ROM_OBJECTS \
	-DDUK_USE_ROM_GLOBAL_INHERIT \
	-DDUK_USE_DEBUG -DDUK_USE_DEBUG_LEVEL=0 \
	--option-yaml 'DUK_USE_DEBUG_WRITE: { "verbatim": "#define DUK_USE_DEBUG_WRITE(level,file,line,func,msg) do {fprintf(stderr, \"%ld %s:%ld (%s): %s\\n\", (long) (level), (file), (long) (line), (func), (msg)); } while(0)" }' \
	-DDUK_USE_ASSERTIONS
#gcc -std=c99 -Wall -Wextra -Os -Idist/src-separate/ -Idist/examples/cmdline dist/src-separate/*.c dist/examples/cmdline/duk_cmdline.c -o _duk -lm
make duk dukd  # XXX: currently fails to start, DUK_CMDLINE_LOGGING_SUPPORT, DUK_CMDLINE_MODULE_SUPPORT modify Duktape object (doesn't work with ROM built-ins)

# Ajduk depends on 'make ajduk' and uses DUK_OPT_xxx feature options.
# This would ideally be done directly using genconfig.py without
# --support-feature-options by moving the options into a genconfig
# YAML config file.
rm -rf dist/src dist/src-noline dist/src-separate
$PYTHON dist/tools/configure.py \
	--source-directory dist/src-input \
	--output-directory dist \
	--rom-support \
	--rom-auto-lightfunc \
	--user-builtin-metadata util/example_user_builtins1.yaml \
	--user-builtin-metadata util/example_user_builtins2.yaml \
	--config-metadata dist/config \
	--support-feature-options \
	-DDUK_USE_ROM_STRINGS \
	-DDUK_USE_ROM_OBJECTS \
	-DDUK_USE_ROM_GLOBAL_INHERIT \
	-DDUK_USE_ASSERTIONS \
	-UDUK_USE_DEBUG
#gcc -std=c99 -Wall -Wextra -Os -Idist/src-separate/ -Idist/examples/cmdline dist/src-separate/*.c dist/examples/cmdline/duk_cmdline.c -o _duk -lm
make ajduk
