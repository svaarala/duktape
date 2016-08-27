=============
Duktape tools
=============

This directory contains various Duktape Python tools which are included in
the end user distributable.

The main tool is ``configure.py`` which simultaneously creates a
``duk_config.h`` configuration file and prepares source files for
compilation.  These two operations are combined because:

1. Configuration options may affect source file preparation.

2. Some features involve metaprogramming.  For example, when using ROM
   built-ins YAML metadata is used for automatic code generation for the
   ROM object initializers.

There are also other miscellaneous tools, e.g. utilities to scan potential
fixed strings from source files, dump bytecode, and resolving file/line
from combined source.

Because this tooling is part of the end user distributable, the tooling must
be as portable as possible.  For example:

* Avoid Unix path separators.

* Avoid depending on Python executable name, use ``sys.executable`` instead
  to launch Python commands.

The tooling has been written for Python 2.x.  There's no support for
Python 3.x at present.
