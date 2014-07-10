======================
duk_hobject algorithms
======================

Overview
========

Purpose
-------

This document discusses, in detail, the internal algorithms for dealing
with objects, in particular for object property access.  These algorithms
are based on the algorithm descriptions in the E5 specification, which
have been refined towards the practical implementation needs e.g. by
combining multiple algorithms, inlining calls, and inlining "exotic
behaviors" (term borrowed from ES6).

The intent is to describe versions of the conceptual algorithms most suited
for implementation, without actually going into implementation level details.
Only complicated core algorithms and built-in methods are covered.

One important question is to identify the *exposed interface* of operations
invoked from concrete object-related expressions of Ecmascript code.  These
primitives are also in almost 1:1 relationship with the internal bytecode
operations.

Call and constructor related algorithms (``[[Call]]`` and ``[[Construct]]``)
are covered in ``execution.rst``.

.. note:: This document is not completely up-to-date: some special behaviors
   (like buffer and proxy objects) have been added on top of the algorithms
   described here.

Related sections of E5 specification
------------------------------------

For raw property algorithms:

* E5 Section 8.12: the default algorithms
* E5 Section 8.6.2: one paragraph lists exotic behaviors (page 33, PDF page 43)
* E5 Section 10.6: arguments object
* E5 Section 15.5.5.2: String object
* E5 Section 15.3.4.5.3, 15.3.5.3, 15.3.5.4: Function object
* E5 Section 15.4.5.1: Array object

For other algorithms:

* E5 Section 8.10.4: FromPropertyDescriptor
* E5 Section 8.10.5: ToPropertyDescriptor
* E5 Section 8.7.1: GetValue
* E5 Section 8.7.2: PutValue
* E5 Section 11.2.1: property accessor expression
* E5 Section 11.13: assignment operators (note that several other places
  also "evaluate" property accessor expressions)
* E5 Section 11.4.1: ``delete`` operator
* E5 Section 11.8.6: ``instanceof`` operator
* E5 Section 11.8.7: ``in`` operator
* E5 Section 15.2.3.3: ``Object.getOwnPropertyDescriptor()``
* E5 Section 15.2.3.6: ``Object.defineProperty()``
* E5 Section 15.2.3.7: ``Object.defineProperties()``

Algorithm overview
------------------

Ecmascript object property access behavior is described by internal property
handling algorithms.  The default algorithms are described in E5 Section 8.12.
There are some objects with exotic behaviors; these have variants of the
default property handling algorithms.  See ``hobject-design.rst`` for a
detailed discussion of exotic behaviors.

The "raw" property algorithms are:

* The default ``[[GetOwnProperty]](P)`` algorithm (E5 Section 8.12.1)

  * Modified behavior for: String object (E5 Section 15.5.5.2)

  * Modified behavior for: Arguments object created for non-strict functions (E5 Section 10.6)

* The default ``[[GetProperty]](P)`` algorithm (E5 Section 8.12.1)

* The default ``[[Get]](P)`` algorithm (E5 Section 8.12.1)

  * Modified behavior for: Arguments object created for non-strict functions (E5 Section 10.6)

  * Modified behavior for: Function object (E5 Section 15.3.5.4)

  * Note that exotic behaviors of ``[[Get]]`` are '''not''' visible through property
    descriptors at all.

* The default ``[[CanPut]](P)`` algorithm (E5 Section 8.12.1)

* The default ``[[Put]](P,V,Throw)`` algorithm (E5 Section 8.12.1)

* The default ``[[HasProperty]](P)`` algorithm (E5 Section 8.12.1)

  * Modified behavior for: Function object (E5 Section 15.3.5.3)

  * Modified behavior for: bound Function objects (E5 Section 15.3.4.5.3)

* The default ``[[Delete]](P,Throw)`` algorithm (E5 Section 8.12.1)

  * Modified behavior for: Arguments object created for non-strict functions (E5 Section 10.6)

* The default ``[[DefineOwnProperty]](P,Desc,Throw)`` algorithm (E5 Section 8.12.1)

  * Modified behavior for: Array object (E5 Section 15.4.5.1)

  * Modified behavior for: Arguments object created for non-strict functions (E5 Section 10.6)

The following figure illustrates the caller-callee relationship between the
default property algorithms (the figure would be somewhat different for
exotic behavior variants)::

                                       [[Put]]
                                         |
                                         |
                          .----+---------+
                          |    |         |
                          |    v         |
 [[HasProperty]]  [[Get]] | [[CanPut]]   |
       |             |    |   |  |       |
       `--------.    |    |   |  |       |
                |    |    |   |  |   .---'
                |    |    |   |  |   |
                v    v    v   v  |   |
  [[Delete]]    [[GetProperty]]  |   |  [[DefineOwnProperty]]
          |             |        |   |           |
          |             |      .-'   |           |
          |             |      |  .--'           |
          `---------.   |      |  |  .-----------'
                    |   |      |  |  |
                    v   v      v  v  v
                    [[GetOwnProperty]]     [[DefaultValue]]

However, these algorithms are only the "raw" property handling algorithms.
Actual property access operations in Ecmascript code are "wrapped" by e.g.:

* The evaluation rules for the specific expression (e.g. property read).
  These usually contain type checks, some coercions, etc.

* ``GetValue()`` and ``PutValue()`` (E5 Section 8.7) are used for property
  property read/write operations in Ecmascript code.  The algorithms are
  wrappers for ``[[Get]]`` and ``[[Put]]`` which allow the base reference
  to be a non-object, coercing it to a (temporary) object first.  This allows
  expressions like::

    print("foo".length);

Important questions
-------------------

From an implementation perspective there are many questions which don't have
an easy answer in the E5 specification, e.g.:

* How are these internal algorithms visible to user code?  This exposed
  interface places hard requirements on viable implementation approaches,
  whereas internal behavior can be implemented in several ways.

* What do the internal algorithms look like after you "inline" the calls
  used in the specification (which often obscure the true semantics)?

* What do the internal algorithms look like if exotic behaviors are
  "inlined" into one algorithm which supports both the default and all the
  exotic behaviors?

* What do the internal algorithms look like once we add "fast paths" for
  array index access (where the fast path avoids string interning if
  possible)?

* What do the internal algorithms look like once we consider the internal
  ``duk_hobject`` representation (e.g. separation between entry and array
  parts)?

The purpose of this document is to provide answers to these questions, and
act as a basis for implementing the rather tricky required behavior
accurately.  The internal algorithms are discussed, inlined, and reformulated
to a more useful form.  The sections are organized on the basis of practical
implementations needs, i.e. the context where internal algorithms are actually
needed.

What's not covered?
-------------------

This document does not go into full implementation detail in the algorithms.
Algorithms remain at a conceptual level.
In particular, the following are not covered:

* Relatively simple algorithms or built-in methods are not covered.
  For instance, ``Object.defineProperty()`` is covered but
  ``Object.seal()`` is not, because ``Object.seal()`` is simple enough
  to be implemented (and later verified) directly.

* Reference counts and reachability for garbage collection.  These are
  critical and sometimes difficult to implement correctly.

* Internal errors such as out-of-memory which may happen at any point
  but are not mentioned in the algorithms.  Where appropriate, the steps
  in abstract algorithms *are* adjusted to minimize inconsistencies if
  an internal error occurs.

* ``duk_hobject`` entry and array part separation, which affects all
  operations dealing with properties.

* Error ``message`` strings for particular kinds of error.  The E5
  specification only mandates error type (its class) but never mandates
  any texts.

* Concrete code structure or ordering; actual implementation may have a
  slightly different structure.

Exposed interface
=================

What is an exposed interface?
-----------------------------

The relevant *exposed interface* is the set of object related operations
which can be invoked from Ecmascript code, e.g.::

  // property write
  o.foo = "bar";

  // property read
  print(o.foo);

  // property deletion
  delete o.foo;

  // property existence check
  print('foo' in o);

  // object class membership test
  print(x instanceof Array);

It also covers intricate built-in methods, such as::

  var t = Object.getOwnPropertyDescriptor(o, 'foo');

  Object.defineOwnProperty(o, 'foo', { value: 'bar' });

  Object.defineProperties(o, {
    foo: { value: 'bar' },
    bar: { value: 'quux' }
  });

These exposed primitives are discussed in this section.

Contexts and property algorithms used
-------------------------------------

The following table lists all contexts where property algorithms are
invoked from user code.  (All ``Object`` built-in methods are listed
for completeness although not all of them invoke property algorithms.)

+---------------------------------------+---------------------------------+
| Context                               | Related algorithms / notes      |
+=======================================+=================================+
| Property read                         | Property accessor reference,    |
|                                       | ``GetValue()`` for the          |
|                                       | reference, ``[[Get]]``.  If     |
|                                       | base reference is not an object,|
|                                       | ``[[GetProperty]]``,            |
|                                       | ``[[Call]]``.                   |
+---------------------------------------+---------------------------------+
| Property write                        | Property accessor reference,    |
|                                       | ``PutValue()`` for the          |
|                                       | reference, ``[[Put]]``.  If     |
|                                       | base reference is not an object,|
|                                       | ``[[CanPut]]``,                 |
|                                       | ``[[GetOwnProperty]]``,         |
|                                       | ``[[GetProperty]]``,            |
|                                       | ``[[Call]]``.                   |
+---------------------------------------+---------------------------------+
| Property ``delete``                   | Property accessor reference,    |
|                                       | ``[[Delete]]``.                 |
+---------------------------------------+---------------------------------+
| ``in``                                | ``[[HasProperty]]``.            |
+---------------------------------------+---------------------------------+
| ``instanceof``                        | ``[[HasInstance]]``.            |
+---------------------------------------+---------------------------------+
| "for-in" enumeration                  | Enumeration order guarantees,   |
|                                       | see ``hobject-design.rst``.     |
+---------------------------------------+---------------------------------+
| ``Object.getPrototypeOf()``           | Just returns internal prototype.|
+---------------------------------------+---------------------------------+
| ``Object.getOwnPropertyDescriptor()`` | ``[[GetOwnProperty]]``,         |
|                                       | ``FromPropertyDescriptor()``    |
|                                       | for a fully populated property  |
|                                       | descriptor.                     |
+---------------------------------------+---------------------------------+
| ``Object.getOwnPropertyNames()``      | Creates a result array, uses    |
|                                       | ``[[DefineOwnProperty]]``       |
|                                       | internally.                     |
+---------------------------------------+---------------------------------+
| ``Object.create()``                   | No direct use of property       |
|                                       | algorithms but conceptually     |
|                                       | calls                           |
|                                       | ``Object.defineProperties()``   |
|                                       | internally.                     |
+---------------------------------------+---------------------------------+
| ``Object.defineProperty()``           | ``ToPropertyDescriptor()``,     |
|                                       | ``[[DefineOwnProperty]]`` with  |
|                                       | an arbitrary descriptor.        |
+---------------------------------------+---------------------------------+
| ``Object.defineProperties()``         | ``ToPropertyDescriptor()``,     |
|                                       | ``[[DefineOwnProperty[]`` with  |
|                                       | an arbitrary descriptor.        |
+---------------------------------------+---------------------------------+
| ``Object.seal()``                     | ``[[GetOwnProperty]]``,         |
|                                       | ``[[DefineOwnProperty]]``;      |
|                                       | sets ``[[Extensible]]`` to      |
|                                       | false.                          |
+---------------------------------------+---------------------------------+
| ``Object.freeze()``                   | ``[[GetOwnProperty]]``,         |
|                                       | ``[[DefineOwnProperty]]``,      |
|                                       | sets ``[[Extensible]]`` to      |
|                                       | false.                          |
+---------------------------------------+---------------------------------+
| ``Object.preventExtensions()``        | Sets ``[[Extensible]]`` to      |
|                                       | false.                          |
+---------------------------------------+---------------------------------+
| ``Object.isSealed()``                 | ``[[GetOwnProperty]]``, reads   |
|                                       | ``[[Extensible]]``.             |
+---------------------------------------+---------------------------------+
| ``Object.isFrozen()``                 | ``[[GetOwnProperty]]``, reads   |
|                                       | ``[[Extensible]]``.             |
+---------------------------------------+---------------------------------+
| ``Object.isExtensible()``             | Reads ``[[Extensible]]``.       |
+---------------------------------------+---------------------------------+
| ``Object.keys()``                     | Key order must match "for-in"   |
|                                       | enumeration order.              |
+---------------------------------------+---------------------------------+
| ``Object.prototype.hasOwnProperty()`` | ``[[GetOwnProperty]]``          |
|                                       | (does *not* use                 |
|                                       | ``[[HasProperty]]``)            |
+---------------------------------------+---------------------------------+

Central exposed primitives
--------------------------

The central exposed primitives are as follows.  Some have been given an
internal name which corresponds to the bytecode instruction:

* GETPROP: property read expression: coercion wrapping, ``GetValue()``,
  ``[[Get]]``, and a special ``[[Get]]`` variant if base is primitive

* PUTPROP: property write expression: coercion wrapping, ``PutValue()``,
  ``[[Put]]``, and a special ``[[Put]]`` variant if base is primitive

* DELPROP: ``delete`` operator: coercion wrapping, ``[[Delete]]``

* HASPROP: ``in`` operator: type check wrapping, ``[[HasProperty]]``

* INSTOF: ``instanceof`` operator: type check wrapping, ``[[HasInstance]]``

  + Not a property related primitive directly, but tied to the
    prototype chain

* ``Object.getOwnPropertyDescriptor()``

  + But not ``[[GetOwnProperty]]`` or ``[[GetProperty]]`` directly
  + Not "fast path" so implementation should be compact
  + Only throwing variant (``Throw`` is ``true``)

* ``Object.defineProperty()`` and ``Object.defineProperties()``

  + But not ``[[DefineOwnProperty]]`` directly
  + Not "fast path" so implementation should be compact
  + Only throwing variant (``Throw`` is ``true``)

These are used to implement the basic property related run-time operations
and some difficult built-in functions.  They are also used to implement
the C API and are also basic bytecode operations.

The remaining primitives (like ``Object.seal()`` etc) are trivial in
comparison, and are not analyzed in this document.

Notes
-----

* ``Object.getOwnPropertyDescriptor()``, ``Object.defineProperty()``,
  and ``Object.defineProperties()`` are the only exposed interfaces where
  property descriptors are explicitly exposed to user code, and also the
  only places where property descriptors are converted between internal and
  external forms.  All other exposed interfaces deal with property
  descriptors and attributes internally only.  These methods always set the
  ``Throw`` flag to ``true``, so the exposed implementation only needs to
  have a "throwing" variant.

* Property read and write handle a non-object base object with a specific
  variant for the basic ``[[Get]]`` and ``[[Put]]``, defined in E5 Sections
  8.7.1 and 8.7.2 (``GetValue()`` and ``PutValue()``).  Property delete uses
  a normal ``ToObject()`` coercion and then calls ``[[Delete]]`` normally.
  Property existence check (``in``) does a type check and throws an error
  if an argument is not already an object.  So, coercion behavior is a bit
  different in each context.

* Property References are established when parsing property access
  expressions, E5 Section 11.2.1.

* Property References are used as right-hand-side values and read using
  ``GetValue()`` from various places:

  + Array initializer
  + Object initializer
  + Grouping operator (parentheses)
  + Property accessor (e.g. ``x['foo']['bar']``)
  + ``new`` operator
  + Function calls
  + Postfix and prefix increment/decrement
  + ``void`` operator
  + Unary operators (plus, minus, bitwise and logical NOT)
  + Binary operators (additive and multiplicative expressions, bitwise,
    logical, and comparison operations)
  + ``instanceof`` operator
  + ``in`` operator
  + Conditional operator (``?:``)
  + Simple and compound assignment (right hand side)
  + Comma operator (``,``)
  + Variable declaration initializer
  + ``if``, ``do-while``, ``while``, ``for``, ``for-in``, ``with`` statements
  + ``throw`` statement

* Property references are used as left-hand-side values and written using
  ``PutValue()`` from various places:

  + Postfix and prefix increment/decrement
  + Simple and compound assignment
  + Variable declaration initializer
  + ``for`` and ``for-in`` statements

Exotic behaviors
================

This section covers the standard algorithms with exotic behaviors inlined.
For each algorithm, a single algorithm with all exotic behaviors inlined
is presented.  Calls to other internal algorithms are not inlined; the
purpose is to clarify how the exotic behaviors can be implemented
reasonably.

Note: the ``String`` object has no exotic behaviors as such, but the
``length`` and array index properties are implemented as virtual properties,
so they are inlined into the algorithms below.

GetOwnProperty
--------------

Related E5 sections:

* E5 Section 8.12.1: default algorithm
* E5 Section 15.5.5: ``String``
* E5 Section 10.5: arguments object

Default algorithm
:::::::::::::::::

1. If ``O`` doesn’t have an own property with name ``P``, return ``undefined``.

2. Let ``D`` be a newly created Property Descriptor with no fields.

3. Let ``X`` be ``O``\ ’s own property named P.

4. If ``X`` is a data property, then

  a. Set ``D.[[Value]]`` to the value of ``X``\ ’s ``[[Value]]`` attribute.

  b. Set ``D.[[Writable]]`` to the value of ``X``\ ’s ``[[Writable]]`` attribute.

5. Else ``X`` is an accessor property, so

  a. Set ``D.[[Get]]`` to the value of ``X``\ ’s ``[[Get]]`` attribute.

  b. Set ``D.[[Set]]`` to the value of ``X``\ ’s ``[[Set]]`` attribute.

6. Set ``D.[[Enumerable]]`` to the value of ``X``\ ’s ``[[Enumerable]]`` attribute.

7. Set ``D.[[Configurable]]`` to the value of ``X``\ ’s ``[[Configurable]]`` attribute.

8. Return ``D``.

Adding String object exotic behavior
::::::::::::::::::::::::::::::::::::

Now consider the ``String`` variant in E5 Section 15.5.5.2.  Step 2 states that if
the default algorithm returns a descriptor (not undefined), the exotic behavior
does not execute at all.  That, is the exotic algorithm is skipped if ``O`` has
an "own property" for key ``P``.

If the default algorithm fails to find an own property, the variant kicks in
checking for a valid array index key which is inside the string length.  If so,
it returns a single character data property descriptor.  The descriptor has
``[[Writable]]`` and ``[[Configurable]]`` set to ``false`` which means that
the property cannot be written or deleted -- the property is thus perfect for
implementation as a virtual property backed to an immutable internal string
value.

.. note:: Ecmascript 5.1 no longer requires the numbered index to be a valid
          array index, any number-like value will do.  This allows strings
          longer than 4G.  The algorithms here don't reflect this correctly.

The ``String`` object ``length`` property is an ordinary (non-exotic)
property, see E5 Section 15.5.5.1.  However, it is non-writable and
non-configurable (and even non-enumerable), so it too is nice and easy
to implement as a exotic property.  We'll thus incorporate the ``length``
property into the algorithm.

Finally note that from an implementation perspective it might be easier
to check for the exotic (virtual) properties before looking at the actual
ones (i.e. reverse the order of checking).  This seems perfectly OK to do,
because *if* the property name matches a virtual property, the object cannot
have a "normal" property of the same name: the initial ``String`` object
does not have such properties, and since the virtual properties cannot be
deleted, they prevent the insertion of normal "own properties" of the same
name.  Hence, if the virtual properties are checked for first and the check
matches, the object is guaranteed not to have a normal property of the same
name.  (Whether this is useful in an implementation is another issue.)

The combined algorithm, assuming the the virtual properties are checked
after the normal property check is as follows:

1. If ``O`` doesn’t have an own property with name ``P``:

  a. If ``O`` is not a ``String`` instance, return ``undefined``.

  b. (``String`` object exotic behavior.)
     Let ``str`` be the String value of the ``[[PrimitiveValue]]``
     internal property of ``O`` and ``len`` be the number of
     characters in ``str``.

  c. If ``P`` is ``"length"``, return a Property Descriptor with the values:

    * ``[[Value]]: len`` (a number)
    * ``[[Enumerable]]: false``
    * ``[[Writable]]: false``
    * ``[[Configurable]]: false``

  d. If ``P`` is not an array index (E5 Section 15.4), return ``undefined``.

  e. Let ``index`` be ``ToUint32(P)``.

  f. If ``len`` <= ``index``, return ``undefined``.

  g. Let ``resultStr`` be a string of length 1, containing one character
     from ``str``, specifically the character at position ``index``, where
     the first (leftmost) character in ``str`` is considered to be at
     position 0, the next one at position 1, and so on.

  h. Return a Property Descriptor with the values:

    * ``[[Value]]: resultStr``
    * ``[[Enumerable]]: true``
    * ``[[Writable]]: false``
    * ``[[Configurable]]: false``

2. Let ``D`` be a newly created Property Descriptor with no fields.

3. Let ``X`` be ``O``\ ’s own property named ``P``.

4. If ``X`` is a data property, then

  a. Set ``D.[[Value]]`` to the value of ``X``\ ’s ``[[Value]]`` attribute.

  b. Set ``D.[[Writable]]`` to the value of ``X``\ ’s ``[[Writable]]`` attribute.

5. Else ``X`` is an accessor property, so

  a. Set ``D.[[Get]]`` to the value of ``X``\ ’s ``[[Get]]`` attribute.

  b. Set ``D.[[Set]]`` to the value of ``X``\ ’s ``[[Set]]`` attribute.

6. Set ``D.[[Enumerable]]`` to the value of ``X``\ ’s ``[[Enumerable]]`` attribute.

7. Set ``D.[[Configurable]]`` to the value of ``X``\ ’s ``[[Configurable]]`` attribute.

8. Return ``D``.

Adding arguments object exotic behavior
:::::::::::::::::::::::::::::::::::::::

Next, consider the exotic ``[[GetOwnProperty]]`` behavior for a non-strict
arguments object described in E5 Section 10.6.  The exotic behavior only
applies if the object *did* contain the own property ``P``, and possibly
modifies the looked up value if the key ``P`` matches a numeric index
magically "bound" to a formal.

Note that the property descriptors for such variables are initially data
property descriptors, so the default algorithm will find a data property
descriptor (and not an accessor property descriptor).  If the property is
later converted to an accessor, the magical variable binding is also
dropped.  So, if the exotic behavior activates, the property is always
a data property.

The exotic behavior can be appended to the above algorithm as follows:

1. If ``O`` doesn’t have an own property with name ``P``:

  a. If ``O`` is not a ``String`` instance, return ``undefined``.

  b. (``String`` object exotic behavior.)
     Let ``str`` be the String value of the ``[[PrimitiveValue]]``
     internal property of ``O`` and ``len`` be the number of
     characters in ``str``.

  c. If ``P`` is ``"length"``, return a Property Descriptor with the values:

    * ``[[Value]]: len`` (a number)
    * ``[[Enumerable]]: false``
    * ``[[Writable]]: false``
    * ``[[Configurable]]: false``

  d. If ``P`` is not an array index (E5 Section 15.4), return ``undefined``.

  e.  Else let ``index`` be ``ToUint32(P)``.

  f. If ``len`` <= ``index``, return ``undefined``.

  g. Let ``resultStr`` be a string of length 1, containing one character
     from ``str``, specifically the character at position ``index``, where
     the first (leftmost) character in ``str`` is considered to be at
     position 0, the next one at position 1, and so on.

  h. Return a Property Descriptor with the values:

    * ``[[Value]]: resultStr``
    * ``[[Enumerable]]: true``
    * ``[[Writable]]: false``
    * ``[[Configurable]]: false``

2. Let ``D`` be a newly created Property Descriptor with no fields.

3. Let ``X`` be ``O``\ ’s own property named ``P``.

4. If ``X`` is a data property, then

  a. Set ``D.[[Value]]`` to the value of ``X``\ ’s ``[[Value]]`` attribute.

  b. Set ``D.[[Writable]]`` to the value of ``X``\ ’s ``[[Writable]]`` attribute.

5. Else ``X`` is an accessor property, so

  a. Set ``D.[[Get]]`` to the value of ``X``\ ’s ``[[Get]]`` attribute.

  b. Set ``D.[[Set]]`` to the value of ``X``\ ’s ``[[Set]]`` attribute.

6. Set ``D.[[Enumerable]]`` to the value of ``X``\ ’s ``[[Enumerable]]`` attribute.

7. Set ``D.[[Configurable]]`` to the value of ``X``\ ’s ``[[Configurable]]`` attribute.

8. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. Let ``isMapped`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``map`` passing ``P`` as the argument.

  c. If the value of ``isMapped`` is not ``undefined``, then:

    1. Set ``D.[[Value]]`` to the result of calling the ``[[Get]]``
       internal method of ``map`` passing ``P`` as the argument.

9. Return ``D``.

Notes:

* Step 1.b: if the object is a ``String`` object, there is no need for the
  arguments object exotic behavior check in step 8: an object can never be
  a ``String`` object and an arguments object simultaenously.

* Step 8: arguments objects for strict mode functions don't have the exotic
  behavior (or a ``[[ParameterMap]]``).  Arguments objects for non-strict
  functions don't always have exotic behavior either: they only do, if there
  is at least one mapped variable.  If so, ``[[ParameterMap]]`` is added, and
  exotic behavior is enabled.  See the main algorithm in E5 Section 10.6,
  step 12.

* Step 8.c.1: this step invokes an internal getter function which looks up
  the magically bound variable.  See E5 Section 10.6, 11.c.ii, and the
  *MakeArgGetter* concept.  A practical implementation may not create such
  internal functions (we don't).

* Step 8.c.1: the rules of maintaining the ``[[ParameterMap]]`` ensures that
  at this point the property is always a data property, so setting the
  ``[[Value]]`` is correct.  If a magically bound value is converted into an
  accessor, the property is deleted from the ``[[ParameterMap]]`` so it no
  longer has exotic behavior.

Final version
:::::::::::::

Final version with some cleanup and simplification:

1. Let ``X`` be ``O``\ ’s own property named ``P``.
   If ``O`` doesn’t have an own property with name ``P``:

  a. If ``O`` is not a ``String`` instance, return ``undefined``.

  b. (``String`` object exotic behavior.)
     Let ``str`` be the String value of the ``[[PrimitiveValue]]``
     internal property of ``O`` and ``len`` be the number of
     characters in ``str``.

  c. If ``P`` is ``"length"``:

    1. Return a Property Descriptor with the values:

      * ``[[Value]]: len`` (a primitive number)
      * ``[[Enumerable]]: false``
      * ``[[Writable]]: false``
      * ``[[Configurable]]: false``

  d. If ``P`` is an array index (E5 Section 15.4):

    1. Let ``index`` be ``ToUint32(P)``.

    2. If ``index`` < ``len``, return a Property Descriptor with the values:

      * ``[[Value]]:`` a primitive string of length 1, containing one character
        from ``str`` at position ``index`` (zero based index)
      * ``[[Enumerable]]: true``
      * ``[[Writable]]: false``
      * ``[[Configurable]]: false``

  e. Return ``undefined``.

2. Let ``D`` be a newly created Property Descriptor filled as follows:

  a. If ``X`` is a data property:

    1. Set ``D.[[Value]]`` to the value of ``X``\ ’s ``[[Value]]`` attribute.

    2. Set ``D.[[Writable]]`` to the value of ``X``\ ’s ``[[Writable]]`` attribute.

  b. Else ``X`` is an accessor property:

    1. Set ``D.[[Get]]`` to the value of ``X``\ ’s ``[[Get]]`` attribute.

    2. Set ``D.[[Set]]`` to the value of ``X``\ ’s ``[[Set]]`` attribute.

  c. For either type of property:

    1. Set ``D.[[Enumerable]]`` to the value of ``X``\ ’s ``[[Enumerable]]`` attribute.

    2. Set ``D.[[Configurable]]`` to the value of ``X``\ ’s ``[[Configurable]]`` attribute.

3. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Set ``D.[[Value]]`` to the result of calling the ``[[Get]]``
       internal method of ``map`` passing ``P`` as the argument.

4. Return ``D``.

Notes:

* Step 3 can be skipped for accessors.

Get
---

Related E5 sections:

* E5 Section 8.12.3: default algorithm
* E5 Section 10.5: arguments object
* E5 Section 15.3.5.4: ``Function``

Default algorithm
:::::::::::::::::

(Note that E5 Section 8.12.3 has broken numbering; fixed below.)

1. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

2. If ``desc`` is ``undefined``, return ``undefined``.

3. If ``IsDataDescriptor(desc)`` is ``true``, return ``desc.[[Value]]``.

4. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true`` so, let
   ``getter`` be ``desc.[[Get]]``.

5. If ``getter`` is ``undefined``, return ``undefined``.

6. Return the result calling the ``[[Call]]`` internal method of ``getter``
   providing ``O`` as the ``this`` value and providing no arguments.

Adding Function object exotic behavior
::::::::::::::::::::::::::::::::::::::

Consider the ``Function`` variant in E5 Section 15.3.5.4.  The behavior only
applies if ``P`` is ``caller`` and the resulting return *value* of the default
function is a strict mode function.

The exotic behavior does not need to be checked in steps 2 or 5 of the
default algorithm, because ``undefined`` is never a strict mode function
value.

So, we can reformulate into:

1. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

2. If ``desc`` is ``undefined``, return ``undefined``.

3. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

4. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``O`` as the ``this`` value and
     providing no arguments.

5. If ``O`` is a ``Function`` object, ``P`` is ``"caller"``, and ``res``
   is a strict mode ``Function`` object, throw a ``TypeError`` exception.

6. Return ``res``.

Adding arguments object exotic behavior
:::::::::::::::::::::::::::::::::::::::

Next, consider the exotic ``[[Get]]`` behavior for a non-strict arguments
object described in E5 Section 10.6.  To be exact, the exotic behaviors
are only enabled for objects with a non-empty initial ``[[ParameterMap]]``
(see E5 Section 10.6, main algorithm, step 12).

There are two exotic behaviors:

1. If the property name ``P`` is magically bound to an identifier
   (through the ``[[ParameterMap]]``) the default ``[[Get]]`` is
   bypassed entirely and the property value is read.
   (Note that the property ``P`` *must* be a data property in this
   case, so no side effects are lost by this behavior.)

2. If the property name ``P`` is *not bound* to an identifier,
   the ``"caller"`` property has exotic behavior essentially
   identical to that of ``Function``.

These can be incorporated as follows:

1. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. Let ``isMapped`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``map`` passing ``P`` as the argument.

  c. If the value of ``isMapped`` is not ``undefined``, then:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

2. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

3. If ``desc`` is ``undefined``, return ``undefined``.

4. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

5. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``O`` as the ``this`` value and
     providing no arguments.

6. If ``O`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

7. Return ``res``.

Note:

* Step 1 can match only when ``P`` is a "numeric" property name, and
  the property value is an own data property.  Magically bound properties
  are initially own data properties, and if they're changed to accessors
  (or deleted), the binding is removed.  Because of this, the arguments
  exotic behavior could just as well be moved to the end of the algorithm.

Final version
:::::::::::::

Final version with some cleanup and simplification:

1. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

2. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

3. If ``desc`` is ``undefined``, return ``undefined``.

4. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

5. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``O`` as the ``this`` value and
     providing no arguments.

6. If ``O`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

7. Return ``res``.

DefineOwnProperty
-----------------

Related E5 sections:

* E5 Section 8.12.9: default algorithm
* E5 Section 15.4.5: ``Array``
* E5 Section 10.5: arguments object

Note that ``String`` exotic properties are taken into account by
``[[DefineOwnProperty]]`` through ``[[GetOwnProperty]]`` which
returns a property descriptor prohibiting any property value or
attribute changes.  However, no explicit checks are needed for
these (virtual) properties.

This is by the far the most complex property algorithm, especially
with exotic behaviors incorporated.  The algorithm itself is
complex, but the ``Array`` variant actually makes multiple calls to
the default variant which is even trickier for "inlining".

Default algorithm
:::::::::::::::::

1. Let ``current`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``O`` with property name ``P``.

2. Let ``extensible`` be the value of the ``[[Extensible]]`` internal
   property of ``O``.

3. If ``current`` is ``undefined`` and ``extensible`` is ``false``,
   then Reject.

4. If ``current`` is ``undefined`` and ``extensible`` is ``true``, then

  a. If ``IsGenericDescriptor(Desc)`` or ``IsDataDescriptor(Desc)`` is
     ``true``, then

    1. Create an own data property named ``P`` of object ``O`` whose
       ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
       ``[[Configurable]]`` attribute values are described by ``Desc``.
       If the value of an attribute field of ``Desc`` is absent, the
       attribute of the newly created property is set to its default
       value.

  b. Else, ``Desc`` must be an accessor Property Descriptor so,

    1. Create an own accessor property named ``P`` of object ``O`` whose
       ``[[Get]]``, ``[[Set]]``, ``[[Enumerable]]`` and ``[[Configurable]]``
       attribute values are described by ``Desc``.  If the value of an
       attribute field of ``Desc`` is absent, the attribute of the newly
       created property is set to its default value.

  c. Return ``true``.

5. Return ``true`` if every field in ``Desc`` is absent.

6. Return ``true``, if every field in ``Desc`` also occurs in ``current``
   and the value of every field in ``Desc`` is the same value as the
   corresponding field in ``current`` when compared using the ``SameValue``
   algorithm (E5 Section 9.12).

7. If the ``[[Configurable]]`` field of ``current`` is ``false`` then

  a. Reject, if the ``[[Configurable]]`` field of ``Desc`` is true.

  b. Reject, if the ``[[Enumerable]]`` field of ``Desc`` is present and
     the ``[[Enumerable]]`` fields of ``current`` and ``Desc`` are the
     Boolean negation of each other.

8. If ``IsGenericDescriptor(Desc)`` is ``true``, then no further validation
   is required.

9. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
   have different results, then 

  a. Reject, if the ``[[Configurable]]`` field of ``current`` is ``false``.

  b. If ``IsDataDescriptor(current)`` is true, then

    1. Convert the property named ``P`` of object ``O`` from a data property
       to an accessor property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  c. Else,

    1. Convert the property named ``P`` of object ``O`` from an accessor
       property to a data property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

10. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    are both true, then

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Reject, if the ``[[Writable]]`` field of ``current`` is ``false``
       and the ``[[Writable]]`` field of ``Desc`` is ``true``.

    2. If the ``[[Writable]]`` field of ``current`` is ``false``, then

      a. Reject, if the ``[[Value]]`` field of ``Desc`` is present and
         ``SameValue(Desc.[[Value]], current.[[Value]])`` is ``false``.

  b. else, the ``[[Configurable]]`` field of ``current`` is ``true``, so
     any change is acceptable.

11. Else, ``IsAccessorDescriptor(current)`` and ``IsAccessorDescriptor(Desc)``
    are both ``true`` so,

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Reject, if the ``[[Set]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Set]], current.[[Set]])`` is ``false``.

    2. Reject, if the ``[[Get]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Get]], current.[[Get]])`` is ``false``.

12. For each attribute field of ``Desc`` that is present, set the
    correspondingly named attribute of the property named ``P`` of object
    ``O`` to the value of the field.

13. Return ``true``.

Notes:

* The default attributes are *not* the same as when ``[[Put]]`` creates a
  new property.  The defaults here are "false" (and NULL for getter/setter),
  see E5 Section 8.6.1, Table 7).

* Step 10.a.1 allows a non-configurable property to change from writable to 
  non-writable, but not vice versa.

* Step 10.b is not necessary (it is more of an assertion), and there is no
  corresponding step 11.b mentioning the same thing.  This step can be removed
  from the description.

* There are multiple exit points for both Reject (throw or return false) and
  true.  For incorporating inline exotic behaviors, these are turned to
  "gotos" below.

Default algorithm reformulated
::::::::::::::::::::::::::::::

Let's first do a little bit of reformulation (see above):

1. Let ``current`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``O`` with property name ``P``.

2. Let ``extensible`` be the value of the ``[[Extensible]]`` internal
   property of ``O``.

3. If ``current`` is ``undefined``:

  a. If ``extensible`` is ``false``, then goto REJECT.

  b. If ``IsGenericDescriptor(Desc)`` or ``IsDataDescriptor(Desc)`` is
     ``true``, then

    1. Create an own data property named ``P`` of object ``O`` whose
       ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
       ``[[Configurable]]`` attribute values are described by ``Desc``.
       If the value of an attribute field of ``Desc`` is absent, the
       attribute of the newly created property is set to its default
       value.

  c. Else, ``Desc`` must be an accessor Property Descriptor so,

    1. Create an own accessor property named ``P`` of object ``O`` whose
       ``[[Get]]``, ``[[Set]]``, ``[[Enumerable]]`` and ``[[Configurable]]``
       attribute values are described by ``Desc``.  If the value of an
       attribute field of ``Desc`` is absent, the attribute of the newly
       created property is set to its default value.

  d. Goto SUCCESS.

4. Goto SUCCESS, if every field in ``Desc`` also occurs in ``current``
   and the value of every field in ``Desc`` is the same value as the
   corresponding field in ``current`` when compared using the ``SameValue``
   algorithm (E5 Section 9.12).  (This also covers the case where
   every field in ``Desc`` is absent.)

5. If the ``[[Configurable]]`` field of ``current`` is ``false`` then

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``Desc`` is true.

  b. Goto REJECT, if the ``[[Enumerable]]`` field of ``Desc`` is present
     and the ``[[Enumerable]]`` fields of ``current`` and ``Desc`` are the
     Boolean negation of each other.

6. If ``IsGenericDescriptor(Desc)`` is ``true``, then goto VALIDATED.

7. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
   have different results, then 

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``current`` is
     ``false``.

  b. If ``IsDataDescriptor(current)`` is true, then

    1. Convert the property named ``P`` of object ``O`` from a data property
       to an accessor property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  c. Else,

    1. Convert the property named ``P`` of object ``O`` from an accessor
       property to a data property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  d. Goto VALIDATED.

8. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
   are both true, then

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Writable]]`` field of ``current`` is
       ``false`` and the ``[[Writable]]`` field of ``Desc`` is ``true``.

    2. Goto REJECT, If the ``[[Writable]]`` field of ``current`` is
       ``false``, and the ``[[Value]]`` field of ``Desc`` is present, and
       ``SameValue(Desc.[[Value]], current.[[Value]])`` is ``false``.

  b. Goto VALIDATED.

9. Else, ``IsAccessorDescriptor(current)`` and ``IsAccessorDescriptor(Desc)``
   are both ``true`` so,

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Set]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Set]], current.[[Set]])`` is ``false``.

    2. Goto REJECT, if the ``[[Get]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Get]], current.[[Get]])`` is ``false``.

  b. Goto VALIDATED.

10. **VALIDATED:** For each attribute field of ``Desc`` that is present,
    set the correspondingly named attribute of the property named ``P``
    of object ``O`` to the value of the field.

11. **SUCCESS:** Return ``true``.

12. **REJECT**: If ``Throw`` is ``true``, then throw a ``TypeError``
    exception, otherwise return ``false``.

Analysis of Array object [[DefineOwnProperty]]
::::::::::::::::::::::::::::::::::::::::::::::

The ``Array`` variant for ``[[DefineOwnProperty]]`` is described in
E5 Section 15.4.5.1.  The variant *seems* to be essentially a pre-check
for ``length`` and array index properties before the default algorithm
runs (see steps 1-4 of the variant).

However, it's much more complex than that, because the variant algorithm
makes multiple calls to the default algorithm.

Let's look at the variant algorithm first (here we assume ``O`` is an
``Array`` with exotic behavior, so no check is made for exotic behavior):

1. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``O`` passing ``"length"`` as the argument.  The
   result will never be ``undefined`` or an accessor descriptor because
   ``Array`` objects are created with a length data property that cannot
   be deleted or reconfigured.

2. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
   (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

3. If ``P`` is ``"length"``, then

  a. If the ``[[Value]]`` field of ``Desc`` is absent, then

    1. Return the result of calling the default ``[[DefineOwnProperty]]``
       internal method (E5 Section 8.12.9) on ``O`` passing ``"length"``,
       ``Desc``, and ``Throw`` as arguments.

  b. Let ``newLenDesc`` be a copy of ``Desc``.

  c. Let ``newLen`` be ``ToUint32(Desc.[[Value]])``.

  d. If ``newLen`` is not equal to ``ToNumber(Desc.[[Value]])``, throw a
     ``RangeError`` exception.

  e. Set ``newLenDesc.[[Value]]`` to ``newLen``.

  f. If ``newLen`` >= ``oldLen``, then

    1. Return the result of calling the default ``[[DefineOwnProperty]]``
       internal method (E5 Section 8.12.9) on ``O`` passing ``"length"``,
       ``newLenDesc``, and ``Throw`` as arguments.

  g. Reject if ``oldLenDesc.[[Writable]]`` is ``false``.

  h. If ``newLenDesc.[[Writable]]`` is absent or has the value ``true``,
     let ``newWritable`` be ``true``.

  i. Else, 

    1. Need to defer setting the ``[[Writable]]`` attribute to ``false`` in
       case any elements cannot be deleted.

    2. Let ``newWritable`` be ``false``.

    3. Set ``newLenDesc.[[Writable]]`` to ``true``.

  j. Let ``succeeded`` be the result of calling the default
     ``[[DefineOwnProperty]]`` internal method (E5 Section 8.12.9) on ``O``
     passing ``"length"``, ``newLenDesc``, and ``Throw`` as arguments.

  k. If ``succeeded`` is ``false``, return ``false``.

  l. While ``newLen`` < ``oldLen`` repeat,

    1. Set ``oldLen`` to ``oldLen – 1``.

    2. Let ``canDelete`` be the result of calling the ``[[Delete]]``
       internal method of ``O`` passing ``ToString(oldLen)`` and ``false``
       as arguments.

    3. If ``canDelete`` is ``false``, then:

      a. Set ``newLenDesc.[[Value]`` to ``oldLen+1``.

      b. If ``newWritable`` is ``false``, set ``newLenDesc.[[Writable]`` to
         ``false``.

      c. Call the default ``[[DefineOwnProperty]]`` internal method (E5
         Section 8.12.9) on ``O`` passing ``"length"``, ``newLenDesc``, and
         ``false`` as arguments.

      d. Reject.

  m. If ``newWritable`` is ``false``, then

    1. Call the default ``[[DefineOwnProperty]]`` internal method (E5 Section
       8.12.9) on ``O`` passing ``"length"``, Property Descriptor
       ``{[[Writable]]: false}``, and ``false`` as arguments.  This call will
       always return ``true``.

  n. Return ``true``.

4. Else if ``P`` is an array index (E5 Section 15.4), then:

  a. Let ``index`` be ``ToUint32(P)``.

  b. Reject if ``index`` >= ``oldLen`` and ``oldLenDesc.[[Writable]]`` is
     ``false``.

  c. Let ``succeeded`` be the result of calling the default
     ``[[DefineOwnProperty]]`` internal method (E5 Section 8.12.9) on ``O``
     passing ``P``, ``Desc``, and ``false`` as arguments.

  d. Reject if ``succeeded`` is ``false``.

  e. If ``index`` >= ``oldLen``:

    1. Set ``oldLenDesc.[[Value]]`` to ``index + 1``.

    2. Call the default ``[[DefineOwnProperty]]`` internal method (E5 Section
       8.12.9) on ``O`` passing ``"length"``, ``oldLenDesc``, and ``false``
       as arguments.  This call will always return ``true``.

  f. Return ``true``.

5. Return the result of calling the default ``[[DefineOwnProperty]]``
   internal method (E5 Section 8.12.9) on ``O`` passing ``P``, ``Desc``,
   and ``Throw`` as arguments.

Notes:

* In E5 Section 15.4.5.1 step 3.l.ii - 3.l.iii the temporary variable
  ``cannotDelete`` seems to be misused; it should probably be ``canDelete``
  and the check in step iii should read "if ``canDelete`` is ``false`` ...".

* Step 5 is the default behavior, assuming nothing "captured" the call
  before.

* Unfortunately steps 3 and 4 call the default ``[[DefineOwnProperty]]``
  internally (multiple times).  We'd like to avoid this, to get a
  non-recursive implementation.  This requires some major restatements.

Let's look at the calls to the default ``[[DefineOwnProperty]]`` (other
than step 5) to see what could be done about them.

First, for ``P`` == ``length``:

* Step 3.a.1:
  If ``Desc.[[Value]]`` is absent, call the default algorithm.

  This is equivalent to:

    - Jumping to step 5.

* Step 3.f.1:
  If ``newLen`` validation succeeds and new length is not shorter
  than previous, call the default algorithm with a modified
  property descriptor, ``newLenDesc``.  The new property descriptor
  is a copy of the original, with ``[[Value]]`` changed to the
  normalized and numeric (32-bit unsigned integer) length value.

  This is equivalent to:

  + Doing length validation and coercion

  + Checking that the new length is not shorter than previous;
    and if so, forcing ``Desc.[[Value]]`` to ``newLen``, and
    then jumping to step 5.

  + Note: the caller's view of ``Desc`` must not change, so ``Desc``
    cannot be a "pass by reference" value.

* Step 3.f.j:
  Here ``newLen`` validation has succeeded, and the new length is shorter
  than previous.  Also, ``Desc.[[Writable]]`` may have been fudged.
  The changes so far are "committed" to ``"length"`` property using the
  default call.

  Note that this call also has the important effect of checking that
  the default algorithm is expected to succeed before we touch any of
  the array elements.

  This is equivalent to:

  + Doing the ``newWritable`` fudging to ``Desc``, and keeping
    ``newWritable`` for later.

  + Jumping to step 5.

  + Adding a post-step to the default algorithm for steps 3.k - 3.m.

* Step 3.l.3.c:
  Here we've started to "shorten" the array but run into a non-deletable
  element.  The ``"length"`` property is updated with the actual final
  length, and ``Desc.[[Writable]]`` is fudged back to its original,
  requested value.

  This is equivalent to:

  + Fudging both ``[[Value]]`` and ``[[Writable]]`` of ``Desc``.

  + Jumping to step 5.

* Step 3.m:
  Here a pending write protection is finally implemented by calling
  the default ``[[DefineOwnProperty]]`` with a property descriptor
  requesting only that the property be changed to non-writable.

  This is equivalent to:

  + Adding a "pending write protect" flag and jumping to 5.

  + Modifying the standard algorithm to recognize a "pending
    write protect" after standard property modifications and
    checks are complete.

Then, for the case when ``P`` is a valid array index:

* Step 4.c:
  The index has been coerced and validated; the algorithm rejects if the
  array index would require that the array ``length`` be increased but
  ``length`` is write protected.

  This is equivalent to:

  + Doing the pre-checks for index vs. ``length``.

  + Jumping to step 5.

  + Adding a post-step to the standard algorithm to handle steps 4.d - 4.f.

* Step 4.e.2:
  This is a step which happens after the default algorithm has finished
  without errors.  If so, and the array index extended the array ``length``,
  the array ``length`` is updated to reflect this.  This is expected to
  always succeed.

  This is equivalent to:

  + Adding a post-step to the standard algorithm.

A draft of modifications to the standard algorithm to avoid recursive
calls could be something like:

* Add a pre-step with:

  + Check for ``P`` == ``length``, and:

    - If ``Desc.[[Value]]`` missing, use default algorithm

    - ``newLen`` validation and updating of ``Desc.[[Value]]``

    - If new length is not shorter than old length, default algorithm
      with the modified ``Desc`` can be used

    - Possible fudging of ``Desc.[[Writable]]`` and check for
      setting ``pendingWriteProtect`` (set if ``newWritable``
      is ``false``)

    - If new length is shorter than old length, run the default
      algorithm successfully first before touching array elements

  + Check for ``P`` being a valid array index, and:

    - Pre-checks for index vs. ``length``

* Modify the standard algorithm:

  + Continuing with the post-step if the standard algorithm succeeds.

* Add a post-step with:

  + Check whether we have a pending array "shortening", i.e.
    ``P`` was ``"length"``, and the new length is shorter than
    old.

    - A complex algorithm for shortening the array needs to run.
      This algorithm may either indicate success or failure, and
      returns the actual final length of the array which may
      differ from the requested one if a non-configurable element
      prevents deletion.

  + Check for ``pendingWriteProtect``; if so, write protect the
    target property (this is for step 3.m).

  + Check whether ``P`` was an array index which should increase
    the length of the array.

    - If so, we've already checked in the pre-step that the length
      can be updated.  So, update the pending new length value.

The algorithm for shortening the array is not inlined (it is a separate
helper in the implementation too) as it's relatively tricky.  It is
instead isolated into ``ShortenArray()`` internal helper with inputs:

* old length
* new length

and outputs:

* success flag (``false`` if some element couldn't be deleted)
* final array length to be updated into ``"length"`` property

Adding ``Array`` object exotic behavior
:::::::::::::::::::::::::::::::::::::::

Incorporating the approach for adding a pre- and post-processing phase
we get something like:

1. Set ``pendingWriteProtect`` to ``false``.

2. If ``O`` is not an ``Array`` object, goto SKIPARRAY.

3. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``O`` passing ``"length"`` as the argument.  The
   result will never be ``undefined`` or an accessor descriptor because
   ``Array`` objects are created with a length data property that cannot
   be deleted or reconfigured.

4. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
   (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

5. If ``P`` is ``"length"``, then

  a. If the ``[[Value]]`` field of ``Desc`` is absent, then goto SKIPARRAY.

  b. Let ``newLen`` be ``ToUint32(Desc.[[Value]])``.

  c. If ``newLen`` is not equal to ``ToNumber(Desc.[[Value]])``, goto
     REJECTRANGE.

  d. Set ``Desc.[[Value]]`` to ``newLen``.

  e. If ``newLen`` >= ``oldLen``, then goto SKIPARRAY.

  f. Goto REJECT if ``oldLenDesc.[[Writable]]`` is ``false``.

  g. If ``Desc.[[Writable]]`` has the value ``false``:

    1. Need to defer setting the ``[[Writable]]`` attribute to ``false``
       in case any elements cannot be deleted.

    2. Set ``pendingWriteProtect`` to ``true``.

    3. Set ``Desc.[[Writable]]`` to ``true``.

  h. Goto SKIPARRAY.  (Rest of the processing happens in the post-step.)

6. Else if ``P`` is an array index (E5 Section 15.4), then:

  a. Let ``index`` be ``ToUint32(P)``.

  b. Goto REJECT if ``index`` >= ``oldLen`` and ``oldLenDesc.[[Writable]]``
     is ``false``.

  c. Goto SKIPARRAY.  (Rest of the processing happens in the post-step.)

7. **SKIPARRAY**:
   Let ``current`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``O`` with property name ``P``.

8. Let ``extensible`` be the value of the ``[[Extensible]]`` internal
   property of ``O``.

9. If ``current`` is ``undefined``:

  a. If ``extensible`` is ``false``, then goto REJECT.

  b. If ``IsGenericDescriptor(Desc)`` or ``IsDataDescriptor(Desc)`` is
     ``true``, then

    1. Create an own data property named ``P`` of object ``O`` whose
       ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
       ``[[Configurable]]`` attribute values are described by ``Desc``.
       If the value of an attribute field of ``Desc`` is absent, the
       attribute of the newly created property is set to its default
       value.

  c. Else, ``Desc`` must be an accessor Property Descriptor so,

    1. Create an own accessor property named ``P`` of object ``O`` whose
       ``[[Get]]``, ``[[Set]]``, ``[[Enumerable]]`` and ``[[Configurable]]``
       attribute values are described by ``Desc``.  If the value of an
       attribute field of ``Desc`` is absent, the attribute of the newly
       created property is set to its default value.

  d. Goto SUCCESS.

10. Goto SUCCESS, if every field in ``Desc`` also occurs in ``current``
    and the value of every field in ``Desc`` is the same value as the
    corresponding field in ``current`` when compared using the ``SameValue``
    algorithm (E5 Section 9.12).  (This also covers the case where
    every field in ``Desc`` is absent.)

11. If the ``[[Configurable]]`` field of ``current`` is ``false`` then

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``Desc`` is true.

  b. Goto REJECT, if the ``[[Enumerable]]`` field of ``Desc`` is present
     and the ``[[Enumerable]]`` fields of ``current`` and ``Desc`` are the
     Boolean negation of each other.

12. If ``IsGenericDescriptor(Desc)`` is ``true``, then goto VALIDATED.

13. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    have different results, then 

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``current`` is
     ``false``.

  b. If ``IsDataDescriptor(current)`` is true, then

    1. Convert the property named ``P`` of object ``O`` from a data property
       to an accessor property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  c. Else,

    1. Convert the property named ``P`` of object ``O`` from an accessor
       property to a data property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  d. Goto VALIDATED.

14. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    are both true, then

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Writable]]`` field of ``current`` is
       ``false`` and the ``[[Writable]]`` field of ``Desc`` is ``true``.

    2. Goto REJECT, If the ``[[Writable]]`` field of ``current`` is
       ``false``, and the ``[[Value]]`` field of ``Desc`` is present, and
       ``SameValue(Desc.[[Value]], current.[[Value]])`` is ``false``.

  b. Goto VALIDATED.

15. Else, ``IsAccessorDescriptor(current)`` and ``IsAccessorDescriptor(Desc)``
    are both ``true`` so,

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Set]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Set]], current.[[Set]])`` is ``false``.

    2. Goto REJECT, if the ``[[Get]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Get]], current.[[Get]])`` is ``false``.

  b. Goto VALIDATED.

16. **VALIDATED:**
    For each attribute field of ``Desc`` that is present, set the
    correspondingly named attribute of the property named ``P`` of object
    ``O`` to the value of the field.

17. **SUCCESS:**
    If ``O`` is an ``Array`` object:

  a. If ``P`` is ``"length"``, and ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. If ``pendingWriteProtect`` is ``true``, update the property
       (``"length"``) to have ``[[Writable]] = false``.

    4. Goto REJECT, if ``shortenSucceeded`` is ``false``.

  b. If ``P`` is an array index and ``index`` >= ``oldLen``:

    1. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
       This always succeeds, because we've checked in the pre-step that the
       ``"length"`` is writable, and since ``P`` is an array index property,
       the length must still be writable here.

18. Return ``true``.

19. **REJECT**:
    If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
    otherwise return ``false``.

20. **REJECTRANGE**:
    Throw a ``RangeError`` exception.  Note that this is unconditional
    (thrown even if ``Throw`` is ``false``).

Adding arguments object exotic behavior
:::::::::::::::::::::::::::::::::::::::

The exotic ``[[DefineOwnProperty]]`` behavior for an arguments object
containing a ``[[ParameterMap]]`` is described in E5 Section 10.6.

The variant algorithm essentially first runs the default algorithm.
If the default algorithm finishes successfully, the variant will then
maintain the parameter map and possibly perform a setter call.

This is easy to incorporate and results in:

1. Set ``pendingWriteProtect`` to ``false``.

2. If ``O`` is not an ``Array`` object, goto SKIPARRAY.

3. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``O`` passing ``"length"`` as the argument.  The
   result will never be ``undefined`` or an accessor descriptor because
   ``Array`` objects are created with a length data property that cannot
   be deleted or reconfigured.

4. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
   (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

5. If ``P`` is ``"length"``, then

  a. If the ``[[Value]]`` field of ``Desc`` is absent, then goto SKIPARRAY.

  b. Let ``newLen`` be ``ToUint32(Desc.[[Value]])``.

  c. If ``newLen`` is not equal to ``ToNumber(Desc.[[Value]])``, goto
     REJECTRANGE.

  d. Set ``Desc.[[Value]]`` to ``newLen``.

  e. If ``newLen`` >= ``oldLen``, then goto SKIPARRAY.

  f. Goto REJECT if ``oldLenDesc.[[Writable]]`` is ``false``.

  g. If ``Desc.[[Writable]]`` has the value ``false``:

    1. Need to defer setting the ``[[Writable]]`` attribute to ``false``
       in case any elements cannot be deleted.

    2. Set ``pendingWriteProtect`` to ``true``.

    3. Set ``Desc.[[Writable]]`` to ``true``.

  h. Goto SKIPARRAY.  (Rest of the processing happens in the post-step.)

6. Else if ``P`` is an array index (E5 Section 15.4), then:

  a. Let ``index`` be ``ToUint32(P)``.

  b. Goto REJECT if ``index`` >= ``oldLen`` and ``oldLenDesc.[[Writable]]``
     is ``false``.

  c. Goto SKIPARRAY.  (Rest of the processing happens in the post-step.)

7. **SKIPARRAY**:
   Let ``current`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``O`` with property name ``P``.

8. Let ``extensible`` be the value of the ``[[Extensible]]`` internal
   property of ``O``.

9. If ``current`` is ``undefined``:

  a. If ``extensible`` is ``false``, then goto REJECT.

  b. If ``IsGenericDescriptor(Desc)`` or ``IsDataDescriptor(Desc)`` is
     ``true``, then

    1. Create an own data property named ``P`` of object ``O`` whose
       ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
       ``[[Configurable]]`` attribute values are described by ``Desc``.
       If the value of an attribute field of ``Desc`` is absent, the
       attribute of the newly created property is set to its default
       value.

  c. Else, ``Desc`` must be an accessor Property Descriptor so,

    1. Create an own accessor property named ``P`` of object ``O`` whose
       ``[[Get]]``, ``[[Set]]``, ``[[Enumerable]]`` and ``[[Configurable]]``
       attribute values are described by ``Desc``.  If the value of an
       attribute field of ``Desc`` is absent, the attribute of the newly
       created property is set to its default value.

  d. Goto SUCCESS.

10. Goto SUCCESS, if every field in ``Desc`` also occurs in ``current``
    and the value of every field in ``Desc`` is the same value as the
    corresponding field in ``current`` when compared using the ``SameValue``
    algorithm (E5 Section 9.12).  (This also covers the case where
    every field in ``Desc`` is absent.)

11. If the ``[[Configurable]]`` field of ``current`` is ``false`` then

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``Desc`` is true.

  b. Goto REJECT, if the ``[[Enumerable]]`` field of ``Desc`` is present
     and the ``[[Enumerable]]`` fields of ``current`` and ``Desc`` are the
     Boolean negation of each other.

12. If ``IsGenericDescriptor(Desc)`` is ``true``, then goto VALIDATED.

13. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    have different results, then 

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``current`` is
     ``false``.

  b. If ``IsDataDescriptor(current)`` is true, then

    1. Convert the property named ``P`` of object ``O`` from a data property
       to an accessor property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  c. Else,

    1. Convert the property named ``P`` of object ``O`` from an accessor
       property to a data property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  d. Goto VALIDATED.

14. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    are both true, then

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Writable]]`` field of ``current`` is
       ``false`` and the ``[[Writable]]`` field of ``Desc`` is ``true``.

    2. Goto REJECT, If the ``[[Writable]]`` field of ``current`` is
       ``false``, and the ``[[Value]]`` field of ``Desc`` is present, and
       ``SameValue(Desc.[[Value]], current.[[Value]])`` is ``false``.

  b. Goto VALIDATED.

15. Else, ``IsAccessorDescriptor(current)`` and ``IsAccessorDescriptor(Desc)``
    are both ``true`` so,

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Set]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Set]], current.[[Set]])`` is ``false``.

    2. Goto REJECT, if the ``[[Get]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Get]], current.[[Get]])`` is ``false``.

  b. Goto VALIDATED.

16. **VALIDATED:**
    For each attribute field of ``Desc`` that is present, set the
    correspondingly named attribute of the property named ``P`` of object
    ``O`` to the value of the field.

17. **SUCCESS:**
    If ``O`` is an ``Array`` object:

  a. If ``P`` is ``"length"``, and ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. If ``pendingWriteProtect`` is ``true``, update the property
       (``"length"``) to have ``[[Writable]] = false``.

    4. Goto REJECT, if ``shortenSucceeded`` is ``false``.

  b. If ``P`` is an array index and ``index`` >= ``oldLen``:

    1. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
       This always succeeds, because we've checked in the pre-step that the
       ``"length"`` is writable, and since ``P`` is an array index property,
       the length must still be writable here.

18. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
    internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. If ``IsAccessorDescriptor(Desc)`` is ``true``, then:

      a. Call the ``[[Delete]]`` internal method of ``map`` passing ``P``,
         and ``false`` as the arguments.  (This removes the magic binding
         for ``P``.)

    2. Else (``Desc`` may be generic or data descriptor):

      a. If ``Desc.[[Value]]`` is present, then:

        1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
           ``Desc.[[Value]]``, and ``Throw`` as the arguments.  (This
           updates the bound variable value.)

      b. If ``Desc.[[Writable]]`` is present and its value is ``false``,
         then:

        1. Call the ``[[Delete]]`` internal method of ``map`` passing ``P``
           and ``false`` as arguments.  (This removes the magic binding
           for ``P``, and must happen after a possible update of the
           variable value.)

19. Return ``true``.

20. **REJECT**:
    If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
    otherwise return ``false``.

21. **REJECTRANGE**:
    Throw a ``RangeError`` exception.  Note that this is unconditional
    (thrown even if ``Throw`` is ``false``).

Final version
:::::::::::::

(See above, currently no additional cleanup.)

Delete
------

Related E5 sections:

* E5 Section 8.12.7: default algorithm
* E5 Section 10.5: arguments object

Default algorithm
:::::::::::::::::

1. Let ``desc`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with property name ``P``.

2. If ``desc`` is ``undefined``, then return ``true``.

3. If ``desc.[[Configurable]]`` is ``true``, then

  a. Remove the own property with name ``P`` from ``O``.

  b. Return ``true``.

4. Else if ``Throw`` is true, then throw a ``TypeError`` exception.

5. Return ``false``.

Adding arguments object exotic behavior
:::::::::::::::::::::::::::::::::::::::

The exotic ``[[Delete]]`` behavior for an arguments object containing a
``[[ParameterMap]]`` is described in E5 Section 10.6.

The variant algorithm essentially first runs the default algorithm.
If the default algorithm finishes successfully, the variant will then
possibly delete a magic variable binding.

This is easy to incorporate and results in:

1. Let ``desc`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with property name ``P``.

2. If ``desc`` is ``undefined``, then goto SUCCESS.

3. If ``desc.[[Configurable]]`` is ``true``, then

  a. Remove the own property with name ``P`` from ``O``.

  b. Goto SUCCESS.

4. Else if ``Throw`` is true, then throw a ``TypeError`` exception.

5. Return ``false``.

6. **SUCCESS:**
   If ``O`` is an arguments object which has a ``[[ParameterMap]]``
   internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

     a. Call the ``[[Delete]]`` internal method of ``map`` passing ``P``,
        and ``false`` as the arguments.  (This removes the magic binding
        for ``P``.)

7. Return ``true``.

Notes:

* In steps 2, if ``desc`` is ``undefined``, it seems unnecessary to go to
  step 6 to check the arguments parameter map.  Can a magically bound
  property exist in the parameter map with the underlying property having
  been deleted somehow?

Final version
:::::::::::::

(See above, currently no additional cleanup.)

HasInstance
-----------

Background
::::::::::

The ``[[HasInstance]]`` internal method is referred to in the following
parts of the E5 specification:

* Section 8.6.2: ``[[HasInstance]]`` is introduced as a ``SpecOp(any)``
  -> ``Boolean`` internal method.  Only ``Function`` objects have a
  ``[[HasInstance]]`` method.

* Section 11.8.6: the ``instanceof`` operator, which is the only "caller"
  for ``[[HasInstance]]`` in the E5 specification.

* Section 13.2: when ``Function`` objects are created, ``[[HasInstance]]``
  is set to the algorithm in Section 15.3.5.3.

* Section 15.3.4.5: when bound functions are created using
  ``Function.prototype.bind()``, ``[[HasInstance]]`` is set to the
  algorithm in Section 15.3.4.5.3.

* Section 15.3.4.5.3: ``[[HasInstance]]`` for bound functions.

* Section 15.3.5.3: ``[[HasInstance]]`` for ordinary (non-bound)
  functions.

The ``[[HasInstance]]`` for ordinary functions is (``F`` is the function
object and ``V`` is the argument value, "V instanceof F"):

1. If ``Type(V)`` is not an ``Object``, return ``false``.

2. Let ``O`` be the result of calling the ``[[Get]]`` internal method of
   ``F`` with property name ``"prototype"``.
   (Note: this is the external prototype, not the internal one.)

3. If ``Type(O)`` is not ``Object``, throw a ``TypeError`` exception.

4. Repeat

  a. Let ``V`` be the value of the ``[[Prototype]]`` internal property of
     ``V``.

  b. If ``V`` is ``null``, return ``false``.

  c. If ``O`` and ``V`` refer to the same object, return ``true``.

Notes:

* In step 2, we're fetching the *external prototype*, which may have any
  values.  It might also have been changed after the instance was created.

* Step 4.a steps the internal prototype chain once before the first check.

The ``[[HasInstance]]`` for bound functions is:

1. Let ``target`` be the value of ``F``\ ’s ``[[TargetFunction]]`` internal
   property.

2. If ``target`` has no ``[[HasInstance]]`` internal method, a ``TypeError``
   exception is thrown.

3. Return the result of calling the ``[[HasInstance]]`` internal method of
   ``target`` providing ``V`` as the argument.

Notes:

* In step 3, the ``target`` may be another bound function, so we may need
  to follow an arbitrary number of bound functions before ending up with an
  actual function object.

Combined algorithm
::::::::::::::::::

The two ``[[HasInstance]]`` methods (for bound and non-bound functions)
can be combined to yield:

1. While ``F`` is a bound function:

  a. Set ``F`` to the value of ``F``\ 's ``[[TargetFunction]]`` internal
     property.

  b. If ``F`` has no ``[[HasInstance]]`` internal method, throw a
     ``TypeError`` exception.
     (Note: ``F`` can be another bound function, so we loop until we find
     the non-bound actual function.)

2. If ``Type(V)`` is not an ``Object``, return ``false``.

3. Let ``O`` be the result of calling the ``[[Get]]`` internal method of
   ``F`` with property name ``"prototype"``.
   (Note: this is the external prototype, not the internal one.)

4. If ``Type(O)`` is not ``Object``, throw a ``TypeError`` exception.

5. Repeat

  a. Let ``V`` be the value of the ``[[Prototype]]`` internal property of
     ``V``.

  b. If ``V`` is ``null``, return ``false``.

  c. If ``O`` and ``V`` refer to the same object, return ``true``.

Final version
:::::::::::::

(See above, currently no additional cleanup.)

Preliminary algorithm work
==========================

In this section we look at the internal algorithms and do some preliminary
work of restating them by: inlining algorithms, merging algorithms, looking
at algorithm behavior with some fixed parameters, etc.  Tricky issues of
algorithms are also discussed to some extent.

The purpose of this section is to provide raw material for the sections
dealing with actual exposed algorithms.

CanPut
------

``[[CanPut]]`` indicates whether a ``[[Put]]`` would cause an error or not.
An error is possible in the following cases for object ``O``, property ``P``:

* ``O`` has ``P`` as own property, it is a plain property, and
  ``[[Writable]]`` is false

* ``O`` has ``P`` as own property, it is an accessor property, and is
  missing the ``[[Set]]`` function

* ``P`` is found in ``O``\ 's prototype chain (not in ``O``), it is a plain
  property, and either ``O.[[Extensible]]`` or property ``[[Writable]]``
  is false

* ``P`` is found in ``O``\ 's prototype chain (not in ``O``), it is an
  accessor property, and is missing the ``[[Set]]`` function

* ``P`` is not found in ``O``\ 's prototype chain, and ``O.[[Extensible]]``
  is false

The algorithm in E5 Section 8.12.4 deals with the "own property" case first
and then looks up the property again from the prototype chain.  If a
property is found, the only difference is between steps 2.b and 8.a: the
``[[Extensible]]`` property of the original object ``O`` must be checked
if the property is found in an ancestor, as a ``[[Put]]`` would actually go
into ``O``, extending its set of properties.

The following simplified (and restated) variant should be equivalent and
requires only one prototype chain lookup:

1. ``desc`` = ``O.[[GetProperty]](P)``.

2. If ``desc`` is ``undefined``, return ``O.[[Extensible]]``.

3. If ``IsAccessorDescriptor(desc)``:

  a. If ``desc.[[Set]]`` is ``undefined``, return ``false``.

  b. Else, return ``true``.

4. Else, ``desc`` must be a data descriptor:

  a. (**CHANGED:**) If ``desc`` was not found in the original object ``O``,
     and ``O.[[Extensible]]`` is ``false``, return ``false``.

  b. Return ``desc.[[Writable]]``.

The step denoted with CHANGED reconciles steps 2.b and 8.a of the original
algorithm.  The "found in the original object ``O``" part can be implemented
in many ways:

* Compare object pointers of original object vs. object where property was
  found: works if an object occurs at most once in a prototype chain (which
  should always be the case)

* The prototype chain lookup ``[[GetProperty]]`` also returns an "inherited"
  flag

GetProperty
-----------

``[[GetProperty]]`` is a very straightforward wrapper over
``[[GetOwnProperty]]`` which follows the prototype chain.  Like
``[[GetOwnProperty]]``, it returns a descriptor.

There is no exotic behavior for ``[[GetProperty]]``, the exotic behaviors
only affect ``[[GetOwnProperty]]`` which is called during ``[[GetProperty]]``.

Original algorithm
::::::::::::::::::

1. Let ``prop`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with property name ``P``.

2. If ``prop`` is not ``undefined``, return ``prop``.

3. Let ``proto`` be the value of the ``[[Prototype]]`` internal property of
   ``O``.

4. If ``proto`` is ``null``, return ``undefined``.

5. Return the result of calling the ``[[GetProperty]]`` internal method of
   ``proto`` with argument ``P``.

Eliminating recursion
:::::::::::::::::::::

This is better unwound into a loop (using ``desc`` instead of ``prop``, as
it is more descriptive):

1. Let ``curr`` be ``O``.

2. While ``curr`` is not ``null``:

  a. Let ``desc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``curr`` with property name ``P``.

  b. If ``desc`` is not ``undefined``, return ``desc``.

  c. Let ``curr`` be the value of the ``[[Prototype]]`` internal property of
     ``curr``.

3. Return ``undefined``.

Less nested form
::::::::::::::::

The following is a less "nested" form (note that ``curr`` is guaranteed to
be non-null in the first loop):

1. Let ``curr`` be ``O``.

2. **NEXT:**
   Let ``desc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``curr`` with property name ``P``.

3. If ``desc`` is not ``undefined``, return ``desc``.

4. Let ``curr`` be the value of the ``[[Prototype]]`` internal property of
   ``curr``.

5. If ``curr`` is not ``null``, goto NEXT.

6. Return ``undefined``

.. note:: A maximum prototype chain depth should be imposed as a safeguard
          against loops.  Note that while it should be impossible to create
          prototype loops with Ecmascript code alone, creating them from C
          code *is* possible.

GetProperty with default GetOwnProperty inlined
:::::::::::::::::::::::::::::::::::::::::::::::

``[[GetOwnProperty]]`` is just creating the descriptor from whatever form
properties are stored.  It has exotic behaviors, so the resulting function
is a bit complicated.

The inlined form for default ``[[GetOwnProperty]]`` is essentially:

1. ``curr`` = ``O``

2. **NEXT:**
   If ``curr`` has own property ``P``:

  a. Let ``D`` be a newly created Property Descriptor with no fields.

  b. Let ``X`` be ``curr``\ ’s own property named P.

  c. If ``X`` is a data property, then

    1. Set ``D.[[Value]]`` to the value of ``X``\ ’s ``[[Value]]``
       attribute.

    2. Set ``D.[[Writable]]`` to the value of ``X``\ ’s ``[[Writable]]``
       attribute.

  d. Else ``X`` is an accessor property, so

    1. Set ``D.[[Get]]`` to the value of ``X``\ ’s ``[[Get]]`` attribute.

    2. Set ``D.[[Set]]`` to the value of ``X``\ ’s ``[[Set]]`` attribute.

  e. Set ``D.[[Enumerable]]`` to the value of ``X``\ ’s ``[[Enumerable]]`` attribute.

  f. Set ``D.[[Configurable]]`` to the value of ``X``\ ’s ``[[Configurable]]`` attribute.

  g. Return ``D``.

3. Let ``curr`` be the value of the ``[[Prototype]]`` internal property of
   ``curr``.

4. If ``curr`` is not ``null``, goto NEXT.

5. Return ``undefined``

This is a relatively useless form, because exotic behaviors are missing.

GetProperty with complete GetOwnProperty inlined
::::::::::::::::::::::::::::::::::::::::::::::::

The following inlines ``[[GetOwnProperty]]`` with all exotic behaviors:

1. ``curr`` = ``O``

2. **NEXT:**
   Let ``X`` be ``curr``\ ’s own property named ``P``.
   If ``curr`` doesn’t have an own property with name ``P``:

  a. If ``curr`` is not a ``String`` instance, goto NOTFOUND.

  b. (``String`` object exotic behavior.)
     Let ``str`` be the String value of the ``[[PrimitiveValue]]``
     internal property of ``O`` and ``len`` be the number of
     characters in ``str``.

  c. If ``P`` is ``"length"``:

    1. Return a Property Descriptor with the values:

      * ``[[Value]]: len`` (a primitive number)
      * ``[[Enumerable]]: false``
      * ``[[Writable]]: false``
      * ``[[Configurable]]: false``

  d. If ``P`` is an array index (E5 Section 15.4):

    1. Let ``index`` be ``ToUint32(P)``.

    2. If ``index`` < ``len``, return a Property Descriptor with the values:

      * ``[[Value]]:`` a primitive string of length 1, containing one character
        from ``str`` at position ``index`` (zero based index)
      * ``[[Enumerable]]: true``
      * ``[[Writable]]: false``
      * ``[[Configurable]]: false``

  e. Goto NOTFOUND.

3. Let ``D`` be a newly created Property Descriptor filled as follows:

  a. If ``X`` is a data property:

    1. Set ``D.[[Value]]`` to the value of ``X``\ ’s ``[[Value]]`` attribute.

    2. Set ``D.[[Writable]]`` to the value of ``X``\ ’s ``[[Writable]]`` attribute.

  b. Else ``X`` is an accessor property:

    1. Set ``D.[[Get]]`` to the value of ``X``\ ’s ``[[Get]]`` attribute.

    2. Set ``D.[[Set]]`` to the value of ``X``\ ’s ``[[Set]]`` attribute.

  c. For either type of property:

    1. Set ``D.[[Enumerable]]`` to the value of ``X``\ ’s ``[[Enumerable]]`` attribute.

    2. Set ``D.[[Configurable]]`` to the value of ``X``\ ’s ``[[Configurable]]`` attribute.

4. If ``curr`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Set ``D.[[Value]]`` to the result of calling the ``[[Get]]``
       internal method of ``map`` passing ``P`` as the argument.

5. Return ``D``.

6. **NOTFOUND:**
   Let ``curr`` be the value of the ``[[Prototype]]`` internal property of
   ``curr``.

7. If ``curr`` is not ``null``, goto NEXT.

8. Return ``undefined``

.. note:: This implementation is currently *not* used.  The implementation for
   ``[[GetOwnProperty]]`` is a separate helper.  See ``duk_hobject_props.c``,
   helper functions: ``get_own_property_desc()`` and ``get_property_desc()``.

Get
---

``[[Get]]`` is straightforward; it gets a property descriptor with
``[[GetProperty]]`` and then coerces it to a value.

Get with GetProperty inlined
----------------------------

``[[Get]]`` was covered above when discussion exotic behaviors, so we'll
skip discussing it again here.

``[[Get]]`` is essentially a ``[[GetProperty]]`` followed by coercion of
the descriptor into a value.  For a data descriptor, simply return its
``[[Value]]``.  For a property accessor, simply call its ``[[Get]]``
function.  The descriptor does not need to be created at all, as we're
just interested in the final value.

The following combines both ``[[GetOwnProperty]]`` and ``[[Get]]`` with
exotic behaviors:

1. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

2. ``curr`` = ``O``

3. **NEXT:**
   Let ``X`` be ``curr``\ ’s own property named ``P``.
   If ``curr`` doesn’t have an own property with name ``P``:

  a. If ``curr`` is not a ``String`` instance, goto NOTFOUND.

  b. (``String`` object exotic behavior.)
     Let ``str`` be the String value of the ``[[PrimitiveValue]]``
     internal property of ``O`` and ``len`` be the number of
     characters in ``str``.

  c. If ``P`` is ``"length"``:

    1. Return ``len`` (a primitive number).
       (No need to check for arguments object exotic
       behavior or ``"caller"`` property exotic behavior.)

  d. If ``P`` is an array index (E5 Section 15.4):

    1. Let ``index`` be ``ToUint32(P)``.

    2. If ``index`` < ``len``:

      a. Return a primitive string of length 1, containing one character
         from ``str`` at position ``index`` (zero based index).
         (No need to check for arguments object exotic behavior or
         ``"caller"`` property exotic behavior.)

  e. Goto NOTFOUND.

4. If ``X`` is a data property:

  a. Set ``res`` to the value of ``X``\ ’s ``[[Value]]`` attribute.

  b. Goto FOUND1

5. Else ``X`` is an accessor property:

  a. Let ``getter`` be ``X``\ 's ``[[Get]]`` attribute.

  b. If ``getter`` is ``undefined``:

    1. Return ``undefined``.
       (Note: arguments object exotic behavior for mapped variables cannot
       apply: if the property is an accessor, it can never be in the arguments
       object ``[[ParameterMap]]``.  Also, the ``"caller"`` exotic behavior
       does not apply, since the result ``undefined`` is not a strict mode
       function.  Thus, no "goto FOUND1" here.)

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``O`` as the ``this`` value and
     providing no arguments.

  d. Goto FOUND2.
     (Note: arguments object exotic behavior for mapped variables cannot
     apply: if the property is an accessor, it can never be in the arguments
     object ``[[ParameterMap]]``.  However, the ``"caller"`` exotic behavior
     might apply, at FOUND2.)

6. **FOUND1**:
   If ``curr`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Set ``res`` to the result of calling the ``[[Get]]`` internal method
       of ``map`` passing ``P`` as the argument.

7. **FOUND2**:
   If ``O`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

8. Return ``res``.

9. **NOTFOUND:**
   Let ``curr`` be the value of the ``[[Prototype]]`` internal property of
   ``curr``.

10. If ``curr`` is not ``null``, goto NEXT.

11. Return ``undefined``.
    (Note: no need for exotic behavior checks here; e.g. result is not a
    strict mode function.)

.. note:: The step 5.c gives the object as the ``this`` binding for the
          getter call.  When properties are actually accessed from Ecmascript
          code, the wrappers (property accessor evaluation, ``GetValue()``)
          have a different behavior: the primitive (uncoerced) object is
          given as the ``this`` binding.

DefineOwnProperty callers
-------------------------

``[[DefineOwnProperty]]`` is defined in E5 Section 8.12.9.
It is a complex algorithm which allows the value and attributes of property
``P`` of object ``O`` to be changed.  It is used for ``[[Put]]`` which is
performance relevant and should thus be "inlined" to the extent possible
(see special case analysis below).  It is also used generically when
initializing newly created objects etc, which can also use a simplified
version.

Note: ``[[DefineOwnProperty]]`` allows some counterintuitive property
attributes changes to be made.  The callers in the specification are
supposed to "guard" against these.  For instance:

* A property which is non-configurable but writable *can* be changed
  to non-writable (but not vice versa).  Non-configurability does not
  guarantee that changes cannot be made.

* A property which is configurable but not writable can have its value
  changed by a ``[[DefineOwnProperty]]`` call.  This is allowed because
  a caller could simply change the property to writable, change its
  value, and then change it back to non-writable (this is possible
  because the property is configurable).  The ``[[Put]]`` algorithms
  prevents writing to a non-writable but configurable property with an
  explicit check, ``[[CanPut]]``.

``[[DefineOwnProperty]]`` is referenced by the following property-related
internal algorithms:

* ``FromPropertyDescriptor``, E5 Section 8.10.4

* ``[[Put]]``, E5 Section 8.12.5

* Array's exotic ``[[DefineOwnProperty]]`` relies on the default one, E5
  Section 15.4.5.1

* Argument object's exotic ``[[DefineOwnProperty]]`` relies on the default
  one, E5 Section 10.6

It is used less fundamentally in many places, e.g. to initialize values
(list probably not complete):

* ``CreateMutableBinding``, E5 Section 10.2.1.2.2

* Arguments object setup, E5 Section 10.6

* Array initializer, E5 Section 11.1.4

* Object initializer, E5 Section 11.1.5

* Function object creation, E5 Section 13.2

* ``[[ThrowTypeError]]`` function object, E5 Section 13.2.3

* ``Object.getOwnPropertyNames``, E5 Section 15.2.3.4

* ``Object.defineProperty``, E5 Section 15.2.3.6

* ``Object.seal``, E5 Section 15.2.3.8

* ``Object.freeze``, E5 Section 15.2.3.9

* ``Object.keys``, E5 Section 15.2.3.14

* ``Function.prototype.bind``, E5 Section 15.3.4.5

* ``Array.prototype.concat``, E5 Section 15.4.4.4

* ``Array.prototype.slice``, E5 Section 15.4.4.10

* ``Array.prototype.splice``, E5 Section 15.4.4.12

* ``Array.prototype.map``, E5 Section 15.4.4.19

* ``Array.prototype.filter``, E5 Section 15.4.4.20

* ``String.prototype.match``, E5 Section 15.5.4.10

* ``String.prototype.split``, E5 Section 15.5.4.14

* ``RegExp.prototype.exec``, E5 Section 15.10.6.2

* ``JSON.parse``, E5 Section 15.12.2

* ``JSON.stringify``, E5 Section 15.12.3

DefineOwnProperty for an existing property in Put
-------------------------------------------------

This case arises when a ``[[Put]]`` is performed and the property already
exists.  The property value is updated with a call to
``[[DefineOwnProperty]]`` with a property descriptor only containing
``[[Value]]``.  See E5 Section 8.12.5, step 3.

We can assume that:

* The property exists (checked by ``[[Put]]``)
* The property is a data property (checked by ``[[Put]]``)
* The property cannot be non-writable (checked by ``[[Put]]``, using
  ``[[CanPut]]``)
* The property descriptor is a data descriptor
* The property descriptor is of the form: ``{ [[Value]]: val }``
* Because the property exists, the ``length`` of an ``Array`` object
  cannot change by a write to an array index; however, a write to
  ``"length"`` may delete array elements

More specifically, we know that in the ``[[DefineOwnProperty]]`` algorithm:

 * ``current`` is not ``undefined``
 * ``IsGenericDescriptor(current)`` is ``false``
 * ``IsDataDescriptor(current)`` is ``true``
 * ``IsAccessorDescriptor(current)`` is ``false``
 * ``IsGenericDescriptor(Desc)`` is ``false``
 * ``IsDataDescriptor(Desc)`` is ``true``
 * ``IsAccessorDescriptor(Desc)`` is ``false``

Taking the ``[[DefineOwnProperty]]`` with all exotic behaviors included,
using the above assumptions, eliminating any unnecessary steps, cleaning
up and clarifying, we get:

1. If ``O`` is an ``Array`` object, and ``P`` is ``"length"``, then:

  a. Let ``newLen`` be ``ToUint32(Desc.[[Value]])``.

  b. If ``newLen`` is not equal to ``ToNumber(Desc.[[Value]])``, throw
     a ``RangeError`` exception.  Note that this is unconditional (thrown
     even if ``Throw`` is ``false``).

  c. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a ``length`` data property that
     cannot be deleted or reconfigured.

  d. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. (Note that ``oldLen``
     is guaranteed to be a unsigned 32-bit integer.)

  e. If ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. Goto REJECT, if ``shortenSucceeded`` is ``false``.

    4. Return.

  f. Update the property (``"length"``) value to ``newLen``.

  g. Return.

2. Set the ``[[Value]]`` attribute of the property named ``P`` of object
   ``O`` to the value of ``Desc.[[Value]]``.  (Since it is side effect
   free to update the value with the same value, no check for that case
   is needed.)

3. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
   internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
       ``Desc.[[Value]]``, and ``Throw`` as the arguments.  (This
       updates the bound variable value.)

4. Return ``true``.

Note that step 1 combines the pre-step and post-step for an ``Array``
object ``length`` exotic behavior.  This is only possible if we know
beforehand that the ``"length"`` property is writable (so that the
write never fails and we always reach the post-step).

We'll refine one more time, by eliminating references to ``Desc`` and using
``val`` to refer to ``Desc.[[Value]]``:

1. If ``O`` is an ``Array`` object, and ``P`` is ``"length"``, then:

  a. Let ``newLen`` be ``ToUint32(val)``.

  b. If ``newLen`` is not equal to ``ToNumber(val)``, throw a ``RangeError``
     exception.  Note that this is unconditional (thrown even if ``Throw``
     is ``false``).

  c. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a ``length`` data property that
     cannot be deleted or reconfigured.

  d. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. (Note that ``oldLen``
     is guaranteed to be a unsigned 32-bit integer.)

  e. If ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. Goto REJECT, if ``shortenSucceeded`` is ``false``.

    4. Return.

  f. Update the property (``"length"``) value to ``newLen``.

  g. Return.

2. Set the ``[[Value]]`` attribute of the property named ``P`` of object
   ``O`` to ``val``.  (Since it is side effect free to update the value
   with the same value, no check for that case is needed.)

3. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
   internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
       ``val``, and ``Throw`` as the arguments.  (This updates the bound
       variable value.)

4. Return ``true``.

We'll need this variant later when creating an inlined version for the full
property write processing.

DefineOwnProperty for a non-existent property in Put
----------------------------------------------------

This case arises when a ``[[Put]]`` is performed and the property does not
already exist as an "own property", and no setter in an ancestor captured
the write.  The property is created with a call to ``[[DefineOwnProperty]]``
with a property descriptor containing a ``[[Value]]``, and the following
set to ``true``: ``[[Writable]]``, ``[[Enumerable]]``, ``[[Configurable]]``.
See E5 Section 8.12.5, step 6.

We can assume that:

* The property does not exist (checked by ``[[Put]]``)
* The object is extensible (checked by ``[[Put]]``)
* The property descriptor is a data descriptor
* The property descriptor has the fields:

  + ``[[Value]]: val``
  + ``[[Writable]]: true``
  + ``[[Enumerable]]: true``
  + ``[[Configurable]]: true``

+ If the object is an ``Array``, the property name ``P`` cannot be
  ``"length"`` (as that would exist)

More specifically, we know that in the ``[[DefineOwnProperty]]`` algorithm:

 * ``current`` is ``undefined``

Taking the ``[[DefineOwnProperty]]`` with all exotic behaviors included,
using the above assumptions, and then eliminating any unnecessary steps,
cleaning up and clarifying, we get:

1. If ``O`` is an ``Array`` object and ``P`` is an array index (E5 Section
   15.4), then:

  a. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a length data property that cannot
     be deleted or reconfigured.

  b. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
     (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

  c. Let ``index`` be ``ToUint32(P)``.

  d. Goto REJECT if ``index`` >= ``oldLen`` and ``oldLenDesc.[[Writable]]``
     is ``false``.

2. Create an own data property named ``P`` of object ``O`` whose
   ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
   ``[[Configurable]]`` attribute values are described by ``Desc``.

3. If ``O`` is an ``Array`` object, ``P`` is an array index and
   ``index`` >= ``oldLen``:

  a. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
     This always succeeds, because we've checked in the pre-step that the
     ``"length"`` is writable, and since ``P`` is an array index property,
     the length must still be writable here.

4. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
   internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
       ``Desc.[[Value]]``, and ``Throw`` as the arguments.  (This
       updates the bound variable value.)

5. Return ``true``.

6. **REJECT**:
    If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
    otherwise return ``false``.

This can be refined further by noticing that the arguments object exotic
behavior cannot be triggered if the property does not exist: all magically
bound properties exist initially, and if they are deleted, the magic
variable binding is also deleted.

We can also change the order of property creation and the postponed array
``length`` write because they are both guaranteed to succeed.

So, we get:

1. If ``O`` is an ``Array`` object and ``P`` is an array index (E5 Section
   15.4), then:

  a. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a length data property that cannot
     be deleted or reconfigured.

  b. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
     (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

  c. Let ``index`` be ``ToUint32(P)``.

  d. If ``index`` >= ``oldLen``:

    1. Goto REJECT ``oldLenDesc.[[Writable]]`` is ``false``.

    2. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
       This always succeeds.

2. Create an own data property named ``P`` of object ``O`` whose
   ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
   ``[[Configurable]]`` attribute values are described by ``Desc``.

3. Return ``true``.

4. **REJECT**:
    If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
    otherwise return ``false``.

We'll refine one more time, by eliminating references to ``Desc`` and using
``val`` to refer to ``Desc.[[Value]]``:

1. If ``O`` is an ``Array`` object and ``P`` is an array index (E5 Section
   15.4), then:

  a. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a length data property that cannot
     be deleted or reconfigured.

  b. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
     (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

  c. Let ``index`` be ``ToUint32(P)``.

  d. If ``index`` >= ``oldLen``:

    1. Goto REJECT ``oldLenDesc.[[Writable]]`` is ``false``.

    2. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
       This always succeeds.

2. Create an own data property named ``P`` of object ``O`` whose attributes
   are:

  * ``[[Value]]: val``
  * ``[[Writable]]: true``
  * ``[[Enumerable]]: true``
  * ``[[Configurable]]: true``

3. Return ``true``.

4. **REJECT**:
   If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
   otherwise return ``false``.

Notes:

* If step 2 fails due to an out-of-memory or other internal error, we
  may have updated ``length`` already.  So, switching steps 2 and
  1.d.2 might be prudent (the check in step 1.d.1 *must* be executed
  before writing anything though).

We'll need this variant later when creating an inlined version for the full
property write processing.

DefineOwnProperty for (some) internal object initialization
-----------------------------------------------------------

This case occurs when internal objects or results objects are created by the
implementation.  We can't simply use a normal property write internally,
because we need to set the property attributes to whatever combination is
required by the context (many different property attribute variants are
used throughout the specification).

Because user code has not had any access to the object, we can narrow down
the possibilities a great deal.  Here we assume that:

* Object is extensible
* Property does not exist
* Property does not have exotic behavior and is not virtual
* Property descriptor is a data descriptor, which is fully populated

With these assumptions, eliminating any unnecessary steps, the algorithm is
simply:

1. Create an own data property named ``P`` of object ``O`` whose
   ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
   ``[[Configurable]]`` attribute values are described by ``Desc``.

2. Return ``true``.

This doesn't cover all the initialization cases, but simply illustraes that
very constrained cases are very simple.

Put
---

"Reject" below is shorthand for:

* If ``Throw`` is ``true``, then throw a ``TypeError`` exception; else return.

Original algorithm
::::::::::::::::::

For object ``O``, property ``P``, and value ``V``:

1. If the result of calling the ``[[CanPut]]`` internal method of ``O`` with
   argument ``P`` is false, then

  a. If ``Throw`` is ``true``, then throw a ``TypeError`` exception.

  b. Else return.

2. Let ``ownDesc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``O`` with argument ``P``.

3. If ``IsDataDescriptor(ownDesc)`` is ``true``, then

  a. Let ``valueDesc`` be the Property Descriptor ``{[[Value]]: V}``.

  b. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``valueDesc``, and ``Throw`` as arguments.

  c. Return.

4. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with argument ``P``. This may be either an own or inherited
   accessor property descriptor or an inherited data property descriptor.

5. If ``IsAccessorDescriptor(desc)`` is ``true``, then

  a. Let ``setter`` be ``desc.[[Set]]`` which cannot be ``undefined``.

  b. Call the ``[[Call]]`` internal method of setter providing ``O`` as the
     ``this`` value and providing ``V`` as the sole argument.

6. Else, create a named data property named ``P`` on object ``O`` as follows

  a. Let ``newDesc`` be the Property Descriptor:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  b. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``newDesc``, and ``Throw`` as arguments.

7. Return.

Notes:

* Step 5.a: ``setter`` cannot be ``undefined`` at this point because
  ``[[CanPut]]`` has checked it (and throws an exception if it is
  ``undefined``).

Minimizing prototype traversal
::::::::::::::::::::::::::::::

The ``ownDesc`` check is necessary because a ``[[Put]]`` on an existing own
property is a change of value; a ``[[Put]]`` on an inherited plain property
is an addition of a new property on the *original* target object (not the
ancestor where the inherited property was found).

To minimize prototype traversal, these can be combined as follows (with
some cleanup):

1. If the result of calling the ``[[CanPut]]`` internal method of ``O`` with
   argument ``P`` is false, then Reject.

2. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with argument ``P``.
   (Note: here we assume that we also get to know whether the property was
   found in ``O`` or in its ancestor.)

3. If ``IsAccessorDescriptor(desc)`` is ``true``, then:

  a. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``O`` as the ``this`` value and providing ``V`` as the sole argument.
     (Note: ``desc.[[Set]]`` cannot be ``undefined``, as this is checked by
     ``[[CanPut]]``.)

4. Else if ``desc`` was found in ``O`` directly (as an "own data property"),
   then:

  a. Let ``valueDesc`` be the Property Descriptor ``{[[Value]]: V}``.

  b. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``valueDesc``, and ``Throw`` as arguments.

5. Else ``desc`` is an inherited data property or ``undefined``, then:

  a. Let ``newDesc`` be the Property Descriptor:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  b. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``newDesc``, and ``Throw`` as arguments.

6. Return.

This still travels the prototype chain twice: once for ``[[CanPut]]``, and
a second time for the actual ``[[Put]]``.  ``[[CanPut]]`` can be inlined
quite easily, as it does very similar checks as ``[[Put]]``.

The result is:

1. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with argument ``P``.
   (Note: here we assume that we also get to know whether the property was
   found in ``O`` or in its ancestor.)

2. If ``IsAccessorDescriptor(desc)`` is ``true``, then:

  a. If ``desc.[[Set]]`` is ``undefined``, Reject.

  b. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``O`` as the ``this`` value and providing ``V`` as the sole argument.

3. Else if ``desc`` is an inherited (data) property, then:

  a. If ``O.[[Extensible]]`` is ``false``, Reject.

  b. If ``desc.[[Writable]]`` is ``false``, Reject.

  c. Let ``newDesc`` be the Property Descriptor:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  d. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``newDesc``, and ``Throw`` as arguments.

4. Else if ``desc`` was not found (is ``undefined``):

  a. If ``O.[[Extensible]]`` is ``false``, Reject.

  b. Let ``newDesc`` be the Property Descriptor:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  c. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``newDesc``, and ``Throw`` as arguments.

5. Else ``desc`` was found in ``O`` directly (as an "own data property"),
   then:

  a. If ``desc.[[Writable]]`` is ``false``, Reject.

  b. Let ``valueDesc`` be the Property Descriptor ``{[[Value]]: V}``.

  b. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``valueDesc``, and ``Throw`` as arguments.

6. Return.

The above can be further refined to (making also the modification required
to ``[[GetProperty]]`` explicit):

1. Let ``desc`` and ``inherited`` be the result of calling the
   ``[[GetProperty]]`` internal method of ``O`` with argument ``P``.

2. If ``IsAccessorDescriptor(desc)`` is ``true``, then:

  a. If ``desc.[[Set]]`` is ``undefined``, Reject.

  b. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``O`` as the ``this`` value and providing ``V`` as the sole argument.

3. Else if ``desc`` is not ``undefined`` and ``inherited`` is ``false``
   (own data property), then:

  a. If ``desc.[[Writable]]`` is ``false``, Reject.

  b. Let ``valueDesc`` be the Property Descriptor ``{[[Value]]: V}``.

  b. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``valueDesc``, and ``Throw`` as arguments.

3. Else ``desc`` is an inherited (data) property or ``undefined``:

  a. If ``O.[[Extensible]]`` is ``false``, Reject.

  b. If ``desc`` is not ``undefined`` and ``desc.[[Writable]]`` is
     ``false``, Reject.
     (In other words: ``desc`` was inherited and is non-writable.)

  c. Let ``newDesc`` be the Property Descriptor:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  d. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` passing
     ``P``, ``newDesc``, and ``Throw`` as arguments.

4. Return.

This can be further improved in actual C code.

Inlining GetProperty
::::::::::::::::::::

When actually implementing, it's useful to "inline" the ``[[GetProperty]]``
loop, which changes the code structure quite a bit:

1. Set ``curr`` to ``O``.

2. While ``curr`` !== ``null``:

  a. If ``O`` does not have own property ``P``:

   1. Set ``curr`` to ``curr.[[Prototype]]``

   1. Continue (while loop)

  b. Let ``desc`` be the descriptor for own property ``P``

  c. If ``IsDataDescriptor(desc)``:

    1. If ``curr`` != ``O`` (property is an inherited data property):
       (Note: assumes there are no prototype loops.)

      a. If ``O.[[Extensible]`` is ``false``, Reject.

      b. If ``desc.[[Writable]]`` is ``false``, Reject.

      c. Let ``newDesc`` be a property descriptor with values:

        * ``[[Value]]: V``

        * ``[[Writable]]: true``

        * ``[[Enumerable]]: true``

        * ``[[Configurable]]: true}``

      d. Call ``O.[[DefineOwnProperty]](P, newDesc, Throw)``.

    2. Else (property is an own data property):

      a. If ``desc.[[Writable]]`` is ``false``, Reject.

      b. Let ``valueDesc`` be ``{ [[Value]]: V }``.

      c. Call ``O.[[DefineOwnProperty]](P, valueDesc, Throw)``.

  e. Else (property is an accessor):

   1. If ``desc.[[Set]]`` is ``undefined``, Reject.

   2. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
      ``O`` as the ``this`` value and providing ``V`` as the sole argument.

  f. Return.

3. Property was not found in the prototype chain:

  a. If ``O.[[Extensible]]`` is ``false``, Reject.

  b. Let ``newDesc`` be a property descriptor with values:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  c. Call ``O.[[DefineOwnProperty]](P, newDesc, Throw)``.

Less nested form
::::::::::::::::

The following is a less "nested" form (note that ``curr`` is guaranteed to
be non-null in the first loop):

1. Let ``curr`` be ``O``.

2. **NEXT:**
   Let ``desc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``curr`` with property name ``P``.

3. If ``desc`` is ``undefined``:

  a. Let ``curr`` be the value of the ``[[Prototype]]`` internal property
     of ``curr``.

  b. If ``curr`` is not ``null``, goto NEXT.

  c. If ``O.[[Extensible]]`` is ``false``, Reject.

  d. Let ``newDesc`` be a property descriptor with values:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  e. Call ``O.[[DefineOwnProperty]](P, newDesc, Throw)``.

  f. Return.

4. If ``IsDataDescriptor(desc)``:

  a. If ``curr`` != ``O`` (property is an inherited data property):
     (Note: assumes there are no prototype loops.)

    1. If ``O.[[Extensible]`` is ``false``, Reject.

    2. If ``desc.[[Writable]]`` is ``false``, Reject.

    3. Let ``newDesc`` be a property descriptor with values:

      * ``[[Value]]: V``

      * ``[[Writable]]: true``

      * ``[[Enumerable]]: true``

      * ``[[Configurable]]: true}``

    4. Call ``O.[[DefineOwnProperty]](P, newDesc, Throw)``.

  b. Else (property is an own data property):

    1. If ``desc.[[Writable]]`` is ``false``, Reject.

    2. Let ``valueDesc`` be ``{ [[Value]]: V }``.

    3. Call ``O.[[DefineOwnProperty]](P, valueDesc, Throw)``.

5. Else (property is an accessor):

  a. If ``desc.[[Set]]`` is ``undefined``, Reject.

  b. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``O`` as the ``this`` value and providing ``V`` as the sole argument.

6. Return.

Note about PutValue
:::::::::::::::::::

Note that ``PutValue()`` has a ``[[Put]]`` variant with two exotic
behaviors related to object coercion.  The above algorithm does not
take those into account.

Property descriptor algorithms
------------------------------

E5 Section 8.10 describes descriptor related algorithms:

 * ``IsAccessorDescriptor(desc)``: ``true``, if ``desc`` contains *either*
   ``[[Set]]`` or ``[[Get]]``

 * ``IsDataDescriptor(desc)``: ``true``, if ``desc`` contains *either*
   ``[[Value]]`` or ``[[Writable]]``

 * ``IsGenericDescriptor(desc)``: ``true`` if both
   ``IsAccessorDescriptor(desc)`` and ``IsGenericDescriptor`` are
   ``false``; concretely:

  * ``desc`` contains none of the following: ``[[Set]]``, ``[[Get]]``,
    ``[[Value]]``, ``[[Writable]]``

  * ``desc`` may contain: ``[[Enumerable]]``, ``[[Configurable]]``

A property descriptor may be fully populated or not.  If fully populated,
it is either a data descriptor or an access descriptor, not a generic
descriptor.

A property descriptor may not be both a data descriptor and access descriptor
(this is stated in E5 Section 8.10).  However, an argument to e.g.
``Object.defineProperty()`` may naturally contain e.g. ``"set"`` and
``"value"`` keys.  In this case:

 * ``defineProperty()`` uses ``ToPropertyDescriptor()`` to convert the
   Ecmascript object into an internal property descriptor

 * ``ToPropertyDescriptor()`` creates a property descriptor and throws a
   ``TypeError`` if the descriptor contains conflicting fields

``ToPropertyDescriptor()`` also coerces the values in its argument
Ecmascript object (e.g. it uses ``ToBoolean()`` for the flags).
The behavior of ``ToPropertyDescriptor()`` is probably easiest to "inline"
into wherever it is needed.  The E5 specification refers to
``ToPropertyDescriptor`` only in ``Object.defineProperty()`` and
``Object.defineProperties()``.

The current implementation does not have partial internal property
descriptors (internal property value and attributes are always fully
populated).

ToPropertyDescriptor
--------------------

The ``ToPropertyDescriptor()`` algorithm is specified in E5 Section 8.10.5
and is as follows:

1. If ``Type(Obj)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``desc`` be the result of creating a new Property Descriptor that
   initially has no fields.

3. If the result of calling the ``[[HasProperty]]`` internal method of
   ``Obj`` with argument ``"enumerable"`` is ``true``, then:

  a. Let ``enum`` be the result of calling the ``[[Get]]`` internal method
     of ``Obj`` with ``"enumerable"``.

  b. Set the ``[[Enumerable]]`` field of ``desc`` to ``ToBoolean(enum)``.

4. If the result of calling the ``[[HasProperty]]`` internal method of
   ``Obj`` with argument ``"configurable"`` is ``true``, then:

  a. Let ``conf`` be the result of calling the ``[[Get]]`` internal method
     of ``Obj`` with argument ``"configurable"``.

  b. Set the ``[[Configurable]]`` field of ``desc`` to ``ToBoolean(conf)``.

5. If the result of calling the ``[[HasProperty]]`` internal method of
   ``Obj`` with argument ``"value"`` is ``true``, then:

  a. Let ``value`` be the result of calling the ``[[Get]]`` internal method
     of ``Obj`` with argument ``“value”``.

  b. Set the ``[[Value]]`` field of ``desc`` to ``value``.

6. If the result of calling the ``[[HasProperty]]`` internal method of
   ``Obj`` with argument ``"writable"`` is ``true``, then:

  a. Let ``writable`` be the result of calling the ``[[Get]]`` internal
     method of ``Obj`` with argument ``"writable"``.

  b. Set the ``[[Writable]]`` field of ``desc`` to ``ToBoolean(writable)``.

7. If the result of calling the ``[[HasProperty]]`` internal method of
   ``Obj`` with argument ``"get"`` is ``true``, then:

  a. Let ``getter`` be the result of calling the ``[[Get]]`` internal
     method of ``Obj`` with argument ``"get"``.

  b. If ``IsCallable(getter)`` is ``false`` and ``getter`` is not
     ``undefined``, then throw a ``TypeError`` exception.

  c. Set the ``[[Get]]`` field of ``desc`` to ``getter``.

8. If the result of calling the ``[[HasProperty]]`` internal method of
   ``Obj`` with argument ``"set"`` is ``true``, then:

  a. Let ``setter`` be the result of calling the ``[[Get]]`` internal
     method of ``Obj`` with argument ``"set"``.

  b. If ``IsCallable(setter)`` is ``false`` and ``setter`` is not
     ``undefined``, then throw a TypeError exception.

  c. Set the ``[[Set]]`` field of ``desc`` to ``setter``.

9. If either ``desc.[[Get]]`` or ``desc.[[Set]]`` are present, then:

  a. If either ``desc.[[Value]]`` or ``desc.[[Writable]]`` are present,
     then throw a ``TypeError`` exception.

10. Return ``desc``.

Notes:

* Since ``[[Get]]`` is used to read the descriptor value fields, they can
  be inherited from a parent object, and they can also be accessors.

* Setter/getter values must be either callable or ``undefined`` if they are
  present.  In particular, ``null`` is not an allowed value.

* Any call to ``[[Get]]`` may cause an exception (e.g. if the property is
  an accessor with a throwing getter).  In addition, there are explicit
  exceptions for object type check and setter/getter check.  The order of
  checking and coercion thus matters, at least if the errors thrown have
  a message indicating the failing check.  All the exceptions are of the
  same type (``TypeError``), so a chance in ordering is not strictly a
  compliance issue (there are no guaranteed error messages).

* ``ToBoolean()`` has no side effects and is guaranteed to succeed.

The algorithm in the specification is expressed quite verbosely; the
following is a reformulation with less text, the target object has also
been renamed to ``O``:

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``desc`` be a new, empty Property Descriptor.

3. If ``O.[[HasProperty]]("enumerable")`` === ``true``, then
   set ``desc.[[Enumerable]]`` to ``ToBoolean(O.[[Get]]("enumerable"))``.

4. If ``O.[[HasProperty]]("configurable")`` === ``true``, then
   set ``desc.[[Configurable]]`` to ``ToBoolean(O.[[Get]]("configurable"))``.

5. If ``O.[[HasProperty]]("value")`` === ``true``, then
   set ``desc.[[Value]]`` to ``O.[[Get]]("value")``.

6. If ``O.[[HasProperty]]("writable")`` === ``true``, then
   set ``desc.[[Writable]]`` to ``ToBoolean(O.[[Get]]("writable"))``.

7. If ``O.[[HasProperty]]("get")`` === ``true``, then:

  a. Set ``desc.[[Get]]`` to ``O.[[Get]]("get")``.

  b. If ``desc.[[Get]]`` !== ``undefined`` and
     ``IsCallable(desc.[[Get]])`` === ``false``, then
     throw a ``TypeError`` exception.

8. If ``O.[[HasProperty]]("set")`` === ``true``, then:

  a. Set ``desc.[[Set]]`` to ``O.[[Get]]("set")``.

  b. If ``desc.[[Set]]`` !== ``undefined`` and
     ``IsCallable(desc.[[Set]])`` === ``false``, then
     throw a ``TypeError`` exception.

9. If either ``desc.[[Get]]`` or ``desc.[[Set]]`` are present, then:

  a. If either ``desc.[[Value]]`` or ``desc.[[Writable]]`` are present,
     then throw a ``TypeError`` exception.

10. Return ``desc``.

NormalizePropertyDescriptor
---------------------------

This algorithm is not defined in the E5 specification, but is used as an
internal helper for implementing ``Object.defineProperties()`` and
``Object.defineProperty()``.

The algorithm is a variant of ``ToPropertyDescriptor()`` which, instead of
an internal descriptor, outputs an equivalent Ecmascript property descriptor
which has been fully validated, and contains only "own" data properties.
If the resulting Ecmascript object, ``desc``, is later given to
``ToPropertyDescriptor()``:

* The call cannot fail.

* The call will yield the same internal descriptor as if given the
  original object.

* There can be no user visible side effects, because ``desc`` only
  contains plain (own) values.

For instance, if the input property descriptor were::

  {
    get value() { return "test"; },
    writable: 0.0,
    configurable: "nonempty",
    enumerable: new Date(),
    additional: "ignored"   // ignored, not relevant to a descriptor
  }

the normalized descriptor would be::

  {
    value: "test",
    writable: false,
    configurable: true,
    enumerable: true
  }

(The example doesn't illustrate the fact that inherited properties are
converted to "own" properties.)

The algorithm is as follows:

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``desc`` be a new, empty Object.

3. If ``O.[[HasProperty]]("enumerable")`` === ``true``, then
   call ``desc.[[Put]]`` with the arguments
   ``"enumerable"``, ``ToBoolean(O.[[Get]]("enumerable"))`` and ``true``.

4. If ``O.[[HasProperty]]("configurable")`` === ``true``, then
   call ``desc.[[Put]]`` with the arguments
   ``"configurable"``, ``ToBoolean(O.[[Get]]("configurable"))`` and ``true``.

5. If ``O.[[HasProperty]]("value")`` === ``true``, then
   call ``desc.[[Put]]`` with the arguments
   ``"value"``, ``O.[[Get]]("value")`` and ``true``.

6. If ``O.[[HasProperty]]("writable")`` === ``true``, then
   call ``desc.[[Put]]`` with the arguments
   ``"writable"``, ``ToBoolean(O.[[Get]]("writable"))`` and ``true``.

7. If ``O.[[HasProperty]]("get")`` === ``true``, then:

  a. Let ``getter`` be ``O.[[Get]]("get")``.

  b. If ``getter`` !== ``undefined`` and
     ``IsCallable(getter)`` === ``false``, then
     throw a ``TypeError`` exception.

  c. Call ``desc.[[Put]]`` with the arguments
     ``"get"``, ``getter`` and ``true``.

8. If ``O.[[HasProperty]]("set")`` === ``true``, then:

  a. Let ``setter`` be ``O.[[Get]]("set")``.

  b. If ``setter`` !== ``undefined`` and
     ``IsCallable(setter)`` === ``false``, then
     throw a ``TypeError`` exception.

  c. Call ``desc.[[Put]]`` with the arguments
     ``"set"``, ``setter`` and ``true``.

9. Validation:

  a. Let ``g`` be ``desc.[[HasProperty]]("get")``.

  b. Let ``s`` be ``desc.[[HasProperty]]("set")``.

  c. Let ``v`` be ``desc.[[HasProperty]]("value")``.

  d. Let ``w`` be ``desc.[[HasProperty]]("writable")``.

  e. If ``(g || s) && (v || w)`` then throw a ``TypeError`` exception.

10. Return ``desc``.

Notes:

* The third argument to ``desc.[[Put]]`` is the ``Throw`` flag.  The value
  is irrelevant as the ``[[Put]]`` calls cannot fail.

GETPROP: exposed property get algorithm
=======================================

Background
----------

Consider the following expression::

  x = y[z]

The following happens compile time:

* ``z`` is parsed as an identifier reference

* ``y`` is parsed as an identifier reference

* ``y[z]`` is parsed as a property accessor (E5 Section 11.2.1)

* When the simple assignment is parsed, the ``y[z]`` compiler knows that
  the property accessor is used as a right-hand-side value, so it emits
  whatever internal bytecode is required to read the property value
  during execution
  
The following happens run time:

* The compiled code contains the sequence described in E5 Section 11.2.1:

  + ``baseValue = GetValue(y)``, where ``y`` is the identifier reference

  + ``propertyNameValue = GetValue(z)``, where ``z`` is the identifier reference

  + ``CheckObjectCoercible(baseValue)``, which throws a ``TypeError`` if the
    ``baseValue`` is ``null`` or ``undefined``

  + Create a property reference with ``baseValue`` as the base reference and
    ``ToString(propertyNameValue)`` as the property name (and strict flag
    based on current code strictness)

* Call ``GetValue()`` for the property reference.  This results in the
  following sub-steps of E5 Section 8.7.1 to be executed:

  + ``base`` is the result of ``GetValue(y)`` (identifier lookup result
    directly)

  + The referenced name is ``ToString(GetValue(z))`` (identifier lookup
    result with coercion)

  + If ``base`` is not a primitive: use ``[[Get]]`` directly for
    ``base`` and the referenced name

  + Else use a variant for ``[[Get]]``

The ``[[Get]]`` variant for a primitive base is specified explicitly in
E5 Section 8.7.1.  This seems a bit odd, as it seems equivalent to:

* Let ``O`` be ``ToObject(base)``

* Call ``[[Get]]`` for ``O`` and referenced name

However, *this is not the case*.  There is a subtle difference in the case
that the property is an accessor.  Normally the ``this`` binding for the
getter is the object given to ``[[Get]]``.  Here the ``this`` binding is
the *uncoerced primitive value*.

This leads to externally visible behavior, illustrated in the following::

  // add test getter
  Object.defineProperty(String.prototype, 'test', { 
    get: function() { print(typeof this); },
    set: function(x) { print(typeof this); },
  });

  "foo".test;  // prints 'string'

  var s = new String("foo");
  s.test;      // prints 'object'

Behavior in Ecmascript implementations seems to vary:

* NodeJS / V8: prints 'string' and 'object' as expected
* Rhino: prints 'object' and 'object'
* Smjs: prints 'object' and 'object'

``GetValue()`` allows the caller to skip creation of the coerced object
(which is one of: a ``Boolean``, a ``Number``, or a ``String``; see E5
Section 9.9, ``ToObject()``).

Note: the replacement ``[[Get]]`` overrides whatever ``[[Get]]`` function
would normally be used for the target object.  For instance, if there were
some primitive-to-object coercion which created an arguments object, the
arguments object exotic ``[[Get]]`` behavior would be skipped.  However,
since the arguments and ``Function`` objects are the only objects with
non-default ``[[Get]]``, this is not an issue in practice.

First draft
-----------

When the property accessor is created, the base reference and property
name are "coerced" to a value using ``GetValue()``.  In the example
above, this causes ``x``\ 's and ``foo``\ 's values to be looked up.
These correspond to steps 1-4 of the property accessor expression in
E5 Section 11.2.1.  When compiling, these are converted into whatever
code is necessary to fetch the two values into VM registers.

The relevant part begins after that in steps 5-8, which first perform
some coercions and then create a property accessor.  The accessor is
then acted upon by ``GetValue()``, and ultimately ``[[Get]]`` or its
variant.

Combining all of these, we get the first draft (for base value ``O``
and property name value ``P``):

1. Let ``orig`` be ``O``.
   (Remember the uncoerced original for a possible getter call.)

2. Call ``CheckObjectCoercible`` with ``O`` as argument.  In practice: if
   ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. Let ``O`` be ``ToObject(O)``.
   (This is side effect free.)

5. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

6. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

7. If ``desc`` is ``undefined``, return ``undefined``.

8. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

9. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``orig`` as the ``this`` value and
     providing no arguments.
     (Note: the difference to a basic ``[[Get]]`` is that the getter ``this``
     binding is the original, uncoerced object.)

10. If ``orig`` is a ``Function`` object or an ``arguments`` object which
    contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

11. Return ``res``.

Notes:

* Steps 2-3 come from the property accessor evaluation rules in E5 Section
  11.2.1.  In particular, ``CheckObjectCoercible()`` is called before the
  key is coerced to a string.  Since the key string coercion may have side
  effects, the order of evaluation matters.

  Note that ``ToObject()`` has no side effects (this can be seen from a
  case by case inspection), so steps 3 and 4 can be reversed.

* Step 4 comes from ``GetValue()``.

* Steps 5 and forward come from ``[[Get]]``; here with exotic behaviors
  inlined, but ``[[GetProperty]]`` not inlined.

We could inline the ``[[GetProperty]]`` call to the algorithm.  However,
because the current implementation doesn't do so, that has been omitted
for now.

Improving type checking of base value
-------------------------------------

A variant where steps 3 and 4 are reversed and expanded is as follows:

1. Let ``orig`` be ``O``.
   (Remember the uncoerced original for a possible getter call.)

2. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part.)

  b. Else if ``O`` is a boolean, a number, or a string, set ``O`` to
     ``ToObject(O)``.

  c. Else if ``O`` is an object, do nothing.

  d. Throw a ``TypeError``.
     (Note that this case should not happen, as steps a-c are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

5. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

6. If ``desc`` is ``undefined``, return ``undefined``.

7. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

8. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``orig`` as the ``this`` value and
     providing no arguments.
     (Note: the difference to a basic ``[[Get]]`` is that the getter ``this``
     binding is the original, uncoerced object.)

9. If ``orig`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

10. Return ``res``.

Avoiding temporary objects
--------------------------

If the base value is not an object, step 4 in the above algorithm creates
a temporary object given to ``[[GetProperty]]`` for a property descriptor
lookup.  The first object in the prototype chain is the temporary object,
while the rest are already established non-temporary objects.

If we knew that the property ``P`` could never be an *own property* of the
temporary object, we could skip creation of the temporary object altogether.
Instead, we could simply start ``[[GetProperty]]`` from the internal
prototype that the coerced object would get without actually creating the
object.

Since the coerced object is created by ``ToObject`` from a primitive value,
we know that it is a ``Boolean`` instance, a ``Number`` instance, or a ``String``
instance (see E5 Section 9.9).  The "own properties" of these are:

* ``Boolean``: none
* ``Number``: none
* ``String``: ``"length"`` and index properties for string characters

So, the coercion can be skipped safely for everything except ``String``\ s.
This is unfortunate, because it is conceivably the string primitive value
which is most likely to be accessed through a coercion, e.g. as in::

  var t = "my string";
  print(t.length);

In any case, avoiding temporary creation for everything but ``Strings``
can be worked into the algorithm e.g. as follows:

1. Let ``orig`` be ``O``.
   (Remember the uncoerced original fora possible getter call.)

2. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part.)

  b. If ``O`` is a boolean: set ``O`` to the built-in ``Boolean``
     prototype object (skip creation of temporary)

  c. Else if ``O`` is a number: set ``O`` to the built-in ``Number``
     prototype object (skip creation of temporary)

  d. Else if ``O`` is a string, set ``O`` to ``ToObject(O)``.

  e. Else if ``O`` is an object, do nothing.

  f. Else, throw a ``TypeError``.
     (Note that this case should not happen, as steps a-e are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

5. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

6. If ``desc`` is ``undefined``, return ``undefined``.

7. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

8. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``orig`` as the ``this`` value and
     providing no arguments.
     (Note: the difference to a basic ``[[Get]]`` is that the getter ``this``
     binding is the original, uncoerced object.)

9. If ``orig`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

10. Return ``res``.

If we change step 2.d to get the related string value (length or character
of the string) directly, no temporaries need to be created due to coercion.
However, if the property name ``P`` is checked, it needs to be string coerced
which happens only later in step 3.  If we add a separate coercion to step 2.d,
``P`` will be coerced twice unless step 3 is then explicitly skipped; this is
not an issue as the latter coercion is a NOP and can in any case be easily
skipped.

This variant is as follows:

1. Let ``orig`` be ``O``.
   (Remember the uncoerced original for a possible getter call.)

2. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part.)

  b. If ``O`` is a boolean: set ``O`` to the built-in ``Boolean``
     prototype object (skip creation of temporary)

  c. Else if ``O`` is a number: set ``O`` to the built-in ``Number``
     prototype object (skip creation of temporary)

  d. Else if ``O`` is a string:

    1. Set ``P`` to ``ToString(P)``.
       (This may have side effects if ``P`` is an object.)

    2. If ``P`` is ``length``, return the length of the primitive string
       value as a number.

    3. If ``P`` is a valid array index within the string length, return
       a one-character substring of the primitive string value at the
       specified index.

    4. Else, set ``O`` to the built-in ``String`` prototype object
       (skip creation of temporary)

    5. Goto LOOKUP.  (Avoid double coercion of ``P``.)

  e. Else if ``O`` is an object, do nothing.

  f. Else, throw a ``TypeError``.
     (Note that this case should not happen, as steps a-e are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. **LOOKUP:**
   If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

5. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

6. If ``desc`` is ``undefined``, return ``undefined``.

7. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

8. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``orig`` as the ``this`` value and
     providing no arguments.
     (Note: the difference to a basic ``[[Get]]`` is that the getter ``this``
     binding is the original, uncoerced object.)

9. If ``orig`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

10. Return ``res``.

Fast path for array indices
---------------------------

When the property name is a number and a *valid array index*, we'd prefer
to be able to lookup the property without coercing the number to a string.
This "fast path" needs to work for the common cases; rare cases can go
through the ordinary algorithm which requires a ``ToString()`` coercion.

There are many ways to do a (compliant) fast path.  The simple case we're
considering here is the case when the target object has an "own property"
matching the property name (a number).

A simple "shallow fast path" could be:

* If ``P`` is a whole number in the range [0,2**32-2] (a valid array index)
  AND ``O`` has an array part
  AND ``O`` has no conflicting "exotic behaviors", then:

  + Let ``idx`` be the array index represented by ``P``

  + If the array part of ``O`` contains ``idx`` and the key exists,
    read and return the value.  Note that the value can be ``undefined``

* Else use normal algorithm.

Some notes:

* The behavior of the fast path must match the behavior of the normal
  algorithm exactly (including side effects).  This should be the case
  here, but can be verified by simulating the normal algorithm with the
  assumption of a number as a property name, with the target property
  present as an "own data property" of the target object.

* The conflicting exotic behaviors are currently: ``String`` object exotic
  behavior, and arguments object exotic behavior.  Array exotic behaviors
  are not conflicting for read operations.

* A certain key in the array can be defined even if the value is ``undefined``.
  The check is whether the key has been defined, i.e. ``[[HasProperty]]``
  would be true.  Internally, the value "undefined unused" is used to denote
  unused entries with unused keys, while the value "undefined actual"
  represents an undefined value with a defined key.  For instance, the
  following defines an array key::

    var a = [];
    a[10] = undefined;  // "10" will now enumerate

* The fast path avoids the ``ToString()`` coercion which *may*, in general,
  have side effects (at least for objects).  However, the fast path only
  applies if ``P`` is a number, and the ``ToString()`` coercion of a number
  is side effect free.

* If the array part does *not* contain the key, the normal algorithm is
  always used, regardless of whether the ancestors contain the key or not.
  This means that if a non-existent key is accessed from the array (even
  if the index is within the current array length), string interning will
  be required with this fast path.  For instance::

    var a = [];
    a[0] = 'foo';
    a[2] = 'bar';

    // fast path ok, no string interning
    print(a[0]);

    // fast path fails, string interned but still not found
    print(a[1]);

Inlining the above shallow fast path with the variant which avoids temporaries
altogether produces:

1. Let ``orig`` be ``O``.
   (Remember the uncoerced original for a possible getter call.)

2. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part.)

  b. If ``O`` is a boolean: set ``O`` to the built-in ``Boolean``
     prototype object (skip creation of temporary)

  c. Else if ``O`` is a number: set ``O`` to the built-in ``Number``
     prototype object (skip creation of temporary)

  d. Else if ``O`` is a string:

    1. Set ``P`` to ``ToString(P)``.
       (This may have side effects if ``P`` is an object.)

    2. If ``P`` is ``length``, return the length of the primitive string
       value as a number.

    3. If ``P`` is a valid array index within the string length, return
       a one-character substring of the primitive string value at the
       specified index.

    4. Else, set ``O`` to the built-in ``String`` prototype object
       (skip creation of temporary)

    5. Goto LOOKUP.  (Avoid double coercion of ``P``.)

  e. Else if ``O`` is an object:

    1. Array fast path: If ``O`` is an object (always true here)
       AND ``P`` is a number and a valid array index (whole number in [0,2**32-2])
       AND ``O`` internal representation has an array part
       AND ``O`` does not have conflicting exotic behaviors (cannot have
       ``String`` or arguments exotic behaviors, may have ``Array``
       behavior), then:

      a. Let ``idx`` be the array index represented by ``P``

      b. If the array part of ``O`` contains ``idx`` and the key exists,
         read and return that value.
         (Note: ``ToString(P)`` is skipped, but it would have no side
         effects as ``P`` is a number.  The ``"caller"`` check for ``P``
         is also skipped, but it would never match because ``P`` is a
         number.)

  f. Else, Throw a ``TypeError``.
     (Note that this case should not happen, as steps a-e are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. **LOOKUP:**
   If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

5. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

6. If ``desc`` is ``undefined``, return ``undefined``.

7. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

8. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``orig`` as the ``this`` value and
     providing no arguments.
     (Note: the difference to a basic ``[[Get]]`` is that the getter ``this``
     binding is the original, uncoerced object.)

9. If ``orig`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

10. Return ``res``.

We can further improve this by adding a fast path for the case where ``O``
is a primitive string (in step 2.d):

1. Let ``orig`` be ``O``.
   (Remember the uncoerced original fora possible getter call.)

2. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part; the throw is
     unconditional.)

  b. If ``O`` is a boolean: set ``O`` to the built-in ``Boolean``
     prototype object (skip creation of temporary)

  c. Else if ``O`` is a number: set ``O`` to the built-in ``Number``
     prototype object (skip creation of temporary)

  d. Else if ``O`` is a string:

    1. If ``P`` is a number, is a whole number, a valid array index, and
       within the string length, return a one-character substring of the
       primitive string value at the specified index.
       (Note: ``ToString(P)`` is skipped, but it would have no side
       effects as ``P`` is a number.  The ``"caller"`` check for ``P``
       is also skipped, but it would never match because ``P`` is a
       number.)

    2. Set ``P`` to ``ToString(P)``.
       (This may have side effects if ``P`` is an object.)

    3. If ``P`` is ``length``, return the length of the primitive string
       value as a number.
       (Note: The ``"caller"`` check for ``P`` is skipped, but would
       never match.)

    4. If ``P`` is a valid array index within the string length, return
       a one-character substring of the primitive string value at the
       specified index.
       (Note: The ``"caller"`` check for ``P`` is skipped, but would
       never match.)

    5. Else, set ``O`` to the built-in ``String`` prototype object
       (skip creation of temporary)

    6. Goto LOOKUP.  (Avoid double coercion of ``P``.)

  e. Else if ``O`` is an object:

    1. Array fast path: If ``O`` is an object (always true here)
       AND ``P`` is a number and a valid array index (whole number in [0,2**32-2])
       AND ``O`` internal representation has an array part
       AND ``O`` does not have conflicting exotic behaviors (cannot have
       ``String`` or arguments exotic behaviors, may have ``Array``
       behavior), then:

      a. Let ``idx`` be the array index represented by ``P``

      b. If the array part of ``O`` contains ``idx`` and the key exists,
         read and return that value.
         (Note: ``ToString(P)`` is skipped, but it would have no side
         effects as ``P`` is a number.  The ``"caller"`` check for ``P``
         is also skipped, but it would never match because ``P`` is a
         number.)

  f. Else, Throw a ``TypeError``.
     (Note that this case should not happen, as steps a-e are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. **LOOKUP:**
   If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
   internal property:

  a. (Arguments object exotic behavior.) Let ``map`` be the value of
     the ``[[ParameterMap]]`` internal property of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``:

    1. Return the result of calling the ``[[Get]]`` internal method of
       ``map`` passing ``P`` as the argument.

5. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

6. If ``desc`` is ``undefined``, return ``undefined``.

7. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

8. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``orig`` as the ``this`` value and
     providing no arguments.
     (Note: the difference to a basic ``[[Get]]`` is that the getter ``this``
     binding is the original, uncoerced object.)

9. If ``orig`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

10. Return ``res``.

We can also move step 4 (arguments exotic behavior) to step 2.e.  This has
the problem that step 4 assumes ``P`` has been string coerced already.  So,
a duplicate coercion is needed (like for strings):

1. Let ``orig`` be ``O``.
   (Remember the uncoerced original for a possible getter call.)

2. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part; the throw is
     unconditional.)

  b. If ``O`` is a boolean: set ``O`` to the built-in ``Boolean``
     prototype object (skip creation of temporary)

  c. Else if ``O`` is a number: set ``O`` to the built-in ``Number``
     prototype object (skip creation of temporary)

  d. Else if ``O`` is a string:

    1. If ``P`` is a number, is a whole number, a valid array index, and
       within the string length, return a one-character substring of the
       primitive string value at the specified index.
       (Note: ``ToString(P)`` is skipped, but it would have no side
       effects as ``P`` is a number.  The ``"caller"`` check for ``P``
       is also skipped, but it would never match because ``P`` is a
       number.)

    2. Set ``P`` to ``ToString(P)``.
       (This may have side effects if ``P`` is an object.)

    3. If ``P`` is ``length``, return the length of the primitive string
       value as a number.
       (Note: The ``"caller"`` check for ``P`` is skipped, but would
       never match.)

    4. If ``P`` is a valid array index within the string length, return
       a one-character substring of the primitive string value at the
       specified index.
       (Note: The ``"caller"`` check for ``P`` is skipped, but would
       never match.)

    5. Set ``O`` to the built-in ``String`` prototype object
       (skip creation of temporary)

    6. Goto LOOKUP.  (Avoid double coercion of ``P``.)

  e. Else if ``O`` is an object:

    1. Array fast path: If ``O`` is an object (always true here)
       AND ``P`` is a number and a valid array index (whole number in [0,2**32-2])
       AND ``O`` internal representation has an array part
       AND ``O`` does not have conflicting exotic behaviors (cannot have
       ``String`` or arguments exotic behaviors, may have ``Array``
       behavior), then:

      a. Let ``idx`` be the array index represented by ``P``.

      b. If the array part of ``O`` contains ``idx`` and the key exists,
         read and return that value.
         (Note: ``ToString(P)`` is skipped, but it would have no side
         effects as ``P`` is a number.  The ``"caller"`` check for ``P``
         is also skipped, but it would never match because ``P`` is a
         number.)

    2. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
       internal property:

      a. Set ``P`` to ``ToString(P)``.

      b. (Arguments object exotic behavior.) Let ``map`` be the value of
         the ``[[ParameterMap]]`` internal property of the arguments object.

      c. If the result of calling the ``[[GetOwnProperty]]`` internal method
         of ``map`` passing ``P`` as the argument is not ``undefined``:

        1. Return the result of calling the ``[[Get]]`` internal method of
           ``map`` passing ``P`` as the argument.

      d. Else, goto LOOKUP.  (Avoid double coercion of ``P``.)

  f. Else, Throw a ``TypeError``.
     (Note that this case should not happen, as steps a-e are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. **LOOKUP:**
   Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

5. If ``desc`` is ``undefined``, return ``undefined``.

6. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

7. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``orig`` as the ``this`` value and
     providing no arguments.
     (Note: the difference to a basic ``[[Get]]`` is that the getter ``this``
     binding is the original, uncoerced object.)

8. If ``orig`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

9. Return ``res``.

.. note:: The above is the current "shallow fast path" approach, which has a
          couple of annoying limitations.  For instance, if the array index
          is not used, the key will be coerced to string (regardless of whether
          ancestors have the key or not).  Many improvements are possible;
          these are future work.

Inlining GetProperty
--------------------

Inlining ``[[GetProperty]]`` (but not ``[[GetOwnProperty]]``),
maintaining the original input value in ``O`` instead of ``orig``,
and using ``curr`` instead of ``O`` otherwise, we get:

1. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part; the throw is
     unconditional.)

  b. If ``O`` is a boolean: set ``curr`` to the built-in ``Boolean``
     prototype object (skip creation of temporary)

  c. Else if ``O`` is a number: set ``curr`` to the built-in ``Number``
     prototype object (skip creation of temporary)

  d. Else if ``O`` is a string:

    1. If ``P`` is a number, is a whole number, a valid array index, and
       within the string length, return a one-character substring of the
       primitive string value at the specified index.
       (Note: ``ToString(P)`` is skipped, but it would have no side
       effects as ``P`` is a number.  The ``"caller"`` check for ``P``
       is also skipped, but it would never match because ``P`` is a
       number.)

    2. Set ``P`` to ``ToString(P)``.
       (This may have side effects if ``P`` is an object.)

    3. If ``P`` is ``length``, return the length of the primitive string
       value as a number.
       (Note: The ``"caller"`` check for ``P`` is skipped, but would
       never match.)

    4. If ``P`` is a valid array index within the string length, return
       a one-character substring of the primitive string value at the
       specified index.
       (Note: The ``"caller"`` check for ``P`` is skipped, but would
       never match.)

    5. Set ``curr`` to the built-in ``String`` prototype object
       (skip creation of temporary)

    6. Goto NEXT.  (Avoid double coercion of ``P``.)

  e. Else if ``O`` is an object:

    1. Set ``curr`` to ``O``.

    2. Array fast path: If ``O`` is an object (always true here)
       AND ``P`` is a number and a valid array index (whole number in [0,2**32-2])
       AND ``O`` internal representation has an array part
       AND ``O`` does not have conflicting exotic behaviors (cannot have
       ``String`` or arguments exotic behaviors, may have ``Array``
       behavior), then:

      a. Let ``idx`` be the array index represented by ``P``.

      b. If the array part of ``O`` contains ``idx`` and the key exists,
         read and return that value.
         (Note: ``ToString(P)`` is skipped, but it would have no side
         effects as ``P`` is a number.  The ``"caller"`` check for ``P``
         is also skipped, but it would never match because ``P`` is a
         number.)

    3. If ``O`` is an ``arguments`` object which contains a ``[[ParameterMap]]``
       internal property:

      a. Set ``P`` to ``ToString(P)``.

      b. (Arguments object exotic behavior.) Let ``map`` be the value of
         the ``[[ParameterMap]]`` internal property of the arguments object.

      c. If the result of calling the ``[[GetOwnProperty]]`` internal method
         of ``map`` passing ``P`` as the argument is not ``undefined``:

        1. Return the result of calling the ``[[Get]]`` internal method of
           ``map`` passing ``P`` as the argument.

      d. Else, goto NEXT.  (Avoid double coercion of ``P``.)

  f. Else, Throw a ``TypeError``.
     (Note that this case should not happen, as steps a-e are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. **NEXT:**
   Let ``desc`` be the result of calling the [[GetOwnProperty]] internal
   method of ``curr`` with property name ``P``.

5. If ``desc`` is ``undefined``:

  a. Let ``curr`` be the value of the ``[[Prototype]]`` internal property
     of ``curr``.

  b. If ``curr`` is not ``null``, goto NEXT.

  c. Return ``undefined``.

6. If ``IsDataDescriptor(desc)`` is ``true``:

  a. Let ``res`` be ``desc.[[Value]]``.

7. Otherwise, ``IsAccessorDescriptor(desc)`` must be ``true``:

  a. Let ``getter`` be ``desc.[[Get]]``.

  b. If ``getter`` is ``undefined``, return ``undefined``.

  c. Else let ``res`` be the result of calling the ``[[Call]]`` internal
     method of ``getter`` providing ``O`` as the ``this`` value and
     providing no arguments.
     (Note: the difference to a basic ``[[Get]]`` is that the getter ``this``
     binding is the original, uncoerced object.)

8. If ``O`` is a ``Function`` object or an ``arguments`` object which
   contains a ``[[ParameterMap]]`` internal property:

  a. (Arguments or Function object exotic behavior.)
     If ``P`` is ``"caller"`` and ``res`` is a strict mode ``Function``
     object, throw a ``TypeError`` exception.

9. Return ``res``.

Final version
-------------

(See above.)

PUTPROP: exposed property put algorithm
=======================================

Background
----------

Properties are written in Ecmascript code in many contexts, e.g.::

  foo.bar = "quux";

A property put expression in Ecmascript code involves:

* A property accessor reference (E5 Section 11.2.1)

* A ``PutValue()`` call (E5 Section 8.7.2)

* A ``[[Put]]`` call (or a ``PutValue()`` specific variant)

The property accessor coercions are the same as for ``GetValue``:

* The base reference is checked with ``CheckObjectCoercible()``

* The property name is coerced to a string

The ``PutValue()`` call is simple:

* If the base reference is primitive, it is coerced to an object, and a
  exotic variant of ``[[Put]]`` is used.

* Otherwise, standard ``[[Put]]`` is used.

The variant ``[[Put]]`` for a primitive base value differs from the
standard ``[[Put]]`` as follows:

* If the coerced temporary object has a matching own data property,
  the put is explicitly rejected (steps 3-4 of the variant algorithm),
  regardless of the property attributes (especially, writability).
  Compare this to the standard ``[[Put]]`` behavior in E5 Section
  8.12.5, steps 2-3 which simply attempts to update the data property,
  provided that the property is writable.

* If the property is found (either in the temporary object or its
  ancestors) and is a setter, the setter call ``this`` binding is
  the primitive value, not the coerced value.  (An own accessor
  property should never be found in practice, as the only possible
  coerced object types as ``Boolean``, ``Number``, and ``String``.)

Like ``GetValue()``, we could skip creation of the coerced object, but
don't take advantage of this now.

Note: if the base reference is a primitive value, the coerced object is
temporary and never exposed to user code.  Some implementations (like V8)
omit a property write entirely if the base value is primitive.  This can
be observed by lack of side effects, e.g. no setter call occurs when it
should::

  // add test getter
  Object.defineProperty(String.prototype, 'test', { 
    get: function() { print(typeof this); },
    set: function(x) { print(typeof this); },
  });

  "foo".test = "bar";    // prints 'string'

V8 will print nothing, while Rhino and Smjs print 'object' (which is also
not correct).

First draft
-----------

The relevant part begins after that in steps 5-8, which first perform
some coercions and then create a property accessor.  The accessor is
then acted upon by ``PutValue()``, and ultimately ``[[Put]]`` or its
variant.

Combining all of these, we get the first draft (for base value ``O``
and property name value ``P``):

1. Let ``orig`` be ``O``.
   (Remember the uncoerced original for a possible setter call.)

2. Call ``CheckObjectCoercible`` with ``O`` as argument.  In practice: if
   ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
   (Note: this is unconditional.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. If ``O`` is not an object, let ``coerced`` be ``true``, else let
   ``coerced`` be ``false``.

5. Let ``O`` be ``ToObject(O)``.
   (This is side effect free.)

6. Let ``curr`` be ``O``.

7. **NEXT:**
   Let ``desc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``curr`` with property name ``P``.

8. If ``desc`` is ``undefined``:

  a. Let ``curr`` be the value of the ``[[Prototype]]`` internal property
     of ``curr``.

  b. If ``curr`` is not ``null``, goto NEXT.

  c. If ``coerced`` is ``true``, Reject.

  d. If ``O.[[Extensible]]`` is ``false``, Reject.

  e. Let ``newDesc`` be a property descriptor with values:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  f. Call ``O.[[DefineOwnProperty]](P, newDesc, Throw)``.

  g. Return.

9. If ``IsDataDescriptor(desc)``:

  a. If ``coerced`` is ``true``, Reject.

  b. If ``curr`` != ``O`` (property is an inherited data property):
     (Note: assumes there are no prototype loops.)

    1. If ``O.[[Extensible]`` is ``false``, Reject.

    2. If ``desc.[[Writable]]`` is ``false``, Reject.

    3. Let ``newDesc`` be a property descriptor with values:

      * ``[[Value]]: V``

      * ``[[Writable]]: true``

      * ``[[Enumerable]]: true``

      * ``[[Configurable]]: true}``

    4. Call ``O.[[DefineOwnProperty]](P, newDesc, Throw)``.

  c. Else (property is an own data property):

    1. If ``desc.[[Writable]]`` is ``false``, Reject.

    2. Let ``valueDesc`` be ``{ [[Value]]: V }``.

    3. Call ``O.[[DefineOwnProperty]](P, valueDesc, Throw)``.

10. Else (property is an accessor):

  a. If ``desc.[[Set]]`` is ``undefined``, Reject.

  b. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``orig`` as the ``this`` value and providing ``V`` as the sole argument.
     (Note: the difference to a basic ``[[Put]]`` is that the setter ``this``
     binding is the original, uncoerced object.)

11. Return.

Notes:

* Steps 2-3 come from the property accessor evaluation rules in E5 Section
  11.2.1.  In particular, ``CheckObjectCoercible()`` is called before the
  key is coerced to a string.  Since the key string coercion may have side
  effects, the order of evaluation matters.

  Note that ``ToObject()`` has no side effects (this can be seen from a
  case by case inspection), so steps 3 and 4-5 can be reversed.

* Step 10.b uses the original object (not the coerced object) as the setter
  ``this`` binding (E5 Section 8.7.2, step 6 of the variant ``[[Put]]``
  algorithm).

* Steps 8.c and 9.a reject attempt to update or create a data property on
  a temporary object (E5 Section 8.7.2, steps 4 and 7 of the variant
  ``[[Put]]`` algorithm).  Note that the "coerced" check is not actually
  needed to guard step 9.c (step 4 of the variant ``[[Put]]``) because the
  only coerced object with own properties is the ``String`` object, and all
  its own properties are non-writable and thus caught by step 9.c.1 anyway.
  This might of course change in a future version, or be untrue for some
  out-of-spec coercion behavior for custom types.  The pre-check *is*
  needed to avoid creating a new property on the temporary object, though.

* An explicit ``coerced`` flag is not needed: we can simply check whether
  or not ``orig`` is an object.

* Since ``curr`` is used for prototype chain walking, we don't need to
  store ``orig`` (``O`` can be used for that instead).

Cleaning up
-----------

1. Call ``CheckObjectCoercible`` with ``O`` as argument.  In practice: if
   ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
   (Note: this is unconditional.)

2. Let ``curr`` be ``ToObject(O)``.
   (This is side effect free.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. **NEXT:**
   Let ``desc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``curr`` with property name ``P``.

5. If ``desc`` is ``undefined``:

  a. Let ``curr`` be the value of the ``[[Prototype]]`` internal property
     of ``curr``.

  b. If ``curr`` is not ``null``, goto NEXT.

  c. If ``O`` is not an object (was coerced), Reject.

  d. If ``O.[[Extensible]]`` is ``false``, Reject.

  e. Let ``newDesc`` be a property descriptor with values:

    * ``[[Value]]: V``

    * ``[[Writable]]: true``

    * ``[[Enumerable]]: true``

    * ``[[Configurable]]: true}``

  f. Call ``O.[[DefineOwnProperty]](P, newDesc, Throw)``.

  g. Return.

6. If ``IsDataDescriptor(desc)``:

  a. If ``O`` is not an object (was coerced), Reject.

  b. If ``curr`` != ``O`` (property is an inherited data property):
     (Note: assumes there are no prototype loops.)

    1. If ``O.[[Extensible]`` is ``false``, Reject.

    2. If ``desc.[[Writable]]`` is ``false``, Reject.

    3. Let ``newDesc`` be a property descriptor with values:

      * ``[[Value]]: V``

      * ``[[Writable]]: true``

      * ``[[Enumerable]]: true``

      * ``[[Configurable]]: true}``

    4. Call ``O.[[DefineOwnProperty]](P, newDesc, Throw)``.

  c. Else (property is an own data property):

    1. If ``desc.[[Writable]]`` is ``false``, Reject.

    2. Let ``valueDesc`` be ``{ [[Value]]: V }``.

    3. Call ``O.[[DefineOwnProperty]](P, valueDesc, Throw)``.

7. Else (property is an accessor):

  a. If ``desc.[[Set]]`` is ``undefined``, Reject.

  b. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``O`` as the ``this`` value and providing ``V`` as the sole argument.
     (Note: the difference to a basic ``[[Put]]`` is that the setter ``this``
     binding is the original, uncoerced object.)

8. Return.

Inlining DefineOwnProperty calls
--------------------------------

The ``[[Put]]`` uses two different calls to ``[[DefineOwnProperty]]``: one to
update an existing property ``[[Value]]`` and another to create a brand new
data property.  These can be inlined into the algorithm as follows (see
the section on preliminary algorithm work).

Before inlining, the cases for "update old property" and "create new property"
are isolated into goto labels (as there are two places where a new property
is created).  The ``[[DefineOwnProperty]]`` calls with exotic behaviors
inlined are then substituted.  "Reject" is also made an explicit label.

The resulting algorithm is:

1. Call ``CheckObjectCoercible`` with ``O`` as argument.  In practice: if
   ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
   (Note: this is unconditional.)

2. Let ``curr`` be ``ToObject(O)``.
   (This is side effect free.)

3. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

4. **NEXT:**
   Let ``desc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``curr`` with property name ``P``.

5. If ``desc`` is ``undefined``:

  a. Let ``curr`` be the value of the ``[[Prototype]]`` internal property
     of ``curr``.

  b. If ``curr`` is not ``null``, goto NEXT.

  c. If ``O`` is not an object (was coerced), goto REJECT.

  d. If ``O.[[Extensible]]`` is ``false``, goto REJECT.

  e. Goto NEWPROP.

6. If ``IsDataDescriptor(desc)``:

  a. If ``O`` is not an object (was coerced), goto REJECT.

  b. If ``curr`` != ``O`` (property is an inherited data property):
     (Note: assumes there are no prototype loops.)

    1. If ``O.[[Extensible]`` is ``false``, goto REJECT.

    2. If ``desc.[[Writable]]`` is ``false``, goto REJECT.

    3. Goto NEWPROP.

  c. Else (property is an own data property):

    1. If ``desc.[[Writable]]`` is ``false``, goto REJECT.

    2. Goto UPDATEPROP.

7. Else (property is an accessor):

  a. If ``desc.[[Set]]`` is ``undefined``, goto REJECT.

  b. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``O`` as the ``this`` value and providing ``V`` as the sole argument.
     (Note: the difference to a basic ``[[Put]]`` is that the setter ``this``
     binding is the original, uncoerced object.)

  c. Return.

8. **UPDATEPROP:**
   (Inlined ``[[DefineOwnProperty]]`` call for existing property.)
   If ``O`` is an ``Array`` object, and ``P`` is ``"length"``, then:

  a. Let ``newLen`` be ``ToUint32(V)``.

  b. If ``newLen`` is not equal to ``ToNumber(V)``, goto REJECTRANGE.

  c. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a ``length`` data property that
     cannot be deleted or reconfigured.

  d. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. (Note that ``oldLen``
     is guaranteed to be a unsigned 32-bit integer.)

  e. If ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. Goto REJECT, if ``shortenSucceeded`` is ``false``.

    4. Return.

  f. Update the property (``"length"``) value to ``newLen``.

  g. Return.

9. Set the ``[[Value]]`` attribute of the property named ``P`` of object
   ``O`` to ``V``.  (Since it is side effect free to update the value
   with the same value, no check for that case is needed.)

10. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
    internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
       ``V``, and ``Throw`` as the arguments.  (This updates the bound
       variable value.)

11. Return.

12. **NEWPROP:**
    (Inlined ``[[DefineOwnProperty]]`` call for new property.)
    If ``O`` is an ``Array`` object and ``P`` is an array index (E5 Section
    15.4), then:

  a. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a length data property that cannot
     be deleted or reconfigured.

  b. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
     (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

  c. Let ``index`` be ``ToUint32(P)``.

  d. If ``index`` >= ``oldLen``:

    1. Goto REJECT ``oldLenDesc.[[Writable]]`` is ``false``.

    2. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
       This always succeeds.

13. Create an own data property named ``P`` of object ``O`` whose attributes
    are:

  * ``[[Value]]: V``
  * ``[[Writable]]: true``
  * ``[[Enumerable]]: true``
  * ``[[Configurable]]: true``

14. Return.

15. **REJECT**:
    If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
    otherwise return.

16. **REJECTRANGE**:
    Throw a ``RangeError`` exception.  (This is unconditional.)

Notes:

* In step 8, we don't need to check for array index updates: the property
  already exists, so array ``length`` will not need an update.

* In step 8, the original ``[[DefineOwnProperty]]`` exotic behavior is
  split into a pre-step and a post-step because the ``"length"`` write
  may fail.  However, because we've inlined ``[[CanPut]]``, we know that
  the write will succeed, so both the pre- and post-behaviors can be
  handled in step 8 internally.

* In step 8, we don't need to check for arguments exotic behavior, as
  only number-like indices have magic bindings (not ``"length"``).

* In steps 12-14, we don't need to check for arguments exotic behavior: any
  "magically bound" property must always be present in the arguments
  object.  If a bound property is deleted, the binding is also deleted
  from the argument parameter map.

* In step 12, we don't need to check for ``length`` exotic behavior: the
  ``length`` property always exists for arrays so we cannot get here with
  arrays.

Avoiding temporary objects
--------------------------

As for ``GetValue()`` the only cases where temporary objects are created are
for ``Boolean``, ``Number``, and ``String``.  The ``PutValue()`` algorithm
rejects a property write on a temporary object if a new data property were to
be created or an existing one updated.

For the possible coerced values, the own properties are:

* ``Boolean``: none
* ``Number``: none
* ``String``: ``"length"`` and index properties for string characters

These can be checked explicitly when coercing (and reject the attempt before
going forwards).  However, ``PutValue()`` *does* allow a property write if an
ancestor contains a setter which "captures" the write so that the temporary
object would not be written to.  Although the built-in prototype chains do not
contain such setters, they can be added by user code at run time, so they do
need to be checked for.

Avoiding temporaries altogether:

1. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part; the throw is
     unconditional.)

  b. If ``O`` is a boolean: set ``curr`` to the built-in ``Boolean``
     prototype object (skip creation of temporary)

  c. Else if ``O`` is a number: set ``curr`` to the built-in ``Number``
     prototype object (skip creation of temporary)

  d. Else if ``O`` is a string:

    1. Set ``P`` to ``ToString(P)``.
       (This may have side effects if ``P`` is an object.)

    2. If ``P`` is ``length``, goto REJECT.

    3. If ``P`` is a valid array index within the string length,
       goto REJECT.

    4. Set ``curr`` to the built-in ``String`` prototype object
       (skip creation of temporary)

    5. Goto NEXT.  (Avoid double coercion of ``P``.)

  e. Else if ``O`` is an object: set ``curr`` to ``O``.

  f. Else, Throw a ``TypeError``.
     (Note that this case should not happen, as steps a-e are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

2. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

3. **NEXT:**
   Let ``desc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``curr`` with property name ``P``.

4. If ``desc`` is ``undefined``:

  a. Let ``curr`` be the value of the ``[[Prototype]]`` internal property
     of ``curr``.

  b. If ``curr`` is not ``null``, goto NEXT.

  c. If ``O`` is not an object (was coerced), goto REJECT.

  d. If ``O.[[Extensible]]`` is ``false``, goto REJECT.

  e. Goto NEWPROP.

5. If ``IsDataDescriptor(desc)``:

  a. If ``O`` is not an object (was coerced), goto REJECT.

  b. If ``curr`` != ``O`` (property is an inherited data property):
     (Note: assumes there are no prototype loops.)

    1. If ``O.[[Extensible]`` is ``false``, goto REJECT.

    2. If ``desc.[[Writable]]`` is ``false``, goto REJECT.

    3. Goto NEWPROP.

  c. Else (property is an own data property):

    1. If ``desc.[[Writable]]`` is ``false``, goto REJECT.

    2. Goto UPDATEPROP.

6. Else (property is an accessor):

  a. If ``desc.[[Set]]`` is ``undefined``, goto REJECT.

  b. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``O`` as the ``this`` value and providing ``V`` as the sole argument.
     (Note: the difference to a basic ``[[Put]]`` is that the setter ``this``
     binding is the original, uncoerced object.)

  c. Return.

7. **UPDATEPROP:**
   (Inlined ``[[DefineOwnProperty]]`` call for existing property.)
   If ``O`` is an ``Array`` object, and ``P`` is ``"length"``, then:

  a. Let ``newLen`` be ``ToUint32(V)``.

  b. If ``newLen`` is not equal to ``ToNumber(V)``, goto REJECTRANGE.

  c. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a ``length`` data property that
     cannot be deleted or reconfigured.

  d. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. (Note that ``oldLen``
     is guaranteed to be a unsigned 32-bit integer.)

  e. If ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. Goto REJECT, if ``shortenSucceeded`` is ``false``.

    4. Return.

  f. Update the property (``"length"``) value to ``newLen``.

  g. Return.

8. Set the ``[[Value]]`` attribute of the property named ``P`` of object
   ``O`` to ``V``.  (Since it is side effect free to update the value
   with the same value, no check for that case is needed.)

9. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
   internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
       ``V``, and ``Throw`` as the arguments.  (This updates the bound
       variable value.)

10. Return.

11. **NEWPROP:**
    (Inlined ``[[DefineOwnProperty]]`` call for new property.)
    If ``O`` is an ``Array`` object and ``P`` is an array index (E5 Section
    15.4), then:

  a. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a length data property that cannot
     be deleted or reconfigured.

  b. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
     (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

  c. Let ``index`` be ``ToUint32(P)``.

  d. If ``index`` >= ``oldLen``:

    1. Goto REJECT ``oldLenDesc.[[Writable]]`` is ``false``.

    2. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
       This always succeeds.

12. Create an own data property named ``P`` of object ``O`` whose attributes
    are:

  * ``[[Value]]: V``
  * ``[[Writable]]: true``
  * ``[[Enumerable]]: true``
  * ``[[Configurable]]: true``

13. Return.

14. **REJECT**:
    If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
    otherwise return.

Notes:

* Step 7: if array exotic behavior exists, we can return right after
  processing the ``length`` update; in particular, step 9 is not
  necessary as an object cannot be simultaneously an array and an
  arguments object.

* Step 11.d.2 (updating ``length``) is a bit dangerous because it happens
  before step 12.  Step 12 may fail due to an out-of-memory or other
  internal condition, which leaves the ``length`` updated but the element
  missing.

Minor improvements
------------------

Addressing the array ``length`` issue:

1. Check and/or coerce ``O`` as follows:

  a. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``.
     (This is the ``CheckObjectCoercible`` part; the throw is
     unconditional.)

  b. If ``O`` is a boolean: set ``curr`` to the built-in ``Boolean``
     prototype object (skip creation of temporary)

  c. Else if ``O`` is a number: set ``curr`` to the built-in ``Number``
     prototype object (skip creation of temporary)

  d. Else if ``O`` is a string:

    1. Set ``P`` to ``ToString(P)``.
       (This may have side effects if ``P`` is an object.)

    2. If ``P`` is ``length``, goto REJECT.

    3. If ``P`` is a valid array index within the string length,
       goto REJECT.

    4. Set ``curr`` to the built-in ``String`` prototype object
       (skip creation of temporary)

    5. Goto NEXT.  (Avoid double coercion of ``P``.)

  e. Else if ``O`` is an object: set ``curr`` to ``O``.

  f. Else, Throw a ``TypeError``.
     (Note that this case should not happen, as steps a-e are exhaustive.
     However, this step is useful as a fallback, and for handling any
     internal types.)

2. Let ``P`` be ``ToString(P)``.
   (This may have side effects if ``P`` is an object.)

3. **NEXT:**
   Let ``desc`` be the result of calling the ``[[GetOwnProperty]]``
   internal method of ``curr`` with property name ``P``.

4. If ``desc`` is ``undefined``:

  a. Let ``curr`` be the value of the ``[[Prototype]]`` internal property
     of ``curr``.

  b. If ``curr`` is not ``null``, goto NEXT.

  c. If ``O`` is not an object (was coerced), goto REJECT.

  d. If ``O.[[Extensible]]`` is ``false``, goto REJECT.

  e. Goto NEWPROP.

5. If ``IsDataDescriptor(desc)``:

  a. If ``O`` is not an object (was coerced), goto REJECT.

  b. If ``curr`` != ``O`` (property is an inherited data property):
     (Note: assumes there are no prototype loops.)

    1. If ``O.[[Extensible]`` is ``false``, goto REJECT.

    2. If ``desc.[[Writable]]`` is ``false``, goto REJECT.

    3. Goto NEWPROP.

  c. Else (property is an own data property):

    1. If ``desc.[[Writable]]`` is ``false``, goto REJECT.

    2. Goto UPDATEPROP.

6. Else (property is an accessor):

  a. If ``desc.[[Set]]`` is ``undefined``, goto REJECT.

  b. Call the ``[[Call]]`` internal method of ``desc.[[Set]]`` providing
     ``O`` as the ``this`` value and providing ``V`` as the sole argument.
     (Note: the difference to a basic ``[[Put]]`` is that the setter ``this``
     binding is the original, uncoerced object.)

  c. Return.

7. **UPDATEPROP:**
   (Inlined ``[[DefineOwnProperty]]`` call for existing property.)
   If ``O`` is an ``Array`` object, and ``P`` is ``"length"``, then:

  a. Let ``newLen`` be ``ToUint32(V)``.

  b. If ``newLen`` is not equal to ``ToNumber(V)``, goto REJECTRANGE.

  c. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a ``length`` data property that
     cannot be deleted or reconfigured.

  d. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. (Note that ``oldLen``
     is guaranteed to be a unsigned 32-bit integer.)

  e. If ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. Goto REJECT, if ``shortenSucceeded`` is ``false``.

    4. Return.

  f. Update the property (``"length"``) value to ``newLen``.

  g. Return.

8. Set the ``[[Value]]`` attribute of the property named ``P`` of object
   ``O`` to ``V``.  (Since it is side effect free to update the value
   with the same value, no check for that case is needed.)

9. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
   internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
       ``V``, and ``Throw`` as the arguments.  (This updates the bound
       variable value.)

10. Return.

11. **NEWPROP:**
    (Inlined ``[[DefineOwnProperty]]`` call for new property.)
    Let ``pendingLength`` be 0 (zero).

12. If ``O`` is an ``Array`` object and ``P`` is an array index (E5 Section
    15.4), then:

  a. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
     internal method of ``O`` passing ``"length"`` as the argument.  The
     result will never be ``undefined`` or an accessor descriptor because
     ``Array`` objects are created with a length data property that cannot
     be deleted or reconfigured.

  b. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
     (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

  c. Let ``index`` be ``ToUint32(P)``.

  d. If ``index`` >= ``oldLen``:

    1. Goto REJECT ``oldLenDesc.[[Writable]]`` is ``false``.

    2. Let ``pendingLength`` be ``index + 1`` (always non-zero).

13. Create an own data property named ``P`` of object ``O`` whose attributes
    are:

  * ``[[Value]]: V``
  * ``[[Writable]]: true``
  * ``[[Enumerable]]: true``
  * ``[[Configurable]]: true``

14. If ``pendingLength`` > ``0``:

  a. Update the ``"length"`` property of ``O`` to the value ``pendingLength``.
     This always succeeds.
     (Note: this can only happen for an ``Array`` object, and the ``length``
     property must exist and has already been checked to be writable.)

15. Return.

16. **REJECT**:
    If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
    otherwise return.

Fast path for array indices
---------------------------

There is currently no fast path for array indices in the implementation.

This is primarily because to implement ``[[Put]`` properly, the prototype
chain needs to be walked when creating new properties, as an ancestor
property may prevent or capture the write.  The current implementation cannot
walk the prototype chain without coercing the key to a string first.
A fast path could be easily added for writing to existing array entries,
though, but it's probably better to solve the problem a bit more comprehensively.

Implementation notes
--------------------

* Property writes may fail for out of memory or other internal reasons.
  In such cases the algorithm should just throw an error and avoid making
  any updates to the object state.  This is easy for normal properties,
  but there are some subtle issues when dealing with exotic behaviors
  which link multiple properties together and should be updated either
  atomically or in some consistent manner.  In particular:

  + For NEWPROP, if the property written is an array index which updates
    array ``length``, the property write should be performed first.  If
    the property write succeeds ``length`` should be updated (and should
    never fail):

Final version
-------------

(See above.)

DELPROP: exposed property deletion algorithm ("delete" operator)
================================================================

Background
----------

Properties are deleted in Ecmascript code with the ``delete`` operator, e.g.::

  delete foo.bar;

This involves:

* A property accessor reference (E5 Section 11.2.1)

* ``delete`` semantics (E5 Section 11.4.1)

* A call to ``[[Delete]]``

The property accessor coercions are the same as for ``GetValue``:

* The base reference is checked with ``CheckObjectCoercible()``

* The property name is coerced to a string

The ``delete`` expression will then:

* Coerce the base value to an object

* Call the ``[[Delete]]`` algorithm

Note that if the base value is not an object, a temporary object will be
created by coercion.  Since a deletion always operates on the "own
properties" of an object, the deletion can only have side effects (error
throwing) side effects.  Any other effects will be lost with the temporary
object.  This is discussed in more detail below, for the deletion algorithm.

Notes:

* ``[[Delete]]`` only checks for the property ``P`` in the original object
  ``O``, and does not follow the prototype chain

* In particular, an inherited property ``P`` which would prevent a ``[[Put]]``
  does not affect the outcome of ``[[Delete]]``

First draft
-----------

Starting from the property accessor, then applying ``delete`` (and skipping any
unused steps):

1. Call ``CheckObjectCoercible`` for the base value.  In practice, throw a
   ``TypeError`` if the base value is ``null`` or ``undefined``.

2. Coerce property name to string using ``ToString()``.

3. Coerce base value to object using ``ToObject()`` and call ``[[Delete]]``
   with the coerced object, the coerced key, and a "Throw" flag set if
   the property reference is contained in strict mode code.

More formally, suppose ``O`` is the base value, ``P`` is the property name
value, and ``currStrict`` is ``true`` if the property deletion expression
occurred in strict code:

1. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``

2. ``P`` = ``ToString(P)``

3. ``O`` = ``ToObject(O)``

4. Call ``O.[[Delete]](P, currStrict)``, and return its result

Avoiding object coercion
------------------------

We want to avoid the object coercion; let's first make it more explicit:

1. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``

2. ``P`` = ``ToString(P)``

3. If ``O`` is an object, call ``[[Delete]](O, P, currStrict)``, and
   return its result

4. Else ``O`` is primitive:

  a. ``O`` = ``ToObject(O)`` (create temporary object)

  b. Call ``O.[[Delete]](P, currStrict)``, and return its result

Avoiding temporary objects
--------------------------

Note that a ``[[Delete]]`` only operates on the "own properties" of the
target object.  When the base value is not an object, the deletion operates
only on the temporary object.  Since the temporary object is immediately
discarded, there are only two possible user visible effects:

* The return value of ``[[Delete]]``, which is:

  + ``true``, if the property does not exist
  + ``true``, if the property exists and could be deleted
  + ``false``, if the property exists, cannot be deleted, and
    ``Throw`` is ``false`` (if ``Throw`` is ``true``, an error is
    thrown instead)
 
* Errors thrown by ``[[Delete]]``, which happens if:

  + The (own) property exists, the property is non-configurable, and the
    Throw flag is set, i.e. we're evaluating ``delete`` in strict code

The coerced temporary object can be:

* a ``Boolean`` instance: no own properties
* a ``Number`` instance: no own properties
* a ``String`` instance: has ``length`` and array indices (inside string
  length) as own properties, all non-configurable

Given these, the algorithm can be changed to avoid creation of temporaries
entirely:

1. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``

2. ``P`` = ``ToString(P)``

3. If ``O`` is an object, call ``[[Delete]](O, P, currStrict)`` and
   return its result

4. Else ``O`` is primitive:

  a. If ``O`` is a boolean, return ``true``

  b. If ``O`` is a number, return ``true``

  c. If ``O`` is a string:

    1. If ``P`` is length or an array index inside the ``O`` string length:

      a. If ``currStrict`` is ``true``, throw a ``TypeError``
      b. Else, return ``false``

    2. Else, return ``true``

  d. Return ``true``
     (This step should never be reached, as the checks above are
     comprehensive.)

Step 4 can be simplified a bit:

1. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``

2. ``P`` = ``ToString(P)``

3. If ``O`` is an object, call ``[[Delete]](O, P, currStrict)`` and
   return its result

4. If ``O`` is a string:

  a. If ``P`` is length or an array index inside the ``O`` string length:

    1. If ``currStrict`` is ``true``, throw a ``TypeError``
    2. Else, return ``false``

5. Return ``true``

Fast path for array indices
---------------------------

It would be straightforward to add a fast path for array indices, but there
is no fast path in the current implementation for array index deletion.
The index is always string coerced and interned.

HASPROP: Exposed property existence check ("in" operator)
=========================================================

Background
----------

Property existence check is done using the ``in`` operator in Ecmascript
code, e.g.::

  print('foo' in bar);  // check whether foo.bar exists

This involves:

* An expression for the left-hand-side

* An expression for the right-hand-side

* ``in`` semantics (E5 Section 11.8.7)

* A call to ``[[HasProperty]]``

First draft
-----------

Starting from the property accessor, then applying ``in`` (and skipping any
unused steps):

1. Call ``CheckObjectCoercible`` for the base value.  In practice, throw a
   ``TypeError`` if the base value is ``null`` or ``undefined``.

2. If the base value is not an object, throw a ``TypeError``.

3. Coerce property name to string using ``ToString()``.

4. Call ``[[HasProperty]]`` with the base object and the coerced property
   name.

Note that the error throwing is unconditional and happens for non-strict
code too::

  // throws TypeError
  "foo" in "bar";

More formally, suppose ``O`` is the base value, ``P`` is the property name
value:

1. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``

2. If ``O`` is not an object, throw a ``TypeError``

3. ``P`` = ``ToString(P)``

4. Call ``O.[[HasProperty]](P)``, and return its result

The step 1 is unnecessary (step 2 suffices):

1. If ``O`` is not an object, throw a ``TypeError``

2. ``P`` = ``ToString(P)``

3. Call ``O.[[HasProperty]](P)``, and return its result

Inlining HasProperty
--------------------

Inlining ``[[HasProperty]]`` from E5 Section 8.12.6:

1. If ``O`` is not an object, throw a ``TypeError``

2. ``P`` = ``ToString(P)``

3. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

4. If ``desc`` is ``undefined``, then return ``false``.

5. Else return ``true``.

INSTOF: exposed object class membership check ("instanceof" operator)
=====================================================================

Background
----------

Object class membership check is done using the ``instanceof`` operator
in Ecmascript code, e.g.::

  print(x instanceof Array);

The language semantics of "class membership" are not as clear cut in
Ecmascript as in some other languages.  But essentially, the ``instanceof``
expression above checks whether ``Array.prototype`` occurs in the internal
prototype chain of ``x``).

This involves:

* An expression for the left-hand-side

* An expression for the right-hand-side

* ``instanceof`` semantics (E5 Section 11.8.6)

* A call to ``[[HasInstance]]``

First draft
-----------

The ``instanceof`` operator is the only "caller" for ``[[HasInstance]]`` and
has the following steps (for evaluating RelationalExpression **instanceof**
ShiftExpression):

1. Let ``lref`` be the result of evaluating RelationalExpression.

2. Let ``lval`` be ``GetValue(lref)``.

3. Let ``rref`` be the result of evaluating ShiftExpression.

4. Let ``rval`` be ``GetValue(rref)``.

5. If ``Type(rval)`` is not ``Object``, throw a ``TypeError`` exception.

6. If ``rval`` does not have a ``[[HasInstance]]`` internal method, throw a
   ``TypeError`` exception.

7. Return the result of calling the ``[[HasInstance]]`` internal method of
   ``rval`` with argument ``lval``.

For implementing ``instanceof``, steps 1-4 can be assumed to be handled by
the compiler and map to a certain bytecode sequence.  Steps 5-7 are the
relevant part.

The following algorithm integrates steps 5-7 from above with the combined
``[[HasInstance]]`` algorithm (``lval`` is renamed to ``V`` and ``rval``
to ``F``):

1. If ``Type(F)`` is not ``Object``, throw a ``TypeError`` exception.

2. If ``F`` does not have a ``[[HasInstance]]`` internal method, throw a
   ``TypeError`` exception.

3. While ``F`` is a bound function:

  a. Set ``F`` to the value of ``F``\ 's ``[[TargetFunction]]`` internal
     property.

  b. If ``F`` has no ``[[HasInstance]]`` internal method, throw a
     ``TypeError`` exception.
     (Note: ``F`` can be another bound function, so we loop until we find
     the non-bound actual function.)

4. If ``V`` is not an object, return ``false``.

5. Let ``O`` be the result of calling the ``[[Get]]`` internal method of
   ``F`` with property name ``"prototype"``.
   (Note: this is the external prototype, not the internal one.)

6. If ``Type(O)`` is not ``Object``, throw a ``TypeError`` exception.

7. Repeat

  a. Let ``V`` be the value of the ``[[Prototype]]`` internal property of
     ``V``.

  b. If ``V`` is ``null``, return ``false``.

  c. If ``O`` and ``V`` refer to the same object, return ``true``.

Notes:

* The initial ``rval`` may be something other than a callable function,
  so it needs an explicit check, whereas the ``[[TargetFunction]]``
  internal property can only exist with a valid callable object value
  (E5 Section 15.3.4.5, step 2 checks for this).

* Step 3.b seems to be unnecessary: ``Function.prototype.bind()`` will
  not create a bound function whose target function is not callable, so
  they should always have a ``[[HasInstance]]`` internal method.  If this
  is just to add some internal robustness, why not also check that the
  target function is an object?

* In step 7 we assume that the internal prototype is always an object or
  ``null``.  If the internal implementation does not constrain this fully,
  it makes sense to check this explicitly.  The current implementation uses
  an ``duk_hobject`` pointer for the internal prototype, so the prototype is
  effectively constrained to be either object or ``null``.

* The loop in step 7 assumes that there are no prototype loops.  An explicit
  sanity check should be inserted.

Cleanup
-------

Steps 1-3 can be combined to a simpler loop with a bit more paranoid checks:

1. Repeat

  a. If ``Type(F)`` is not ``Object``, throw a ``TypeError`` exception.

  b. If ``F`` does not have a ``[[HasInstance]]`` internal method, throw a
     ``TypeError`` exception.

  c. If ``F`` is a normal (non-bound) function, break repeat loop.

  d. If ``F`` is *not* a bound function, throw a ``TypeError`` exception.
     (Note: this should never happen, but is nice to check.)

  e. Set ``F`` to the value of ``F``\ 's ``[[TargetFunction]]`` internal
     property and repeat from a).
     (Note: ``F`` may be another bound function when exiting this step,
     so we must repeat until the final, non-bound function is found.)

2. If ``V`` is not an object, return ``false``.

3. Let ``O`` be the result of calling the ``[[Get]]`` internal method of
   ``F`` with property name ``"prototype"``.
   (Note: this is the external prototype, not the internal one.)

4. If ``Type(O)`` is not ``Object``, throw a ``TypeError`` exception.

5. Repeat

  a. Let ``V`` be the value of the ``[[Prototype]]`` internal property of
     ``V``.

  b. If ``V`` is ``null``, return ``false``.

  c. If ``O`` and ``V`` refer to the same object, return ``true``.

Exposed Object.getOwnPropertyDescriptor()
=========================================

Original algorithm
------------------

The algorithm is specified in E5 Section 15.2.3.3:

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``name`` be ``ToString(P)``.
   (Note: this may have a side effect.)

3. Let ``desc`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with argument ``name``.

4. Return the result of calling ``FromPropertyDescriptor(desc)``
   (E5 Section 8.10.4).

FromPropertyDescriptor
----------------------

The ``FromPropertyDescriptor()`` algorithm in E5 Section 8.10.4 is
as follows:

1. If ``Desc`` is ``undefined``, then return ``undefined``.

2. Let ``obj`` be the result of creating a new object as if by the expression
   ``new Object()`` where ``Object`` is the standard built-in constructor with
   that name.

3. If ``IsDataDescriptor(Desc)`` is ``true``, then

  a. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
     arguments ``"value"``, Property Descriptor {[[Value]]: Desc.[[Value]],
     [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true}, and
     ``false``.

  b. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
     arguments ``"writable"``, Property Descriptor {[[Value]]:
     Desc.[[Writable]], [[Writable]]: true, [[Enumerable]]: true,
     [[Configurable]]: true}, and ``false``.

4. Else, ``IsAccessorDescriptor(Desc)`` must be ``true``, so

  a. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
     arguments ``"get"``, Property Descriptor {[[Value]]: Desc.[[Get]],
     [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true}, and
     ``false``.

  b. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
     arguments ``"set"``, Property Descriptor {[[Value]]: Desc.[[Set]],
     [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true}, and
     ``false``.

5. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
   arguments ``"enumerable"``, Property Descriptor {[[Value]]:
   Desc.[[Enumerable]], [[Writable]]: true, [[Enumerable]]: true,
   [[Configurable]]: true}, and ``false``.

6. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
   arguments ``"configurable"``, Property Descriptor {[[Value]]:
   Desc.[[Configurable]], [[Writable]]: true, [[Enumerable]]: true,
   [[Configurable]]: true}, and ``false``.

7. Return ``obj``.

Notes:

* Since all the ``[[DefineOwnProperty]]`` calls create new property values,
  and the property attributes match the defaults for ``[[Put]]``, we can
  simply use ``[[Put]]`` instead.  The ``Throw`` flag does not matter as
  the ``[[Put]]`` operations cannot fail (except for some internal reason,
  which is thrown unconditionally without regard for ``Throw`` anyway).

* The order of settings properties to ``obj`` matters since it will affect
  the enumeration order of ``obj``.

Changing ``[[DefineOwnProperty]]`` to ``[[Put]]`` and renaming ``Desc``
to ``desc`` (for compatibility with ``Object.getOwnPropertyDescriptor()``
algorithm):

1. If ``desc`` is ``undefined``, then return ``undefined``.

2. Let ``obj`` be the result of creating a new object as if by the expression
   ``new Object()`` where ``Object`` is the standard built-in constructor with
   that name.

3. If ``IsDataDescriptor(desc)`` is ``true``, then

  a. Call ``obj.[[Put]]`` with arguments
     ``"value"``, ``desc.[[Value]]``, and ``false``.

  b. Call ``obj.[[Put]]`` with arguments
     ``"writable"``, ``desc.[[Writable]]``, and ``false``.

4. Else, ``IsAccessorDescriptor(Desc)`` must be ``true``, so

  a. Call ``obj.[[Put]]`` with arguments
     ``"get"``, ``desc.[[Get]]``, and ``false``.
     (Note: ``desc.[[Get]]`` may be ``undefined``.)

  b. Call ``obj.[[Put]]`` with arguments
     ``"set"``, ``desc.[[Set]]``, and ``false``.
     (Note: ``desc.[[Set]]`` may be ``undefined``.)

5. Call ``obj.[[Put]]`` with arguments
   ``"enumerable"``, ``desc.[[Enumerable]]``, and ``false``.

6. Call ``obj.[[Put]]`` with arguments
   ``"configurable"``, ``desc.[[Configurable]]``, and ``false``.

7. Return ``obj``.

Inlining FromPropertyDescriptor
-------------------------------

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``name`` be ``ToString(P)``.
   (Note: this may have a side effect.)

3. Let ``desc`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with argument ``name``.

4. If ``desc`` is ``undefined``, then return ``undefined``.

5. Let ``obj`` be the result of creating a new object as if by the expression
   ``new Object()`` where ``Object`` is the standard built-in constructor with
   that name.

6. If ``IsDataDescriptor(desc)`` is ``true``, then

  a. Call ``obj.[[Put]]`` with arguments
     ``"value"``, ``desc.[[Value]]``, and ``false``.

  b. Call ``obj.[[Put]]`` with arguments
     ``"writable"``, ``desc.[[Writable]]``, and ``false``.

7. Else, ``IsAccessorDescriptor(Desc)`` must be ``true``, so

  a. Call ``obj.[[Put]]`` with arguments
     ``"get"``, ``desc.[[Get]]``, and ``false``.
     (Note: ``desc.[[Get]]`` may be ``undefined``.)

  b. Call ``obj.[[Put]]`` with arguments
     ``"set"``, ``desc.[[Set]]``, and ``false``.
     (Note: ``desc.[[Set]]`` may be ``undefined``.)

8. Call ``obj.[[Put]]`` with arguments
   ``"enumerable"``, ``desc.[[Enumerable]]``, and ``false``.

9. Call ``obj.[[Put]]`` with arguments
   ``"configurable"``, ``desc.[[Configurable]]``, and ``false``.

10. Return ``obj``.

Exposed Object.defineProperty()
===============================

Original algorithm
------------------

The algorithm is specified in E5 Section 15.2.3.6:

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``name`` be ``ToString(P)``.
   (Note: this may have side effects.)

3. Let ``desc`` be the result of calling ``ToPropertyDescriptor`` with
   ``Attributes`` as the argument.

4. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` with
   arguments ``name``, ``desc``, and ``true``.
   (Note: the last argument, ``true``, is the ``Throw`` flag.)

5. Return ``O``.

The algorithm returns the object, which allows chaining; for instance::

  var o = {};
  Object.defineProperty(o, 'foo',
    { value: 'bar' }
  ).seal();

``ToPropertyDescriptor()`` is a helper called only from
``Object.defineProperty()`` and ``Object.defineProperties()``.  It
converts a property descriptor expressed as an Ecmascript object into
a "specification descriptor", doing boolean coercions and cross checking
the descriptor.  For instance, ``ToPropertyDescriptor()`` will reject
any property descriptor which contains fields indicating it is both
a data property descriptor and an accessor property descriptor.
Example from Node / V8::

  > var o = {};
  > Object.defineProperty(o, 'foo',
  ...   { value: 'bar', set: function() {} });
  TypeError: Invalid property.  A property cannot both
  have accessors and be writable or have a value, #<Object>

The result of the property descriptor conversion is an "internal descriptor".
Note that unlike when dealing with existing object properties, this descriptor
may not be fully populated, i.e. may be missing fields.  From an implementation
perspective this means that the descriptor needs to be represented differently.
The current implementation doesn't have an explicit representation for the
"internal descriptor" which exists for the duration of
``Object.defineProperty()``; the descriptor is represented by a bunch of local
variables indicating the presence and coerced values of the descriptor fields
(for instance: ``has_writable`` and ``is_writable`` are separate variables).

The ``ToPropertyDescriptor()`` algorithm is reformulated in the restatements
section.

Other notes:

* The key is coerced to a string leniently while the object is just checked
  and never coerced.

* ``[[DefineOwnProperty]]`` is always called with ``Throw`` set to ``true``,
  so the implementation doesn't need to expose a "throw flag".

First draft
-----------

Starting from the original algorithm and inlining both
``ToPropertyDescriptor()`` and the ``[[DefineOwnProperty]]`` algorithm
with exotic behaviors, we get:

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``name`` be ``ToString(P)``.
   (Note: this may have side effects.)

3. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

4. Let ``desc`` be a new, empty Property Descriptor.

5. If ``O.[[HasProperty]]("enumerable")`` === ``true``, then
   set ``desc.[[Enumerable]]`` to ``ToBoolean(O.[[Get]]("enumerable"))``.

6. If ``O.[[HasProperty]]("configurable")`` === ``true``, then
   set ``desc.[[Configurable]]`` to ``ToBoolean(O.[[Get]]("configurable"))``.

7. If ``O.[[HasProperty]]("value")`` === ``true``, then
   set ``desc.[[Value]]`` to ``O.[[Get]]("value")``.

8. If ``O.[[HasProperty]]("writable")`` === ``true``, then
   set ``desc.[[Writable]]`` to ``ToBoolean(O.[[Get]]("writable"))``.

9. If ``O.[[HasProperty]]("get")`` === ``true``, then:

  a. Set ``desc.[[Get]]`` to ``O.[[Get]]("get")``.

  b. If ``desc.[[Get]]`` !== ``undefined`` and
     ``IsCallable(desc.[[Get]])`` === ``false``, then
     throw a ``TypeError`` exception.

10. If ``O.[[HasProperty]]("set")`` === ``true``, then:

  a. Set ``desc.[[Set]]`` to ``O.[[Get]]("set")``.

  b. If ``desc.[[Set]]`` !== ``undefined`` and
     ``IsCallable(desc.[[Set]])`` === ``false``, then
     throw a ``TypeError`` exception.

11. If either ``desc.[[Get]]`` or ``desc.[[Set]]`` are present, then:

  a. If either ``desc.[[Value]]`` or ``desc.[[Writable]]`` are present,
     then throw a ``TypeError`` exception.

12. Let ``Throw`` be ``true``.

13. Set ``pendingWriteProtect`` to ``false``.

14. If ``O`` is not an ``Array`` object, goto SKIPARRAY.

15. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
    internal method of ``O`` passing ``"length"`` as the argument.  The
    result will never be ``undefined`` or an accessor descriptor because
    ``Array`` objects are created with a length data property that cannot
    be deleted or reconfigured.

16. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
    (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

17. If ``P`` is ``"length"``, then

  a. If the ``[[Value]]`` field of ``Desc`` is absent, then goto SKIPARRAY.

  b. Let ``newLen`` be ``ToUint32(Desc.[[Value]])``.

  c. If ``newLen`` is not equal to ``ToNumber(Desc.[[Value]])``, goto
     REJECTRANGE.

  d. Set ``Desc.[[Value]]`` to ``newLen``.

  e. If ``newLen`` >= ``oldLen``, then goto SKIPARRAY.

  f. Goto REJECT if ``oldLenDesc.[[Writable]]`` is ``false``.

  g. If ``Desc.[[Writable]]`` has the value ``false``:

    1. Need to defer setting the ``[[Writable]]`` attribute to ``false``
       in case any elements cannot be deleted.

    2. Set ``pendingWriteProtect`` to ``true``.

    3. Set ``Desc.[[Writable]]`` to ``true``.

  h. Goto SKIPARRAY.  (Rest of the processing happens in the post-step.)

18. Else if ``P`` is an array index (E5 Section 15.4), then:

  a. Let ``index`` be ``ToUint32(P)``.

  b. Goto REJECT if ``index`` >= ``oldLen`` and ``oldLenDesc.[[Writable]]``
     is ``false``.

  c. Goto SKIPARRAY.  (Rest of the processing happens in the post-step.)

19. **SKIPARRAY**:
    Let ``current`` be the result of calling the ``[[GetOwnProperty]]``
    internal method of ``O`` with property name ``P``.

20. Let ``extensible`` be the value of the ``[[Extensible]]`` internal
    property of ``O``.

21. If ``current`` is ``undefined``:

  a. If ``extensible`` is ``false``, then goto REJECT.

  b. If ``IsGenericDescriptor(Desc)`` or ``IsDataDescriptor(Desc)`` is
     ``true``, then

    1. Create an own data property named ``P`` of object ``O`` whose
       ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
       ``[[Configurable]]`` attribute values are described by ``Desc``.
       If the value of an attribute field of ``Desc`` is absent, the
       attribute of the newly created property is set to its default
       value.

  c. Else, ``Desc`` must be an accessor Property Descriptor so,

    1. Create an own accessor property named ``P`` of object ``O`` whose
       ``[[Get]]``, ``[[Set]]``, ``[[Enumerable]]`` and ``[[Configurable]]``
       attribute values are described by ``Desc``.  If the value of an
       attribute field of ``Desc`` is absent, the attribute of the newly
       created property is set to its default value.

  d. Goto SUCCESS.

22. Goto SUCCESS, if every field in ``Desc`` also occurs in ``current``
    and the value of every field in ``Desc`` is the same value as the
    corresponding field in ``current`` when compared using the ``SameValue``
    algorithm (E5 Section 9.12).  (This also covers the case where
    every field in ``Desc`` is absent.)

23. If the ``[[Configurable]]`` field of ``current`` is ``false`` then

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``Desc`` is true.

  b. Goto REJECT, if the ``[[Enumerable]]`` field of ``Desc`` is present
     and the ``[[Enumerable]]`` fields of ``current`` and ``Desc`` are the
     Boolean negation of each other.

24. If ``IsGenericDescriptor(Desc)`` is ``true``, then goto VALIDATED.

25. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    have different results, then 

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``current`` is
     ``false``.

  b. If ``IsDataDescriptor(current)`` is true, then

    1. Convert the property named ``P`` of object ``O`` from a data property
       to an accessor property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  c. Else,

    1. Convert the property named ``P`` of object ``O`` from an accessor
       property to a data property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  d. Goto VALIDATED.

26. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    are both true, then

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Writable]]`` field of ``current`` is
       ``false`` and the ``[[Writable]]`` field of ``Desc`` is ``true``.

    2. Goto REJECT, If the ``[[Writable]]`` field of ``current`` is
       ``false``, and the ``[[Value]]`` field of ``Desc`` is present, and
       ``SameValue(Desc.[[Value]], current.[[Value]])`` is ``false``.

  b. Goto VALIDATED.

27. Else, ``IsAccessorDescriptor(current)`` and ``IsAccessorDescriptor(Desc)``
    are both ``true`` so,

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Set]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Set]], current.[[Set]])`` is ``false``.

    2. Goto REJECT, if the ``[[Get]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Get]], current.[[Get]])`` is ``false``.

  b. Goto VALIDATED.

28. **VALIDATED:**
    For each attribute field of ``Desc`` that is present, set the
    correspondingly named attribute of the property named ``P`` of object
    ``O`` to the value of the field.

29. **SUCCESS:**
    If ``O`` is an ``Array`` object:

  a. If ``P`` is ``"length"``, and ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. If ``pendingWriteProtect`` is ``true``, update the property
       (``"length"``) to have ``[[Writable]] = false``.

    4. Goto REJECT, if ``shortenSucceeded`` is ``false``.

  b. If ``P`` is an array index and ``index`` >= ``oldLen``:

    1. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
       This always succeeds, because we've checked in the pre-step that the
       ``"length"`` is writable, and since ``P`` is an array index property,
       the length must still be writable here.

30. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
    internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. If ``IsAccessorDescriptor(Desc)`` is ``true``, then:

      a. Call the ``[[Delete]]`` internal method of ``map`` passing ``P``,
         and ``false`` as the arguments.  (This removes the magic binding
         for ``P``.)

    2. Else (``Desc`` may be generic or data descriptor):

      a. If ``Desc.[[Value]]`` is present, then:

        1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
           ``Desc.[[Value]]``, and ``Throw`` as the arguments.  (This
           updates the bound variable value.)

      b. If ``Desc.[[Writable]]`` is present and its value is ``false``,
         then:

        1. Call the ``[[Delete]]`` internal method of ``map`` passing ``P``
           and ``false`` as arguments.  (This removes the magic binding
           for ``P``, and must happen after a possible update of the
           variable value.)

31. Return ``O``.

32. **REJECT**:
    If ``Throw`` is ``true``, then throw a ``TypeError`` exception,
    otherwise return ``false``.

33. **REJECTRANGE**:
    Throw a ``RangeError`` exception.  Note that this is unconditional
    (thrown even if ``Throw`` is ``false``).

Notes:

* Step 3 is redundant (it comes from ``ToPropertyDescriptor()`` because
  of step 1.

* Since ``Throw`` is always ``true``, step 12 can be removed and
  step 32 changed to throw ``TypeError`` unconditionally.  Note that
  ``Throw`` is also given as a parameter in step 30.b.2.1 as an
  argument for an internal ``[[Put]]`` to the parameter map.  This
  actually has no effect on behavior (the internal setter will be
  called, and the ``Throw`` flag is not visible to the setter).

Some cleanup
------------

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``name`` be ``ToString(P)``.
   (Note: this may have side effects.)

3. Let ``desc`` be a new, empty Property Descriptor.

4. If ``O.[[HasProperty]]("enumerable")`` === ``true``, then
   set ``desc.[[Enumerable]]`` to ``ToBoolean(O.[[Get]]("enumerable"))``.

5. If ``O.[[HasProperty]]("configurable")`` === ``true``, then
   set ``desc.[[Configurable]]`` to ``ToBoolean(O.[[Get]]("configurable"))``.

6. If ``O.[[HasProperty]]("value")`` === ``true``, then
   set ``desc.[[Value]]`` to ``O.[[Get]]("value")``.

7. If ``O.[[HasProperty]]("writable")`` === ``true``, then
   set ``desc.[[Writable]]`` to ``ToBoolean(O.[[Get]]("writable"))``.

8. If ``O.[[HasProperty]]("get")`` === ``true``, then:

  a. Set ``desc.[[Get]]`` to ``O.[[Get]]("get")``.

  b. If ``desc.[[Get]]`` !== ``undefined`` and
     ``IsCallable(desc.[[Get]])`` === ``false``, then
     throw a ``TypeError`` exception.

9. If ``O.[[HasProperty]]("set")`` === ``true``, then:

  a. Set ``desc.[[Set]]`` to ``O.[[Get]]("set")``.

  b. If ``desc.[[Set]]`` !== ``undefined`` and
     ``IsCallable(desc.[[Set]])`` === ``false``, then
     throw a ``TypeError`` exception.

10. If either ``desc.[[Get]]`` or ``desc.[[Set]]`` are present, then:

  a. If either ``desc.[[Value]]`` or ``desc.[[Writable]]`` are present,
     then throw a ``TypeError`` exception.

11. Set ``pendingWriteProtect`` to ``false``.

12. If ``O`` is not an ``Array`` object, goto SKIPARRAY.

13. Let ``oldLenDesc`` be the result of calling the ``[[GetOwnProperty]]``
    internal method of ``O`` passing ``"length"`` as the argument.  The
    result will never be ``undefined`` or an accessor descriptor because
    ``Array`` objects are created with a length data property that cannot
    be deleted or reconfigured.

14. Let ``oldLen`` be ``oldLenDesc.[[Value]]``. 
    (Note that ``oldLen`` is guaranteed to be a unsigned 32-bit integer.)

15. If ``P`` is ``"length"``, then

  a. If the ``[[Value]]`` field of ``Desc`` is absent, then goto SKIPARRAY.

  b. Let ``newLen`` be ``ToUint32(Desc.[[Value]])``.

  c. If ``newLen`` is not equal to ``ToNumber(Desc.[[Value]])``, goto
     REJECTRANGE.

  d. Set ``Desc.[[Value]]`` to ``newLen``.

  e. If ``newLen`` >= ``oldLen``, then goto SKIPARRAY.

  f. Goto REJECT if ``oldLenDesc.[[Writable]]`` is ``false``.

  g. If ``Desc.[[Writable]]`` has the value ``false``:

    1. Need to defer setting the ``[[Writable]]`` attribute to ``false``
       in case any elements cannot be deleted.

    2. Set ``pendingWriteProtect`` to ``true``.

    3. Set ``Desc.[[Writable]]`` to ``true``.

  h. Goto SKIPARRAY.  (Rest of the processing happens in the post-step.)

16. Else if ``P`` is an array index (E5 Section 15.4), then:

  a. Let ``index`` be ``ToUint32(P)``.

  b. Goto REJECT if ``index`` >= ``oldLen`` and ``oldLenDesc.[[Writable]]``
     is ``false``.

  c. Goto SKIPARRAY.  (Rest of the processing happens in the post-step.)

17. **SKIPARRAY**:
    Let ``current`` be the result of calling the ``[[GetOwnProperty]]``
    internal method of ``O`` with property name ``P``.

18. Let ``extensible`` be the value of the ``[[Extensible]]`` internal
    property of ``O``.

19. If ``current`` is ``undefined``:

  a. If ``extensible`` is ``false``, then goto REJECT.

  b. If ``IsGenericDescriptor(Desc)`` or ``IsDataDescriptor(Desc)`` is
     ``true``, then

    1. Create an own data property named ``P`` of object ``O`` whose
       ``[[Value]]``, ``[[Writable]]``, ``[[Enumerable]]`` and
       ``[[Configurable]]`` attribute values are described by ``Desc``.
       If the value of an attribute field of ``Desc`` is absent, the
       attribute of the newly created property is set to its default
       value.

  c. Else, ``Desc`` must be an accessor Property Descriptor so,

    1. Create an own accessor property named ``P`` of object ``O`` whose
       ``[[Get]]``, ``[[Set]]``, ``[[Enumerable]]`` and ``[[Configurable]]``
       attribute values are described by ``Desc``.  If the value of an
       attribute field of ``Desc`` is absent, the attribute of the newly
       created property is set to its default value.

  d. Goto SUCCESS.

20. Goto SUCCESS, if every field in ``Desc`` also occurs in ``current``
    and the value of every field in ``Desc`` is the same value as the
    corresponding field in ``current`` when compared using the ``SameValue``
    algorithm (E5 Section 9.12).  (This also covers the case where
    every field in ``Desc`` is absent.)

21. If the ``[[Configurable]]`` field of ``current`` is ``false`` then

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``Desc`` is true.

  b. Goto REJECT, if the ``[[Enumerable]]`` field of ``Desc`` is present
     and the ``[[Enumerable]]`` fields of ``current`` and ``Desc`` are the
     Boolean negation of each other.

22. If ``IsGenericDescriptor(Desc)`` is ``true``, then goto VALIDATED.

23. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    have different results, then 

  a. Goto REJECT, if the ``[[Configurable]]`` field of ``current`` is
     ``false``.

  b. If ``IsDataDescriptor(current)`` is true, then

    1. Convert the property named ``P`` of object ``O`` from a data property
       to an accessor property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  c. Else,

    1. Convert the property named ``P`` of object ``O`` from an accessor
       property to a data property.  Preserve the existing values of the
       converted property’s ``[[Configurable]]`` and ``[[Enumerable]]``
       attributes and set the rest of the property’s attributes to their
       default values.

  d. Goto VALIDATED.

24. Else, if ``IsDataDescriptor(current)`` and ``IsDataDescriptor(Desc)``
    are both true, then

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Writable]]`` field of ``current`` is
       ``false`` and the ``[[Writable]]`` field of ``Desc`` is ``true``.

    2. Goto REJECT, If the ``[[Writable]]`` field of ``current`` is
       ``false``, and the ``[[Value]]`` field of ``Desc`` is present, and
       ``SameValue(Desc.[[Value]], current.[[Value]])`` is ``false``.

  b. Goto VALIDATED.

25. Else, ``IsAccessorDescriptor(current)`` and ``IsAccessorDescriptor(Desc)``
    are both ``true`` so,

  a. If the ``[[Configurable]]`` field of ``current`` is ``false``, then

    1. Goto REJECT, if the ``[[Set]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Set]], current.[[Set]])`` is ``false``.

    2. Goto REJECT, if the ``[[Get]]`` field of ``Desc`` is present and
       ``SameValue(Desc.[[Get]], current.[[Get]])`` is ``false``.

  b. Goto VALIDATED.

26. **VALIDATED:**
    For each attribute field of ``Desc`` that is present, set the
    correspondingly named attribute of the property named ``P`` of object
    ``O`` to the value of the field.

27. **SUCCESS:**
    If ``O`` is an ``Array`` object:

  a. If ``P`` is ``"length"``, and ``newLen`` < ``oldLen``, then:

    1. Let ``shortenSucceeded``, ``finalLen`` be the result of calling the
       internal helper ``ShortenArray()`` with ``oldLen`` and ``newLen``.

    2. Update the property (``"length"``) value to ``finalLen``.

    3. If ``pendingWriteProtect`` is ``true``, update the property
       (``"length"``) to have ``[[Writable]] = false``.

    4. Goto REJECT, if ``shortenSucceeded`` is ``false``.

  b. If ``P`` is an array index and ``index`` >= ``oldLen``:

    1. Update the ``"length"`` property of ``O`` to the value ``index + 1``.
       This always succeeds, because we've checked in the pre-step that the
       ``"length"`` is writable, and since ``P`` is an array index property,
       the length must still be writable here.

28. If ``O`` is an arguments object which has a ``[[ParameterMap]]``
    internal property:

  a. Let ``map`` be the value of the ``[[ParameterMap]]`` internal property
     of the arguments object.

  b. If the result of calling the ``[[GetOwnProperty]]`` internal method
     of ``map`` passing ``P`` as the argument is not ``undefined``, then:

    1. If ``IsAccessorDescriptor(Desc)`` is ``true``, then:

      a. Call the ``[[Delete]]`` internal method of ``map`` passing ``P``,
         and ``false`` as the arguments.  (This removes the magic binding
         for ``P``.)

    2. Else (``Desc`` may be generic or data descriptor):

      a. If ``Desc.[[Value]]`` is present, then:

        1. Call the ``[[Put]]`` internal method of ``map`` passing ``P``,
           ``Desc.[[Value]]``, and ``true`` as the arguments.
           (This updates the bound variable value.  Note that the ``Throw``
           flag is irrelevant, ``true`` used now.)

      b. If ``Desc.[[Writable]]`` is present and its value is ``false``,
         then:

        1. Call the ``[[Delete]]`` internal method of ``map`` passing ``P``
           and ``false`` as arguments.  (This removes the magic binding
           for ``P``, and must happen after a possible update of the
           variable value.)

29. Return ``O``.

30. **REJECT**:
    Throw a ``TypeError`` exception.

31. **REJECTRANGE**:
    Throw a ``RangeError`` exception.

Exposed Object.defineProperties()
=================================

Implementation approach discussion
----------------------------------

Since ``Object.defineProperty()`` and ``Object.defineProperties()`` are
such expensive functions (from a code footprint point of view), we'd
really like to have only one implementation with some wrappers.  For
instance, we could have an actual implementation of
``Object.defineProperty()`` and then have ``Object.defineProperties()``
call it as a helper (or vice versa). 

Considering the case where ``Object.defineProperties()`` would use
``Object.defineProperty()`` as a helper, the ``Object.defineProperties()``
algorithm is unfortunate: it coerces all property descriptors with
``ToPropertyDescriptor()`` and puts them on an internal list
(``descriptors``) *before* doing any operations on the target object.
The coercion includes property descriptor validation, and implies some
way of storing the internal descriptors (other than local variables).

Note that the ``ToPropertyDescriptor()`` coercion may also have arbitrary
user visible side effects because it calls ``[[Get]]`` on the relevant
properties.  The ``[[Get]]`` may invoke a getter call, which may in
pathological cases even *modify* the other descriptors -- creating both an
ordering and a call count dependency.  Consider the pathological case::

  var desc2 = { value: 0 };
  var desc1 = {
    get value() {
      print("desc1 value getter");
      desc2.value++;  // increment for every call
      return "test";
    }
  };

  var descs = { foo: desc1, bar: desc2 };

  var o = {};
  Object.defineProperties(o, descs);
  print(o.foo);  // should print test
  print(o.bar);  // should print 1, as getter is called exactly once

If the implementation were to, for instance, call ``ToPropertyDescriptor()``
twice (once to validate, discarding any results, and a second time when
calling ``Object.defineProperty()`` internally as a helper), it would
fail the above test.

On the other hand, if the implementation simply called
``Object.defineProperty()`` for each descriptor in turn, it would not
be compliant if there is an invalid descriptor in the ``Properties``
argument list of ``Object.defineProperties()``.  No changes to the target
object can be made if there is an invalid descriptor in the list.

There are other pathological cases too, e.g. a getter removing elements
from the ``Properties`` argument of ``Object.defineProperties()``.

Another implementation approach is to make ``Object.defineProperties()`` the
main algorithm and have ``Object.defineProperty()`` be a wrapper around it.
This works but still has issues:

* ``Object.defineProperties()`` still needs to have a list of *coerced*
  descriptors internally, which implies some storage (other than local
  variables) for coerced (internal) descriptors.

* ``Object.defineProperty()`` would need to create a temporary object
  for containing the one property descriptor it gets as an input, e.g.::

    Object.defineProperty(o, 'foo', { value: 'bar' });

  needs to become::

    Object.defineProperties(o, { foo: { value: 'bar' }});

A way around creating an internal representation for partially populated
descriptors is to use an internal Ecmascript object representing a
validated and normalized descriptor with all property values already
coerced and checked; any getter calls would be done during coercion
and the final value would be a plain one.  In the pathological example
above, the internal descriptors could be::

  {
    foo: {
      value: "test"
    },
    bar: {
      value: 1
    }
  }

The coercions could then be executed first, and the coerced descriptors
then given one at a time (as Ecmascript objects) to
``Object.defineProperty()``.

This would eliminate any side effects of the coercion and would allow
validation of the descriptors before any object changes.  The downside
is the need for an additional helper, and creating temporary objects
for each ``Object.defineProperties()`` (but not ``Object.defineProperty()``)
call.

This is the current implementation approach.  The coercion helper is defined
as ``NormalizePropertyDescriptor`` in the restatements section and will be
inlined below.  Note that this helper is not part of the E5 specification.

Original algorithm
------------------

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``props`` be ``ToObject(Properties)``.

3. Let ``names`` be an internal list containing the names of each enumerable
   own property of ``props``.

4. Let ``descriptors`` be an empty internal List.

5. For each element ``P`` of ``names`` in list order,

  a. Let ``descObj`` be the result of calling the ``[[Get]]`` internal method
     of ``props`` with ``P`` as the argument.

  b. Let ``desc`` be the result of calling ``ToPropertyDescriptor`` with
     ``descObj`` as the argument.
     (Note: this step may fail due for invalid property descriptors, and may
     have user visible side effects due to potential getter calls.)

  c. Append ``desc`` to the end of ``descriptors``.

6. For each element ``desc`` of ``descriptors`` in list order,

  a. Call the ``[[DefineOwnProperty]]`` internal method of ``O`` with
     arguments ``P``, ``desc``, and ``true``.

7. Return ``O``.

Notes:

* In Step 6.a ``P`` should refer to the name related to the descriptor being
  processed, but there is no assignment for ``P`` after step 5.  This seems
  like a small typo in the specification.

Using NormalizePropertyDescriptor
---------------------------------

Below, the standard algorithm has been changed to use
``NormalizePropertyDescriptor()`` and to call ``Object.defineProperty()``
instead of ``[[DefineOwnProperty]]``:

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``props`` be ``ToObject(Properties)``.

3. Let ``descriptors`` be an empty internal Object.
   (Note: we assume that the object has enumeration order matching property
   insertion order.)

4. For each enumerable property ``P`` of ``props`` (in normal enumeration
   order),

  a. Let ``descObj`` be the result of calling the ``[[Get]]`` internal method
     of ``props`` with ``P`` as the argument.

  b. Let ``desc`` be the result of calling ``NormalizePropertyDescriptor``
     with ``descObj`` as the argument.
     (Note: this step may fail due for invalid property descriptors, and may
     have user visible side effects due to potential getter calls.)

  c. Call the ``[[Put]]`` internal method of ``descriptors`` with
     ``P``, ``desc`` and ``true`` as arguments.

5. For each enumerable property ``P`` of ``descriptors`` (in insertion
   order),

  a. Let ``desc`` be the result of calling the ``[[Get]]`` internal method
     of ``descriptors`` with ``P`` as the argument.
     (Note: this is guaranteed to succeed and yield a valid descriptor
     object.)

  b. Call the ``Object.defineProperty()`` built-in method with the arguments
     ``O``, ``P`` and ``desc``, ignoring its result value.
     (Note: this call may fail due to an exception.)

6. Return ``O``.

Changing ``[[DefineOwnProperty]]`` to ``Object.defineProperty()`` should be
semantically correct.  Consider the steps of ``Object.defineProperty()``
in E5 Section 15.2.3.6:

* Step 1: already covered by step 1 above.

* Step 2: a no-op because all property names (``P``) above are naturally
  strings.

* Step 3: guaranteed to succeed and be side-effect free, and to produce
  the same result as it normally would.

* Step 4: makes a call to ``[[DefineOwnProperty]]``

* Step 5: return value is ignored.

Future work
===========

* Add ES6 Proxy object or a Lua metatable-like mechanism and integrate it
  into the Ecmascript algorithms in a natural way (``[[Get]]``, ``[[GetOwnProperty]]``,
  ``[[HasProperty]]``, and ``[[DefineOwnProperty]]`` most likely).

* Integrate other ES6 features into the basic object representation, with
  possible some impact on these algorithms.

* Array fast path improvements for both reading of non-existent elements
  and writing elements in general.

* Review the algorithms for robustness against internal errors such as
  out of memory.  The order of operations should be chosen to minimize
  any inconsistency in state if an internal error occurs.

