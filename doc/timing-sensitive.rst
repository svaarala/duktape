=============================
Timing sensitive environments
=============================

Overview
========

Timing sensitive environments include e.g. games.  In these environments
long blocking times are problematic.  Stop-and-go garbage collection is
also a potential issue.

This document describes suggested feature options for reducing Duktape
latency in timing sensitive environments.

The following genconfig option file template enables most timing
sensitivity related options: ``config/examples/timing_sensitive.yaml``.

Suggested feature options
=========================

* Use the default memory management settings (reference counting and
  mark-and-sweep) but enable ``DUK_OPT_NO_VOLUNTARY_GC`` to eliminate
  mark-and-sweep pauses.  Use explicit GC calls (either ``duk_gc()``
  from C or ``Duktape.gc()`` from Ecmascript) when possible to collect
  circular references.
