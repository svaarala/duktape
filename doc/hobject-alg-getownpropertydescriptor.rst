=========================================
Exposed Object.getOwnPropertyDescriptor()
=========================================

Original algorithm
==================

The algorithm is specified in E5 Section 15.2.3.3:

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``name`` be ``ToString(P)``.
   (Note: this may have a side effect.)

3. Let ``desc`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with argument ``name``.

4. Return the result of calling ``FromPropertyDescriptor(desc)``
   (E5 Section 8.10.4).

FromPropertyDescriptor
======================

The ``FromPropertyDescriptor()`` algorithm in E5 Section 8.10.4 is
as follows:

1. If ``Desc`` is ``undefined``, then return ``undefined``.

2. Let ``obj`` be the result of creating a new object as if by the expression
   ``new Object()`` where ``Object`` is the standard built-in constructor with
   that name.

3. If ``IsDataDescriptor(Desc)`` is ``true``, then

   a. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
      arguments ``"value"``, Property Descriptor {[[Value]]: Desc.[[Value]],
      [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true}, and
      ``false``.

   b. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
      arguments ``"writable"``, Property Descriptor {[[Value]]:
      Desc.[[Writable]], [[Writable]]: true, [[Enumerable]]: true,
      [[Configurable]]: true}, and ``false``.

4. Else, ``IsAccessorDescriptor(Desc)`` must be ``true``, so

   a. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
      arguments ``"get"``, Property Descriptor {[[Value]]: Desc.[[Get]],
      [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true}, and
      ``false``.

   b. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
      arguments ``"set"``, Property Descriptor {[[Value]]: Desc.[[Set]],
      [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true}, and
      ``false``.

5. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
   arguments ``"enumerable"``, Property Descriptor {[[Value]]:
   Desc.[[Enumerable]], [[Writable]]: true, [[Enumerable]]: true,
   [[Configurable]]: true}, and ``false``.

6. Call the ``[[DefineOwnProperty]]`` internal method of ``obj`` with
   arguments ``"configurable"``, Property Descriptor {[[Value]]:
   Desc.[[Configurable]], [[Writable]]: true, [[Enumerable]]: true,
   [[Configurable]]: true}, and ``false``.

7. Return ``obj``.

Notes:

* Since all the ``[[DefineOwnProperty]]`` calls create new property values,
  and the property attributes match the defaults for ``[[Put]]``, we can
  simply use ``[[Put]]`` instead.  The ``Throw`` flag does not matter as
  the ``[[Put]]`` operations cannot fail (except for some internal reason,
  which is thrown unconditionally without regard for ``Throw`` anyway).

* The order of settings properties to ``obj`` matters since it will affect
  the enumeration order of ``obj``.

Changing ``[[DefineOwnProperty]]`` to ``[[Put]]`` and renaming ``Desc``
to ``desc`` (for compatibility with ``Object.getOwnPropertyDescriptor()``
algorithm):

1. If ``desc`` is ``undefined``, then return ``undefined``.

2. Let ``obj`` be the result of creating a new object as if by the expression
   ``new Object()`` where ``Object`` is the standard built-in constructor with
   that name.

3. If ``IsDataDescriptor(desc)`` is ``true``, then

   a. Call ``obj.[[Put]]`` with arguments
      ``"value"``, ``desc.[[Value]]``, and ``false``.

   b. Call ``obj.[[Put]]`` with arguments
      ``"writable"``, ``desc.[[Writable]]``, and ``false``.

4. Else, ``IsAccessorDescriptor(Desc)`` must be ``true``, so

   a. Call ``obj.[[Put]]`` with arguments
      ``"get"``, ``desc.[[Get]]``, and ``false``.
      (Note: ``desc.[[Get]]`` may be ``undefined``.)

   b. Call ``obj.[[Put]]`` with arguments
      ``"set"``, ``desc.[[Set]]``, and ``false``.
      (Note: ``desc.[[Set]]`` may be ``undefined``.)

5. Call ``obj.[[Put]]`` with arguments
   ``"enumerable"``, ``desc.[[Enumerable]]``, and ``false``.

6. Call ``obj.[[Put]]`` with arguments
   ``"configurable"``, ``desc.[[Configurable]]``, and ``false``.

7. Return ``obj``.

Inlining FromPropertyDescriptor
===============================

1. If ``Type(O)`` is not ``Object`` throw a ``TypeError`` exception.

2. Let ``name`` be ``ToString(P)``.
   (Note: this may have a side effect.)

3. Let ``desc`` be the result of calling the ``[[GetOwnProperty]]`` internal
   method of ``O`` with argument ``name``.

4. If ``desc`` is ``undefined``, then return ``undefined``.

5. Let ``obj`` be the result of creating a new object as if by the expression
   ``new Object()`` where ``Object`` is the standard built-in constructor with
   that name.

6. If ``IsDataDescriptor(desc)`` is ``true``, then

   a. Call ``obj.[[Put]]`` with arguments
      ``"value"``, ``desc.[[Value]]``, and ``false``.

   b. Call ``obj.[[Put]]`` with arguments
      ``"writable"``, ``desc.[[Writable]]``, and ``false``.

7. Else, ``IsAccessorDescriptor(Desc)`` must be ``true``, so

   a. Call ``obj.[[Put]]`` with arguments
      ``"get"``, ``desc.[[Get]]``, and ``false``.
      (Note: ``desc.[[Get]]`` may be ``undefined``.)

   b. Call ``obj.[[Put]]`` with arguments
      ``"set"``, ``desc.[[Set]]``, and ``false``.
      (Note: ``desc.[[Set]]`` may be ``undefined``.)

8. Call ``obj.[[Put]]`` with arguments
   ``"enumerable"``, ``desc.[[Enumerable]]``, and ``false``.

9. Call ``obj.[[Put]]`` with arguments
   ``"configurable"``, ``desc.[[Configurable]]``, and ``false``.

10. Return ``obj``.
