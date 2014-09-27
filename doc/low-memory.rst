=======================
Low memory environments
=======================

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

* If you don't need the Duktape-specific additional JX/JC formats, use both
  ``DUK_OPT_NO_JX`` and ``DUK_OPT_NO_JC``.

* Features borrowed from Ecmascript E6 can usually be disabled:

  - ``DUK_OPT_NO_ES6_OBJECT_SETPROTOTYPEOF``

  - ``DUK_OPT_NO_ES6_OBJECT_PROTO_PROPERTY``

  - ``DUK_OPT_NO_ES6_PROXY``

* If you don't need regexp support, use ``DUK_OPT_NO_REGEXP_SUPPORT``.

* Duktape debug code uses a large, static temporary buffer for formatting
  debuglog lines.  Use e.g. ``-DDUK_OPT_DEBUG_BUFSIZE=2048`` to reduce
  this overhead.
