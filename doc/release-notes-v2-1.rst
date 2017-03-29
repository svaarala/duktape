=========================
Duktape 2.1 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* TBD.

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

Known issues
============

TBD.

Raw issues from test runs
=========================

API tests
---------

TBD.

Ecmascript tests
----------------

TBD.

test262
-------

TBD.
