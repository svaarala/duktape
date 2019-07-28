=========================
Duktape 2.4 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* Symbol built-in is now enabled by default.

* Add duk_push_bare_array() API call which pushes a bare Array, i.e. one
  that doesn't inherit from Array.prototype.

* Add duk_to_stacktrace() and duk_safe_to_stacktrace() to make it easier
  to get stacktraces in C code.

* Add duk_require_constructable() and duk_require_constructor_call().

* Various fixes and portability improvements.  Special thanks to Renata
  Hodovan for several issues found using Fuzzinator
  (https://github.com/renatahodovan/fuzzinator).

Upgrading from Duktape 2.3
==========================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v2.3.x.  Note the following:

* Symbol built-in (Symbol(xxx), Symbol.toPrimitive, etc) is now enabled
  by default.  If you don't want the built-in, disable
  ``DUK_USE_SYMBOL_BUILTIN`` in tools/configure.py.

* The ``DUK_USE_USER_DECLARE`` config option was removed.  If in use, replace
  with a ``configure.py`` fixup file/line.
