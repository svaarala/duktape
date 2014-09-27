=============================
Timing sensitive environments
=============================

Suggested feature options
=========================

* Use the default memory management settings (reference counting and
  mark-and-sweep) but enable ``DUK_OPT_NO_VOLUNTARY_GC`` to eliminate
  mark-and-sweep pauses.  Use explicit GC calls (either ``duk_gc()``
  from C or ``Duktape.gc()`` from Ecmascript) when possible to collect
  circular references.
