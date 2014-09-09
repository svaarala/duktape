================
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
==============

Related E5 sections:

* E5 Section 8.12.1: default algorithm

* E5 Section 15.5.5: ``String``

* E5 Section 10.5: arguments object

Default algorithm
-----------------

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
------------------------------------

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
---------------------------------------

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
-------------

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
-----------------

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
--------------------------------------

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
---------------------------------------

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
-------------

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
=================

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
-----------------

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
------------------------------

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
----------------------------------------------

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

  + Jumping to step 5.

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
---------------------------------------

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
---------------------------------------

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
-------------

(See above, currently no additional cleanup.)

Delete
======

Related E5 sections:

* E5 Section 8.12.7: default algorithm

* E5 Section 10.5: arguments object

Default algorithm
-----------------

1. Let ``desc`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with property name ``P``.

2. If ``desc`` is ``undefined``, then return ``true``.

3. If ``desc.[[Configurable]]`` is ``true``, then

   a. Remove the own property with name ``P`` from ``O``.

   b. Return ``true``.

4. Else if ``Throw`` is true, then throw a ``TypeError`` exception.

5. Return ``false``.

Adding arguments object exotic behavior
---------------------------------------

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

      1. Call the ``[[Delete]]`` internal method of ``map`` passing ``P``,
         and ``false`` as the arguments.  (This removes the magic binding
         for ``P``.)

7. Return ``true``.

Notes:

* In steps 2, if ``desc`` is ``undefined``, it seems unnecessary to go to
  step 6 to check the arguments parameter map.  Can a magically bound
  property exist in the parameter map with the underlying property having
  been deleted somehow?

Final version
-------------

(See above, currently no additional cleanup.)

HasInstance
===========

Background
----------

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
------------------

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
-------------

(See above, currently no additional cleanup.)
