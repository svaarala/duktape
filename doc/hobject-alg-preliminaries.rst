==========================
Preliminary algorithm work
==========================

In this section we look at the internal algorithms and do some preliminary
work of restating them by: inlining algorithms, merging algorithms, looking
at algorithm behavior with some fixed parameters, etc.  Tricky issues of
algorithms are also discussed to some extent.

The purpose of this section is to provide raw material for the sections
dealing with actual exposed algorithms.

CanPut
======

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
===========

``[[GetProperty]]`` is a very straightforward wrapper over
``[[GetOwnProperty]]`` which follows the prototype chain.  Like
``[[GetOwnProperty]]``, it returns a descriptor.

There is no exotic behavior for ``[[GetProperty]]``, the exotic behaviors
only affect ``[[GetOwnProperty]]`` which is called during ``[[GetProperty]]``.

Original algorithm
------------------

1. Let ``prop`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with property name ``P``.

2. If ``prop`` is not ``undefined``, return ``prop``.

3. Let ``proto`` be the value of the ``[[Prototype]]`` internal property of
   ``O``.

4. If ``proto`` is ``null``, return ``undefined``.

5. Return the result of calling the ``[[GetProperty]]`` internal method of
   ``proto`` with argument ``P``.

Eliminating recursion
---------------------

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
----------------

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
-----------------------------------------------

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
------------------------------------------------

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

         * ``[[Value]]:`` a primitive string of length 1, containing one
           character from ``str`` at position ``index`` (zero based index)

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
===

``[[Get]]`` is straightforward; it gets a property descriptor with
``[[GetProperty]]`` and then coerces it to a value.

Get with GetProperty inlined
============================

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
=========================

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
=================================================

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
====================================================

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
===========================================================

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
===

"Reject" below is shorthand for:

* If ``Throw`` is ``true``, then throw a ``TypeError`` exception; else return.

Original algorithm
------------------

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
------------------------------

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
--------------------

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
----------------

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
-------------------

Note that ``PutValue()`` has a ``[[Put]]`` variant with two exotic
behaviors related to object coercion.  The above algorithm does not
take those into account.

Property descriptor algorithms
==============================

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
====================

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
===========================

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
