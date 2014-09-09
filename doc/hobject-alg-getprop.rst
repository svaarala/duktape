=======================================
GETPROP: exposed property get algorithm
=======================================

Background
==========

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
===========

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
=====================================

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
==========================

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
===========================

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
====================

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
=============

(See above.)
