=============
Duktape tools
=============

This directory contains various Duktape Python tools which are included in
the end user distributable.

The main use case is config-and-prepare which simultaneously creates
a ``duk_config.h`` configuration file and prepares source files for
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

The tooling has been written for Python 2.x and there's no support for
Python 3.x at present.

TODO
====

- Go through all .py files once more and figure out what to move into tools

- Go through all moves, and git grep for "call sites"

- Add dist/config/genconfig.py copy for convenience?

STEPS
=====

- Move all scripts that should be in the end user distributable into 'tools/'.

- Change all references for the scripts for their new location so that existing
  dist and build works.  No other changes at this point.

- Split make_dist.py into two parts:

  1. Revised dist part which first copies files into the end user distributable
     and then runs the prepare step *in the distributable* for the default
     configuration.

  2. Separate config-and-prepare utility which does most of the current dist
     processing.  This will take the --rom-auto-lightfunc and other stuff now.
     It also interfaces with genconfig.

- Update documentation: new locations (e.g. in genconfig docs, READMEs etc),
  new config process, ROM built-in stuff, etc.

- At this point the initial rework is complete.
