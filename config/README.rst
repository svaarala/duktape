=================
Duktape genconfig
=================

Overview
========

``genconfig`` is a helper script for coming up with a ``duk_config.h`` for
compiling Duktape for your platform.

To support this:

* It creates a Duktape 1.2.x compatible ``duk_config.h`` with automatic
  platform detection and ``DUK_OPT_xxx`` feature options.

* It helps to create a ``duk_config.h`` for your platform/compiler
  combination.  You can give a base configuration and then force certain
  values manually based on a YAML configuration file.

* It autogenerates documentation for config options (and Duktape 1.2.x
  feature options) based on option metadata files written in YAML.

Usage
=====

To create an autodetect duk_config.h header (compatible with Duktape 1.2.x)::

    $ python config/genconfig.py --metadata config --output /tmp/duk_config.h \
          autodetect-header

To create a barebones duk_config.h header for a specific platform (easier to
edit manually)::

    $ python config/genconfig.py --metadata config --output /tmp/duk_config.h \
          --platform linux --compiler gcc --architecture x64 \
          barebones-header

There are further commands to e.g. autogenerate config option documentation;
see ``genconfig.py`` for details.
