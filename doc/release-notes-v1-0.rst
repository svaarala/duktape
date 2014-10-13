=========================
Duktape 1.0 release notes
=========================

Release overview
================

First release with a stable API.  This release will get bug fixes at least
until 1.1.0 is released (and probably some time beyond that).

Known issues
============

This release has the following known issues worth noting.

Ecmascript features
-------------------

* Non-compliant behavior for array indices near 2G or 4G elements.

* RegExp parser is strict and won't accept some real world RegExps which
  are technically not compliant with Ecmascript E5/E5.1 specification.

* Final mantissa bit rounding issues in the internal number-to-string
  conversion.

Portability and platforms
-------------------------

* Clang 3.3 on FreeBSD 10: when ``-m32`` is used, Duktape will end up using
  the 8-byte packed value representation but clang will generate incorrect
  code for union assignment (see ``misc/clang_aliasing.c``).  The issue can
  be detected by defining ``DUK_OPT_SELF_TESTS``.  A workaround is to avoid
  packed types in this case by defining ``DUK_OPT_NO_PACKED_TVAL``.

* Clang 3.3 on FreeBSD 10: compilation may produce a warning "generic
  selections are a C11-specific feature".  The warning should be harmless.

* On some GCC versions and compilation options you may get a warning
  "variable idx_func might be clobbered by longjmp or vfork [-Wclobbered]".
  This warning seems spurious and causes no known problems.

* GCC ``-O4`` may produce a warning "assuming signed overflow does not occur
  when assuming that (X - c) > X is always false" for some assertions.  This
  warning seems spurious and causes no known problems.

* GCC ``-pedantic`` without -std=c99 causes the ``unsigned long long`` type
  to be used by Duktape, and an associated warning about the type.  This is
  harmless and most easily fixed by simply using ``-std=c99``.
