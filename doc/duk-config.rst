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

Some Duktape features (like ROM built-in support and Unicode tables used) also
require changes to "prepared" Duktape source code.  The ``configure.py``
utility combines the preparation of a ``duk_config.h`` header and Duktape
source files; it accepts the same command line options as genconfig.py (and
more).  **Since Duktape 2.0 ``tools/configure.py`` is the recommended tool to
create both a config header and prepared Duktape sources for build**.  This
document describes genconfig.py usage but you should normally use configure.py
wherever genconfig.py is used.

The ``DUK_VERSION`` define is available for duk_config.h, so that application
configuration snippets can react to Duktape version if necessary (e.g. enable
features only for a newer version).

While an external config header provides much more flexibility it also needs
a bit more thought especially when adapting Duktape to an exotic environment.
This document describes various approaches on creating a config header and
updating it when a new Duktape release is taken into use.

Coming up with a duk_config.h
=============================

``duk_config.h`` is always external to Duktape main source code so that
it's always possible, if necessary as a last resort, to manually edit the
configuration file or even create one from scratch.

As such there are multiple ways to come up with a config header; for common
platforms you don't usually need to do much while for more exotic platforms
more manual work may be needed.  There's no "right way", but the more manual
modifications are made, the more effort is needed to deal with Duktape updates.

The basic options are:

* **Use default duk_config.h in distribution**:
  Duktape distributable includes a default duk_config.h which autodetects the
  the platform, compiler, and architecture, and uses default values for all
  Duktape configuration options.  This header should work "out of the box" for
  Linux, OS X, Windows, and also for several more exotic platforms.  If you're
  using one of the supported platform and default options are acceptable, this
  should be your default choice.  Note that ``DUK_OPT_xxx`` compiler command
  line options are no longer supported in Duktape 2.x; to use non-default
  options, run either configure.py (recommended) or genconfig.py.

* **Use default duk_config.h with manual modifications**:
  You can modify the default duk_config.h directly if only a small change
  is needed.  Such changes can be manual, or scripted using e.g. ``sed``.
  Using scripting is less error prone when Duktape is upgraded and the
  source duk_config.h changes (which is usual for new versions).  See separate
  section below on how to tweak a header using a script.

* **Use genconfig.py to create an autodetect duk_config.h**:
  You can use ``genconfig.py`` to create a custom autodetecting duk_config.h
  and specify config option overrides on genconfig command line.  See separate
  section below on how to use genconfig.

* **Use genconfig.py to create a barebones duk_config.h**:
  While the autodetect duk_config.h is convenient, it won't work on exotic
  platforms.  To support exotic platforms, ``genconfig.py`` can generate a
  template duk_config.h for a specified platform, compiler, and architecture
  combination (each can be either specified or left as "autodetect") which
  should match your target as closely as possible.  You can then modify the
  header manually or through scripting.

* **Edit the genconfig metadata and regenerate duk_config.h**:
  You can also add support for your custom platform directly into the
  genconfig metadata.  For example, to support a custom compiler, you'll
  need to add a compiler-specific C header snippets to detect the compiler
  and to override default macros which are inappropriate for that compiler.
  The ``duk_config.h`` can then be regenerated using updated metadata.

* **Write a duk_config.h from scratch**:
  You could also write a duk_config.h from scratch, but because there are
  quite many typedefs, macros, and config options, it's probably easiest
  to modify the default or genconfig-generated ``duk_config.h``.

NOTE: While you can run ``genconfig.py`` directly, it's recommended to use
``tools/configure.py`` instead.  The same configuration options (and more)
are accepted by configure.py.

Using genconfig
===============

Overview of genconfig
---------------------

Genconfig (``tools/genconfig.py``) is a small utility for config handling
with two basic purposes:

* Generate a ``duk_config.h`` for a specified platform, compiler, and
  architecture.  Each can be specified explicitly (e.g. use "gcc" for
  the compiler) or be left up to automatic compile-time detection.
  The default ``duk_config.h`` is generated with everything left up to
  automatic detection.  A barebones, target specific header can be
  generated by defining platform, compiler, and architecture explicitly.

* Generate documentation for config options.

Config headers are generated based on config option and target metadata
files, and manually edited header snippets which are combined to create
a final header.  Documentation is generated based on config option metadata.
Metadata is expressed as YAML files for easy editing and good diff/merge
behavior.

This document doesn't cover all available tool options; use
``python tools/genconfig.py --help`` or ``python tools/configure.py --help``
for a full list of current options.

Generating an autodetect duk_config.h
-------------------------------------

To generate an autodetect header suitable for directly supported platforms::

    $ cd duktape-2.0.0
    $ python tools/genconfig.py \
        --metadata config/ \
        --output /tmp/duk_config.h \
        duk-config-header

The resulting header in ``/tmp/duk_config.h`` can then either be used as is
or edited manually or through scripting.  The equivalent operation using
``tools/configure.py`` is::

    $ cd duktape-2.0.0
    $ python tools/configure.py \
        --source-directory src-input \
        --config-metadata config/ \
        --output-directory /tmp/output

The result directory ``/tmp/output`` contains a ``duk_config.h`` header
but also ``duktape.c`` and ``duktape.h`` to be included in your build.

You can override individual defines using in several ways (see "Option
overrides" section below for more details): C compiler format (-D and -U
options) and YAML config through a file or inline.

If you're building Duktape as a DLL, you should use the ``--dll`` option::

    $ python tools/genconfig.py \
        --metadata config/ \
        --dll \
        --output /tmp/duk_config.h \
        duk-config-header

The ``configure.py`` equivalent::

    $ python tools/configure.py \
        --source-directory src-input \
        --config-metadata config/ \
        --output-directory /tmp/output \
        --dll

DLL builds cannot be detected automatically and they affect symbol visibility
attributes on Windows.  The ``-dll`` option creates a header which assumes
that a DLL will be built.

Some changes such as reworking ``#include`` statements cannot be represented
as override files; you'll need to edit the resulting config header manually
or using some scripting approach.

Generating a barebones duk_config.h
-----------------------------------

To generate a barebones header you need to specify a platform, compiler, and
architecture for genconfig::

    $ python tools/genconfig.py \
        --metadata config/ \
        --platform linux \
        --compiler gcc \
        --architecture x64 \
        --output /tmp/duk_config.h \
        duk-config-header

The barebones header in ``/tmp/duk_config.h`` can then either be used as is
or edited manually or through scripting.

The platform, compiler, and architecture names map to genconfig header snippet
files.  Duktape config options will be assigned their default values specified
in config option metadata files in ``config/config-options/``.

You can override individual defines using in several ways (see "Option
overrides" section below for more details): C compiler format (-D and -U
options) or YAML config through a file or inline.

Some changes such as reworking ``#include`` statements cannot be represented
as override files; you'll need to edit the resulting config header manually
or using some scripting approach.

Genconfig option overrides
==========================

Genconfig provides multiple ways of overriding config options when generating
an autodetect or barebones ``duk_config.h`` header:

* C compiler format::

      -DDUK_USE_TRACEBACK_DEPTH=100
      -DDUK_USE_JX
      -UDUK_USE_JC

* YAML config read from a file or given inline on the command line::

      --option-file my_config.yaml
      --option-yaml 'DUK_USE_FASTINT: true'

* A verbatim fixup header can declare custom prototypes and include custom
  headers, and can tweak ``DUK_USE_xxx`` options.  However, since Duktape 2.x
  some config options control automatic pruning of built-in objects and
  properties, and such options (like ``DUK_USE_BUFFEROBJECT_SUPPORT``)
  **MUST NOT** be modified by fixups.  It's thus recommended to modify options
  via the C compiler format or YAML.

These option formats can be mixed which allows you to specify an option
baseline (say ``--option-file low_memory.yaml``) and then apply
further overrides in various ways.  All forced options in C compiler
format and YAML format are processed first, with the last override
winning.

C compiler format
-----------------

The usual C compiler like format is supported because it's quite familiar.
In this example a low memory base configuration is read from a YAML config
file, and a few options are then tweaked using the C compiler format.  An
autodetect header is then generated::

    $ cd duktape
    $ python tools/genconfig.py \
        --metadata config/ \
        --option-file low_memory.yaml \
        -DDUK_USE_TRACEBACK_DEPTH=100 \
        -UDUK_USE_JX -UDUK_USE_JC \
        --output /tmp/duk_config.h \
        duk-config-header

YAML config
-----------

A YAML config file allows options to be specified in a structured,
programmatic manner.  An example YAML config file, ``my_config.yaml``
could contain::

    DUK_USE_OS_STRING: "\"hack-os\""  # force os name for Duktape.env
    DUK_USE_ALIGN_BY: 8  # force align-by-8
    DUK_USE_FASTINT: true
    DUK_UNREF:
      verbatim: "#define DUK_UNREF(x) do { (void) (x); } while (0)"

This file, another override file, and a few inline YAML forced options
could be used as follows to generate a barebones header::

    $ cd duktape
    $ python tools/genconfig.py \
        --metadata config/ \
        --platform linux \
        --compiler gcc \
        --architecture x64 \
        --option-file my_config.yaml \
        --option-file more_overrides.yaml \
        --option-yaml 'DUK_USE_JX: false' \
        --option-yaml 'DUK_USE_JC: false' \
        --output /tmp/duk_config.h \
        duk-config-header

For inline YAML, multiple forced options can be given either by using a YAML
value with multiple keys, or by using multiple options::

    # Multiple values for one option
    --option-yaml '{ DUK_USE_JX: false, DUK_USE_DEBUG: true }'

    # Multiple options
    --option-yaml 'DUK_USE_JX: false' \
    --option-yaml 'DUK_USE_DEBUG: true'

The YAML format for specifying options is simple: the top level value must be
an object whose keys are define names to override.  Values are as follows:

* ``false``: ``#undef`` option::

      # Produces: #undef DUK_USE_DEBUG
      DUK_USE_DEBUG: false

* ``true``: ``#define`` option::

      # Produces: #define DUK_USE_DEBUG
      DUK_USE_DEBUG: true

* number: decimal value for define::

      # Produces: #define DUK_USE_TRACEBACK_DEPTH 10
      DUK_USE_TRACEBACK_DEPTH: 10

      # Produces: #define DUK_USE_TRACEBACK_DEPTH 100000L
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

Fixup header
------------

In addition to YAML-based option overrides, genconfig has an option for
appending direct "fixup headers" to deal with situations which cannot be
handled with individual option overrides.  For example, you may want to
inject specific environment sanity checks.  This mechanism is similar to
Duktape 1.x ``duk_custom.h`` header.

Since Duktape 2.x some config options control automatic pruning of built-in
objects and properties, and such options (like ``DUK_USE_BUFFEROBJECT_SUPPORT``)
**MUST NOT** be modified by fixups.  It's thus recommended to modify options
via the C compiler format or YAML metadata files.

Fixup headers are emitted after all individual option overrides (in either
C compiler or YAML format) have been resolved, but before emitting option
sanity checks (if enabled).

For example, to generate a barebones header with two fixup headers::

    $ python tools/genconfig.py \
        --metadata config/ \
        --platform linux \
        --compiler gcc \
        --architecture x64 \
        --fixup-file my_env_strings.h \
        --fixup-file my_no_json_fastpath.h \
        --output /tmp/duk_config.h \
        duk-config-header

The ``my_env_strings.h`` fixup header could be::

    /* Force OS string. */
    #undef DUK_USE_OS_STRING
    #if !defined(__WIN32__)
    #error this header is Windows only
    #endif
    #define DUK_USE_OS_STRING "windows"

    /* Force arch string. */
    #undef DUK_USE_ARCH_STRING
    #if !defined(__amd64__)
    #error this header is x64 only
    #endif
    #define DUK_USE_ARCH_STRING "x64"

    /* Force compiler string. */
    #undef DUK_USE_COMPILER_STRING
    #if !defined(__GNUC__)
    #error this header is gcc only
    #endif
    #if defined(__cplusplus__)
    #define DUK_USE_COMPILER_STRING "g++"
    #else
    #define DUK_USE_COMPILER_STRING "gcc"
    #endif

The example fixup header uses dynamic detection and other environment checks
which cannot be easily expressed using individual option overrides.

The ``my_no_json_fastpath.h`` fixup header could be::

    /* Disable JSON fastpath for reduced footprint. */
    #undef DUK_USE_JSON_STRINGIFY_FASTPATH

This could have also been expressed using a simple override, e.g. as
``-UDUK_USE_JSON_STRINGIFY_FASTPATH``.

Fixup headers are appended verbatim so they must be valid C header files,
contain appropriate newlines, and must ``#undef`` any defines before
redefining them if necessary.  Fixup headers can only be used to tweak C
preprocessor defines, they naturally cannot un-include headers or un-typedef
types.

There's also a command line option to append a single fixup line for
convenience::

    # Append two lines to forcibly enable fastints
    --fixup-line '#undef DUK_USE_FASTINT' \
    --fixup-line '#define DUK_USE_FASTINT'

These can be mixed with ``--fixup-file`` options and are processed
in sequence.

Modifying a duk_config.h manually or using scripting
====================================================

The basic approach when using scripted modifications is to take a base header
(either an autodetect or barebones header) and then make specific changes
using a script.  The advantage of doing so is that if the base header is
updated, the script may often still be valid without any manual changes.

Scripting provides much more flexibility than tweaking individual options in
genconfig, but the cost is more complicated maintenance over time.

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

Using sed (or awk, etc) to modify an option in-place
----------------------------------------------------

If an option is defined on a single line in the base header, e.. either as::

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

For more stateful changes you can use ``awk`` or other scripting languages
(Python, Perl, etc).

Modifying defines at __OVERRIDE_DEFINES__
-----------------------------------------

Instead of modifying options in-place as in the sed example above, you can
simply append additional preprocessor directives to undefine/redefine options
as necessary.  This is much easier to maintain in version updates than when
modifications are made in-place.

Genconfig has a direct option to append "fixups" after the main generated
header::

    # my_custom.h is applied after generated header; functionally similar
    # to Duktape 1.2.x duk_custom.h

    $ python tools/genconfig.py [...] --fixup-file my_custom.h [...]

A genconfig-generated barebones header also has the following line near the end
for detecting where to add override defines; this is easy to detect reliably::

    /* __OVERRIDE_DEFINES__ */

The ``__OVERRIDE_DEFINES__`` line is near the end of the file, before any
automatically generated option sanity checks (which are optional) so that the
sanity checks will be applied after your tweaks have been done::

    #!/bin/bash

    CONFIG_IN=duk_config.h.base
    CONFIG_OUT=duk_config.h.new

    cat $CONFIG_IN | sed -e '
    /^\/\* __OVERRIDE_DEFINES__ \*\/$/ {
        r my_overrides.h
        d
    }' > $CONFIG_OUT

Modifying defines near the end of the file is relatively easy but has a few
limitations:

* You can't change typedefs this way because there's no way to un-typedef.

* You can't undo any ``#include`` directives executed.

Modifying defines at the end of the file
----------------------------------------

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

Dealing with #include files
---------------------------

Include files are often a portability problem on exotic targets:

* System headers may be missing.  You may need to provide replacement functions
  for even very basic features like string formatting functions.

* System headers may be present but in non-standard include paths.  Duktape
  can't easily autodetect such paths because there's no "#include if available"
  directive: an ``#include`` either succeeds or causes compilation to fail.

* System headers may be present but broken in some fashion so you want to avoid
  them entirely.

* Sometimes custom programming environments have "SDK headers" that conflict
  with standard headers so that you can't include them both at the same time.
  It may be necessary to include the SDK headers but provide manual declarations
  for the system functions needed.

In such cases you may need to replace all the ``#include`` statements of a
base header file and provide alternate include files or manual declarations.

Keeping a manually created duk_config.h up-to-date
--------------------------------------------------

When new Duktape versions are released, the set of config options and
other macros required of the ``duk_config.h`` config header may change.
This is the case for even minor version updates, though incompatible
changes are of course avoided when possible.

Nevertheless, when a new version is taken into use, you may need to
update your config header to match.  How to do that depends on how you
created the config header:

* If you're using the default header, no changes should be necessary.
  You should check out new config options and decide if the defaults are
  OK for them.

* If you're using a script to modify the default or genconfig-generated
  header, you should ensure your script works when the source header is
  updated to the new Duktape release.

* If you're editing a config header manually, you should look at the
  diff between the previous and new default config header to see what
  defines have changed, and then implement matching changes in your
  updated header.

Adding a new compiler, platform, or architecture
================================================

Adding a new platform "Acme OS"
-------------------------------

* Add a new detection snippet ``config/helper-snippets/DUK_F_ACMEOS.h.in``.

* Create a new ``config/platforms/platform_acmeos.h.in``.  Platform files
  should have the necessary ``#include`` statements, select the Date provider,
  and can override various broken platform calls.  For example, if ``realloc()``
  doesn't handle NULL and/or zero size correctly, you can override that.
  Compare to existing platform files for reference.

* Add the platform to ``config/platforms.yaml``, reference ``DUK_F_ACMEOS``
  for detection.

That should be enough for an autogenerated ``duk_config.h`` to support Acme OS
detection.

Adding a compiler or an architecture
------------------------------------

The process is similar for compilers and architectures; see existing files
for reference.

Notes
-----

Byte order
::::::::::

Byte order is a awkward to detect automatically:

* Sometimes byte order is best determined based on architecture, especially
  for architectures with a fixed byte order.  Some architectures can support
  multiple endianness modes, however, and it depends on the platform which
  one is used.

* Sometimes byte order is best determined from compiler defines; for example
  GCC and Clang provide built-in defines which mostly provide the necessary
  endianness information without the need to use system headers.

* Sometimes byte order is best determined from platform ``#include`` headers.
  There's a lot of variability in what defines are available, and where the
  related headers are located.

To allow endianness to be determined in each phase, platform, architecture,
and compiler files should only define endianness when not already defined::

    #if !defined(DUK_USE_BYTE_ORDER)
    #define DUK_USE_BYTE_ORDER 1
    #endif

Alignment
:::::::::

Alignment is similar to byte order for detection: it can be sometimes
detected from architecture, sometimes from platform, etc.  There are
architectures where alignment requirements are configurable, e.g. on X86
it's up to the operating system to decide if AC (Alignment Check) is enabled
for application code.

As a result, platform, architecture, and compiler files should avoid
redefinition::

    #if !defined(DUK_USE_ALIGN_BY)
    #define DUK_USE_ALIGN_BY 4
    #endif

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

Duktape config options are available in a machine parseable metadata form:

* ``config/config-options/DUK_USE_*.yaml``: Duktape config options.
