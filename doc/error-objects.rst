=============
Error objects
=============

Ecmascript allows throwing of arbitrary values, although most user code
throws objects inheriting from the ``Error`` constructor.  Ecmascript
``Error`` instances are quite barebones: they only contain a ``name``
and a ``message``.  Most Ecmascript implementations provide additional
error properties like file name, line number, and traceback.

This document describes how Duktape creates and throws ``Error`` objects
and what properties such objects have.  The internal traceback data format
and the mechanism for providing human readable tracebacks is also covered.
Also see the user documentation which covers the exposed features in a
more approachable way.

Error augmentation overview
===========================

Duktape allows error objects to be augmented at (1) their creation, and
(2) when they are just about the be thrown.  Augmenting an error object
at its creation time is usually preferable to augmenting it when it is
being thrown: an object is only created once but can be thrown and
rethrown multiple times (however, there are corner cases related to object
creation too, see below for details).

When an instance of ``Error`` is created:

* Duktape first adds traceback or file/line information (depending on
  activated features) to the error object.

* Then, if ``Duktape.errCreate`` is set, it is called to augment the error
  further (or replace it completely).  The callback is called an **error
  handler** inside the implementation.   The user can set an error handler
  if desired, by default it is not set.

Note that only error values which are instances of ``Error`` are augmented,
other kinds of values (even objects) are left alone.  A user error handler
only gets called with ``Error`` instances.

When **any value** is thrown (or re-thrown):

* If ``Duktape.errThrow`` is set, it is called to augment or replace the
  value thrown.  The user can set an error handler, by default it is not
  set.

Note that all values are given to ``Duktape.errThrow`` to process, not just
``Error`` instances, so the user error handler must be careful to handle all
value types properly.  The error handler also needs to handle re-throwing
in whatever way is appropriate in the user context.

Error object creation
=====================

Errors can be created in multiple ways:

* From Ecmascript code by creating an error, usually (but not always) tied
  to a ``throw`` statement, e.g.::

    throw new Error('my error');

  In this case the Error object should capture the file and line of the
  file creating the Error object (with ``new Error(...)``).

* From C code using the Duktape API, e.g.::

    duk_error(ctx, DUK_ERR_RANGE_ERROR, "invalid argument: %d", argvalue);

  In these cases the ``__FILE__`` and ``__LINE__`` of the throw site are
  very useful.  API calls which create an error object are implemented as
  macros to capture ``__FILE__`` and ``__LINE__`` conveniently.  This is
  very important to create useful tracebacks.

* From inside the Duktape implementation, usually with the ``DUK_ERROR()``
  macro, e.g.::

    DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid argument");

  In this case the Duktape internal file and line is useful and must be
  captured.  However, it is not "blamed" as the source of the error as far
  as filename and line number of the error are concerned (after all, the
  user doesn't usually care about the internal line numbers).

When errors are thrown using the Duktape API or from inside the Duktape
implementation, the value thrown is always an instance of ``Error`` and
is therefore augmented.  Error creation and throwing happens at the same
time.

When errors are thrown from Ecmascript code the situation is different.
There is nothing preventing user code from separating the error creation
and error throwing from each other::

  var err = new Error('value too large');
  if (arg >= 100) {
    throw err;
  }

In fact, the user may never intend to throw the error but may still want
to access the traceback::

  var err = new Error('currently here');
  print('debug: reached this point\n' + err.stack);

As discussed above, it's usually preferable to augment errors when they are
created rather than when they are thrown: re-throwing an error might cause it
to be augmented multiple times (overwriting previous values), and some errors
may never even be thrown but would still benefit from having traceback
information.

Duktape's built-in augmentation (essentially adding a traceback) happens at
error creation time; optional error handlers allow user to additionally process
errors both at their creation and just before they are thrown.

In more concrete terms, when a constructor call is made (i.e. ``new Foo()``)
the final result which is about to be returned to calling code is inspected.
This is a change to the standard handling of constructor calls and applies
uniformly whenever any object is created (and unfortunately carries some
overhead).  If the final value is an ``Error`` instance, i.e. its internal
prototype chain contains ``Error.prototype``:

* If the object is also extensible, the value gets augmented with error
  information (e.g. tracedata) by Duktape's built-in augmentation.

* If ``Duktape.errCreate`` is set, the error gets further processed by
  a user callback; note that the object doesn't need to be extensible
  for this to happen, but it still must be an ``Error`` instance.

Duktape refuses to add additional fields to the object if it already contains
fields of the same name.  For instance, if the created object has a ``_Tracedata``
field, it won't get overwritten by the augmentation process.  (User error
handler has no such restrictions, and it may replace the error value entirely.)

Although a particular object is never as such constructed twice, the current
approach may lead to an error object being augmented twice during its creation.
This can be achieved e.g. as follows::

  function Constructor() {
    return new Error('my error');
  }

  var e = new Constructor();

Here, error augmentation (including Duktape's own augmentation handling and
a user error handler) would happen twice:

1. When ``new Error('my error')`` executes, the result gets augmented.
   If a user error handler (``errCreate``) exists, it is called.

2. When the ``new Constructor()`` call returns, the returned error value
   replaces the default object given to the constructor.  The replacement
   value (i.e. the result of ``new Error('my error')``) gets augmented.

To avoid issues with this behavior, Duktape's augmentation code refuses
to add any field to an error if it's already present.  This ensures that
traceback data is not overwritten in step 2 above.  A user ``errCreate``
error handler must also deal properly with multiple calls for the same
error object.  It is easiest to do something like::

  Duktape.errCreate = function (e) {
      if ('timestamp' in e) {
          return e;  // only touch once
      }
      e.timestamp = new Date();
      return e;
  }

The downside of augmenting during creation is that the error information may
not accurately reflect the actual ``throw`` statement which throws the error.
In particular, user code may create an error value in a completely different
place at a completely different time than where and when the error is actually
thrown.  User code may even throw the same error value multiple times.

Error objects can also be created by calling the ``Error`` constructor (or a
constructor of a subclass) as a normal function.  In the standard this is
semantically equivalent to a constructor call.  Duktape will also augment an
error created by calling a built-in error constructor with a normal function
call.  However, any Error sub-classes created by the user don't exhibit this
behavior.  For instance::

  MyError = function(msg) { this.message = msg; this.name = 'MyError'; return this; }
  MyError.prototype = Error.prototype;

  var e1 = new Error('test 1');    // augmented, constructor call
  var e2 = Error('test 2');        // augmented, special handling
  var e3 = new MyError('test 3');  // augmented, constructor call
  var e4 = MyError('test 4');      // not augmented

  print(e1.stack);
  print(e2.stack);
  print(e3.stack);
  print(e4.stack);

Prints out::

  Error: test 1
          global test.js:4 preventsyield
  Error: test 2
          Error (null) native strict preventsyield
          global test.js:5 preventsyield
  MyError: test 3
          global test.js:6 preventsyield
  undefined

Note that because of internal details, the traceback is different for the
``Error`` constructor when it is called as a normal function.

Fixing this behavior so that even user errors get augmented when called with
a non-constructor call seems difficult.  It would be difficult to detect
when augmentation is appropriate and it would also add overhead to every
normal function call.

Error throwing
==============

When **any error value** is thrown, an optional user error handler set to
``Duktape.errThrow`` can process or replace the error value.  This applies
to all types, because any value can be thrown.

The user error handler must deal with the following:

* Restricting error value modification to only relevant values, e.g. only
  to ``Error`` instances.

* Dealing with re-throwing properly.

For example, the following would add a timestamp to an error object on their
first throw::

  Duktape.errThrow = function (e) {
      if (!(e instanceof Error)) {
          return e;  // only touch errors
      }
      if ('timestamp' in e) {
          return e;  // only touch once
      }
      e.timestamp = new Date();
      return e;
  }

Specifying error handlers
=========================

The current create/throw error handlers are stored in ``Duktape.errCreate``
and ``Duktape.errThrow``.  This has several advantages:

* The ``Duktape`` object is easy to access from both C and Ecmascript code
  without additional API bindings.

* It works relatively well with sandboxing: the ``Duktape`` object can be
  moved to a stash (not accessible from user code) during sandbox init,
  and error handlers can be controlled through the stash from C code.

* The scope for the error handlers is all threads sharing the same ``Duktape``
  built-in - i.e., threads sharing the same global environment.  This means
  that the error handlers are automatically effective in resumed threads,
  for instance, which is probably a good default behavior.

There are several approaches to the current approach, though.  One could store
the error handler(s) in:

* Internal data structures, e.g. ``thr->errcreate`` and ``thr->errthrow``.
  This would be stronger from a sandboxing point-of-view, but would require
  custom bindings to get/set the handlers.  Also memory management would need
  to know about the fields.

* Calling thread's value stack (in a caller's frame), only for the duration of
  a specific protected call.  This model is used by Lua and was also used by
  Duktape up to 0.9.0.  The downside is that protected calls need to manage
  error handlers which are quite rarely used.

* Global object.  This seems overall worse than using the ``Duktape`` object,
  as it would be worse for sandboxing with no apparent advantages.

* Thread object.  This would require some extra code to "inherit" error
  handler(s) to a resumed thread (as that seems like a good default behavior).

* Global stash.  Good for sandboxing, but would only be accessible from C code
  by default.

* Thread stash.  Good for sandboxing, error handler "inherit" issue.

Error object properties
=======================

The following table summarizes properties of ``Error`` objects constructed
within the control of the implementation:

+-----------------+----------+--------------------------------------------+
| Property        | Standard | Description                                |
+=================+==========+============================================+
| name            | yes      | e.g. ``TypeError`` for a TypeError         |
|                 |          | (usually inherited)                        |
+-----------------+----------+--------------------------------------------+
| message         | yes      | message given when constructing (or empty) |
|                 |          | (own property)                             |
+-----------------+----------+--------------------------------------------+
| fileName        | no       | name of the file where constructed         |
|                 |          | (inherited accessor)                       |
+-----------------+----------+--------------------------------------------+
| lineNumber      | no       | line of the file where constructed         |
|                 |          | (inherited accessor)                       |
+-----------------+----------+--------------------------------------------+
| stack           | no       | printable stack traceback string           |
|                 |          | (inherited accessor)                       |
+-----------------+----------+--------------------------------------------+
| _Tracedata      | no       | stack traceback data, internal raw format  |
|                 |          | (own, internal property)                   |
+-----------------+----------+--------------------------------------------+

The ``Error.prototype`` contains the following non-standard properties:

+-----------------+----------+--------------------------------------------+
| Property        | Standard | Description                                |
+=================+==========+============================================+
| stack           | no       | Accessor property for getting a printable  |
|                 |          | traceback based on _Tracedata.             |
+-----------------+----------+--------------------------------------------+
| fileName        | no       | Accessor property for getting a filename   |
|                 |          | based on _Tracedata.                       |
+-----------------+----------+--------------------------------------------+
| lineNumber      | no       | Accessor property for getting a linenumber |
|                 |          | based on _Tracedata.                       |
+-----------------+----------+--------------------------------------------+

All of the accessors are in the prototype in case the object instance does
not have an "own" property of the same name.  This allows for flexibility
in minimizing the property count of error instances while still making it
possible to provide instance-specific values when appropriate.  Note that
the setters allow user code to write an instance-specific value as an "own
property" of the error object, thus shadowing the accessors in later reads.

Notes:

* The ``stack`` property name is from V8 and behavior is close to V8.
  V8 allows user code to write to the ``stack`` property but does not
  create an own property of the same name.  The written value is still
  visible when ``stack`` is read back later.

* The ``fileName`` and ``lineNumber`` property names are from Rhino.

* The ``_Tracedata`` has an internal format which may change from version
  to version (even build to build).  It should never be serialized or
  used outside the life cycle of a Duktape heap.

* In size-optimized builds traceback information may be omitted.  In such
  cases ``fileName`` and ``lineNumber`` are concrete own properties.

* In size-optimized builds errors created by the Duktape implementation
  will not have a useful ``message`` field.  Instead, ``message`` is set
  to a string representation of the error ``code``.  Exceptions thrown
  from user code will carry ``message`` normally.

* The ``_Tracedata`` property contains function references to functions in
  the current call stack.  Because such references are a potential sandboxing
  concern, the tracedata is stored in an internal property.

Cause chains
============

There is currently no support for cause chains: Ecmascript doesn't have a
cause chain concept nor does there seem to be an unofficial standard for
them either.

A custom cause chain could be easily supported by allowing a ``cause``
property to be set on an error, and making the traceback formatter obey it.

A custom mechanism for setting an error cause would need to be used.
A very non-invasive approach would be something like::

  try {
    f();
  } catch (e) {
    var e2 = new Error("something went wrong");  // line N
    e2.cause = e;                                // line N+1
    throw e2;                                    // line N+2
  }

This is quite awkward and error line information is easily distorted.
The line number issue can be mitigated by putting the error creation
on a single line, at the cost of readability::

  try {
    f();
  } catch (e) {
    var e2 = new Error("something went wrong"); e2.cause = e; throw e2;
  }

One could also extend the error constructor to allow a cause to be specified
in a constructor call.  This would mimic how Java works and would be nice to
use, but would have more potential to interfere with standard semantics::

  try {
    f();
  } catch (e) {
    throw new Error("something went wrong", e);
  }

Using a setter method inherited from ``Error.prototype`` would be a very bad
idea as any such calls would be non-portable and cause errors to be thrown
when used in other Ecmascript engines::

  try {
    f();
  } catch (e) {
    var e2 = new Error("something went wrong", e);
    e2.setCause(e);  // throws error if setCause is undefined!
    throw e2;
  }

Since errors are also created (and thrown) from C code using the Duktape
API and from inside the Duktape implementation, cause handling would need
to be considered for these too.

Because the ``cause`` property can be set to anything, the implementation
would need to tolerate e.g.::

  // non-Error causes (print reasonably in a traceback)
  e.cause = 1;

  // cause loops (detect or sanity depth limit traceback)
  e1.cause = e2;
  e2.cause = e1;

Traceback format (_Tracedata)
=============================

The purpose of the ``_Tracedata`` value is to capture the relevant call stack
information very quickly before the call stack is unwound by error handling.
In many cases the traceback information is not used at all, so it should be
recorded in a compact and cheap manner.

To fulfill these requirements, the current format, described below, is a bit
arcane.  The format is version dependent, and is not intended to be accessed
directly by user code.  The implementation should provide stable helpers for
getting e.g. readable tracebacks or inspecting the traceback entries.

The ``_Tracedata`` value is a flat array, populated with values describing
the contents of the call stack, starting from the call stack top and working
downwards until either the call stack bottom or the maximum traceback depth
is reached.

If a call has a related C ``__FILE__`` and ``__LINE__`` those are first
pushed to ``_Tracedata``:

* The ``__FILE__`` value as a string.

* A number (double) containing the expression::

    (flags << 32) + (__LINE__)

  The only current flag indicates whether or not the ``__FILE__`` /
  ``__LINE__`` pair should be "blamed" as the error location when the user
  requests for a ``fileName`` or ``lineNumber`` related to the error.

After that, for each call stack element, the array entries appended to
``_Tracedata`` are pairs consisting of:

* The function object of the activation.  The function object contains the
  function type and name.  It also contains the filename (or equivalent, like
  "global" or "eval") and possibly PC-to-line debug information.  These are
  needed to create a printable traceback.

* A number (double) containing the expression::

    (activation_flags << 32) + (activation_pc)

  For C functions, the program counter value is zero.  Activation flag
  values are defined in ``duk_hthread.h``.  The PC value can be converted
  to a line number with debug information in the function object.  The
  flags allow e.g. tailcalls to be noted in the traceback.

The default ``Error.prototype.stack`` accessor knows how to convert this
internal format into a human readable, printable traceback string.  It is
currently the only function processing the tracedata, although it would be
useful to provide user functions to access or decode elements of the
traceback individually.

Notes:

* An IEEE double can hold a 53-bit integer accurately so there is space
  for plenty of flags in the current representation.  Flags must be in
  the low end of the flags field though (bit 20 or lower)

* The number of elements appended to the ``_Tracedata`` array for each
  activation does not need to constant, as long as the value can be decoded
  starting from the beginning of the array (in other words, random access is
  not important at the moment).

* The ``this`` binding, if any, is not currently recorded.

* The variable values of activation records are not recorded.  They would
  actually be available because the call stack can be inspected and register
  maps (if defined) would provide a way to map identifier names to registers.
  This is definitely future work and may be needed for better debugging
  support.

* The ``_Tracedata`` value is currently an array, but it may later be changed
  into an internal type of its own right to optimize memory usage and
  performance.  The internal type would then basically be a typed buffer
  which garbage collection would know how to visit.

