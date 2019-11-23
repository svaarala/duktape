=========================
Duktape 2.5 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* Move CBOR support from extras to Duktape internals.  Both the C API
  (duk_cbor_encode(), duk_cbor_decode()) and ECMAScript API (CBOR.encode(),
  CBOR.decode()) are enabled by default.

* Add duk_pull() API call.

* Add "globalThis" binding, enabled by default.  This replaces the previous
  "global" binding (which was disabled by default) to match the revised
  "global" specification.

* Various fixes and portability improvements.

Upgrading from Duktape 2.4
==========================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v2.4.x.  Note the following:

* CBOR built-in is enabled by default.  You can disable it by disabling the
  ``DUK_USE_CBOR_SUPPORT`` and ``DUK_USE_CBOR_BUILTIN`` config options.
  If you're using the CBOR extra, you should migrate to the built-in CBOR
  support.

* ``globalThis`` binding is now enabled by default; it can be disabled
  by disabling the ``DUK_USE_GLOBAL_BINDING`` config option.
