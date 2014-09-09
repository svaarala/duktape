===============================
Exposed Object.defineProperty()
===============================

Original algorithm
==================

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
===========

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
============

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
