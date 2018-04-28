=========================
Object enumeration issues
=========================

This document provides some design notes for pre-ES2015 and ES2015/ES2016
enumeration.

Key order during enumeration
============================

ECMAScript E3 or E5 do not require a specific key order during enumeration.
However, some existing code apparently relies on some ordering behavior:

* http://code.google.com/p/chromium/issues/detail?id=2605

* http://ejohn.org/blog/javascript-in-chrome/

    However, specification is quite different from implementation. All modern
    implementations of ECMAScript iterate through object properties in the
    order in which they were defined. Because of this the Chrome team has
    deemed this to be a bug and will be fixing it.

User code seems to rely roughly on the following order:

* For arrays, return all used array index keys in ascending order first

* Then return all other keys in the order in which they have been first
  defined

* The properties of the object itself are enumerated first, followed by
  its prototype's properties, and so on

ES5 doesn't guarantee a specific ordering for enumeration.  ES2015 and ES2016
also don't guarantee a specific ordering for ``for-in`` and ``Object.keys()``
but does guarantee ordering for e.g. ``Object.getOwnPropertyNames()``.

Specification (E6 and E7)
=========================

The situation seems unchanged from ES5 for ``for-in`` and ``Object.keys()``:

* For-in:

  - ES2015 http://www.ecma-international.org/ecma-262/6.0/#sec-runtime-semantics-forin-div-ofheadevaluation-tdznames-expr-iterationkind
    algorithm step 7 is taken (iterationKind is "enumerate"), target is
    ToObject() coerced and ``[[Enumerate]]`` is applied.
    http://www.ecma-international.org/ecma-262/6.0/#sec-ordinary-object-internal-methods-and-internal-slots-enumerate
    states "The mechanics and order of enumerating the properties is not
    specified but must conform to the rules specified below.".

  - ES2016 calls ``EnumerateObjectProperties()``,
    http://www.ecma-international.org/ecma-262/7.0/#sec-enumerate-object-properties,
    which has the same requirements as ES2015:
    "The mechanics and order of enumerating the properties is not specified
    but must conform to the rules specified below.".

* Object.keys():

  - ES2015 http://www.ecma-international.org/ecma-262/6.0/#sec-object.keys
    states: "If an implementation defines a specific order of enumeration for
    the for-in statement, the same order must be used for the elements of the
    array returned in step 4."  The algorithm references EnumerableOwnNames,
    http://www.ecma-international.org/ecma-262/6.0/#sec-enumerableownnames,
    which states "The order of elements in the returned list is the same as the
    enumeration order that is used by a for-in statement.".

  - ES2016 calls ``EnumerableOwnNames()``,
    http://www.ecma-international.org/ecma-262/7.0/#sec-enumerableownnames,
    whose step 5 says "Order the elements of names so they are in the same
    relative order as would be produced by the Iterator that would be returned
    if the EnumerateObjectProperties internal method was invoked with O.".
    So the guarantees are the same as for ``for-in`` in ES2016 too.

There are differences to ES5 in the following:

* ``Object.getOwnPropertyNames()`` (a binding already present in ES5)

  - ES5 has no guarantees for key ordering:
    http://www.ecma-international.org/ecma-262/5.1/#sec-15.2.3.4;
    the section just states "For each named own property P of O".

  - ES2015 relies on ``GetOwnPropertyKeys()`` operation:
    http://www.ecma-international.org/ecma-262/6.0/#sec-object.getownpropertynames
    which in turn calls ``[[OwnPropertyKeys]]``
    For ordinary objects, ES2015 ``[[OwnPropertyKeys]]`` provides the ordering
    often referred to as "the ES2015 enumeration order",
    http://www.ecma-international.org/ecma-262/6.0/#sec-ordinary-object-internal-methods-and-internal-slots-ownpropertykeys:

    + For each own property key P of O that is an integer index, in ascending
      numeric index order ...

    + For each own property key P of O that is a String but is not an integer
      index, in property creation order ...

    + For each own property key P of O that is a Symbol, in property creation
      order ...

  - ES2016 matches ES2015 and invokes ``GetOwnPropertyKeys()``.

* ``Object.getOwnPropertySymbols()`` is new in ES2015 and has the same ordering
  guarantees as above.  In practice, because it only returns symbols, the
  symbols must be returned in insertion order.

The ``[[OwnPropertyKeys]]`` ordering is what's typically referred to as the
"ES2015 enumeration order".  Most engines, including Duktape 2.x, use it also for
``for-in`` and ``Object.keys()`` even if it's not required for them.

Specification (E5)
==================

A specific ordering is not required:

* Section 12.6.4 (The for-in statement):

    The mechanics and order of enumerating the properties [...] is not
    specified.

However, if an implementation provides a consistent ordering, it must do
so in all relevant situations:

* Section 15.2.3.7 (Object.defineProperties):

    If an implementation defines a specific order of enumeration for the
    for-in statement, that same enumeration order must be used to order
    the list elements in step 3 of this algorithm.

* Section 15.2.3.14 (Object.keys):

    If an implementation defines a specific order of enumeration for the
    for-in statement, that same enumeration order must be used in step 5
    of this algorithm.

As a side note, E5 defines a specific meaning for a "sparse" array in
Section 15.4: an array is sparse essentially if it contains one or more
"undefined" values in the range [0,length-1].  The "sparse" term used
occasionally in the Duktape implementation is unfortunately slightly
different (sparse enough to cause array part to be abandoned).

Rhino 1.7 release 2 2010 02 06
==============================

For objects (no prototype)
--------------------------

::

  js> var x = {};
      x.foo = 1;
      x.bar = 1;
      x[0] = 1;
      x.quux = 1;
      x[3] = 1;
      x[1] = 1;
      x.foo = 2;
      for (var i in x) { print(i); }
  foo
  bar
  0
  quux
  3
  1

The behavior is consistent: all keys (including array indices) are returned
in the order in which they are first defined.  If a key is deleted and
re-added, its enumeration order changes::

  js> var x = {};
      x.foo = 1;
      x.bar = 1;
      for (var i in x) { print(i); };
  foo
  bar
  js> delete x.foo;
      x.foo = 1;
      for (var i in x) { print(i); };
  bar
  foo

For arrays (no prototype)
-------------------------

::

  js> var x = [];
      x.foo = 1;
      x[0] = 1;
      x[3] = 1;
      x[1] = 1;
      x.bar = 1;
      for (var i in x) { print(i); };
  0
  1
  3
  foo
  bar

For small, dense arrays, the behavior is consistent: array keys (with
defined values) are enumerated first, followed by keys in definition order.

However, this behavior breaks down with sparse arrays::

  // still OK
  js> var x = [];
      x.foo = 1;
      x[0] = 1;
      x[8] = 1;
      x[5] = 1;
      x.bar = 1;
      for (var i in x) { print(i); };
  0
  5
  8
  foo
  bar

  // 1000 appears after keys
  js> x[1000] = 1;
      for (var i in x) { print(i); };
  0
  5
  8
  foo
  bar
  1000

  // ... and is also followed by a newly defined key
  js> x.quux = 1;
      for (var i in x) { print(i); };
  0
  5
  8
  foo
  bar
  1000
  quux

  // here '9' is higher than last well-behaving index (8) but still
  // enumerates before string keys -- while '10' enumerates like
  // a string key
  js> x[10] = 1; x[9] = 1; for (var i in x) { print(i); };
  0
  5
  8
  9
  foo
  bar
  1000
  quux
  10

Objects (with prototype)
------------------------

One prototype level::

  js> function F() { }
      F.prototype = { foo: 1, bar: 1 };
      x = new F();
      x.abc = 1;
      x.quux = 1;
      for (var i in x) { print(i); }
  abc
  quux
  foo
  bar

Object's keys are enumerated first, then prototype's keys.  Prototype
keys with same name as properties of the object are not enumerated::

  js> function F() { }
      F.prototype = { foo: 1, bar: 1 };
      x = new F();
      x.quux = 1;
      x.foo = 1;
      x.xyz = 1;
      for (var i in x) { print(i); }
  quux
  foo
  xyz
  bar

Here ``foo`` is not enumerated again because it was already enumerated
as part of the object's keys.

Object with an Array prototype
------------------------------

::

  // test 1
  js> function F() { }
      F.prototype = [1,2,3];
      x = new F();
      print("length: " + x.length);
      for (var i in x) { print(i); }
  length: 3
  0
  1
  2

  // test 2
  js> x[1] = 9;
      print("length: " + x.length);
      for (var i in x) { print(i); }
  length: 3
  1
  0
  2

  // test 3
  js> x.length = 2;  // sets enumerable own property 'length'
      print("length: " + x.length);
      for (var i in x) { print(i); }
  length: 2
  1
  length
  0
  2

  // test 4
  js> x[10] = 10;
      print("length: " + x.length);
      for (var i in x) { print(i); }
  length: 2
  1
  length
  10
  0
  2

Test 1 demonstrates enumeration of an empty object whose prototype is
an array of three elements.  Enumeration lists the prototype keys
("0", "1", "2").

Test 2 shows that object enumeration comes first ("1") followed by
prototype keys not "shadowed" by object keys ("0", "2"; "1" is shadowed).

Test 3 shows that even though the object itself is forced to be of
length 2, prototype enumeration still lists all keys of the prototype,
including "2" which is beyond the array length.

Test 4 shows that 'length' is not exotic for an object which has an
array as a prototype.  Exotic semantics of 'length' do not apply to
the object because the property write goes to the object, which is not
an array.  This also explains the result of test 3.
