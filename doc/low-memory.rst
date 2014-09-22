=======================
Low memory environments
=======================

One important portability target are low memory environments.  The default
Duktape options are quite memory conservative, and significant Ecmascript
programs can be executed with, say, 1 megabyte of memory.  Currently realistic
memory targets are roughly:

* 256kB flash memory (code) and 256kB system RAM

  - Duktape compiled with default options is feasible

* 256kB flash memory (code) and 128kB system RAM

  - Duktape feature options are needed to reduce memory usage

  - A custom memory allocation with manually tuned pools may be required

  - Only very small programs can currently be executed

This document describes suggested feature options for reducing Duktape
memory usage for memory-constrained environments.

Suggested feature options
=========================

* Use the default memory management settings: although reference counting
  increases heap header size, it also reduces memory usage fluctuation
  which is often more important than absolute footprint.

* Reduce error handling footprint with one or more of:

  - ``DUK_OPT_NO_AUGMENT_ERRORS``

  - ``DUK_OPT_NO_TRACEBACKS``

  - ``DUK_OPT_NO_VERBOSE_ERRORS``

  - ``DUK_OPT_NO_PC2LINE``

* If you don't need the Duktape-specific additional JX/JC formats, use:

  - ``DUK_OPT_NO_JX``

  - ``DUK_OPT_NO_JC``

* Features borrowed from Ecmascript E6 can usually be disabled:

  - ``DUK_OPT_NO_ES6_OBJECT_SETPROTOTYPEOF``

  - ``DUK_OPT_NO_ES6_OBJECT_PROTO_PROPERTY``

  - ``DUK_OPT_NO_ES6_PROXY``

* If you don't need regexp support, use:

  - ``DUK_OPT_NO_REGEXP_SUPPORT``.

* Duktape debug code uses a large, static temporary buffer for formatting
  debuglog lines.  Use e.g. the following to reduce this overhead:

  - ``-DDUK_OPT_DEBUG_BUFSIZE=2048``

* For very low memory environments, consider using lightweight functions
  for your Duktape/C bindings and to force Duktape built-ins to be lightweight
  functions:

  - ``DUK_OPT_LIGHTFUNC_BUILTINS``
