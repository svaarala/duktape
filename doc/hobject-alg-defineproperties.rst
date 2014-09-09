=================================
Exposed Object.defineProperties()
=================================

Implementation approach discussion
==================================

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
==================

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
=================================

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
