==========================
Lightweight function value
==========================

Overview
========

This document describes the changes necessary to support a lightweight
function value type, which refers to a native Duktape/C function without
needing a representative Ecmascript Function object.  This is useful to
minimize footprint of typical Duktape/C binding functions.

Changes needed
==============

* Add a new tagged type, DUK_TAG_LIGHTFUNC, which points to a Duktape/C
  function.

  - A lightweight function can be a constructor, but will never have an
    automatic prototype object (a property slot would be needed to store it).
    A lightweight constructor function can create a replacement object from
    scratch and discard the automatically created instance object.

* The representation depends on the ``duk_tval`` layout:

  - For 8-byte packed type: 32-bit function pointer (pointing to the
    Duktape/C function) and 16 bits for function metadata.

  - For unpacked type: function pointer and 16 bits options field.

* The options field should contain:

  - Function argument count / varargs indicator.  Also used to deduce
    a virtual "length" property.

  - Depending on how many functions there are with a "length" property
    different from their internal "nargs" property, perhaps a field for
    external "length" virtual property.

  - A small magic value, often needed in internals.

* Add support in call handling for calling such a function.

* Add support in traceback handling.

* Add virtual object properties so that lightweight functions will appear
  like ordinary Function objects to some extent, e.g.:

  - "length": based on argument count, 0 if vararg.

  - "name": as no name can be stored, this should maybe be some useful
    string containing the function pointer, e.g. "lightfunc:0xdeadbeef".

  - "fileName": as no name can be stored, this should maybe be something
    like "lightfunc", or perhaps same as "name".

  - Perhaps some virtual properties like "caller" and "arguments", as
    given to strict Ecmascript functions.

* It would be nice to be able to convert a lightweight function to a
  normal function object (which would be non-unique) if necessary.

* Add an option to change built-in functions into lightweight functions
  instead of Function objects.  This should not be active by default,
  because this change makes the built-ins strictly non-compliant.  However,
  this is quite useful in RAM constrained environments.

* Extend the public API to allow the user to push lightweight function
  pointers in addition to ordinary ones.  Or perhaps make the default
  behavior to push a lightweight function (arguments permitting).

Motivation
==========

Normal function representation
------------------------------

In Duktape 0.11.0 functions are represented as:

* A ``duk_hcompiledfunction`` (a superset of ``duk_hobject``): represents
  an Ecmascript function which may have a set of properties, and points to
  the function's data area (bytecode, constants, inner function refs).

* A ``duk_hnativefunction`` (a superset of ``duk_hobject``): represents
  a Duktape/C function which may also have a set of properties.  A pointer
  to the C function is inside the ``duk_hnativefunction`` structure.

This heavyweight representation is a RAM footprint issue, as discussed below.

Ecmascript functions
--------------------

An ordinary Ecmascript function takes over 200 bytes of RAM.  There are
two objects: the function itself and its automatic prototype object.
The function contains a ``.prototype`` property while the prototype
contains a ``.constructor`` property, so that both functions require a
property table.  This is the case even for the majority of user functions
which will never be used as constructors; built-in functions are oddly
exempt from having an automatic prototype.  Taken together these four
allocations take over 200 bytes.

Duktape/C functions
-------------------

A Duktape/C function takes about 70-80 bytes of RAM.  Unlike Ecmascript
functions, Duktape/C function are already stripped of unnecessary properties
and don't have an automatic prototype object.

Even so, there are close to 200 built-in functions, so the footprint of
the ``duk_hnativefunction`` objects is around 16kB, not taking into account
allocator overhead.
