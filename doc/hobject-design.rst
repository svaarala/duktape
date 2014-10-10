===========
duk_hobject
===========

Overview
========

The ``duk_hobject`` type represents an object with key-value properties,
and is the most important type from an implementation point of view.
It provides objects for various purposes:

* Objects with E5 normal object semantics

* Objects with E5 array object exotic behavior

* Objects with E5 string object exotic behavior

* Objects with E5 arguments object exotic behavior

* Objects with no E5 semantics, for internal use

This document discusses the ``duk_hobject`` object in detail, including:

* Requirements overview

* Features of Ecmascript E5 objects

* Internal data structure and algorithms

* Enumeration guarantees

* Ecmascript property behavior (default and exotic)
* Design notes, future work

The details of property-related algorithms in E5 are pretty intricate
and are described separately in ``hobject-algorithms.rst``.

The following parts of Ecmascript E5 are useful background:

+-----------+-------------------------------------------------------------+
| Section   | Description                                                 |
+===========+=============================================================+
| 8.6       | Object type, internal properties, property attributes       |
+-----------+-------------------------------------------------------------+
| 8.10      | Property descriptors                                        |
+-----------+-------------------------------------------------------------+
| 8.12      | Default property access methods                             |
+-----------+-------------------------------------------------------------+
| 10.6      | Arguments object exotic behavior                            |
+-----------+-------------------------------------------------------------+
| 15.4.5.1  | Array object exotic behavior                                |
+-----------+-------------------------------------------------------------+
| 15.5.5.2  | String object exotic behavior                               |
+-----------+-------------------------------------------------------------+

See also the following documentation:

* ``hobject-algorithms.rst``: detailed derivation of object algorithms

* ``hobject-enumeration.rst``: more discussion on enumeration

* ``error-objects.rst``: error object properties

* ``function-objects.rst``: function template and instance properties

Requirements overview
=====================

Ecmascript object compatibility requires:

* Properties with a string key and a value that is either a plain data
  value or an accessor (getter/setter)

* Property attributes which control the behavior of individual properties
  (e.g. enumerability and writability)

* Object extensibility flag which controls addition of new properties

* Prototype-based inheritance of properties along a loop-free prototype chain

* Some very basic enumeration guarantees for both mutating and non-mutating
  enumeration

* Object internal properties (at a conceptual level)

Additional practical requirements include:

* Additional enumeration guarantees (e.g. enumeration order matches key
  insertion order); see separate discussion on enumeration

* Minimal memory footprint, especially for objects with few properties
  which dominate common use

* Near constant property lookup performance, even for large objects

* Near constant amortized property insert performance, even for large objects

* Fast read/write access for array entries, in particular avoiding string
  interning whenever possible

* Sparse array support (e.g. ``var x=[]; x[0]=1; x[1000000]=2;``): must be
  compliant, shouldn't allocate megabytes of memory, but does not have to
  be fast

* Support long-lived objects with an arbitrary number of key insertions
  and deletions (implies "compaction" of keys / ordering structure)

There are unavoidable trade-offs involved, the current trade-off preferences
are roughly as follows (most important to least important):

#. Compliance

#. Compactness

#. Performance

#. Low complexity

Compliance is a must-have goal for all object features.  Performance is only
really relevant for common idioms.  Rare cases need to be compliant but not
especially compact or performant: for instance, sparse arrays don't perform
very well but are still compliant.

Object features
===============

Named and internal properties
-----------------------------

An Ecmascript object consists of:

* A set of externally visible *named properties*

* A set of (conceptual) *internal properties*

The externally visible named properties are characterized by:

* A string key

  + 16-bit characters (any 16-bit unsigned integer codepoints may be used)

  + Even array indices are strings, e.g. ``x[0]`` really means ``x["0"]``

* A property value which may be:

  + A *data property*, a plain Ecmascript value

  + An *accessor property*, a setter/getter function pair invoked
    for property accesses

* Property attributes which control property accesses:

  + For data properties:

    - ``[[Configurable]]``

    - ``[[Enumerable]]``

    - ``[[Value]]``

    - ``[[Writable]]``
 
  + For accessor properties:

    - ``[[Configurable]]``

    - ``[[Enumerable]]``

    - ``[[Get]]``

    - ``[[Set]]``

* The ``[[Extensible]]`` internal property determines whether new (own) keys
  can be added to an object.  Many other internal properties exist.

Internal properties are used in E5 to specify required behavior; the
concrete property implementation is implementation specific.  The
current implementation for internal properties is covered in more
detail below.

Property attributes and descriptors
-----------------------------------

Property attributes affect property access algorithms internally.  They are
also externally visible and can be manipulated through built-in methods.
The property attributes are:

* ``[[Configurable]]``

* ``[[Enumerable]]``

* ``[[Value]]``

* ``[[Writable]]``

* ``[[Get]]``

* ``[[Set]]``

New properties added to objects by an assignment are by default data
properties with the following attributes: ``[[Writable]]=true``,
``[[Enumerable]]=true``, ``[[Configurable]]=true``.  This is implicit
in the ``[[Put]]`` algorithm (E5 Section 8.12.5, step 6).  Note that
these defaults differ from the "official default values" (all attributes
``false``) in E5 Section 8.6.1 which are used for e.g.
``[[DefineOwnProperty]]``.

User code can deviate from the defaults for assignments by defining or
modifying properties using ``Object.defineProperty()``.  This is not
very common, so almost all user properties have default attributes.
Built-in objects often have properties with non-default attributes, though.

A *property descriptor* contains zero or more property attributes,
and is used both internally and externally to describe or modify
property attributes.  Property descriptors are used internally in the E5
specification with the following notation::

  { ``[[Value]]``: 42, ``[[Writable]]``: true }

The same property descriptor would be represented as an external Ecmascript
value::

  { "value": 42, "writable": true }

The internal and external property descriptors are converted through the
internal ``FromPropertyDescriptor()`` and ``ToPropertyDescriptor()``
methods.

Property descriptors are classified into several categories based on
what keys they contain:

* Data property descriptor: contains ``[[Value]]`` or ``[[Writable]]``

* Accessor property descriptor: contains ``[[Set]]`` or ``[[Get]]``

* Generic property descriptor: a descriptor which is neither a data nor
  an accessor property descriptor, i.e. does not contain
  ``[[Value]]``, ``[[Writable]]``, ``[[Set]]``, or ``[[Get]]``

Although a property descriptor can technically be both a data property
descriptor and an accessor property descriptor at the same time, such
descriptors are rejected whenever they are encountered.  The
``[[Configurable]]`` and ``[[Enumerable]]`` attributes can be in any
kind of a descriptor.

A property descriptor is *fully populated* if it contains all the keys of
its type, i.e.:

* A fully populated data descriptor contains all of the following:
  ``[[Configurable]]``, ``[[Enumerable]]``, ``[[Value]]``, ``[[Writable]]``

* A fully populated accessor descriptor contains all of the following:
  ``[[Configurable]]``, ``[[Enumerable]]``, ``[[Get]]``, ``[[Set]]``

The property attributes stored in the object for a certain property always
form a fully populated property descriptor.  Any missing values are always
filled in with defaults when a property is first inserted, and although
attribute values can be changed after insertion, the attributes themselves
cannot be removed.

Partially populated property descriptors are used internally in the
specification for describing property modifications.  They can also be
used externally (for similar purposes) through ``Object.defineProperty()``.

Consider the following example, which illustrates how properties and
their attributes can be defined in various ways::

  // array initializer
  var o = [ "one", "two" ];  // also sets "length" to 2

  // add "foo" (writable, enumerable, configurable)
  o.foo = 1;

  // modify attributes after insertion
  Object.defineProperty(o, "foo", {
    writable: false
  });

  // insert an accessor, with [[DefineOwnProperty]] default
  // attributes (non-enumerable, non-configurable)
  Object.defineProperty(o, "bar", {
    "get": function() { return "bar"; },
    "set": function(x) { throw new Error("cannot write bar"); }
  });

The object would have the following internal state (represented as
external property descriptors)::

  "0"       -->  { "configurable": true,
                   "enumerable": true,
                   "writable": true,
                   "value": 'one' }

  "1"       -->  { "configurable": true,
                   "enumerable": true,
                   "writable": true,
                   "value": 'two' }

  // non-default attributes (E5 Section 15.4.5.2)
  "length"  -->  { "configurable": false,
                   "enumerable": false,
                   "writable": true,
                   "value": 2 }

  "foo"     -->  { "configurable": true,
                   "enumerable": true,
                   "writable": false,
                   "value": 1 }

  // [[DefineOwnProperty]] defaults for new properties
  // differ from [[Put]] (defaults to false)
  "bar"     -->  { "configurable": false,
                   "enumerable": false,
                   "get": <function reference>,
                   "set": <function reference> }

Property descriptors can also be read back from user code through
``Object.getOwnPropertyDescriptor()``.  Example using NodeJS / V8::

  var o = {
    foo: 1,
  
    get bar() { return "bar"; },
    set bar(x) { throw new Error("cannot write bar"); }
  };

  // Prints:
  // { value: 1,
  //   writable: true,
  //   enumerable: true,
  //   configurable: true }

  console.log(Object.getOwnPropertyDescriptor(o, "foo"));

  // Prints:
  // { get: [Function: bar],
  //   set: [Function: bar],
  //   enumerable: true,
  //   configurable: true }

  console.log(Object.getOwnPropertyDescriptor(o, "bar"));

The visibility of property attributes to user code poses some implementation
challenges.  Ordinary property access occurs through the ``[[Get]]``,
``[[Put]]``, and ``[[Delete]]`` algorithms, hiding some of the internal
complexity regarding property attributes etc.  However, the built-in method
``Object.defineProperty()`` exposes the internal ``[[DefineOwnProperty]]``
in all its complexity to user code.

The current implementation manages to use fully populated descriptors
internally, and expose partial descriptors only through
``Object.defineProperty()``.

Prototype chain
---------------

Each object has a non-mutable *internal prototype* established at
object creation (``[[Prototype]]`` internal property).  The value of
the internal prototype must be another object or null.  Since the
prototype object may also have an internal prototype (and so on), the
objects form a non-mutable *prototype chain* terminating at an object
whose internal prototype is ``null``.

The prototype chain affects most property access algorithms with the
general principle that if a property is not found in a certain object,
the prototype chain is then searched in ascending order.  To simplify:

* Property read operations return the value found in the first object
  in the prototype chain containing the property.  If an accessor
  property is found, the getter is called.

* Property write operations first check the prototype chain to see
  whether the property exists.  If so, the property may prevent the
  write (if a non-writable data property), cause a setter call (if an
  accessor property), or allow the write.  The write is allowed and
  not captured by a setter, the property is added to the *original target
  object* (instead of an object possibly higher up in the prototype chain).

* Property delete operations do not consult the prototype chain and
  only have an effect on the target object.

The non-mutability of the prototype chain is not very explicit in the
specification (nor does the current implementation assume or take
advantage of its non-mutability).  The requirement is stated in a
footnote in E5 Section 8.6.2:

  NOTE This specification defines no ECMAScript language operators or
  built-in functions that permit a program to modify an object’s
  ``[[Class]]`` or ``[[Prototype]]`` internal properties or to change
  the value of [[Extensible]] from false to true.

The prototype chain is required to be loop-free; it is required to
terminate in a null reference (again, in E5 Section 8.6.2):

  Every ``[[Prototype]]`` chain must have finite length (that is, starting
  from any object, recursively accessing the [[Prototype]] internal
  property must eventually lead to a null value).

The current implementation makes no specific effort to ensure this because
plain Ecmascript code cannot create prototype loops (though C code can
easily do so).  To see why this is the case, assume that the current set of
objects have no prototype loops and a new object is created.  The internal
prototype of the created object is either set to ``null`` or one of the
existing objects during its creation.  Since the prototypes of the existing
objects cannot be updated, the result is a new set of objects without
prototype loops.  Even so, all prototype walking loops in the implementation
contain a sanity limit for prototype chain length to break out should a
loop be somehow created.

Valid array index and length
----------------------------

The E5 specification has very specific definitions for *valid array index*
property names and *valid array length* property values.  Only these trigger
the ``Array`` specific ``[[DefineOwnProperty]]`` behavior in E5 Section
15.4.5.1 steps 3 and 4.

Note that these requirements do not simply specify a minimum length for
supported arrays: they also specify a maximum length for arrays at least
with respect to how ``length`` behaves.  It would be nice to be able to
support larger arrays as a build option, but there are probably no realistic
cases with arrays larger than 4G elements which would still be relevant for
embedding Duktape.

Array index
:::::::::::

Ecmascript E5 Section 15.4 states:

  A property name ``P`` (in the form of a String value) is an *array index*
  if and only if ``ToString(ToUint32(P))`` is equal to ``P`` and
  ``ToUint32(P)`` is not equal to 2^32-1 (0xffffffff).

This implies that the maximum array length is 2**32-1 (0xffffffff) and
the maximum array index is one less, 2**32-2 (0xfffffffe).  All valid
array index and length values can be represented with unsigned 32-bit
values.  Because 0xffffffff is not a valid array index, it is used internally
as a convenient "no array index" marker (``DUK_HSTRING_NO_ARRAY_INDEX``).
For instance, a coercion call can return the marker to indicate that an input
was not a valid array index.

Because Ecmascript object property keys are strings, all array indices
encountered in property access expressions are conceptually first coerced
to a string form using ``ToString()`` and then checked whether they are
valid array indexes (this is unlike array ``length`` values, which can
have any type, see below).  (Obviously, this explicit coercion should be
avoided whenever possible.)

The following table lists the possible coercions:

+---------------+-----------------+---------------------------------------+
| Property name | ``ToString``    | Valid array index?                    |
+===============+=================+=======================================+
| ``undefined`` | ``"undefined"`` | no                                    |
+---------------+-----------------+---------------------------------------+
| ``null``      | ``"null"``      | no                                    |
+---------------+-----------------+---------------------------------------+
| ``false``     | ``"false"``     | no                                    |
+---------------+-----------------+---------------------------------------+
| ``true``      | ``"true"``      | no                                    |
+---------------+-----------------+---------------------------------------+
| a number      | various         | yes, if a whole number in the range   |
|               |                 | [0,2**32-2]                           |
+---------------+-----------------+---------------------------------------+
| a string      | same            | yes, if a "canonical" representation  |
|               |                 | for a whole number in the range       |
|               |                 | [0,2**32-2] (``"2"`` is valid, while  |
|               |                 | ``"0.2e1"`` is not)                   |
+---------------+-----------------+---------------------------------------+
| an object     | various         | depends on coerced string value       |
+---------------+-----------------+---------------------------------------+

Note that for instance ``"0.2e1"`` which numerically represents 2 is not
a valid array index: ``ToString(ToUint32("0.2e1"))`` produces ``"2"``, but
this is not equal to the original string ``"0.2e1"`` (which is required).

Array length
::::::::::::

The requirements for a valid *array length* are a bit different, because
array length is assigned as an arbitrary property value and is not therefore
automatically coerced to a string first.

The requirements for a valid array length are implicit in E5 Section 15.4.5.1,
steps 3.c to 3.d:

* Step 3.c: Let ``newLen`` be ``ToUint32(Desc.[[Value]])``.

* Step 3.d: If ``newLen`` is not equal to ``ToNumber(Desc.[[Value]])``, throw
  a ``RangeError`` exception

The requirements boils down to (for input value ``X``):

* ``ToUint32(X)`` == ``ToNumber(X)``

The requirements are seemingly similar to the array index requirements, but
in fact allow a wider set of values, such as:

* ``true`` represents array length ``1``, but is not a valid array index

* ``"0.2e1"`` represents array length ``2``, but is not a valid array index

* ``0xffffffff`` represents array length 2**32-1, but is not a valid array index

A potential ``length`` value ``X`` is treated as follows (see E5 Sections
9.3 and 9.6 for definitions of the coercions ``ToNumber`` and ``ToUint32``):

+----------------+--------------+--------------+--------------------------+
| Property value | ``ToNumber`` | ``ToUint32`` | Valid array length?      |
+================+==============+==============+==========================+
| ``undefined``  | ``NaN``      | ``+0``       | no                       |
+----------------+--------------+--------------+--------------------------+
| ``null``       | ``+0``       | ``+0``       | yes, length ``0``        |
+----------------+--------------+--------------+--------------------------+
| ``false``      | ``+0``       | ``+0``       | yes, length ``0``        |
+----------------+--------------+--------------+--------------------------+
| ``true``       | ``1``        | ``1``        | yes, length ``1``        |
+----------------+--------------+--------------+--------------------------+
| a number       | various      | various      | yes, if whole number in  |
|                |              |              | the range [0,2**32-1]    |
+----------------+--------------+--------------+--------------------------+
| a string       | various      | various      | yes, if representation of|
|                |              |              | a whole number in the    |
|                |              |              | range [0,2**32-1] (does  |
|                |              |              | not need to be canonical,|
|                |              |              | e.g. ``"2"``, ``"2.0"``, |
|                |              |              | ``"0.2e1"`` are all      |
|                |              |              | acceptable               |
+----------------+--------------+--------------+--------------------------+
| an object      | various      | various      | depends on coerced       |
|                |              |              | values                   |
+----------------+--------------+--------------+--------------------------+

As an example of using a non-number as Array length::

  duk> var a = [ 'foo', 'bar', 'quux' ]; a
  = foo,bar,quux
  duk> a.length = true;  // same as a.length = 1
  = true
  duk> a
  = foo
  duk> a.length
  = 1

Enumeration requirements
------------------------

Enumeration requirements are discussed in a separate section below,
together with the current implementation for enumerating object keys.

.. raw:: LaTeX

   \newpage

Structure overview
==================

The memory layout of an ``duk_hobject`` is illustrated below::

  duk_hobject                       property allocation
  (fixed allocation)                (dynamic allocation)
  
  +------------------------+        +---------------------------+
  | duk_heaphdr    (flags) |  .---->| entry part keys           |
  +========================+  |     | (e_size x duk_hstring *)  |
  | duk_u8 *p -------------+--'     +---------------------------+
  | duk_u32 e_size         |     .->| entry part values         |
  | duk_u32 e_next         |     :  | (e_size x duk_propvalue)  |
  | duk_u32 a_size         |     :  +---------------------------+
  | duk_u32 h_size         |     +->| entry part flags          |
  | duk_hobject *prototype |     :  | (e_size x duk_u8)         |
  +------------------------+     :  +---------------------------+
  : duk_hcompiledfunction  :     +->| array part values         |
  : duk_hnativefunction    :     :  | (a_size x duk_tval)       |
  : duk_hthread            :     :  +---------------------------+
  :                        :     +->| hash part indices         |
  : (extended structures)  :     :  | (h_size x duk_u32)        |
  +------------------------+     :  +---------------------------+
                                 :
  'p' is NULL if no property     `- these pointers are computed
  allocation exists                 on-the-fly using e_size and
                                    a_size

There are multiple memory layouts for the property allocation part,
each containing the same parts but in a different order.  The different
layouts are used to best suit the target platform's alignment needs.
The layout is automatically selected by Duktape during compilation
(feature detection).

The heap header structure ``duk_heaphdr`` contains:

* flags with both heap level flags (``DUK_HEAPHDR_FLAG_*`` in
  ``duk_heaphdr.h``) and object specific flags (``DUK_HOBJECT_FLAG_*``
  in ``duk_hobject.h``)
* heap allocated list linkage
* reference counter field

The object specific part of ``duk_hobject`` contains:

* property allocation: A data structure for storing properties

* internal prototype field for fast prototype chain walking;
  other internal properties are stored in the property allocation

* ``duk_hcompiledfunction``, ``duk_hnativefunction``, and ``duk_hthread``
  object sub-types have an extended structure with more fields

The property allocation part is a single memory allocation containing all
the object properties, both external and internal.  It is subdivided
internally into the following parts:

* *Entry part* stores ordered key-value properties with arbitrary
  property attributes (flags), and supports accessor properties
  (getter/setter properties), i.e., full E5 semantics

* *Array part* (optional) stores plain values with default property
  attributes (writable, enumerable, configurable) for valid array indices
  (``"0"``, ``"1"``, ..., ``"4294967294"``); does not support accessor
  properties

* *Hash part* (optional) provides accelerated key lookups for the
  entry part, mapping a key into an entry part index

Internal properties are stored in the entry part, and are only distinguished
from normal properties in that their keys are invalid UTF-8 sequences which
cannot be generated (and thus not accessed) from Ecmascript code.  Internal
properties should never be enumerable or visible in other ways.  See separate
discussion of internal properties later in the document.

The ``duk_hobject`` allocation is fixed and its address never changes after
initialization.  The property allocation part is allocated on demand, and
its address may change when the object is resized; this currently always
happens because there is no in-place resizing.  The resizing process is
described in a separate section below.  The property allocation part can also
be missing, the ``p`` pointer is NULL in this case.

To avoid storing multiple pointers/offsets pointing to the individual sections
of the property allocation, the different parts are reached with run-time
pointer computations.  There are a lot of convenience accessor macros in
``duk_hobject.h`` to access the various parts and elements within the parts.
**Always** use these macros to manipulate object properties so that changing
the layout is contained to a small section of code.

Notes:

* For a newly allocated object with no properties, there is no property
  allocation, and the ``p`` pointer is ``NULL``.  It may also become
  ``NULL`` later if all object properties are deleted and the object is
  then compacted.

* The array part is assumed to be comprehensive, i.e. if the array part
  exists, all valid array index keys must reside in the array part.  If
  this invariant would need to be violated, the array part is abandoned
  and its entries moved into the entry part.

* The array part entries are assumed to have default property attributes
  (writable, configurable, enumerable).  If this invariant would need to
  be violated, the array part is also abandoned.

* The array part is also abandoned if the array part would become too
  sparse, i.e. it would take too much memory compared to the number of
  entries actually present.  This behavior is not compliance related.

* A certain key can be present at most once (in either the entry or array
  part).  This invariant must be enforced when adding new keys into the
  object.  Other implementation code can simply assume it.

* The default attributes for new properties depend on how they are inserted:

  + For ordinary assignment, the defaults are defined in the ``[[Put]]``
    algorithm (E5 Section 8.12.5, step 6): writable, enumerable, configurable.
    Note that these differ from the official default values defined in
    E5 Section 8.6.2.

  + For ``[[DefineOwnProperty]]`` the defaults are defined in E5 Section
    8.12.9 step 4, which refers to the official "default attribute values"
    in E5 Section 8.6.2: non-writable, non-enumerable, non-configurable.

Entry and hash part
===================

The entry part contains ordered key-value pairs, and supports full Ecmascript
E5 semantics: property values can be plain *data properties* or *accessor
properties*, and can have any property attributes, stored in property flags.
Internal properties, identified with a special key prefix, can also be stored.

The hash part is optional, and allows faster lookups into the entry part.
It is only used for objects with at least ``DUK_HOBJECT_E_USE_HASH_LIMIT``
properties in the entry part.

.. raw:: LaTeX

   \newpage

Layout
------

The entry part consists of three separate arrays arranged sequentially:
keys, values, flags.  An optional hash part may exist to speed up key
lookups::

    +---------+                  
    | key 0   |   entry part keys
    | NULL    |   (duk_hstring *)
 .->| key 2   |
 |  | / / / / |
 |  | / / / / |
 |  +---------+
 |  | value 0 |   entry part values
 |  | / / / / |   (duk_propvalue)
 +->| value 2 |
 |  | / / / / |
 |  | / / / / |
 |  +---------+
 |  | flags 0 |   entry part flags
 |  | / / / / |   (duk_u8)
 +->| flags 2 |
 |  | / / / / |
 |  | / / / / |   / / / /   Denotes uninitialized data
 |  +---------+             which is not reachable from
 |  :         :             a GC perspective
 |  : array   :
 |  : part    :
 |  :         :
 |  +---------+
 |  | UNUSED  |   hash part
 `--| 2       |   (duk_u32)
    | DELETED |
    | UNUSED  |   UNUSED  = DUK_HOBJECT_HASHIDX_UNUSED
    | 0       |           = 0xffffffffU
    | UNUSED  |
    | UNUSED  |   DELETED = DUK_HOBJECT_HASHIDX_DELETED
    +---------+           = 0xfffffffeU
 
                  DELETED entries don't terminate hash
                  probe sequences, UNUSED entries do.
 
    Here, e_size = 5, e_next = 3, h_size = 7.

.. FIXME for some unknown reason the illustration breaks with pandoc

Each array in the entry part contains ``e_size`` allocated entries.
The entries at indices [0,\ ``e_next``\ [ are currently in use, and any
entries above that are uninitialized (garbage) data, and not reachable
from a GC perspective.

New keys are always appended to the current ``e_next`` position (with the
entry part resized if it is already full).  Existing entries are deleted
by marking a key as ``NULL``; they are not reused for new properties to
avoid disrupting the key enumeration order (which should match insertion
order).  NULL entries are removed (compacted) whenever the property
allocation is resized.  If a key entry is ``NULL``, the corresponding value
and flag fields MUST NOT be interpreted, and are not considered reachable
from a GC perspective.  Thus, the property value must be decref'd when the
key is set to NULL.  If a key entry is non-\ ``NULL``, it is considered
reachable and must be incref'd on insertion.

Flags are represented by an ``duk_u8`` field, with flags defined in
``duk_hobject.h``.  The current flags are:

* ``DUK_PROPDESC_FLAG_WRITABLE``

* ``DUK_PROPDESC_FLAG_ENUMERABLE``

* ``DUK_PROPDESC_FLAG_CONFIGURABLE``

* ``DUK_PROPDESC_FLAG_ACCESSOR``

The value field is a union of (1) a plain value, and (2) an accessor value
which contains ``get`` and ``set`` function pointers.  The interpretation
of the union depends on the ``DUK_PROPDESC_FLAG_ACCESSOR``; if set, the
value is treated as the accessor part of the union, otherwise it is treated
as the value part of the union.  This interpretation must be done everywhere
where the value is accessed, otherwise garbage values will be read.
In particular, the reference count and garbage collection code must always
interpret the union correctly based on the current entry flags.

The hash part (if it exists) maps an ``duk_hstring`` key ``K`` to an index
``I`` of the current entries part, or indicates that the key does not exist
in the object.  The hash index structure has no bearing on garbage
collection; in particular, the index references from the hash part to the
entry part are not considered counted references.

If the hash part exists, it is always kept up-to-date with the entry part
so that both structures always contain the same keys.  Deleted entries
in the hash data structure are explicitly marked DELETED.  Such entries
don't terminate hash probe sequences but act otherwise as UNUSED entries;
see more detailed discussion below.  DELETED entries are eliminated
(converted to UNUSED) when the property allocation is resized, improving
hash part performance.

Notes:

* This layout of three separate arrays has been chosen so that a linear key
  scan is efficient, e.g. works nicely with cache lines and prefetches,
  which is important because "small" objects don't have a hash part at all.
  Linear scan is more space efficient and often also faster than a
  hash lookup, which does one or more random accesses to the hash part when
  going through the probe sequence.

* Objects in dynamic languages often don't guarantee a key enumeration
  order, which allows objects to be implemented with easy and efficient
  "pure" hash tables.  Although Ecmascript E5 does not require a particular
  key ordering for enumeration, a practical implementation must provide
  some ordering guarantees to be compatible with existing code base.  Such
  guarantees include enumerating keys in their insertion order; see the
  section on enumeration for details.  This has a big impact on the viable
  data structure alternatives; the current entry and hash part model is a
  relatively simple approach to satisfy the practical requirements.

Hash part details
-----------------

The hash part maps a key ``K`` to an index ``I`` of the entry part or
indicates that ``K`` does not exist.  The hash part uses a `closed hash
table`__, i.e. the hash table has a fixed size and a certain key has
multiple possible locations in a *probe sequence*.  The current probe
sequence uses a variant of *double hashing*.

__ http://en.wikipedia.org/wiki/Hash_table#Open_addressing

.. note:: The current hash algorithm does not perform especially well,
          and it is future work to make it work better especially with
          high load factors.

The hash part is an array of ``h_size`` ``duk_u32`` values.  Each value
is either an index to the entry part, or one of two markers:

* ``UNUSED``: entry is currently unused

* ``DELETED``: entry has been deleted

Hash table size (``h_size``) is selected relative to the maximum number
of inserted elements ``N`` (equal to ``e_size`` in practice) in two steps:

#. A temporary value ``T`` is selected relative to the number of entries,
   as ``c * N`` where ``c`` is currently about 1.2.

#. ``T`` is rounded upwards to the closest prime from a pre-generated
   list of primes with an approximately fixed prime-to-prime ratio.

   + The list of primes generated by ``genhashsizes.py``, and is encoded
     in a bit packed format, decoded on the fly.  See ``genhashsizes.py``
     for details.

   + The fact that the hash table size is a prime simplifies probe sequence
     handling: it is easy to select probe steps which are guaranteed to
     cover all entries of the hash table.

   + The ratio between successive primes is currently about 1.15.
     As a result, the hash table size is about 1.2-1.4 times larger than
     the maximum number of properties in the entry part.  This implies a
     maximum hash table load factor of about 72-83%.

   + The current minimum prime used is 17.

The probe sequence for a certain key is guaranteed to walk through every
hash table entry, and is generated as follows:

#. The initial hash index is computed directly from the string hash,
   modulo hash table size as: ``I = string_hash % h_size``.

#. The probe step is then selected from a pre-generated table of 32
   probe steps as: ``S = probe_steps[string_hash % 32]``.

   + The probe steps are is guaranteed to be non-zero and relatively prime
     to all precomputed hash table size primes.  See ``genhashsizes.py``.

   + Currently the precomputed steps are small primes which are not present
     in the precomputed hash size primes list.  Technically they don't need
     to be primes (or small), as long as they are relatively prime to all
     possible hash table sizes, i.e. ``gcd(S, h_size)=1``, to guarantee that
     the probe sequence walks through all entries of the hash.

#. The probe sequence is: ``(X + i*S) % h_size`` where i=0,1,...h_size-1.

When looking up an element from the hash table, we walk through the probe
sequence looking at the hash table entries.  If a UNUSED entry is found, the
probe sequence is terminated, and we determine that the entry cannot be in
the hash (and thus, not in the entry part).  If a DELETED entry is found,
we continue with the probe sequence.  This is necessary to handle deletions
correctly.

When inserting an element to the hash table, we must first ensure it does
not already exist.  The probe sequence must be verified up to the first
UNUSED entry (but not beyond).  The element is then inserted to the first
UNUSED *or* DELETED entry.  DELETED entries can thus be reused, but they
can never be marked UNUSED, otherwise probe sequences would be "broken".

When an element is deleted, it is first located by following the probe
sequence, and if found, is then replaced with a DELETED marker.

If the hash part is full, the probe sequence eventually comes back to the
initial entry and is thus in an infinite loop.  An explicit loop check
would be an unnecessary cost: it suffices to ensure there is at least one
UNUSED entry in the hash part.  As the probe sequence is guaranteed to
cover every hash entry, it will eventually hit the UNUSED entry and
terminate.

DELETED entries don't terminate hash probe sequences.  If they did, existing
hash chains could be broken as a side effect of deletions.  Since the hash
must contain at least one UNUSED entry, DELETED entries must be "purged" from
time to time: if all entries were either occupied or marked DELETED, probe
sequences would never terminate.  Currently DELETED entries are only removed
during property allocation resizing, which always rehashes all entries,
purging any DELETED entries as a side effect.  The handling of key insertion
and deletion in the entry part actually guarantees that a rehashing occurs
before the hash part fills up with DELETED entries as follows.

Because all new entries are appended to the existing entry part key array
(deleted entry part keys are marked ``NULL`` but not reused until a resize
happens), the hash part contains exactly ``e_next`` used and DELETED entries
combined, and exactly ``h_size - e_next`` UNUSED entries.  As long as the hash
part is larger than the entry part (``h_size > e_size``) the hash is thus
guaranteed to contain at least one UNUSED entry.  When an insertion is
attempted to a full entry part (``e_next = e_size``), a property allocation
resize is triggered which also resizes and rehashes the hash part, purging
any ``DELETED`` entries.

.. raw:: LaTeX

   \newpage

Array part
==========

Layout
------

The array part simply contains a sequence of tagged values::

  values
  (duk_tval)

  +---------+
  | value 0 |  Represents the array:
  | UNUSED  |    { "0": (value 0), "2": (value 2) }
  | value 2 |
  | UNUSED  |  UNUSED = duk_tval 'undefined unused' value
  | UNUSED  |           (DUK_TVAL_IS_UNDEFINED_UNUSED(tv))
  +---------+

  Here, a_size = 5.

The array part stores all properties whose string key is a *valid
array index*, a canonical string representation of a whole number in
the range [0,0xfffffffe] (discussed in more detail above).  The array
part is *comprehensive*, which means that if an object has an array part,
any string key which is a valid array index *must* reside in the array
part (it can never be in the entry part).  (Because 0xffffffff is not a
valid array index, it is used internally as a convenient "no array index"
marker, ``DUK_HSTRING_NO_ARRAY_INDEX``.)

The array part does not store any property attribute flags: all entries
are implicity assumed to be data properties with the default ``[[Put]]``
property attributes: writable, enumerable, configurable.  This assumption is
true for almost all real world code.  If a property insertion or modification
were to violate this assumption, the entire array part needs to be abandoned
and moved to the entry part to maintain E5 semantics.

All array entries are always reachable from a GC perspective, up to
the allocated size, ``a_size``.  Unused values are marked with the special
"undefined unused" value, set using the
``DUK_TVAL_SET_UNDEFINED_UNUSED`` macro.  Any other entries, including
"undefined actual" values, set using the ``DUK_TVAL_SET_UNDEFINED_ACTUAL``
macro, are considered to be in use, and their corresponding key is
considered to exist in the object, and they are thus visible to enumeration.
In the illustration above, values at indices "0" and "2" are considered used,
so an attempt to enumerate the array part would result in ``["0", "2"]``, also
e.g. ``2 in obj`` would be true while ``3 in obj`` would be false.

Notes:

* The array part is an optimized structure for reading and writing
  array indexed properties efficiently.  It can be used for *any* object,
  not just the Ecmascript ``Array`` object, and Ecmascript ``Array``
  exotic behaviors are unrelated to the array part's existence.

* A non-\ ``Array`` object with an array part does not get the ``Array``
  related exotic behaviors (like automatic interaction between array
  indexed elements and the ``length`` property).

* An ``Array`` object may be created without an array part, or may have its
  array part abandoned.  The ``Array`` exotic behaviors must keep on
  working even if the ``Array`` object has no array part.

* The ``Array`` ``length`` property is stored as an ordinary property in
  the entries part, and has no relation with array part size (``a_size``).

Abandoning the array part
-------------------------

The array entries are assumed to be data properties with default
attributes (writable, configurable, enumerable).  This has the
following implications:

* When a new property with an array index outside the currently allocated
  array part is being added (e.g. as part of a property write), we must
  either:

  #. extend the array allocation to cover the new entry; or

  #. abandon the entire array part, moving all array part entries to the
     entry part.

  The first option may not be viable if the array were to become very
  sparse (e.g. when executing: ``var a = []; a[1000000000] = 1``).

* When a property in the array part would become an accessor property
  (getter/setter) or would need to have incompatible attributes, the entire
  array part must be abandoned.

  Note that the property cannot be stored in the entry part while keeping
  the array part, because the array part is assumed to be comprehensive.

When an array part is abandoned, its entries are all moved into the entries
part as ordinary key-value properties with string keys.  If an array part
is abandoned for a certain object it is currently never reinstated.  The
current implementation performs array abandonment only as part of a property
allocation resize; the need to abandon the array thus triggers a resize with
the side effect of doing key compaction, rehashing, etc.

The enumeration ordering of keys is preserved for existing array index
keys by adding them first to the resized entry part, before non-array-index
keys.  However, key ordering behavior for new array indexed entries after
the resize (which abandons the array part) differs from an object with an
array part: array index keys are appended to the entry part as ordinary keys.
This implementation specific behavior is illustrated below::

  // two example arrays
  var a = [1,2,3]; a.foo = "bar";
  var b = [1,2,3]; b.foo = "bar";

  // force 'a' to abandon array part
  a[1000000] = 4;  // array part abandoned
  a.length = 3;    // array part not reinstated

  // arrays 'a' and 'b' have the same enumeration
  // ordering at this point: [ "0", "1", "2", "foo" ].

  a[4] = 5;
  a[3] = 4;
  b[4] = 5;
  b[3] = 4;

  // enumeration ordering differs here:
  print(Object.keys(a));   // -> 0,1,2,foo,4,3
  print(Object.keys(b));   // -> 0,1,2,3,4,foo

Note that Ecmascript implementation behavior differs greatly when it comes
to "sparse arrays".  For instance, the above example has varying results
with existing Ecmascript implementations::

  // Rhino (Rhino 1.7 release 3 2012 02 13)
  // (behavior matches example above)
  [...]
  0,1,2,foo,4,3
  0,1,2,3,4,foo

  // smjs (JavaScript-C 1.8.5+ 2011-04-16)
  // (enumerates based purely on insertion order
  // for both objects)
  [...]
  0,1,2,foo,4,3
  0,1,2,foo,4,3

  // V8 (nodejs v0.4.12)
  // (enumerates array indices before other keys
  // for both objects)
  print = console.log;
  [...]
  [ '0', '1', '2', '3', '4', 'foo' ]
  [ '0', '1', '2', '3', '4', 'foo' ]

Fast array access
-----------------

The reason why a separate array part exists is to:

* Store normal array structures compactly: normal arrays are dense and
  have default properties

* Provide relatively fast access to array elements: avoid entry or hash
  part lookup

* Avoid string interning of array index keys for numeric indices

Ecmascript array indices are always strings, so conceptually arrays
map string indices of the form "0", "1", etc to arbitrary values.
Non-string keys for property accesses are coerced to strings at run time.
For instance::

  var a = [1,2];
  print(a[0]);    // 0 coerced to "0" before access
  print(a["0"]);  // equivalent access

  var i = 1;
  print(a[i]);    // equivalent to a["1"]

If the property access key happens to be a number which is also a valid
array index (whole number in the range [0,2**32-1[) and the target array
happens to have an array part, we can avoid the string interning and
look up the entry directly.  The same applies to assignments to array
index properties.

This is not trivial to implement in practice because of the prototype
chain, the details of property access algorithms etc.  Currently the
"fast path" behavior applies to a very narrow set of circumstances.
See the following functions in ``duk_hobject_props.c``:

* ``duk_hobject_get_value_u32()``

* ``duk_hobject_get_value_tval()``

* ``duk_hobject_has_property_u32()``

* ``duk_hobject_has_property_tval()``

There is currently no fast path for array writes, which means the key is
temporarily interned for the duration of the array write.  The array write
fast path is a bit tricky: if the element does not already exist, a property
higher up in the prototype chain may block or capture the write, and currently
the prototype chain lookup is only possible with a string key.  See future
work.

Resizing the property allocation
================================

The property allocation resizing algorithm handles: growing and shrinking
of the entry, array, and hash parts; abandoning the array part, key compaction
(elimination of ``NULL`` keys); and rehashing (elimination of ``DELETED``
entries).  Only one resizing algorithm is used; all parts of the property
allocation are always processed during resizing.  Multiple resizing
algorithms would be useful (e.g. to just resize one part, perhaps in-place),
but would increase code size.  The current resize algorithm is
``realloc_props()`` in ``duk_hobject_props.c``.

The property allocation is currently resized e.g. when:

* The entry part runs out during insertion of a new property.

* The array part needs to be extended during insertion of a new
  property.

* The array part needs to be abandoned due to:

  + a property insert which would result in a too sparse array part;

  + a property insert incompatible with the array part assumptions; or

  + a property modification incompatible with the array part assumptions.

* The object is compacted, i.e. its active entry and array part properies
  are counted, and an optimal (small) new size is allocated.

The resizing algorithm:

* Allocates a new memory area for properties (in-place resizing is not
  supported).  This may trigger a garbage colleciton, and may fail.

* If array abandoning is requested, existing array properties are first
  moved into the beginning of the new entry part to keep the enumeration
  ordering identical to that before abandonding (array indices are normally
  enumerated before other entry keys).  The array abandoning process is a
  bit tricky because it requires string interning which may trigger garbage
  collection and may also fail.  Any temporary values must thus be reachable
  and correctly referenced counted for every intern call.

* Existing entry part properties are moved into the new entry part.  Any
  ``NULL`` keys are skipped, so that the entry part keys are "compacted".

* If the new allocation has a hash part, the new entry part keys are
  hashed into the new hash part.  Note that an existing hash part (of
  the current allocation) is irrelevant and is ignored here; in any case,
  the new hash part contains no ``DELETED`` entries.

If the array part is not abandoned, reference counts for the object as a
whole remain constant: the reachable keys and values are exactly the same.
If the array part is abandoned, the newly interned array index string keys
(e.g. ``"0"``) will be newly reachable and need to be incref'd.

Some complications:

* The tricky reachability issues related to array abandoning are handled by
  using the current thread's value stack as a place to store temporaries;
  the value stack has an existing process for cleanup if an error occurs.
  This is not the whole story, though; see code for details.

* The allocation calls required during resizing (for the new memory area,
  string interning, and value stack resizing) may cause a garbage collection.
  The garbage collection may attempt to resize any object as part of an
  "emergency GC" compaction.  This needs to be prevented for the current
  object (or in general, for any object being concurrently resized).

  The current solution is to use the ``heap->mark_and_sweep_base_flags``
  mechanism to prevent finalizers from running (= prevents attempts to add,
  remove, or modify properties in the middle of a resize) and to prevent
  object compaction (so that a certain object won't be resized when it is
  already being resized).

Enumeration
===========

Enumeration poses a lot of problems for implementing the Ecmascript
object/array semantics efficiently.

Below, the relevant parts of the specification are first discussed
(and quoted for easy reference), followed by some useful additional
requirements and features.  Some implementation pitfalls are then
discussed.  Finally, the current enumeration mechanism is discussed
in some detail.

The current implementation can be found in ``duk_hobject_enum.c``.

Ecmascript specification requirements
-------------------------------------

E5 Section 12.6.4: "The for-in statement" contains the main requirements
for enumeration in the E5 specification:

* The mechanics and order of enumerating the properties [...] is
  not specified.

* Properties of the object being enumerated may be deleted during
  enumeration. If a property that has not yet been visited during
  enumeration is deleted, then it will not be visited.

* If new properties are added to the object being enumerated during
  enumeration, the newly added properties are not guaranteed to be
  visited in the active enumeration.

* Enumerating the properties of an object includes enumerating
  properties of its prototype, and the prototype of the prototype,
  and so on, recursively; but a property of a prototype is not
  enumerated if it is "shadowed" because some previous object in
  the prototype chain has a property with the same name.

E5 Section 15.2.3.7: "Object.defineProperties ( O, Properties )"
requires that when multiple properties are defined with
``Object.defineProperties()``, the order should be kept:

* If an implementation defines a specific order of enumeration
  for the for-in statement, that same enumeration order must be
  used to order the list elements in step 3 of this algorithm.

E5 Section 15.2.3.14: "Object.keys ( O )" requires that the "for-in"
enumeration order should also be used for ``Object.keys()``:

* If an implementation defines a specific order of enumeration
  for the for-in statement, that same enumeration order must be
  used in step 5 of this algorithm.

Practical enumeration requirements
----------------------------------

The E5 requirements for enumeration are rather loose; for instance, there
is no requirement that object keys are enumerated in their insertion order
or even that array indexes are enumerated in an ascending order.  However,
real world code sometimes makes such assumptions.

For instance, it is a common idiom to assume that the following works::

  var a = [ "foo", "bar", "quux" ];
  for (var i in a) {
    print(i, a[i]);
  }

Many programmers expect this to print::

  0 foo
  1 bar
  2 quux

where it might just as well, while being fully E5 compliant, print::

  2 quux
  0 foo
  1 bar

Similarly, much existing code assumes that properties are enumerated
in the order they were inserted.  See, for instance:

* http://code.google.com/p/chromium/issues/detail?id=2605

* http://ejohn.org/blog/javascript-in-chrome/

  "However, specification is quite different from implementation.
  All modern implementations of ECMAScript iterate through object
  properties in the order in which they were defined. Because of
  this the Chrome team has deemed this to be a bug and will be fixing it."

* ``hobject-enumeration.rst`` for practical testing results with
  actual implementations.

We impose the following additional requirements for compatibility:

* Non-array-index keys should be enumerated in their insertion order.

* The keys for ``Array`` elements should be enumerated in an
  ascending order, and before non-array-index keys.

  + This is currently provided for all objects with an array part.
    Ecmascript ``Array`` instances should thus always have an array
    part (at least when they are created).

  + If an object has an array part which is abandoned, e.g. because
    the array becomes too sparse, the enumeration ordering reverts
    to enumerating entries in insertion order (regardless of whether
    the property is a valid array index or not).

* All keys of a certain object should be enumerated (including both
  array index and non-array-index keys) before proceeding to the
  prototype.  Keys already enumerated must not be repeated during
  enumeration even if they occur again in the prototype chain.

* If an entry is deleted during enumeration before it has appeared
  in the enumeration sequence, it must not turn up later in the
  enumeration.

* A certain key must never appear twice in the enumeration sequence,
  despite any mutation.

* A key which was present during the "initialization" of the enumeration
  (before the first key was enumerated) must not be omitted from the
  enumeration sequence, if they are not deleted during enumeration
  (before they have appeared in the enumeration sequence).

Note the following *non-requirements*:

* New entries added during enumeration are not required to show up
  during the enumeration in progress.

  + The current implementation will *never* enumerate such keys.
    This is not desirable as such, but is a side effect of the
    (simplistic) implementation strategy.

  + The same behavior seems to apply to smjs, Rhino, and V8 at the
    time of writing.

Implementation issues
---------------------

It is tempting to implement enumeration with sort of enumerator state
which maintains some iteration pointers or indices to the target object
and steps through object properties and the (immutable) prototype chain
on request.  However, this approach has many practical difficulties:

* Object mutation may cause the internal structure of the target object
  (or any object in its prototype chain) to change.

  + This poses a problem for any approach based on maintaining an index
    to the array/entry part, as an index may be invalidated by internal 
    data structure maintenance such as compaction of keys.

  + This problem can be avoided if the object is "frozen" for enumeration,
    but this requires awkward book-keeping, which must work even if errors
    are thrown, threads yield (and perhaps never resume, or are garbage
    collected) etc.

* Any keys may be deleted during enumeration.

  + This poses a problem for any approach based on maintaining a key
    based state, e.g. "current key".  The key in question may be
    deleted; how can one then find the next key in the sequence?

Current enumeration mechanism
-----------------------------

The current enumeration approach is based on creating an internal
enumeration object (enumerator) when enumeration is initialized, before
any keys are needed.  The entire list of enumerated keys is generated
during initialization and stored in the enumerator.  This avoids any
issues with mutation, because no user code runs while the enumerator
is being initialized.  This step is memory intensive; in particular,
all enumerated array index keys are interned.

The enumerator is then used to generate a sequence of keys on demand.
When a new key is requested, the enumerator advances to the next key
in its internal sequence.  The key is then checked to ensure it has
not been deleted during enumeration; if it has, we skip and try the
next key in the sequence.

Note that the key may be deleted and still found in an ancestor (and
should still be enumerated)::

  function F() {};
  F.prototype = { "foo": "inherited" };

  var a = new F();
  a.bar = "skip";
  a.foo = "own";

  // enumeration order: "bar", "foo"
  for (var i in a) {
    delete a.foo;  // only affects 'a', not F.prototype
    print(i, a[i]);
  }

This is expected to print::

  bar skip
  foo inherited

However, behavior seems to differ across implementations: V8 and Smjs
work as above, while Rhino does not enumerate ``"foo"``.  Rhino *will*
enumerate ``foo`` if the for-loop is executed twice.  There are other
corner cases in enumeration too, see test cases.

Suppose the enumeration target is::

  x = [ "foo", "bar", "quux" ];
  x.foo = "bar";

After initialization, the internal enumerator object would contain
the following::

  e = {
    // internal control properties first
    "_Target": (target object),
    "_Next": (numeric index),

    // followed by enumeration keys
    // (as properties, not array entries)
    "0": true,
    "1": true,
    "2": true,
    "foo": true
  }

The enumerator object takes advantage of two features:

#. Keys inserted into an object maintain their order in the entry part
   (even during resizes).  Thus, we can insert keys into the enumerator
   and trust that their order is maintained.  The entry part is always
   gap-free, i.e. there are no NULL keys in the sequence.

#. Inserting enumerated keys as properties instead of array entries
   allows duplicate keys to be handled correctly.  Duplicate keys
   may occur when the prototype chain is walked.  The first occurrence
   is recorded in its correct position, and any later occurrences are
   ignored.

The ``_Next`` internal property is a numeric index which indicates
where to find the next potential key.  It is an index to the *entry
part* of the enumerator, and it's initialized to the value 2 because
positions 0 and 1 are taken by ``_Target`` and ``_Next`` properties,
respectively.  Entry index 2 in the example above is the "0" key.  This
needs to be managed carefully as the indexing model depends on the entry
part having a very specific, unchanged form.

Note that the enumerator is not mutated after its creation, so this
entry part index approach is reliable.  It is reliable even if the
enumerator is resized, as long as properties are not deleted; that
would cause problems in a resize, when the entry part was compacted.

Notes
-----

The current implementation has some nice qualities:

* It is very simple and robust, and avoids any issues with mutation
  (except that keys added during mutation are never enumerated, which
  is not nice but a common feature in other implementations, too).

* It has small code space.

* It has minimal impact on anything else, e.g. it requires no co-operation
  from the object, such as avoiding key compaction until enumeration is
  over.

However, it has many drawbacks:

* It has a relatively large memory footprint for the enumerator.  Because
  the keys are stored as key-value properties (not as array entries), each
  enumerated key takes about 13 bytes on a typical 32-bit architecture
  (4 bytes for key, 8 bytes for value, 1 byte for flags).

  + This footprint could be reduced somewhat by using the property-based
    approach to generate the enumeration sequence (eliminating duplicate
    keys etc), and then converting that to an array; array entries typically
    take 8 bytes.  But this would temporarily increase memory footprint
    even more.

* Numeric key indices of an array part or the virtual numeric key indices
  of a ``String`` object are interned and are reachable simultaneously
  during enumeration.

* Execution of program code stops while the enumerator is initially
  created.  This probably has little impact in most cases, but it might
  be an issue if a very large object is being enumerated (consider for
  instance enumerating a very large array).

As a concrete illustration of some of the drawbacks, consider the
following::

  var a = [... large array of 1 million elements ...];
  for (var i in a) {
    print(i, a[i]);
  }

The enumerator created in this example would contain a million interned
keys for "0", "1", and so on.  *All* of these keys would remain reachable
for the entire duration of the enumeration.  The following code would
perform *much* better (and would be more portable, as it makes no
assumptions on enumeration order)::

  var a = [... large array of 1 million elements ...];
  var n = a.length;
  for (var i = 0; i < n; i++) {
    print(i, a[i]);
  }

This problem could be alleviated a bit by deleting any already-enumerated
keys from the enumerator as enumeration proceeds.  Care would then have
to be taken to avoid any possibility of a resize of the object to avoid
mixing up the key order (due to key compaction).  This might not be easy
to arrange, if GC is allowed to opportunistically compact objects
(at least in "emergency GC" mode).  A much easier approach would be to
replace enumeration entry keys with a fixed string (like the empty string)
instead of deleting them.  This would still free the string resources but
keep the object property key list intact.  However, it would lead to
duplicate keys in the entry part, which needs careful consideration to work
correctly.

Internal properties
===================

Duktape implements E5 internal properties in differing ways, depending
on the property in question:

* concretely stored internal properties

* ``duk_hobject`` header flags

* ``duk_hobject`` structure fields (only internal prototype currently)
* implicit behaviors in specification algorithms based on e.g.
  object flags, type, or class

The current approach for storing internal properties which are not visible
to ordinary program code and never overlap with externally visible named
properties is simple: since all standard keys encode into valid UTF-8
sequences (valid CESU-8 sequences to be exact) in memory, internal properties
are prefixed with an invalid UTF-8 sequence which standard Ecmascript code
cannot generate and thus cannot access.  The current prefix is a single
``0xff`` byte.  The prefix is denoted with an underscore in this document;
e.g. ``_Map`` would be represented as the byte sequence: ``0xff`` ``'M'``
``'a'`` ``'p'`` in memory.  User C code can also use internal properties for
its own purposes, as long as the property names don't conflict with Duktape's
internal properties.

To avoid complications:

* Internal properties MUST NOT be enumerable

  - Duktape prevents enumeration of internal properties regardless of their
    ``[[Enumerable]]`` attribute.  This makes it easier for user code to
    read/write internal properties as ordinary put/get primitives can be
    used.

* Internal properties MUST NOT be visible in any other way either, e.g.
  through ``Object.getOwnPropertyNames()`` which outputs also
  non-enumerable properties

  - Duktape prevents this in each relevant built-in function.

* User Ecmascript code should not be given references to internal strings,
  i.e. strings other than valid UTF-8/CESU-8 encodings

* Untrusted Ecmascript code should have no access to buffer values or
  buffer constructors because it's easy to create an internal property
  name with buffers

User C code can access internal properties; C code has full memory access
anyway so it must be trustworthy in any case.  User code should never access
Duktape's internal properties as the internal properties may change arbitrarily
between versions.

Internal property names use a bit of shorthand and often the same internal
key is reused in many contexts.  This is simply to save a bit of memory with
minimal impact on readability.

The following table summarizes the internal properties specified in E5,
and how they are mapped to the ``duk_hobject`` implementation.  The
double brackets are omitted from the specification property names
(e.g. ``[[Class]]`` is listed as "Class").

+-------------------+------------------------------------------------------+
| Property          | Implementation                                       |
+===================+======================================================+
| Prototype         | ``duk_hobject`` struct ``prototype`` field.          |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| Class             | ``duk_hobject`` flags field, encoded as a number.    |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| Extensible        | ``duk_hobject`` flag ``DUK_HOBJECT_FLAG_EXTENSIBLE``.|
|                   |                                                      |
+-------------------+------------------------------------------------------+
| Get               | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| GetOwnProperty    | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| GetProperty       | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| Put               | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| CanPut            | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| HasProperty       | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| Delete            | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| DefaultValue      | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| DefineOwnProperty | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| PrimitiveValue    | Internal property ``_Value``.                        |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| Construct         | Not stored, implicit in algorithms.  ``duk_hobject`` |
|                   | flag ``DUK_HOBJECT_FLAG_CONSTRUCTABLE`` indicates    |
|                   | whether the object is a constructor, i.e.            |
|                   | conceptually implements the internal                 |
|                   | ``[[Construct]]`` function.  Note that all callable  |
|                   | objects are not constructable.                       |
+-------------------+------------------------------------------------------+
| Call              | Not stored, implicit in algorithms.  ``duk_hobject`` |
|                   | macro ``DUK_HOBJECT_IS_CALLABLE`` determines whether |
|                   | the object is callable, i.e. conceptually implements |
|                   | the internal ``[[Call]]`` function.  The check is    |
|                   | made using (other) object type flags, there is no    |
|                   | dedicated "callable" flag.                           |
+-------------------+------------------------------------------------------+
| HasInstance       | Not stored, implicit in algorithms.                  |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| Scope             | Internal properties ``_Lexenv`` and ``_Varenv``.     |
|                   | (Unlike E5, global and eval code are also compiled   |
|                   | into functions, hence two scope fields are needed.)  |
+-------------------+------------------------------------------------------+
| FormalParameters  | Internal property ``_Formals``.                      |
|                   |                                                      |
+-------------------+------------------------------------------------------+
| Code              | An Ecmascript function (``duk_hcompiledfunction``)   |
|                   | has a pointer to compiled bytecode and associated    |
|                   | data (such as constants), see                        |
|                   | ``duk_hcompiledfunction.h``.                         |
|                   | A C function (``duk_hnativefunction``) has a pointer |
|                   | to a C function and some related control data, see   |
|                   | ``duk_hnativefunction.h``.                           |
+-------------------+------------------------------------------------------+
| TargetFunction    | ``duk_hobject`` flag ``DUK_HOBJECT_FLAG_BOUND`` is   |
|                   | set, and the internal property ``_Target`` is set    |
|                   | to the target function.                              |
+-------------------+------------------------------------------------------+
| BoundThis         | ``duk_hobject`` flag ``DUK_HOBJECT_FLAG_BOUND`` is   |
|                   | set and the internal property ``_This`` is set       |
|                   | to the ``this`` binding.                             |
+-------------------+------------------------------------------------------+
| BoundArguments    | ``duk_hobject`` flag ``DUK_HOBJECT_FLAG_BOUND`` is   |
|                   | set and the internal property ``_Args`` is set       |
|                   | to a list of bound arguments.                        |
+-------------------+------------------------------------------------------+
| Match             | Not stored, implicit in algorithms.  Object type     |
|                   | (class number is DUK_HOBJECT_CLASS_REGEXP) determines|
|                   | whether ``[[Match]]`` is conceptually supported.     |
|                   | The compiled regexp and its flags are stored as the  |
|                   | ``_Bytecode`` internal property, whose value is an   |
|                   | internal string.                                     |
+-------------------+------------------------------------------------------+
| ParameterMap      | Internal property ``_Map``.                          |
|                   |                                                      |
+-------------------+------------------------------------------------------+

Exotic behavior and virtual properties
======================================

Terminology
-----------

The E5 specification defines default property access algorithms like
``[[GetProperty]]`` and ``[[DefineOwnProperty]]`` in E5 Section 8.12.
Some objects have behavior which differs from default behavior; we call
these *exotic properties* (or properties with *exotic behavior*), as
opposed to *normal properties* (or properties with *normal behavior* or
*default behavior*).

Conceptually each object has a number of algorithms "stored" in its
internal properties (E5 Section 8.6.2), including all the property access
algorithms.  The current implementation of property access is completely
different: there are fixed algorithms for property access, which change
their behavior based on object type and flags.  The exotic behaviors are
thus "inlined" into a single algorithm.

From a purely implementation viewpoint some properties are stored in a data
structure as concrete key-value pairs, while others are computed
on-the-fly.  The former are called *concrete properties* and the latter
*virtual properties*.  Whether a property is concrete or virtual should
have no externally visible impact with respect to compliance. 
Note that these two concepts ("being exotic" and "being virtual") are
indepedent: a exotic property can be implemented with a concrete property
storing its value, and a normal property can be implemented as a virtual
property.

Exotic behaviors in E5 specification
------------------------------------

Exotic behaviors are discussed at least in the following places in the E5
specification (page numbers refer to page numbers on the page contents, not
the "PDF page number").

Section 8.6.2, pages 32-33 summarizes exotic behavior and refers to:

* Array objects: ``[[DefineOwnProperty]]``, E5 Section 15.4.5.1

* String objects: ``[[GetOwnProperty]]``, E5 Section 15.5.5.2

* Arguments objects: ``[[Get]]``, ``[[GetOwnProperty]]``,
  ``[DefineOwnProperty]]``, ``[[Delete]]``, E5 Section 10.6
  (the exotic behavior of a non-strict arguments object is pretty
  intricate and is discussed separately in ``arguments-object.rst``)

* Function objects: ``[[Get]]``, E5 Section 15.3

Exotic behavior for ``[[Get]]``:

* The ``arguments`` object: E5 Section 10.5

  + If ``arguments.caller`` has a value, which is a strict function object,
    the ``[[Get]]`` operation fails after standard lookup is complete.

  + Note that the exotic behavior occurs at the level of ``[[Get]]`` and
    is *not* visible through property descriptors, e.g. through
    ``[[GetProperty]]`` or ``[[GetOwnProperty]]``.

  + Exotic behavior only applies to non-strict arguments objects.

* The ``Function`` object: E5 Section 15.3.5.4

  + Same exotic behavior for ``caller`` property as for ``arguments``
    object.

Exotic behavior for ``[[GetOwnProperty]]``:

* ``String`` object array-index properties: E5 Section 15.5.5.2

  + Covers properties which are valid array indexes as specified in E5
    Section 15.4, i.e. P for which ``ToString(ToUint32(P)) == P`` and
    ``ToUint32(P) != 0xffffffff``.

  + Ecmascript E5.1 extended behavior to all number-like properties,
    and thus allows strings longer than 4G characters.

* ``Array`` ``length`` property: E5 Section 15.4.5

  + May be implemented as a concrete property or as a virtual property.
    Currently implemented as a concrete property.

* The ``arguments`` object: E5 Section 10.5

  + The ``[[Value]]`` of a property descriptor may be overridden for
    "magically bound" properties (some numeric indices).

  + Exotic behavior only applies to non-strict arguments objects.

Exotic behavior for ``[[DefineOwnProperty]]``:

* ``Array`` ``length`` property: E5 Section 15.4.5.1

  + Has side effects on array elements (deleting elements above newly
    written length).

* ``Array`` index properties: E5 Section 15.4.5.1

  + Has the side effect of automatically updating array ``length``.

* The ``arguments`` object: E5 Section 10.6

  + Automatic interaction with "magically bound" variables (some
    numeric indices).  May also remove magic binding.

  + Exotic behavior only applies to non-strict arguments objects.

Exotic behavior for ``[[Delete]]``:

* The ``arguments`` object: E5 Section 10.6

  + Automatic interaction with "magically" bound variables (some
    numeric indices), may remove magic binding.

  + Exotic behavior only applies to non-strict arguments objects.

When implementing exotic or virtual properties, property attributes must
be respected normally.  Exotic or virtual properties may have specific
initial attributes, but these are not fixed and may be changed later by
user code.  The *only* properties which are "truly fixed" are:

* Non-configurable, non-writable data properties

* Non-configurable accessor properties

In particular, a data property which is non-configurable but writable
*can* be changed to non-writable (see E5 Section, step 10).  The property
cannot be changed back to writable after that.  This has the practical
implication that only "truly fixed" properties can be easily implemented
as stateless virtual properties.

Summary of exotic properties
----------------------------

The following table summarizes exotic properties defined in the E5
specification, along with their (initial) property attributes in the
columns W(ritable), E(numerable), and C(onfigurable): 
``y`` means "true", ``n`` means "false", ``a`` means "any":

+------------+------------+---+---+---+-----------------------------------+
| Object     | Property   | W | E | C | Notes                             |
+============+============+===+===+===+===================================+
| ``Array``  | ``length`` | y | n | n | Write may affect array elements   |
| instance   |            |   |   |   | (indices above new length are     |
|            |            |   |   |   | deleted)                          |
+------------+------------+---+---+---+-----------------------------------+
| ``Array``  | array      | a | a | a | Write may affect array            |
| instance   | indices    |   |   |   | ``length`` (if new index is above |
|            |            |   |   |   | existing length)                  |
+------------+------------+---+---+---+-----------------------------------+
| ``String`` | ``length`` | n | n | n | No exotic behavior as such, but   |
| instance   |            |   |   |   | easy to implement as a virtual    |
|            |            |   |   |   | property because not writable     |
|            |            |   |   |   | or configurable.                  |
+------------+------------+---+---+---+-----------------------------------+
| ``String`` | array      | n | y | n | No exotic behavior as such, but   |
| instance   | indices    |   |   |   | maps individual characters to     |
|            | inside     |   |   |   | indicates; affects enumeration.   |
|            | string     |   |   |   |                                   |
|            | length     |   |   |   |                                   |
+------------+------------+---+---+---+-----------------------------------+
| plain      | ``length`` | n | n | n | See notes below.                  |
| string     |            |   |   |   |                                   |
| value      |            |   |   |   |                                   |
+------------+------------+---+---+---+-----------------------------------+
| plain      | array      | n | y | n | See notes below.                  |
| string     | indices    |   |   |   |                                   |
| value      | inside     |   |   |   |                                   |
|            | string     |   |   |   |                                   |
|            | length     |   |   |   |                                   |
+------------+------------+---+---+---+-----------------------------------+
| Arguments  | some       | y | y | y | Some numeric indices of an        |
| object,    | numeric    |   |   |   | arguments object "magically bind" |
| non-strict | indices    |   |   |   | to formal arguments.  Only affects|
|            |            |   |   |   | a non-strict arguments object.    |
+------------+------------+---+---+---+-----------------------------------+
| Arguments  | ``caller`` | a | a | a | If *value* of ``caller`` property |
| object,    |            |   |   |   | is a strict function, ``[[Get]]`` |
| non-strict |            |   |   |   | fails (but ``[[GetOwnProperty]]`` |
|            |            |   |   |   | does not!).  Only affects a       |
|            |            |   |   |   | non-strict arguments object.      |
+------------+------------+---+---+---+-----------------------------------+

Notes:

* The exotic properties for ``String`` instances (which are objects) also
  apply in practice to plain strings, because properties of plain strings
  can also be accessed (the string is automatically promoted to a temporary
  object; the implementation handles this without an actual temporary object
  being created).

* The ``caller`` property of a non-strict arguments object is curious: it has
  exotic behavior but no such property is established for non-strict argument
  objects.  (This is why its property attributes are listed as "any" above.)

* The only exotic properties which are easy to implement as fully virtual,
  stateless properties are the ``String`` instance ``length`` and
  array index properties, because they are non-configurable and non-writable.
  They are enumerable, though, which must be taken into account in enumeration.

* The array ``length`` property has an initial value which is a valid array
  length (32-bit unsigned integer).  The exotic behavior of the property
  ensures that whatever values are assigned to it, they are either rejected
  or coerced into a valid array length (32-bit unsigned integer).

Implementation of exotic properties
-----------------------------------

The following table summarizes the implementation of exotic properties at
the moment.

+------------+------------+-----------------------------------------------+
| Object     | Property   | Description                                   |
+============+============+===============================================+
| ``Array``  | ``length`` | Stored as a concrete property.                |
| instance   |            | ``DUK_HOBJECT_FLAG_EXOTIC_ARRAY`` enables     |
|            |            | exotic behavior in:                           |
|            |            | ``duk_hobject_put_value()``,                  |
|            |            | ``duk_hobject_object_define_property()``.     |
+------------+------------+-----------------------------------------------+
| ``Array``  | array      | Stored as conrete properties in array part    |
| instance   | indices    | or entry part (if array part abandoned).      |
|            |            | ``DUK_HOBJECT_FLAG_EXOTIC_ARRAY`` enables     |
|            |            | exotic behavior in:                           |
|            |            | ``duk_hobject_put_value()``,                  |
|            |            | ``duk_hobject_object_define_property()``.     |
+------------+------------+-----------------------------------------------+
| ``String`` | ``length`` | Virtual property computed from the string     |
| instance   |            | length of the internal ``_Value`` property.   |
|            |            | ``DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ`` enables |
|            |            | exotic behavior in:                           |
|            |            | ``get_own_property_desc()``.                  |
+------------+------------+-----------------------------------------------+
| ``String`` | array      | Virtual properties computed by looking up     |
| instance   | indices    | characters of the internal ``_Value``         |
|            | inside     | property.                                     |
|            | string     | ``DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ`` enables |
|            | length     | exotic behavior in:                           |
|            |            | ``get_own_property_desc()``.                  |
+------------+------------+-----------------------------------------------+
| plain      | ``length`` | Exotic handling in property access code,      |
| string     |            | return string character length without        |
| value      |            | promoting the plain string value to a         |
|            |            | temporary ``String`` instance.                |
+------------+------------+-----------------------------------------------+
| plain      | array      | Exotic handling in property access code,      |
| string     | indices    | return individual character value without     |
| value      | inside     | promoting the plain string value to a         |
|            | string     | temporary ``String`` instance.                |
|            | length     |                                               |
+------------+------------+-----------------------------------------------+
| Arguments  | some       | Stored as concrete property values.           |
| object,    | numeric    | ``DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS`` enables |
| non-strict | indices    | exotic behavior in:                           |
|            |            | ``get_own_property_desc()``,                  |
|            |            | ``duk_hobject_get_value()``,                  |
|            |            | ``duk_hobject_put_value()``,                  |
|            |            | ``duk_hobject_delete_property()``,            |
|            |            | ``duk_hobject_object_define_property()``.     |
+------------+------------+-----------------------------------------------+
| Arguments  | ``caller`` | Stored as a concrete property.                |
| object,    |            | ``DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS`` enables |
| non-strict |            | exotic behavior in:                           |
|            |            | ``duk_hobject_get_value()``.                  |
|            |            | This exotic behavior only affects             |
|            |            | ``[[Get]]``, it is not visible through e.g.   |
|            |            | property descriptors or                       |
|            |            | ``[[GetOwnProperty]]``.                       |
+------------+------------+-----------------------------------------------+

Notes:

* The only virtual properties are ``String`` object ``length`` and array
  index properties.  These are easy to implement as virtual properties
  because they are non-configurable and non-writable.  However, they *are*
  enumerable which affects enumeration handling.

* If array ``length`` becomes non-writable, the exotic behavior ensures
  no elements above the specified length can ever be inserted.  The array
  part could thus be compacted without risk of it being extended afterwards.

Notes on array length and array indices
---------------------------------------

The exotic array semantics only apply to valid array indices.  Nothing
prevents user code from writing to numeric array indices higher than the
maximum valid array index, but such writes will get no exotic behavior::

  var x = [];
  x["4294967294"] = 1;    // has array semantics, updates length
  print(x.length);        // length is 4294967295

  x["9999999999"] = 2;    // no array semantics
  print(x.length);        // length is still 4294967295

  print(x[4294967294]);   // coerced to string -> value "1"
  print(x[9999999999]);   // like above -> value "2"

  print(Object.keys(x));  // -> ["4294967294", "999999999"]

  x.length = 0;           // deletes valid array indices
  print(x[4294967294]);   // undefined
  print(x[9999999999]);   // value still "2", not auto-deleted

Note that assigning an ``undefined`` value extends the array length, as
it is a valid value::

  var x = [];
  print(x.length);        // -> 0
  x[10] = undefined;
  print(x.length);        // -> 11

  print(Object.keys(x));  // -> [ "10" ]

Internal objects
================

The following internal objects are currently used:

* Function templates which are "instantiated" into concrete closures

* A declarative environment record

* An object environment record

* Function formals name list

* Function variable map

Internal objects don't always need Ecmascript properties like:

* Enumeration order

* Property attributes

* Prototype chain

The current implementation does not take advantage of these: internal
objects are handled just like Ecmascript objects.

Function instances
==================

The creation of function instances is described in E5 Section 13.2.
Each function instance (each closure created from a function
expression or declaration) has the following properties:

* ``length``

* ``prototype``: points to a fresh object which has a ``constructor``
  property pointing back to the function

* ``caller``: thrower (strict functions only)

* ``arguments``: thrower (strict functions only)

There is considerable variance in practical implementations:

* smjs::

    // the "name" property is non-standard; "arguments" and "caller" are
    // present for a non-strict function

    js> f = function foo() {}
    (function () {})
    js> Object.getOwnPropertyNames(f)
    ["prototype", "length", "name", "arguments", "caller"]

    // for strict mode, the same properties are present.

    js> f = function foo() { "use strict"; }
    (function foo() {"use strict";})
    js> Object.getOwnPropertyNames(f);
    ["prototype", "length", "name", "arguments", "caller"]

    // the "name" property contains the function expression name

    js> f.name
    "foo"

    // "name" is non-writable, non-configurable (and non-enumerable)
    // -> works as a reliable "internal" property too

    js> Object.getOwnPropertyDescriptor(f, 'name')
    ({configurable:false, enumerable:false, value:"foo", writable:false})

* nodejs (v8)::

    // "name" is non-standard; "arguments" and "caller" are present
    // for even a non-strict function

    > f = function foo() {}
    [Function: foo]
    > Object.getOwnPropertyNames(f)
    [ 'length',
      'caller',
      'arguments',
      'name',
      'prototype' ]
    > f.name
    'foo'

    // strict mode is the same

    > f = function foo() { "use strict"; }
    [Function: foo]
    > Object.getOwnPropertyNames(f)
    [ 'name',
      'length',
      'arguments',
      'prototype',
      'caller' ]

    // 'name' is writable but not configurable/enumerable

    > f.name
    'foo'
    > Object.getOwnPropertyDescriptor(f, 'name')
    { value: 'foo',
      writable: true,
      enumerable: false,
      configurable: false }

* rhino::

    // "name" is non-standard, "arity" is non-standard, "arguments"
    // is present (but "caller" is not)

    js> f = function foo() {}
    [...]
    js> Object.getOwnPropertyNames(f)
    arguments,prototype,name,arity,length

    // name is non-writable, non-enumerable, non-configurable

    js> pd = Object.getOwnPropertyDescriptor(f, 'name')
    [object Object]
    js> pd.writable
    false
    js> pd.enumerable
    false
    js> pd.configurable
    false

    // strict mode functions are similar

See ``function-objects.rst`` for more discussion.

Built-in functions
------------------

The properties of built-in functions are a special case, because
they are not created with the algorithm in E5 Section 13.2;
instead, their properties are described explicitly in E5 Section 15.

There is considerable variance between implementations on what
properties built-in functions get.

Design notes and future work
============================

ES6 Proxy objects or Lua-like metatables
----------------------------------------

It would be nice to have a Lua metatable like mechanism for creating
custom object behavior extensions and full object virtualization,
see http://www.lua.org/pil/13.html for a description of Lua metatables.
There is a similar mechanism in the Ecmascript 6 draft called "Proxy
object":

* https://people.mozilla.org/~jorendorff/es6-draft.html#sec-proxy-object-internal-methods-and-internal-slots

The ES6 Proxy object is of course a natural target for implementation,
but it's not clear what the underlying mechanism should be: should the
Proxy object mechanism be the internal mechanism, or should there be some
other implementation specific mechanism underneath which is used to provide
the Proxy object implementation but also provide non-standard additional
features?

Nice-to-have features:

* Sufficient for creating arbitrary "host objects"

* Sufficient for providing array-like access to byte buffers

* Allow "full virtualization" of E5 semantics

Some notes:

* Should interact reasonably with the E5 object model, e.g. property
  descriptors.

* Should metatable behavior only affect non-existent properties (as in
  Lua)?  To apply it to all properties, simply use an empty table.

* May require raw access functions for dealing with the underlying
  properties.

* What's the best level for capturing operations?

  a. Concrete, exposed operations like getprop, putprop, hasprop, delprop,
     Object.getOwnPropertyDescriptor(), Object.defineProperty(), etc?

  b. Specification functions like ``[[GetOwnProperty]]``,
     ``[[DefineProperty]]`` etc?

* In addition, should cover:

  + Enumeration
  + Getting object ``[[Class]]``
  + Garbage collection => finalizer

ES6 features
------------

There are many ES6 features which may need changes to the basic object model.

For instance, there are keyed collections:

* https://people.mozilla.org/~jorendorff/es6-draft.html#sec-keyed-collection

The ``Map`` object provides an arbitrary collection of key/value pairs,
where keys and values can be arbitrary Ecmascript objects.  This is very
useful compared to the standard Ecmascript object whose keys can only be
strings.

How to implement this?  One could extend the basic model to provide enough
functionality to implement ``Map`` values.  One could also implement them
entirely separate from the basic object model.  The trouble is, the ``Map``
keys and values must be GC reachable so they have to reside either in basic
objects or there needs to be additional native structure for them.

Array-like access to underlying byte buffers
--------------------------------------------

There are various proposals for typed access to an underlying buffer.
For instance:

* http://www.khronos.org/registry/typedarray/specs/latest/
* http://nodejs.org/docs/v0.4.7/api/buffers.html

See ``buffers.txt``.

WindowProxy
-----------

The HTML5 WindowProxy object seems to require behavior outside E5.

See: http://www.w3.org/TR/html5/browsers.html#windowproxy.

The ES6 Proxy object probably addresses this.

Script origin for security checks
---------------------------------

HTML5 web storage, as an example, uses 'script origin' to do required
security checks on property access.

Possible implementation e.g. through: a metatable-like mechanism for
capturing accesses, and some way to introspect the current caller or
call chain to check script origin.

It would be nice to be able to determine an origin for every function,
mapped to an Ecmascript object with a bunch of propeties like URI,
load time.

Alternatives to current entry part
----------------------------------

Are there better alternatives to the current entry part model?

The pointer computations have a run-time performance cost and also a
code footprint cost.  Is it worth it?  This depends on the number of
objects.  For instance, if there are 1000 objects and a pointer to the
array part and hash part are added to ``duk_hobject``, this would have
a footprint of 8000 extra bytes on a 32-bit platform.

Hash algorithm notes
--------------------

Some hash algorithm goals:

* Minimal memory allocation

* High load factor (minimizes memory use)

* Small code space

Closed hashing (open addressing) provides fixed allocation, but requires a
"probe sequence" to deal with hash collisions.  Options for dealing with
collisions include:

* http://en.wikipedia.org/wiki/Linear_probing

* http://en.wikipedia.org/wiki/Quadratic_probing

* http://en.wikipedia.org/wiki/Double_hashing

* http://en.wikipedia.org/wiki/Cuckoo_hashing

Notes on current solution:

* Linear probing is more cache efficient but requires a lower load (= higher
  allocated size relative to used size) to avoid 'clustering' issues.
  Current approach prefers compact object size over cache efficiency.

* Slightly better probe would be: ``probe_steps[(hash >> 16) % 32]``.
  This could possibly correlate less with the initial hash value, but
  requires an extra shift operation.  Most likely not worth it.

* The current algorithm is not very good, as it requires the load factor
  to be relatively low (around 70-80%) to be efficient.  Much better
  results are possible.  This is definite future work.

Property key ordering
---------------------

Because of the practical enumeration requirements, a data structure for
object keys must both maintain keys in their insertion order, and also
support some form of indexed access (e.g. a hash table) to work efficiently
with large objects.

One such approach is to use an "ordered hash table" which maintains
entries both in the hash structure and a linked list for order.

The current approach is to maintain a simple, ordered entry part, and
then provide an optional hash table on top of that.  This has some
nice properties:

* The entry part maintains key ordering trivially.

* The hash part is optional, which minimizes object size.  For small
  objects a linear scan of keys is also relatively efficient because
  the keys are adjacent in memory (being in the own "sub array").

* The hash part is non-critical.  For instance, it could be dumped in an
  emergency out of memory situation without losing any information.

* It would be relatively straightforward to support multiple hash
  algorithms for e.g. different object sizes.  In particular, it might
  be useful to have a variant for medium size objects, where entry part
  indices could be 8-bit or 16-bit values (instead of 32-bit).

What are the practical alternatives?

Separate object type for internal objects
-----------------------------------------

There used to be a separate object type for internal objects.

The differences to standard objects include: no need to support
array part, no property attributes, no accessor properties,
no need to preserve property ordering, no exotic behaviors, etc.

However, the extra cost of having another object data structure
does not seem worth it.  The effects are:

* Code size is increased by several kilobytes.

* Internal objects data size decreases slightly (no need to track
  property attributes, for instance).

* Internal object property lookup is slightly more performant.

Currently it seems to make more sense to use the same object
abstraction but to provide "short cut" raw property accessors
for faster / simpler accessing, if necessary.

Array part "freezing" vs abandoning
-----------------------------------

The current policy is that the array part is either actively used and contains
all array indexed properties, or it is abandoned entirely and all entries
moved to the entry part.  An alternative design would allow some array entries
to be in the array part and others in the entry part.

For instance, the array part could be "frozen" and entries above the
allocated size could reside in the entry part.  This is an easy and
relatively straightforward policy too:

* If the "frozen" flag is set, never resize the array part (except perhaps
  to abandon it entirely).

* If an array index key is not found from the array part, and the "frozen"
  flag is *not* set, no need to look at the entry part (this would be the
  common case).  If the "frozen" flag is set, need to look at the entry
  part, too.

This policy has the nice side effect that there is no need to necessarily
support array abandoning at all.  However, without array abandoning the
frozen array part could be left arbitrarily sparse after deletions.

Array size management improvements
----------------------------------

* When to do compaction checks (e.g. property deletion)?

* Periodic actual density checks (relative to e_size + a_size to keep
  average cost limited)?

* Triggered density checks?

More fast paths for array indexing
----------------------------------

It would be nice to have more fast paths (intern free accesses) for the
array part.  In particular, it would be very nice to have a fast path for
``[[Put]]``, at least for the very common case of a plain Array value
inheriting from ``Array.prototype`` with no array indexes entries anywhere
in the prototype chain.

How to do the prototype chain check required by ``[[Put]]`` efficiently?
In other hands, how to the check *without interning the index* that:

* The property does not exist in any ancestor.

* Or if it exists in an ancestor, the write is not prevented by a 
  non-writable property or "captured" by an accessor property.

Some approaches:

* Rework the property lookup to do lazy interning in general.

* Add an object flag ``DUK_HOBJECT_FLAG_NO_ARRIDX_PROPS``.

  + If set, the flag guarantees that the object has no concrete or virtual
    array indexed properties.

  + The prototype walking check can then simply check that no object in the
    prototype chain has this flag cleared.  The flag must be cleared for e.g.
    ``String`` objects which have virtual array indexed properties.

  + Unfortunately the ``Array`` prototype is itself an array, see E5 Section
    15.4.4.  However, it normally has no array elements, so it could have the
    flag set initially, and if someone set an array index to the prototype
    (which does not really make sense) the flag would be cleared, wrecking
    performance.

  + Whenever adding a new property (by whichever means) and the key is an
    array index, the flag must be cleared for the target object.

Note that the fast path really only needs to work for ``Array`` instances
in practice.  Their prototype chain is::

      null
       ^
       |
  Object prototype (E5 Section 15.2.4)
       ^
       |
  Array prototype (E5 Section 15.4.4)
       ^
       |
  Array instance (see E5 Section 15.4.2)

The exotic flag based approach would work if the object and array prototypes
have no array indexed keys.

Existing Ecmascript implementations do not seem to implement the
official ``[[Put]]`` algorithm accurately, especially for checking the
ancestor properties for protection.  For instance, NodeJS / V8::

  > var p = Array.prototype;
  > Object.defineProperty(p, '0', { 'value': 'inherited', 'writable': false });
  []
  > var a = [];
  > a[0] = 'own';  // should fail
  'own'
  > console.log(a[0]);  // but does not
  own

NodeJS / V8 doesn't seem to respect ``[[Put]]`` for normal objects either::

  > var p = Object.prototype;
  > Object.defineProperty(p, 'foo', { 'value': 'inherited', 'writable': false });
  {}
  > var o = {};
  > o.foo = 'own';  // should fail
  'own'
  > console.log(o.foo);  // but does not
  own

Rhino respects the algorithm in both cases::

  Rhino 1.7 release 3 2012 02 13
  js> var p = Array.prototype;
  js> Object.defineProperty(p, '0', { 'value': 'inherited', 'writable': false });
  inherited
  js> var a = [];
  js> a[0] = 'own';  // should fail
  own
  js> print(a[0]);
  inherited

and::

  Rhino 1.7 release 3 2012 02 13
  js> var p = Object.prototype;
  js> Object.defineProperty(p, 'foo', { 'value': 'inherited', 'writable': false });
  [object Object]
  js> var o = {};
  js> o.foo = 'own';  // should fail
  own
  js> print(o.foo);
  inherited

Similarly for Smjs::

  js> var p = Array.prototype;
  js> Object.defineProperty(p, '0', { 'value': 'inherited', 'writable': false });
  ["inherited"]
  js> var a = [];
  js> a[0] = 'own';  // should fail
  "own"
  js> print(a[0]);
  inherited

and::

  js> var p = Object.prototype;
  js> Object.defineProperty(p, 'foo', { 'value': 'inherited', 'writable': false });
  ({})
  js> var o = {};
  js> o.foo = 'own';
  "own"
  js> print(o.foo);
  inherited

Delete keys from enumerator state during enumeration
----------------------------------------------------

It would be nice to delete keys from enumerator state once the keys have
been enumerated.  This would reduce memory pressure as quickly as it is
currently possible.  Care must be taken to avoid a resize and key compaction
while enumeration is running.

A more practical alternative is to replace the already enumerated keys with
a fixed string (e.g. the empty string).  This would allow the enumerated keys
to be freed as the enumeration progresses.  However, this would create duplicate
keys in the entry part, something which is not currently allowed and would thus
need careful consideration.

Enumeration ordering vs. mutation
---------------------------------

Enumeration vs. mutation causes a lot of implementation headache, as no
internal reorganizations (hash part resizing, array part reallocation or
dropping, compactions) can cause enumeration to fail. There are at least
three basic approaches to manage these:

#. Freeze the object when one or more enumerations are in progress.
   However, to do this, enumerations must be tracked which complicates
   execution.

#. Design the data structures to work with enumeration (especially mutation
   during enumeration) so that the object does not need to know that
   enumerations are in progress.  This is difficult.

#. Create a snapshot of keys to-be-enumerated when the enumerator is
   created.  This wastes memory but is guaranteed to work - although newly
   added keys will not show up in the enumeration (which is compliant
   behavior).

The current approach is to create a snapshot of keys and then re-check them
to see whether they've been deleted during enumeration.

More modular virtual/exotic properties
---------------------------------------

More modular (but still compact) way of implementing virtual and exotic
properties?

Test cases
----------

Black box and white box test cases.
