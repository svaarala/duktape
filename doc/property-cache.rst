==============
Property cache
==============

This document describes property caching, a mechanism added in Duktape 2.1.0
to speed up property reads (and maybe writes in the future).

Data structure
==============

The property cache is an array of entries to speed up GETPROP operations.
Each entry contains:

* Object and key reference for original lookup.  The object reference stored
  is for the lookup starting point, which is often different than the object
  actually holding the property (inheritance).

* Property value.

* Generation count to allow quick invalidation.

All key/value references are borrowed.  Only concrete properties (not getters
or virtual property values) can be cached due to the property value not being
reference counted.  In other words, when a property is cached:

* There must be a guarantee that the object, key, and value are reachable by
  some means.

* A GETPROP operation from the same starting point using the same key would
  yield the same value.

The ``duk_heap`` structure holds the property cache which is just an array of
entries of (preferably) 2^N size.  The heap structure also holds the current
generation count which starts from 1; entries with generation count 0 are thus
ignored automatically.

Read handling
=============

* When GETPROP is called, compute a property cache lookup key by combining
  object pointer (for lack of an object hash) with the key string hash.

* Check the entry at the hash index.  If the generation count doesn't match
  current generation count (which is a heap-wide value), the entry must be
  ignored as obsolete.  Otherwise check if the object and the key match, and
  if so, return the cached value.

* If there's a cache miss, proceed with normal property lookup.  There's no
  probing sequence.

* If no property is found, no property cache changes are made.  It would be
  possible to implement negative property caching.

  - Negative property caching would improve e.g. JSON stringify() performance
    because negative .toJSON() lookups could be cached.

* If a property is found, and there are no conditions to prevent caching,
  overwrite the cache entry at prophash: set generation count to current,
  set object and key reference (object reference must be for the lookup
  starting point -- not the final object), and property value.

* There is currently no hash probing/chaining: the existing entry is simply
  overwritten with the hope that collision are relatively rare in practice.

* Conditions preventing caching:

  - Value came from a getter: side effects, value may change per lookup.

  - Value came from a Proxy: side effects, value may change per lookup.

  - Value is virtual and may change per lookup.  Virtual values that won't
    change over time can be read cached however.

  - Array index lookups probably shouldn't be cached because the same index
    is not usually read many times for caching to be useful.  These entries
    would also purge useful entries unnecessarily.

Invalidation
============

Invalidation must be triggered by any operation that might compromise the
cache integrity requirements, i.e.:

* The operation might cause object, key, or value (all borrowed) to become
  unreachable, thus leading to memory unsafe behavior.

* The operation might change the property value if a GETPROP were done.
  For example, looking up a property storage location is by itself not an
  issue, but overwriting the value there is.

The basic challenge with partial invalidation where only actually affected
entries are removed is that: (1) knowing what entries to remove requires
complicated tracking state, and (2) actually scrubbing affected entries
may require multiple lookups if a change in an inherited property affects
multiple lookup cache entries.

The current solution is thus to invalidate the entire cache very cheaply by
using a generation count.  Entries whose generation count don't match current
are entirely ignored, so that simply bumping the current generation count is
enough to invalidate all entries without touching the entries themselves.

Technically, if the generation count wraps, entries should be scrubbed
fully.  This can be achieved by detecting that the generation count became
zero, setting the whole cache to zero, and then bumping the generation count
once more to ignore the zeroed entries.

Operations that require invalidation include:

* PUTPROP

* DELPROP

* defineProperty() and all its variants

* Changes to object internal prototypes: duk_set_prototype(),
  Object.setPrototypeOf(), etc.  Internal direct prototype changes when there's
  any possibility a property access might have happened.

* Any code that looks up an existing property and modifies its storage location
  directly.  Easiest approach, at least initially, is to invalide the cache on
  that storage lookup.

* Any code modifying structures that affect non-array-index virtual properties.
  For example, writing to ``duk_harray.length`` which appears as a virtual
  ``.length`` property.  (Needed if virtual ``.length`` is cached; generally
  virtual properties cannot be cached because heap-allocated borrowed values
  can't be used for dynamic values.)

* Mark-and-sweep might do compaction which affects property storage locations.
  But currently only the value, not its storage location, is cached so that
  compaction by itself is not an issue.

* Mark-and-sweep may finalize objects and then free them however.

* Assuming that the property keys are reachable through the object, there
  should be no chance that the cached key would be freed without the object
  being freed.

* An object may be freed however without any property related operation
  taking place.  There are only a few places where the object is actually
  freed, and those locations can invalidate the property cache.

Future work
===========

* Caching "own property" lookups would speed up some cases where the same
  inheritance path is used by multiple lookups.  Intuitively this would seem
  to be of less practical use.

* Caching property misses would be straightforward; use DUK_TVAL_SET_UNUSED()
  to flag the value as missing.  However, negative caching seems of little
  practical use because most repeated property lookups are for property hits.

* Caching property writes would be possible by caching the storage location
  rather than the value only.  Property attributes can be cached to allow
  writes to be validated.  Alternatively, if only writable properties are
  cached, writability can be assumed if the property is found in the cache.

* Caching array reads/writes would be possible to some extent, but would
  require a slightly different approach to avoid polluting the property
  cache with array indices.  The caching approach could speed up locating
  the ultimate array/array-like object for the index read/write to avoid
  walking through the inheritance chain.  However, subclassed Arrays or
  typed arrays are relatively rare (though Node.js Buffer instances inherit
  from Uint8Array).

* Caching variable reads using the initial property read caching (value only)
  approach doesn't work because every executor register write would need to
  invalidate the read cache.  However, if the cache contained a storage
  reference, variable location caching would be possible and could be applied
  to slow path read/write references.  Inherited properties (scope chain)
  could also be cached, provided that there are no Proxy object bindings.
  Value stack resize and call handling would need to invalidate the variable
  lookup cache because the storage locations might change.

* Hash probing for the cache locations might help some cases where property
  lookup hashes collide.  However, it adds another step to the property
  lookup, and more importantly causes another cache line to be fetched.
  So it may be of relatively little practical value, at least when the
  property cache can just be made larger instead.

* Making ``duk_propcache_entry`` size 2^N would speed up the cache lookup
  and align the lookups better.

* Caching own properties instead of GETPROP/PUTPROP would reduce the benefit
  of caching for inheritance chains, but would make caching simple otherwise.
  For example, a property creation for one object wouldn't invalidate cached
  properties of another because inheritance chains don't matter for caching.
