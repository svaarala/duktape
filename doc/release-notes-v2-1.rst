=========================
Duktape 2.1 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* Performance, footprint, and portability improvements.

* API additions for more convenient handling of optional arguments:
  duk_opt_xxx() and duk_get_xxx_default().

* Allow duk_push_heapptr() for objects which have become unreachable and
  are pending finalization.  In such a case a duk_push_heapptr() cancels
  the pending finalizer call and automatically rescues the object.

* ES2015 additions like String.prototype.{startsWith,endsWith,includes}()
  and HTML comment syntax.  Non-standard shebang ("#!...") comment support.

* Finalizer handling rework for reference counting and mark-and-sweep to fix
  a few "side effect" bugs.  Also improved torture test coverage for ensuring
  side effects are handled correctly in Duktape internals.

* DUK_VERSION is now visible to duk_config.h so it's possible for duk_config.h
  to support multiple Duktape versions.  For example, some config options may be
  disabled prior to a certain patch level.

Upgrading from Duktape 2.0
==========================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v2.0.x.  Note the following:

* The Duktape thread used for finalizer calls is now always the initial thread
  (heap_thread), for both reference counting and mark-and-sweep triggered
  finalization.  This should be taken into account in finalizer functions;
  in particular, if there are multiple global environments, finalizers will
  execute in the first global environment created for the heap.

  Prior to 2.1 the finalizer thread could also be heap_thread but usually the
  current thread would be used.
