=========================
Duktape 2.2 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* TBD.

Upgrading from Duktape 2.0
==========================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v2.1.x.  Note the following:

* Call stack (including both activation records and catcher records) is no
  longer a resized monolithic allocation which improves memory behavior for
  very low memory targets.  If you're using a pool allocator, you may need to
  measure and adjust pool sizes/counts.

* Functions pushed using duk_push_c_function() and duk_push_c_lightfunc() now
  inherit from an intermediate prototype (func -> %NativeFunctionPrototype%
  -> Function.prototype) which provides ``.name`` and ``.length`` getters.
  The setters are intentionally missing so direct writes for these properties
  fail, but you can write them using ``Object.defineProperty()`` or
  ``duk_def_prop()``.  The inherited getters can also be replaced if necessary.
  The intermediate prototype doesn't have a named global binding, but you can
  access it by reading the prototype of a pushed function.
