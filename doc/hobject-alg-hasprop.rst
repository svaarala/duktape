=========================================================
HASPROP: Exposed property existence check ("in" operator)
=========================================================

Background
==========

Property existence check is done using the ``in`` operator in Ecmascript
code, e.g.::

  print('foo' in bar);  // check whether foo.bar exists

This involves:

* An expression for the left-hand-side

* An expression for the right-hand-side

* ``in`` semantics (E5 Section 11.8.7)

* A call to ``[[HasProperty]]``

First draft
===========

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
====================

Inlining ``[[HasProperty]]`` from E5 Section 8.12.6:

1. If ``O`` is not an object, throw a ``TypeError``

2. ``P`` = ``ToString(P)``

3. Let ``desc`` be the result of calling the ``[[GetProperty]]`` internal
   method of ``O`` with property name ``P``.

4. If ``desc`` is ``undefined``, then return ``false``.

5. Else return ``true``.
