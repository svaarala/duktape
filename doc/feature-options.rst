=======================
Duktape feature options
=======================

Overview
========

The effective set of Duktape features is resolved in three steps in Duktape 1.x:

* User defines ``DUK_OPT_xxx`` feature options.  These are essentially
  requests to enable/disable some feature.  (These will be removed in
  Duktape 2.x and ``DUK_USE_xxx`` flags will be used directly.)

* Duktape feature resolution in the default "auto-detecting" ``duk_config.h``
  (previously internal ``duk_features.h.in``) takes into account the
  requested features, the platform, the compiler, the operating system
  etc, and defines ``DUK_USE_xxx`` internal use flags.  Other parts of
  Duktape only listen to these "use flags", so that feature resolution is
  strictly contained.

* The final ``DUK_USE_xxx`` flags can be tweaked in several ways:

  - The generated ``duk_config.h`` header can be edited directly (manually,
    through scripting, etc).

  - The ``genconfig`` utility can be used to generate a ``duk_config.h``
    header with user-supplied option overrides given either as YAML config
    file(s) or C header snippets included in the config header.

  - User may optionally have a ``duk_custom.h`` header which can tweak the
    defines (see ``DUK_OPT_HAVE_CUSTOM_H`` for more discussion; this feature
    option will be removed in Duktape 2.x.)

Starting from Duktape 1.3 an external ``duk_config.h`` is required; it may
be a prebuilt multi-platform header or a user-modified one.  Duktape 2.x
will remove support for ``DUK_OPT_xxx`` feature options entirely so that
all config changes are made by editing ``duk_config.h`` or by regenerating
it e.g. using genconfig.

Feature option naming
=====================

Feature options that enable a certain (default) feature are named::

  DUK_OPT_MY_FEATURE

Feature options that disable a (default) feature are named::

  DUK_OPT_NO_MY_FEATURE

Both flags are reserved at the same time.  One of the options will match
the default behavior, so it won't actually be implemented.

Some feature options have a value associated with them.  This is the case
for e.g. ``DUK_OPT_PANIC_HANDLER`` or ``DUK_OPT_FORCE_ALIGN``.  These are
handled case by case.

Avoid using words like "disable" in the feature naming.  This will lead to
odd names if the default behavior changes and a "no disable" flag is needed.

Feature option documentation
============================

Feature documentation is automatically generated from config/feature option
metadata:

* http://wiki.duktape.org/Configuring.html

* http://wiki.duktape.org/ConfigOptions.html

* http://wiki.duktape.org/FeatureOptions.html
