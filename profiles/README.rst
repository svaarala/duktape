==============================
Duktape configuration profiles
==============================

This directory provides a few configuration profiles which are useful starting
points for coming up with an application configuration.  You can, for example,
use the low memory base configuration and then enable JSON fast paths if those
are important for your application.

The profiles are expressed as concrete shell commands in the ``Makefile``:

* The ``configure.py`` tool is used to create a ``duk_config.h`` and prepared
  sources.  YAML metadata files are used to provide configuration parameters
  as well as possible modifications to built-in objects.

* GCC is used to compile the result.  The ``-m32`` option is used because
  many embedded targets are 32 bits so the results will be similar to those.
  GCC link time optimization features are used to remove unused functions.

* The test program is a command line eval utility which evaluates arguments
  and prints out the results after string coercing them.  This example
  doesn't involve the full Duktape API but a significant portion of it.
  This has some footprint impact because not all of the API functions are
  pulled in (which is also the case for many actual applications).

The Makefile is Linux only, but the commands are easily lifted and adapted
to work in an arbitrary application build.  Python 2.x is required to run
the Duktape tools.
