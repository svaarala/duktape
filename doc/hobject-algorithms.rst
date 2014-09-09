======================
duk_hobject algorithms
======================

Overview
========

Purpose
-------

This document and referenced sub-documents discuss, in detail, the internal
algorithms for dealing with objects, in particular for object property access.
These algorithms are based on the algorithm descriptions in the E5
specification, which have been refined towards the practical implementation
needs e.g. by combining multiple algorithms, inlining calls, and inlining
"exotic behaviors" (term borrowed from ES6).

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

Detailed algorithms
===================

========================================== =================================================
Document                                   Description
========================================== =================================================
``hobject-alg-preliminaries``              preliminary algorithm work
``hobject-alg-exoticbehaviors``            standard algorithms with exotic behaviors inlined
``hobject-alg-getprop``                    property read
``hobject-alg-putprop``                    property write
``hobject-alg-delprop``                    property deletion
``hobject-alg-hasprop``                    property existence check
``hobject-alg-instof``                     ``instanceof`` operator
``hobject-alg-getownpropertydescriptor``   ``Object.getOwnPropertyDescriptor()``
``hobject-alg-defineproperty``             ``Object.defineProperty()``
``hobject-alg-defineproperties``           ``Object.defineProperties()``
========================================== =================================================

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
