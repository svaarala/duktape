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
