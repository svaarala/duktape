=======================================
PUTPROP: exposed property put algorithm
=======================================

Background
==========

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
===========

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
===========

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
================================

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
==========================

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
==================

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
===========================

There is currently no fast path for array indices in the implementation.

This is primarily because to implement ``[[Put]`` properly, the prototype
chain needs to be walked when creating new properties, as an ancestor
property may prevent or capture the write.  The current implementation cannot
walk the prototype chain without coercing the key to a string first.
A fast path could be easily added for writing to existing array entries,
though, but it's probably better to solve the problem a bit more comprehensively.

Implementation notes
====================

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
=============

(See above.)
