==================================
Performance sensitive environments
==================================

Overview
========

This document describes suggested feature options for optimizing Duktape
performance for performance sensitive environments.

Suggested feature options
=========================

* On some platforms ``setjmp/longjmp`` store the signal mask and may be
  much slower than alternative like ``_setjmp/_longjmp`` or
  ``sigsetjmp/siglongjmp``.  Use the long control transfer options to use
  an alternative:

  - ``DUK_OPT_UNDERSCORE_SETJMP``

  - ``DUK_OPT_SIGSETJMP``

  - On some platforms (e.g. OSX/iPhone) Duktape will automatically use
    a faster alternative.
