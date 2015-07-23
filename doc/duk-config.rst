===================================
Duktape config header, duk_config.h
===================================

Purpose of duk_config.h
=======================

**duk_config.h is an external configuration header ("config header") which
provides all platform, compiler, and architecture specific features** so that
the main Duktape source code can compile without relying on platform specific
headers or functionality.  The header also provides active Duktape config
options (``DUK_USE_xxx``) for enabling/disabling various optional Duktape
features.

The external ``duk_config.h`` config header replaces the built-in platform and
feature detection used by Duktape 1.2 and prior.  Moving the built-in platform
and configuration logic into an external header avoids the need to use compiler
command line defines (``-DDUK_OPT_xxx``) which were needed both when compiling
Duktape and the application.  An external config header is also easier to
manually adapt to exotic environments without the need to change Duktape
internals such as platform detection.  Finally, a config header is a much better
match to Unix distributions than relying on compiler command line defines.

While the external config header provides much more flexibility it also needs
a bit more thought especially when adapting Duktape to an exotic environment.

This document describes various approaches on creating a config header and
updating it when a new Duktape release is taken into use.

Coming up with a duk_config.h
=============================

``duk_config.h`` is always external to Duktape main source code so that it's
always possible, if necessary, to manually edit the configuration file or
even create one from scratch as a last resort.

As such there are multiple ways to come up with a config header; for common
platforms you don't usually need to do much while for more exotic platforms
more manual work may be needed.  There's no "right way", but the more manual
modifications are made, the more effort is needed to deal with Duktape updates.

Default duk_config.h with platform autodetection
------------------------------------------------

Duktape distributable includes a default duk_config.h that is compatible
with Duktape 1.2: it autodetects the platform, compiler, and architecture,
and resolves active config options through feature options (DUK_OPT_xxx).

This header should work "out of the box" for Linux, OS X, and Windows and
is a drop-in replacement for Duktape 1.2 behavior.  If you're using one of
these platforms, this should be your default choice.

In future Duktape versions the ``DUK_OPT_xxx`` feature options will be
removed altogether so that config options are only controlled through
``DUK_USE_xxx`` options, avoiding the two-level structure which is no
longer well justified if ``DUK_USE_xxx`` options are configured directly
in the config header.

Modifying the default duk_config.h manually
-------------------------------------------

You can modify the default duk_config.h directly if only a small change
is needed.  Such changes can be manual, or scripted using e.g. ``sed``.
Using scripting is less error prone when Duktape is upgraded and the
source duk_config.h changes (which is usual for new versions).

See separate section below on how to tweak a header using a script.

Using genconfig.py to create a barebones duk_config.h
-----------------------------------------------------

To support exotic platforms, Duktape distributable also includes a
``genconfig.py`` script which can generate a template duk_config.h for
a range of platforms.  You can generate a header most closely matching
your target, and then modify it manually or via scripting.

See separate section below on using genconfig.

Writing a duk_config.h from scratch
-----------------------------------

You could also write a duk_config.h from scratch, but because there are
quite many typedefs, macros, and config options, it's probably easiest
to modify the default or genconfig-generated duk_config.h.

Modifying a duk_config.h using scripting
========================================

The basic approach when using scripted modifications is to take a base header
(either the autodetecting Duktape default header or a genconfig-generated
header) and then make specific changes using a script.  The advantage of doing
so is that if the base header is updated, the script may often still be valid
without any manual changes.

Using diff/patch
----------------

* Make the necessary changes to the base header manually.

* Use ``diff`` to store the changes::

      $ diff -u duk_config.h.base duk_config.h.edited > edits.diff

* In your build script::

      $ cp duk_config.h.base duk_config.h
      $ patch duk_config.h edits.diff

* If the patch fails (e.g. there is too much offset), you need to
  rebuild the diff file manually.

Using sed to modify an option in-place
--------------------------------------

If an option is defined on a single line in the base header (this is true
for Duktape config options in the genconfig "barebones" header for example),
e.g. either as::

   #define DUK_USE_FOO

or as::

   #undef DUK_USE_FOO

you can use ``sed`` to easily flip such an option::

    # enable shuffle torture
    cat duk_config.h.base | \
        sed -r -e 's/^#\w+\s+DUK_USE_SHUFFLE_TORTURE.*$/#define DUK_USE_SHUFFLE_TORTURE  \/*forced*\//' \
        > duk_config.h

The above example would flip DUK_USE_SHUFFLE_TORTURE on, regardless of
its previous setting.  You can also use a more verbose sed format which
is easier to read especially if there are multiple changes::

    cat duk_config.h.base | sed -r -e '
    s/^#\w+\s+DUK_USE_SHUFFLE_TORTURE.*$/#define DUK_USE_SHUFFLE_TORTURE  \/*forced*\//
    s/^#\w+\s+DUK_USE_OS_STRING.*$/#define DUK_USE_OS_STRING "my-custom-os"  \/*forced*\//
    ' > duk_config.h

This approach won't work if the defined option is defined/undefined
multiple times or if the define has a multiline value.

Using awk or other script language
----------------------------------

For more stateful changes you can use ``awk`` or other scripting languages
(Python, Perl, etc).

Modifying defines at the end of the file
----------------------------------------

Instead of modifying options in-place as in the sed example above, you can
simply append additional preprocessor directives to undefine/redefine options
as necessary.  This is much easier to maintain in version updates than when
modifications are made in-place.

Genconfig has a direct option to append "fixups" after the main generated
header::

    # my_custom.h is applied after generated header; functionally similar
    # to Duktape 1.2.x duk_custom.h

    $ python genconfig.py [...] --fixup-header-file my_custom.h [...]

A genconfig-generated barebones header also has the following line near the end
for detecting where to add override defines; this is easy to detect reliably::

    /* __OVERRIDE_DEFINES__ */

The ``__OVERRIDE_DEFINES__`` line is near the end of the file, before any
automatically generated option sanity checks (which are optional) so that the
sanity checks will be applied after your tweaks have been done.

Another simple approach is to simply assume that an ``#endif`` line (include
guard) is the last line in the file, i.e. there are no trailing empty lines.
Changes will then be applied after option sanity checks which is not ideal::

    #!/bin/bash

    CONFIG_IN=duk_config.h.base
    CONFIG_OUT=duk_config.h.new

    if tail -1 $CONFIG_IN | grep endif ; then
        echo "Final line of $CONFIG_IN is an #endif as expected, modifying config"
    else
        echo "Final line of $CONFIG_IN is not an #endif!"
        exit 1
    fi

    head -n -1 $CONFIG_IN > $CONFIG_OUT
    cat >> $CONFIG_OUT <<EOF
    /*
     *  Config hacks for platform XYZ.
     */

    #undef DUK_USE_FASTINT  /* undef first to avoid redefine */
    #define DUK_USE_FASTINT

    /* compiler on XYZ has a custom "unreferenced" syntax */
    #undef DUK_UNREF
    #define DUK_UNREF(x) do { __foo_compiler_unreferenced((x)); } while (0)

    #endif  /* DUK_CONFIG_H_INCLUDED */
    EOF

    echo "Wrote new config to $CONFIG_OUT, diff -u:"
    diff -u $CONFIG_IN $CONFIG_OUT

Modifying defines at the end of the file is relatively easy but has a few
limitations:

* You can't change typedefs this way because there's no way to un-typedef.

* You can't undo any ``#include`` directives executed.

Dealing with #include files
---------------------------

Include files are often a portability problem on exotic targets:

* System headers may be missing.  You may need to provide replacement functions
  for even very basic features like string formatting functions.

* System headers may be present but broken in some fashion so you want to avoid
  them entirely.

* Sometimes custom programming environments have "SDK headers" that conflict
  with standard headers so that you can't include them both at the same time.
  It may be necessary to include the SDK headers but provide manual declarations
  for the system functions needed.

In such cases you may need to replace all the ``#include`` statements of a
base header file and provide alternate include files or manual declarations.

Keeping a manually created duk_config.h up-to-date
==================================================

When new Duktape versions are released, the set of config options and
other macros required of the ``duk_config.h`` config header may change.
This is the case for even minor version updates, though incompatible
changes are of course avoided when possible.

Nevertheless, when a new version is taken into use, you may need to
update your config header to match.  How to do that depends on how you
created the config header:

* If you're using the default header, no changes should be necessary.
  You should check out new ``DUK_OPT_xxx`` feature options and decide
  if you want to use any of them.

* If you're using a script to modify the default or genconfig-generated
  header, you should ensure your script works when the source header is
  updated to the new Duktape release.

* If you're editing a config header manually, you should look at the
  diff between the previous and new default config header to see what
  defines have changed, and then implement matching changes in your
  updated header.

Using genconfig
===============

Overview of genconfig
---------------------

Genconfig (``config/genconfig.py``) is a helper script which provides
several commands related to config handling:

* Generate the default, autodetecting ``duk_config.h``.

* Generate a barebones ``duk_config.h`` for a specific platform, compiler,
  and architecture, with possible config option overrides.

* Generate documentation for feature and config options.

Config headers are generated based on config option and target metadata
files, and manually edited header snippets which are combined to create
a final header.  Documentation is generated based on config option metadata.
Metadata is expressed as YAML files for easy editing and good diff/merge
behavior.

Generating a barebones duk_config.h
-----------------------------------

**FIXME: this now works for linux-gcc-x64 example but there are no other
platform/compiler/architecture files yet.**

To generate a barebones header you need to specify a platform, compiler, and
architecture for genconfig::

    $ cd duktape/config
    $ python genconfig.py \
        --platform linux \
        --compiler gcc \
        --architecture x64 \
        --output /tmp/duk_config.h \
        generate-barebones-header

The barebones header in ``/tmp/duk_config.h`` can then either be used as is
or edited manually or through scripting.

The platform, compiler, and architecture names map to genconfig header snippet
files.  Duktape config options will be assigned their default values specified
in config option metadata files in ``config/config-options/``.

You can override individual defines using a config file in YAML format (see
section below for a detailed discussion of the format).  For example,
``my_config.yaml`` could contain::

    DUK_USE_OS_STRING: "\"hack-os\""  # force os name for Duktape.env
    DUK_USE_ALIGN_4: false
    DUK_USE_ALIGN_8: true   # force align-by-8
    DUK_USE_FASTINT: true
    DUK_UNREF:
      verbatim: "#define DUK_UNREF(x) do { (void) (x); } while (0)"

You can give one or more such files to genconfig (last override wins)::

    $ cd duktape/config
    $ python genconfig.py \
        --platform linux \
        --compiler gcc \
        --architecture x64 \
        --force-option-file my_config.yaml \
        --force-option-file more_overrides.yaml \
        --output /tmp/duk_config.h \
        generate-barebones-header

You can also give forced options directly on the command line in YAML format::

    $ cd duktape/config
    $ python genconfig.py \
        --platform linux \
        --compiler gcc \
        --architecture x64 \
        --force-option-file my_config.yaml \
        --force-option-file more_overrides.yaml \
        --force-option-yaml 'DUK_USE_DEEP_C_STACK: false' \
        --output /tmp/duk_config.h \
        generate-barebones-header

Multiple forced options can be given either by using a YAML value with
multiple keys, or by using multiple options::

    # Multiple values for one option
    --force-option-yaml '{ DUK_USE_DEEP_C_STACK: false, DUK_USE_DEBUG: true }'

    # Multiple options
    --force-option-yaml 'DUK_USE_DEEP_C_STACK: false' \
    --force-option-yaml 'DUK_USE_DEBUG: true'

Some changes such as reworking ``#include`` statements cannot be represented
as override files; you'll need to edit the resulting config header manually
or using some scripting approach.

In addition to YAML-based option overrides, genconfig has an option for
appending direct "fixup headers" (similar to Duktape 1.2.x ``duk_custom.h``)::

    $ python genconfig.py \
        --platform linux \
        --compiler gcc \
        --architecture x64 \
        --fixup-header-file my_arch_string.h \
        --fixup-header-file my_no_json_fastpath.h \
        --output /tmp/duk_config.h \
        generate-barebones-header

In this example ``my_arch_string.h`` could be::

    /* Force arch string. */
    #undef DUK_USE_ARCH_STRING
    #define DUK_USE_ARCH_STRING "myarch"

and ``my_no_json_fastpath.h`` could be::

    /* Disable JSON fastpath for reduced footprint. */
    #undef DUK_USE_JSON_STRINGIFY_FASTPATH

Fixup headers are appended verbatim so they must be valid C header files,
contain appropriate newlines, and must ``#undef`` any defines before
redefining them if necessary.  Fixup headers can only be used to tweak C
preprocessor defines, they naturally cannot un-include headers or un-typedef
types.

Format for option overrides
---------------------------

The override file keys are define names, and values can be:

* ``false``: ``#undef`` option::

      # Produces: #undef DUK_USE_DEBUG
      DUK_USE_DEBUG: false

* ``true``: ``#define`` option::

      # Produces: #define DUK_USE_DEBUG
      DUK_USE_DEBUG: true

* number: decimal value for define::

      # Produces: DUK_USE_TRACEBACK_DEPTH: 10
      DUK_USE_TRACEBACK_DEPTH: 10

      # Produces: DUK_USE_TRACEBACK_DEPTH: 100000L
      # (a long constant is used automatically if necessary)
      DUK_USE_TRACEBACK_DEPTH: 100000

* string: verbatim string used as the define value::

      # Produces: #define DUK_USE_TRACEBACK_DEPTH (10 + 7)
      DUK_USE_TRACEBACK_DEPTH: "(10 + 7)"

      # Produces: #define DUK_USE_OS_STRING "linux"
      DUK_USE_OS_STRING: "\"linux\""

* C string for value::

      # Produces: #define DUK_USE_OS_STRING "linux"
      DUK_USE_OS_STRING:
        string: "linux"

* verbatim text for entire define::

      # Produces: #define DUK_UNREF(x) do {} while (0)
      DUK_UNREF:
        verbatim: "#define DUK_UNREF(x) do {} while (0)"

Defines provided by duk_config.h
================================

The role of ``duk_config.h`` is to provide all typedefs, macros, structures,
system headers, etc, which are platform dependent.  Duktape internals can
then just assume these are in place and will remain clean of any detection.

These typedefs, macros, etc, include:

* Including platform specific headers (``#include <...>``) needed by any of
  the config header macros, including:

  - Standard library functions like ``sprintf()`` and ``memset()``

  - Math functions like ``acos()``

  - Any other functions called by macros defined in duk_config.h, e.g. the
    functions needed by a custom Date provider

* Typedefs for integer and floating point types (``duk_uint8_t``, etc),
  and their limit defines.

* Some IEEE double constants, including NaN and Infinity, because some
  constants cannot be reliably expressed as constants in all compilers.

* Wrapper macros for platform functions, covering string operations,
  file I/O, math, etc.  For example: ``DUK_FOPEN()``, ``DUK_SPRINTF()``,
  ``DUK_ACOS()``), etc.  Typically these are just mapped 1:1 to platform
  functions, but sometimes tweaks are needed.

* Various compiler specific macros: unreachable code, unreferenced
  variable, symbol visibility attributes, inlining control, etc.

* Duktape config options, ``DUK_USE_xxx``, including a possible custom
  Date provider.

The required defines and typedefs are also available in a machine parseable
metadata form:

* ``config/other-defines/c_types.yaml``: required integer and other types
  and their limits.

* ``config/other-defines/platform_functions.yaml``: required platform
  function wrappers.

* ``config/other-defines/other_defines.yaml``: compiler specific macros
  and other misc defines.

* ``config/config-options/DUK_USE_*.yaml``: Duktape config options.

Motivation for duk_config.h
===========================

Duktape 1.2 feature option benefits
-----------------------------------

* Works out of the box for many targets

  - With default options and a supported platform just compile and run

  - Preprocessor-based detection works well with cross compilation compared
    to e.g. autoconf or similar

* Feature options only needed to deviate from defaults

  - No need to read through all feature options to start using

  - Learn relevant feature options when they become relevant

Duktape 1.2 feature option problems
-----------------------------------

* Monolithic detection

  - One large file which becomes more and more difficult to maintain

  - Doesn't serve mainline platforms well: clutter from exotic platforms

  - Doesn't server exotic platforms well: doesn't support nearly all exotic
    platforms, and difficult to maintain a patched version

* Difficult to support proprietary or broken platforms

  - Cannot easily merge support to mainline

  - Even if could, some hacks needed by broken platforms may be extreme

* Preprocessor detection is not always possible

  - Some platform features may not be detectable through preprocessor
    defines

  - Some detection code may go wrong because a platform provides incorrect
    defines

* Awkward requirement to provide same feature options (DUK_OPT_xxx) for both
  Duktape and application compilation

  - Easy to forget when compiling an application

  - Error prone to maintain option lists for Duktape and application when
    they're compiled separately

  - Difficult to install as a system library unless using default feature
    options: how would custom options be passed to applications?

Nice-to-have features
=====================

Should use a configuration header (duk_config.h):

* Common model for many libraries, works well with distributions

Should provide a default configuration header which works out of the box:

* Similar to Duktape 1.2: automatic detection of at least mainline platforms

* Platform support for automatic detection can be narrowed from Duktape 1.2

Default configuration header should be 1.2 backwards compatible:

* In other words, current DUK_OPT_xxx feature options should be supported

* Allows easier transition and avoids the need to bump the major version

Should document DUK_USE_xxx options and other defines expected from
duk_config.h so that a configuration header can be built manually:

* Human readable documentation and/or programmatic metadata

* If programmatic metadata, automatic generation of option documentation

Should provide a configuration utility for generating template headers:

* Such a template can then more easily be edited manually

* Allow forced deviations from defaults directly in the utility so that
  a generated header is already customized
