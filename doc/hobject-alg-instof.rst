=====================================================================
INSTOF: exposed object class membership check ("instanceof" operator)
=====================================================================

Background
==========

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
===========

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
=======

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
