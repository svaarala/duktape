==============================
Moving objects to code section
==============================

For low memory targets it's be very useful to be able to relocate built-in
strings, built-in objects, and even user strings/objects into the code
section to avoid RAM usage.

This document provides some design notes on the issues involved.

Misc notes
==========

* Built-ins in code section will be read only, but need to share the same
  structure as their RAM counterparts (for the most part) to avoid
  complicating internals too much.  Any attempt to write over the structs
  (flags, refcount, heap next/prev link pointers, etc) may be a segfault,
  depending on the platform.

* Because internal structures like ``duk_heaphdr``, ``duk_hstring``,
  and ``duk_hobject`` are normally writable, static ROM initializers
  (which are ``const``) must be eventually cast into non-const pointers.
  This causes harmless warnings and/or requires a cast workaround to
  silence the warnings.

* If objects are moved into the code section, their property tables will
  become fixed.  The objects will be implicitly non-extensible, so it'd
  make sense to mark them as non-extensible explicitly.

* Somewhat unintuitively the properties of non-writable ROM objects
  need to be "writable" from the ECMAScript attributes perspective to
  ensure it's possible to create objects inheriting from the ROM
  objects and establish overriding properties on the objects created.

* From a mark-and-sweep perspective the built-ins form an island: there
  can be references to the built-ins from outside, but the built-ins
  cannot reference values outside.  One implication of this is that when
  mark-and-sweep encounters a read-only built-in, it can stop marking
  and doesn't need to recurse further because no heap allocated objects
  can be reached via the built-ins.

* While most built-ins can be read-only, a lot of user code will expect
  to see a writable global object.  A memory conservative approach for
  this is to use an empty global object which inherits its (unchanged)
  properties from a ROM-based global object ancestor.

* Pointer compression of ROM pointers is non-trivial because ROM pointers
  are outside Duktape heap and because ROM pointers must also be compressed
  at compile time to create proper static initializers.

Object structure fields
=======================

duk_heaphdr
-----------

* Can't update flags.

  - Mark-and-sweep: cannot mark object reachable, temproot, etc.  Built-in
    strings/objects must not be marked "visited".

  - No finalizer support: cannot mark finalizable, finalized.  Also no need
    for finalization because ROM objects don't need to be freed.

* Can't update refcount.

  - Refcount macros: must check and avoid writing refcount; increases code
    footprint because affects every INCREF/DECREF.  If user code is compiled
    without refcounts this impact is avoided.

* Can't use prev/next pointers.

  - Objects cannot be queued to e.g. refzero or finalization lists.

  - No refcount handling, no refzero queuing, no finalization queueing.

  - Pointers are set to NULL.

duk_hstring
-----------

* String hash will be fixed at dist time.

  - Can't incorporate a dynamic seed into the string hash.  The string hash
    seed must be dropped or fixed at dist time.

  - Multiple hash variants must be precomputed.

* Strings can be added to the string table, but this would be
  counterproductive (and unnecessary).

  - Strings don't have a prev/next pointer at all, string table refers
    to them from outside.

  - But adding built-in strings to the string table would consume ~2-4
    bytes per string.

* If ``duk_hstring`` objects are in code section, their pointers will be
  outside of Duktape heap.

  - Current solution is for ROM pointers to be compressed specially,
    see discussion below.

* With Duktape 2.1 ``duk_hstring`` values have a ``h_next`` link pointer
  for a revised string table structure.

  - This link pointer is unused for ROM strings.  It can be used for string
    data if the structure that follows is empty (assumes no arridx, and no
    clen field).

duk_hobject
-----------

* Many built-in objects are actually native functions, so the relevant
  structure is often ``duk_hnatfunc``.

* Heap header will have more bits in use, but no effect otherwise.

* Property table pointer 'p' is pointer compressed and assumed to be in
  Duktape heap at the moment.

  - Solved using ROM pointer compression.

* Internal prototype pointer 'prototype' is pointer compressed and assumed
  to be in Duktape heap at the moment.

  - Solved using ROM pointer compression.

* Entry, array, and hash sizes are not an issue.

* If hash part is present (for large objects), need to duplicate hash probing
  in dist code.  For lowmem environments hash part is usually disabled, so
  this should not be necessary in practice.

duk_hobject properties allocation
---------------------------------

* There are three layouts: static initializers generated by ``genbuiltins.py``
  must duplicate each and choose active layout at compile time.

* Property key pointer is *not* compressed at the moment.

* Property value format depends on packing.  To avoid union initializers may
  need separate ``duk_tval_string``, ``duk_tval_boolean``, etc struct
  definitions.

  - A lot of variants will be needed for packed and unpacked ``duk_tval``,
    endianness variants for packed ``duk_tval``, etc.

  - Union initializers would be available assuming C99 which might be
    reasonable for ROM object support.

* Property values may contain heap pointers; these are not compressed so this
  should not be an issue even if pointers are for built-ins moved to code
  section (objects or strings).

* Property values may be in circular references, so built-in objects may
  need forward declarations before their actual definition.

* Properties can be accessors; ``duk_propvalue`` is a union.  Probably best
  to duplicate into ``duk_propvalue_dataprop`` and ``duk_propvalue_accessorprop``
  so that non-union initializers can be used (which is more portable).

duk_hnatfunc
------------

Same issues as ``duk_hobject`` plus the following:

* Function pointer should have no issues.

* Nargs and magic should have no issues.

duk_hcompfunc
-------------

Same issues as ``duk_hobject`` plus the following:

* Bytecode constants table may refer to ROM builtins, should not be a
  difficult issue.

* Bytecode needs to be precompiled, which is mainly a tooling issue.

User strings and objects
========================

The initializers for built-in strings and objects are rather arcane:

* To avoid union initializers a lot of concrete initializers where union(s)
  are replace with specific values are needed.  These seem impossible to
  automate cleanly with e.g. macros.

* At least strings and property tables have variable size initializers.
  Property tables further have multiple initializer variants depending on
  chosen object memory layout.

The best approach for supporting user strings and objects so far is to
include them in the dist (genbuiltins) processing:

* User build script runs the dist process, giving YAML metadata file(s)
  to provide information about user strings and/or objects.

* The user strings and objects are merged with the active built-in strings
  and objects and then processed normally: all layout variants are generated,
  arcane initializers are generated etc.

The downside of this that a fresh dist is required as part of the user build,
but this will probably become the normal process for low memory targets anyway
to support proper optional built-ins.

Practical issues
================

Avoiding writes on read-only objects
------------------------------------

* Refcount operations

* Setting/clearing heaphdr or object/string flags

* Reallocating object ``props`` allocation (e.g. compact)

* Inserting, deleting, and modifying properties

* Compact, seal, freeze

* ``setPrototypeOf``: from ECMAScript code this is prevented because
  the ROM built-ins are not extensible and setPrototypeOf() fails if
  the target is not extensible.

* ``duk_set_prototype``: from C code allowed for any object, must reject
  this for read-only objects explicitly.

Dynamic initializations
-----------------------

* ``Duktape.modLoaded`` is established using an initialization JS script.
  This won't work with a read-only Duktape; modLoaded has to be part of
  init data.

* ``Duktape.errCreate`` and ``Duktape.errThrow`` must be established when
  creating the ROM built-ins as they cannot be set at runtime.

* ``Date.prototype.toGMTString`` must be the *same object reference* as
  ``Date.prototype.toUTCString``.  For RAM built-ins this was handled by
  omitting ``toGMTString`` from the init data and inserting it during
  init.  For ROM built-ins both have to be present from the beginning,
  but must point to the same object.

Compile time pointer compression
--------------------------------

If ROM strings/objects are enabled and pointer compression is used, Duktape
must be able to compress and decompress ROM pointers pointing to strings and
objects using the user-supplied compression macros.  This poses a few issues:

* ROM pointers are outside the Duktape heap which must be accounted for in
  the compression/decompression macros.  A simple approach is to dedicate a
  certain compressed pointer value range for ROM pointers.

* Compressed ROM pointers are needed to express the static ROM initializers
  so ROM pointers must compress to values *known at compile time*.  There
  are at least two basic approaches to this:

  - Require user code to provide a pointer compression macro which supports
    ROM pointers and can be computed at compile time (which is critical so
    that the string/object initializer can go into the read only section).
    This can get very messy.

  - Collect a list of ROM pointers in need of compression during genbuiltins
    (the list itself also going to ROM).  Expose this list to the user provided
    compression macros which can then determine if a certain pointer is a ROM
    pointer, and use the list index to compute a compressed pointer.  For
    example, if the range ``[0xf800,0xffff]`` is dedicated to compressed ROM
    pointers, the first ROM pointer in the list would compress to 0xf800 + 0 =
    0xf800, the second to 0xf800 + 1 = 0xf801, etc.  Pointer decompression
    would similarly recognize that range and use the ROM pointer list to
    decompress the pointer.  Finally, genbuiltins can also compress pointers
    to that range, assuming it knows the base value (0xf800 here).

For now the approach is based on that ROM pointer table; the integration with
user code is not (yet) very clean, see:

* ``examples/cmdline/duk_cmdline_lowmem.c``
