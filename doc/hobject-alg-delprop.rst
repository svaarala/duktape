================================================================
DELPROP: exposed property deletion algorithm ("delete" operator)
================================================================

Background
==========

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
===========

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
========================

We want to avoid the object coercion; let's first make it more explicit:

1. If ``O`` is ``null`` or ``undefined``, throw a ``TypeError``

2. ``P`` = ``ToString(P)``

3. If ``O`` is an object, call ``[[Delete]](O, P, currStrict)``, and
   return its result

4. Else ``O`` is primitive:

   a. ``O`` = ``ToObject(O)`` (create temporary object)

   b. Call ``O.[[Delete]](P, currStrict)``, and return its result

Avoiding temporary objects
==========================

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
===========================

It would be straightforward to add a fast path for array indices, but there
is no fast path in the current implementation for array index deletion.
The index is always string coerced and interned.
