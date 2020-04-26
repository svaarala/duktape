==================================
Performance sensitive environments
==================================

Overview
========

This document describes suggested feature options for optimizing Duktape
performance for performance sensitive environments.

The following genconfig option file template enables most performance
related options: ``config/examples/performance_sensitive.yaml``.

Compiler optimization level
===========================

Size optimization using ``-Os`` is a good default when performance is
not critical.  However, it's not ideal when performance matters for
several reasons:

* Although ``-Os`` optimized code performs reasonably well, even
  ``-O2`` will yield significantly better results.

* Code performance with ``-Os`` can vary a great deal even when source
  code changes are innocent.  It's not uncommon for some performance
  test result to change +/- 10-30% with unrelated changes.  Presumably
  this is caused by changes in code alignment etc.

  Because of this, ``-Os`` is definitely a bad idea for measuring
  performance.

* Overall suggestion is to use ``-O2`` and try ``-O3`` if the end result
  is better.  Note that ``-O3`` is not always better because the code is
  larger and may not fit in caches as well as with ``-O2``.

Profile guided optimization (PGO)
=================================

Duktape source files contain some performance attributes like forced inline
forced noinline, and hot/cold attributes.

A better alternative is to use profile guided optimization (PGO) which is
highly recommended for performance sensitive environments.  For example,
GCC -O2 with PGO can be around 20% faster than GCC -O2 without PGO.

See for example the following:

* http://stackoverflow.com/questions/13881292/gcc-profile-guided-optimization-pgo

* https://msdn.microsoft.com/en-us/library/e7k32f4k.aspx

With GCC PGO is relatively simple:

* Use ``-fprofile-generate`` to compile Duktape and your application.

* Execute the result with representative (this is important) source files.

* Use ``-fprofile-use`` to recompile Duktape and your application.

Suggested feature options
=========================

* On some platforms ``setjmp/longjmp`` store the signal mask and may be
  much slower than alternative like ``_setjmp/_longjmp`` or
  ``sigsetjmp/siglongjmp``:

  - Check the current provider from ``duk_config.h`` or ``config/platforms/``
    header snippets.

  - Edit ``DUK_SETJMP``, ``DUK_LONGJMP``, and ``DUK_JMPBUF_TYPE`` to change
    the setjmp provider.

  - On some platforms (e.g. macOS/iOS) Duktape will automatically use
    ``_setjmp()``.

* Consider enabling "fastints":

  - ``#define DUK_USE_FASTINT``

  Fastints are often useful on platforms with soft floats, but they can also
  speed up execution on some hard float platforms (even on x64).  The benefit
  (or penalty) depends on the kind of ECMAScript code executed, e.g. code
  heavy on integer loops benefits.

* Enable specific fast paths:

  - ``#define DUK_USE_JSON_STRINGIFY_FASTPATH``

  - ``#define DUK_USE_JSON_QUOTESTRING_FASTPATH``

  - ``#define DUK_USE_JSON_DECSTRING_FASTPATH``

  - ``#define DUK_USE_JSON_DECNUMBER_FASTPATH``

  - ``#define DUK_USE_JSON_EATWHITE_FASTPATH``

* If you don't need debugging support or execution timeout support, ensure
  the following are **not enabled**:

  - ``#define DUK_USE_INTERRUPT_COUNTER``

  - ``#define DUK_USE_DEBUGGER_SUPPORT``

  Especially interrupt counter option will have a measurable performance
  impact because it includes code executed for every bytecode instruction
  dispatch.

* Disable safety check for value stack resizing so that if calling code
  fails to ``duk_check_stack()`` value stack, the result is memory unsafe
  behavior rather than an explicit error, but stack operations are faster:

  - ``#undef DUK_USE_VALSTACK_UNSAFE``
