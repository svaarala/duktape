===============
CBOR absent tag
===============

Overview
========

This document specifies a CBOR [1] tag applied to CBOR Undefined (0xf7)
for indicating an absent value (i.e., a gap) in a CBOR Array::

  Tag: 31
  Data item: Undefined (0xf7)
  Semantics: Absent value in a CBOR Array
  Point of contact: Sami Vaarala <sami.vaarala@iki.fi>
  Description of semantics: https://github.com/svaarala/duktape/blob/master/doc/cbor-absent-tag.rst

The application of this tag to a tag content other than CBOR Undefined is left
for further study.

Semantics
=========

CBOR Maps make a distinction between an absent value and a key with
a value of CBOR Undefined.  CBOR Arrays don't make this distinction;
the keyspace is assumed to be the sequence 0, 1, ..., LEN-1 with all
keys present.

The CBOR absent tag allows one to indicate an absent CBOR Array index,
with the same semantics as a key missing from a Map.  The distinction
may be useful in some environments (such as ECMAScript) but may not
matter in others.

Example
=======

The following ECMAScript array has two gaps in the middle::

  var arr = [ 'foo', , , 'bar' ]

One possible CBOR encoding for the array uses CBOR Undefined (0xf7) to
represent the gaps::

  84                   -- Array of length 4
     63 66 6f 6f       -- Text string "foo"
     f7                -- Undefined
     f7                -- Undefined
     63 62 61 72       -- Text string "bar"

The gaps are encoded as CBOR Undefined (0xf7).  This encoding would decode
back into the following ECMAScript array, with ECMAScript ``undefined`` values
replacing the gaps::

  [ 'foo', undefined, undefined, 'bar' ]

With the "absent" tag applied, the array could be encoded as::

  84                   -- Array of length 4
     63 66 6f 6f       -- Text string "foo"
     d8 1f f7          -- Undefined tagged with 'absent' tag
     d8 1f f7          -- Undefined tagged with 'absent' tag
     63 62 61 72       -- Text string "bar"

If the decoder understood the tag, it would decode back into the original
value containing the gaps.

Rationale
=========

Some programming languages such as ECMAScript differentiate between an
``undefined`` value and an absent value.  For example, consider the following
ECMAScript arrays::

  var arr1 = [ 'foo', , , 'bar' ];
  var arr2 = [ 'foo', undefined, undefined, 'bar' ];

Both arrays have length 4.  The first array has two absent values, at
indices 1 and 2, while the second array has explicit ECMAScript ``undefined``
values in these indices.  For the most part the two arrays behave the same;
for example::

  > print(arr1.length);
  4

  > print(arr2.length);
  4

  > print(arr1[1]);
  undefined

  > print(arr2[1]);
  undefined

However, there are also differences::

  // Gaps are not considered present.
  > print(1 in arr1);
  false

  > print(1 in arr2);
  true

  // Key space has gaps.
  > arr1.forEach(function (v, k) { print(k, v); });
  0 foo
  3 bar

  > arr2.forEach(function (v, k) { print(k, v); });
  0 foo
  1 undefined
  2 undefined
  3 bar

In some situations it may be preferable to preserve the absent vs. undefined
difference so that an encoded value can be decoded without loss of information.
This allows, for example, better roundtripping of ECMAScript values.

A tag applied to CBOR Undefined (0xf7) seems to be the cleanest solution:

* It is backwards compatible with no need to update encoders/decoders.
  Adding a new simple value would affect all encoders and decoders, which
  seems disproportionate because the semantic difference between absent
  and undefined only matters in certain environments.

* When the tag is ignored, the ordinary CBOR Undefined semantics remain
  useful, and in many situations the lost information is not significant.

References
==========

* [1] C. Bormann and P. Hoffman. "Concise Binary Object Representation (CBOR)".
  RFC 7049, October 2013.

Author
======

Sami Vaarala ``<sami.vaarala@iki.fi>``
