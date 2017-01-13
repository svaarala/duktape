============
String table
============

Overview
========

Duktape 1.x to 2.0
------------------

The ``duk_hstring`` struct doesn't contain heap link (next/prev) pointers and
strings are assumed to be tracked within the string table.  Tracking is
necessary at all times so that strings can be properly freed by mark-and-sweep
heap destruction, etc.

Default string table, uses probe sequence
-----------------------------------------

::

	heap->strtable
	 |
	 `-->  duk_hstring *     Entries are duk_hstring pointers, with a special
               duk_hstring *     pointer value for DELETED entries.
               ...
               duk_hstring *

String table size is a prime, probe sequence uses modulus.  The string table
is resized based on a load factor check.

Alternative string table for low memory, uses chaining
------------------------------------------------------

::

	heap->strtable
	 |
	 `-->  duk_strtab_entry --> listlen=0/n, union { str, strlist }
	       duk_strtab_entry
	       ...
	       duk_strtab_entry

Each string table entry is either:

* Unused (listlen == 0, str pointer == NULL)
* Single string (listlen == 0, str pointer != NULL)
* Array of strings in a separate allocation (listlen > 0, strlist pointer != NULL)

The string table size is a constant, and the string table is not resized.

Duktape 2.1
-----------

The two string table alternatives were replaced by a single algorithm in
Duktape 2.1.  The main approach is:

* Add one-way link pointers (h_next field) to the ``duk_hstring`` entries so
  that the strings can be efficiently chained without an external container.

* Use a string table of size ``2 ** N`` so that a bit mask can be used to
  find string position.

* In-place resize from ``2 ** N`` to ``2 ** (N + 1)`` and ``2 ** (N - 1)``.
  For example, to reduce size, rehash in place to half of current size, and
  then reallocate the memory chunk.  If the memory allocator can satisfy the
  realloc() in place, there's no need for a copy.

* The hash table size is changed based on load factor (number of strings
  compared to table size).  The size can be also be made constant for low
  memory environments where predictability is key; this imposes no string
  count limit because of chaining.

Duktape 2.1 details
===================

Resizing the top level strtable
-------------------------------

Load factor (average chain length) is easily computed by keeping track of
number of strings inserted into the table::

    load_factor = avg_len = num_strings / strtable_size

Average length is a cheap way of making resizing decisions.  Also maximum chain
length would be interesting for pathological cases -- however, resizing may not
always solve pathological issues if strings with matching hashes are created on
purpose.  To solve that case, the hash algorithm would need to be changed on
the fly to adapt to the input (difficult because existing strings need
rehashing too).

When using chaining there's no mandatory need to do a shrink/grow at any
specific time because the string table can hold an arbitrary number of strings
at some performance cost.  There are also no issues like accumulation of
DELETED entries in the hash probe approach.  Because of this, it's OK to do a
resize check only from time to time, for example::

    if ((num_strings & 0xff) == 0) {
        /* Resize check when string count is a multiple of 256. */
    }

The grow/shrink check can also be limited to string insertion; while removals
could technically lead to the need to shrink the allocation, it can almost
always be delayed to later insertion processing.

The growth step is limited to doubling the current size of the string table.
Except for very small string table sizes this is sufficient because when the
need to resize is detected, only a limited number of string table inserts have
been processed.  So in practice there is no need to do multiple growth steps
at once.

The shrink step is also limited to halving the current size.  Because a lot of
strings may be removed without any inserts in between (consider a large array
of strings becoming unreachable), the string table load factor may remain very
low (way below the shrink limit) until some inserts are done to detect the
situation; and even after that it may take multiple resize checks to shrink the
string table enough to get within the desired load factor range.  However, this
kind of very fast shrinks are not common in practice, and the current
implementation will just do one halving per resize check.  Emergency GC also
does a resize check so each emergency GC round can halve the string table size
if that's useful to satisfy the failing allocation.

The memory cost per string of the top level strtable is::

    sizeof(duk_hstring *) * strtable_size / num_strings

For example:

* For a load factor 1.0 the cost per string is exactly the pointer size.

* For a load factor 2.0 the cost per string is half the pointer size.

Config options provide minimum and maximum sizes, and grow/shrink load
factor limits expressed as fixed point integers.  When minimum and maximum
size are equal, no resize checks are done; this is useful for very low memory
targets where a fixed size string table is often preferred.

Resizing in place
-----------------

When the top level strtable is resized, it's useful to resize it in place so
that the structure can be ``realloc()``'d.  In some cases that will avoid
making an unnecessary copy.

For growing in place:

* Reallocate the structure to twice previous size first.

* Because the hash mask size grows by one bit, the new highest mask bit
  maps one existing bucket into two separate hash buckets.  Strings need
  to be rehashed into their proper bucket.

For shrinking in place:

* Two hash buckets, again distinguished by their highest hash mask bit,
  will now be merged into a single bucket.  There's no need to do any
  has checks, the two buckets are just combined into one.

* When complete, reallocate the structure.  This is a shrink ``realloc()``
  which we assume will **always** succeed.  (Handling a realloc() failure
  would also be straightforward: just run the "grow" algorithm to restore
  strings into their original buckets.)

Because ``realloc()`` may have side effects, the following precautions are
needed:

* Recursive strtable resizes must be prevented.  Because the hash chains don't
  have fixed capacity limits, this never leads to a dead end.

* When ``realloc()`` is called, the strtable size, mask, etc must be valid so
  that any strings interned/freed by side effects can be handled normally
  without being aware of the resize.  For example, when shrinking, the buckets
  must first be combined, and the strtable size, mask, etc updated *before*
  the ``realloc()`` call is made.

Zero-way linking, single linking, and double linking
====================================================

Zero-way linking was used up to Duktape 2.0: ``duk_hstring`` itself doesn't
contain any link pointers.  For a hash+probe approach this is fine because
strings don't need any linking: they are in the hash table as is.  For a
hash+chain approach Duktape 2.0 used separate allocations for lists of string
pointers.

Single linking means a ``duk_hstring`` has only a "next" pointer; double
linking means a string also has a "prev" pointer which means more pointer
manipulation but makes some operations (e.g. random string unlinking) easier.

Upsides of double linking:

* Unlinking doesn't require scanning to find the previous element, so that
  both insert and remove are O(1).

Downsides of double linking:

* One more pointer per ``duk_hstring``, more memory usage.

* More pointer manipulation because both directions of the list need managing.

Upsides of single linking:

* Only a single link pointer, less memory usage.

* Less pointer manipulation.

Downsides of single linking:

* Unlinking by refcount requires scanning from the hash chain root, so that the
  previous string can be located and its 'next' pointer updated which makes
  removal slower.  However, it's only a significant issue if the average chain
  length is high, so resizing the top level strtable array should minimize the
  issue.  For low memory targets with a fixed top level strtable array this may
  be a more concrete issue; however, the number of strings there is also limited
  by memory.

In Duktape 2.1 a single linked ``duk_hstring`` is used.  On average, the cost
of scanning in removal is offset by less pointer manipulation in inserts.  This
holds at least when the load factor is small (say <= 2.0).

Delayed freeing
---------------

While refcounts can be used to free strings immediately, it would also be easy
to delay string freeing to the mark-and-sweep pass.  This would even allow the
refcount field to be dropped from strings (which would need adjustment to
refcount macros).

The upside of delaying string freeing is that if a string is repeatedly created
and then forgotten, the interned string which is otherwise unreachable can be
reused.

While not very common, this happens in some algorithms repeatedly; for example
when using a string as a "lookup table"::

    var nybbles = "0123456789abcdef";
    var res = '';  // inefficient concatenation for simplicity

    for (var i = 0; i < n; i++) {
        res += nybbles[inp[i] >> 4];
        res += nybbles[inp[i] & 0x0f];
    }

Here each lookup creates a one-character substring which is interned, appended
to ``res``, and then freed (unless a reference exists elsewhere).

For low memory targets delayed freeing would be nice because it reduces the
string header size by the refcount field.  The downside is that the refcount
macros need adjustment: not all heaphdr refcount operations are the same.
Memory usage would also be less snappy.

Other changes in Duktape 2.1
============================

External string handling
------------------------

External strings are always supported by string table code in Duktape 2.0 but
only created when some external string macros are enabled.  The ``duk_hstring``
accessor macros only support external strings when ``DUK_USE_HSTRING_EXTDATA``
is enabled.  Revise this behavior:

* Disable external string checks also in string intern code if external
  string support macros are not enabled.

For future consideration:

* External string support might be enabled in the public C API because it's
  quite useful for things like memory mapped source files.

* When not pressed for RAM, add an explicit string pointer to the standard
  header so that string data access doesn't need a flag check at every turn.

Future work
===========

Header definition
-----------------

In Duktape 2.0 ``duk_hstring`` header starts with a ``duk_heaphdr`` and is
then followed by further fields.  If ``duk_heaphdr`` is not naturally aligned
this introduces unnecessary padding inside the struct.

Change so that shared ``duk_heaphdr`` fields are provided by a macro which
can be called in ``duk_hstring`` definition.  This also simplifies field
access, e.g. ``h_str->hdr.h_next`` can be written as ``h_str->h_next``.

Raising a looked up string to top of hash chain
-----------------------------------------------

When an intern check is done and the string is already present in the string
table, the string could be "bubbled" to the top of the hash table chain when
found.  If application code recreates the same string (or set of strings)
many times over, this would make further lookups faster.  This could happen
e.g. when a string is used as a "lookup table" and the same substrings are
looked up over and over.

In practical testing this technique did improve some individual tests, it
didn't have a net positive effect.  The added shuffling for every intern
check seems to nullify the potential (rare) benefits.

ROM string link pointer reuse
-----------------------------

ROM strings don't need a ``h_next`` field.  It could be used fo string
data, provided that both arridx and clen have been dropped so that the
``duk_hstring`` struct itself is actually empty.  This does need a change
to ``DUK_HSTRING_GET_DATA()`` macro though.

Remove heap->st_size field
--------------------------

Because heap->st_size is always equal to heap->st_mask + 1 (except during
heap init) the explicit heap->st_size field can be removed which reduces
book-keeping.
