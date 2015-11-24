=============
Error objects
=============

Standard Ecmascript ``Error`` instances are quite barebones: they only contain
a ``name`` and a ``message``.  Most Ecmascript implementations, including
Duktape, provide additional error properties like file name, line number, and
traceback.  Ecmascript allows throwing of arbitrary values, although most user
code throws objects inheriting from the ``Error`` constructor.

This document describes how Duktape creates and throws ``Error`` objects,
what properties such objects have, and what error message verbosity levels
are available.  The internal traceback data format and the mechanism for
providing human readable tracebacks is also covered.

Also see the user documentation which covers the exposed features in a more
approachable way.

Error message verbosity levels
==============================

There are three levels of error message verbosity, controlled by indicated
defines:

+------------------------+-------------------------+-----------------------------------------------------------------+
| DUK_USE_VERBOSE_ERRORS | DUK_USE_PARANOID_ERRORS | Description                                                     |
+========================+=========================+=================================================================+
| set                    | not set                 | Verbose messages with offending keys/values included, e.g.      |
|                        |                         | ``number required, found 'xyzzy' (index -3)``.  This is the     |
|                        |                         | default behavior.                                               |
+------------------------+-------------------------+-----------------------------------------------------------------+
| set                    | set                     | Verbose messages with offending keys/values not included, e.g.  |
|                        |                         | ``number required, found string (index -3)``.  Useful when      |
|                        |                         | keys/values in error messages are considered a potential        |
|                        |                         | security issue.                                                 |
+------------------------+-------------------------+-----------------------------------------------------------------+
| not set                | ignored                 | Error objects won't have actual error messages; error code      |
|                        |                         | converted to a string is provided in ``.message``.  Useful for  |
|                        |                         | very low memory targets.                                        |
+------------------------+-------------------------+-----------------------------------------------------------------+

Future work:

* It would be useful to have low memory error messages where error message
  strings were present, but there would be a minimal number of different
  message strings.  For example, all errors from ``duk_require_xxx()`` type
  mismatches could result in ``"unexpected type"`` and all stack index
  errors could result in ``"invalid argument"``.

Error augmentation overview
===========================

Duktape allows error objects to be augmented at (1) their creation, and
(2) when they are just about the be thrown.  Augmenting an error object
at its creation time is usually preferable to augmenting it when it is
being thrown: an object is only created once but can be thrown and
rethrown multiple times (however, there are corner cases related to object
creation too, see below for details).

When an instance of ``Error`` is created:

* Duktape first adds traceback or file/line information (depending on active
  config options) to the error object.

* Then, if ``Duktape.errCreate`` is set, it is called to augment the error
  further or replace it completely.  The callback is called an **error
  handler** inside the implementation.   The user can set an error handler
  if desired, by default it is not set.

Note that only error values which are instances of ``Error`` are augmented,
other kinds of values (even objects) are left alone.  A user error handler
only gets called with ``Error`` instances.

When **any value** is thrown (or re-thrown):

* If ``Duktape.errThrow`` is set, it is called to augment or replace the
  value thrown.  The user can set an error handler if desired, by default
  it is not set.

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

    DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid argument: %d", argvalue);

  In these cases the ``__FILE__`` and ``__LINE__`` of the throw site are
  included in the stack trace, but are not "blamed" as the source of the
  error for the Error object's ``.fileName`` and ``.lineNumber``: the
  file/line is Duktape internal and not the most useful for user code.

  There are several helper macros for specific errors which work similarly
  to the basic ``DUK_ERROR()`` macro.

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
approach may lead to an error object being augmented twice during its creation
in special cases.  This can be achieved e.g. as follows::

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

Current approach
----------------

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

Design alternatives
-------------------

There are several alternatives to the current approach, though.  One could
store the error handler(s) in:

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
  by default.  This seems like one of the best alternatives for the current
  behavior.

* Thread stash.  Good for sandboxing, error handler "inherit" issue.

Error object properties
=======================

The following table summarizes properties of ``Error`` objects constructed
within the control of the implementation, with default Duktape config options
(in particular, tracebacks enabled):

+-----------------+----------+-----------+--------------------------------------------+
| Property        | Standard | Inherited | Description                                |
+=================+==========+===========+============================================+
| name            | yes      | yes       | e.g. ``TypeError`` for a TypeError         |
|                 |          |           | (usually inherited)                        |
+-----------------+----------+-----------+--------------------------------------------+
| message         | yes      | no        | message given when constructing (or empty) |
|                 |          |           | (own property)                             |
+-----------------+----------+-----------+--------------------------------------------+
| fileName        | no       | yes       | name of the file where constructed         |
|                 |          |           | (inherited accessor)                       |
+-----------------+----------+-----------+--------------------------------------------+
| lineNumber      | no       | yes       | line of the file where constructed         |
|                 |          |           | (inherited accessor)                       |
+-----------------+----------+-----------+--------------------------------------------+
| stack           | no       | yes       | printable stack traceback string           |
|                 |          |           | (inherited accessor)                       |
+-----------------+----------+-----------+--------------------------------------------+
| _Tracedata      | no       | no        | stack traceback data, internal raw format  |
|                 |          |           | (own, internal property)                   |
+-----------------+----------+-----------+--------------------------------------------+

The ``Error.prototype`` contains the following non-standard properties:

+-----------------+----------+--------------------------------------------+
| Property        | Standard | Description                                |
+=================+==========+============================================+
| stack           | no       | accessor property for getting a printable  |
|                 |          | traceback based on _Tracedata              |
+-----------------+----------+--------------------------------------------+
| fileName        | no       | accessor property for getting a filename   |
|                 |          | based on _Tracedata                        |
+-----------------+----------+--------------------------------------------+
| lineNumber      | no       | accessor property for getting a linenumber |
|                 |          | based on _Tracedata                        |
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

* In Duktape 1.3.0 and prior user code couldn't directly write ``.fileName``,
  ``.lineNumber``, or ``.stack`` because the inherited setter would capture
  and ignore such writes.  User code could use ``Object.defineProperty()`` or
  ``duk_def_prop()`` to create overriding properties.  In Duktape 1.4.0 the
  setter was changed to make writes work transparently: they're still captured
  by the setter, but the setter automatically creates the own property.

* The ``_Tracedata`` has an internal format which may change from version
  to version (even build to build).  It should never be serialized or
  used outside the life cycle of a Duktape heap.

* In size-optimized builds traceback information may be omitted.  In such
  cases ``fileName`` and ``lineNumber`` are concrete own properties, and
  ``.stack`` is an inherited property which returns a ``ToString()``
  coerced error string, e.g. ``TypeError: my error message``.

* In size-optimized builds errors created by the Duktape implementation
  will not have a useful ``message`` field.  Instead, ``message`` is set
  to a string representation of the error ``code``.  Exceptions thrown
  from user code will carry ``message`` normally.

* The ``_Tracedata`` property contains function references to functions in
  the current callstack.  Because such references are a potential sandboxing
  concern, the tracedata is stored in an internal property.

Choosing .fileName and .lineNumber to blame for an error
========================================================

Overview of the issue
---------------------

When an error is created/thrown, it's not always clear which file/line to
"blame" as the source of the error: the ``.fileName`` and ``.lineNumber``
properties of the Error object should be useful for the application programmer
to pinpoint the most likely cause of the error.

The relevant file/line pairs are:

* **The __FILE__ and __LINE__ of the C call site**.  While these often refer
  to a line in a Duktape/C function, it's possible for the Duktape/C function
  to call into a helper in a different file which throws the error.  The C
  call site may also be inside Duktape, e.g.  if user code calls
  ``duk_require_xxx()`` which then throws using the internal ``DUK_ERROR()``
  macro.  Finally, it's also possible to throw an error when no callstack
  entries are present; the C call site information will still be available.

* **The file/line of a source text being compiled**.  This is only relevant
  for errors thrown during compilation (typically SyntaxErrors but may be
  other errors too).

* **Actual callstack entries (activations) leading up to the error**.  These
  may be Duktape/C and Ecmascript functions.  The functions have a varying set
  of properties: for example, Ecmascript functions have both a ``.name`` and a
  ``.fileName`` property by default, while Duktape/C functions don't.  It's
  possible to add and remove properties after function creation.

The following SyntaxError illustrates all the relevant file/line sources::

    duk> try { eval('\n\nfoo='); } catch (e) { print(e.stack); print(e.fileName, e.lineNumber); }
    SyntaxError: parse error (line 3)
            input:3                                        <-- file/line of source text (SyntaxError)
            duk_js_compiler.c:3612                         <-- __FILE__ / __LINE__ of DUK_ERROR() call site
            eval  native strict directeval preventsyield   <-- innermost activation, eval() function
            global input:1 preventsyield                   <-- second innermost activation, caller of eval()
    input 3   <-- .fileName and .lineNumber blames source text for SyntaxError

From the application point of view the most relevant file/line is usually the
closest "user function", as opposed to "infrastructure function", in the
callstack.  The following are often not useful for blaming:

* Any Duktape/C or Ecmascript functions which are considered infrastructure
  functions, such as errors checkers, one-to-one system call wrappers, etc.

* C call sites inside Duktape; these are essentially always infrastructure
  functions.

* Any Duktape/C or Ecmascript functions missing a ``.fileName`` property.
  Such functions should be ignored even if they're user functions because
  the resulting file/line information would be pointless.

For this ideal outcome to be possible, Duktape needs to be able to determine
whether or not a function should be ignored for blaming.  This is not yet
possible; subsections below describe the current behavior.

Note that while file/line information is important for good error reporting,
all the relevant information is always available in the stack trace anyway.
Incorrect file/line blaming is annoying but usually not a critical issue.

Duktape 1.3 behavior
--------------------

The rules for blaming a certain file/line for an error are relatively simple
in Duktape 1.3:

* For error thrown during compilation the source text file/line is always
  blamed.  The compilation errors are typically SyntaxErrors, but may also
  be e.g. out-of-memory internal errors.

* For errors thrown from Duktape internals (including Duktape API functions
  like ``duk_require_xxx()``) the C call site is ignored, and the innermost
  activation is used for file/line information.  This is the case even when
  the innermost activation's function has no ``.fileName`` property so that
  the error ``.fileName`` becomes ``undefined``.

* For errors created/thrown using the Duktape API (``duk_push_error_object()``,
  ``duk_error()``, etc) the C call site is always blamed, so that the error's
  file/line information matches the C call site's ``__FILE__``/``__LINE__``.
  This behavior is hardcoded; user code may override the behavior by
  defining ``.fileName`` and ``.lineNumber`` on the error object.

These rules have a few shortcomings.

First, the C call site is blamed for all user-thrown errors which is often
not the best behavior.  For example::

    /* foo/bar/quux.c */

    static duk_ret_t my_argument_validator(duk_context *ctx) {
            /* ... */

            /* The duk_error() call site's __FILE__ and __LINE__ will be
             * recorded into _Tracedata and will be provided when reading
             * .fileName and .lineNumber of the error, e.g.:
             *
             *     err.fileName   --> "foo/bar/quux.c"
             *     err.lineNumber --> 1234
             *
             * If this an "infrastructure function", e.g. a validator for
             * an argument value, the file/line blamed is not very useful.
             */

            duk_error(ctx, DUK_ERR_RANGE_ERROR, "argument out of range");

            /* ... */
    }

Second, when the C call site is not blamed and the innermost activation
does not have a ``.fileName`` property (which is the default for Duktape/C
functions) the error's ``.fileName`` will be ``undefined``.  For example::

    ((o) Duktape 1.3.0 (v1.3.0)
    duk> try { [1,2,3].forEach(123); } catch (e) { err = e; }
    = TypeError: type error (rc -105)
    duk> err.fileName
    = undefined
    duk> err.lineNumber
    = 0
    duk> err.stack
    = TypeError: type error (rc -105)
            forEach  native strict preventsyield
            global input:1 preventsyield
    duk> Array.prototype.forEach.name
    = forEach
    duk> Array.prototype.forEach.fileName
    = undefined

While ``forEach()`` has a ``.name`` property, it doesn't have a ``.fileName``
that ``err.fileName`` becomes ``undefined``.  This is obviously not very
useful; it'd be more useful to blame the error on ``input``, which is the
closest call site with a filename.

Duktape 1.4.0 behavior
----------------------

Duktape 1.4.0 improves the blaming behavior slightly when the C call site
information is not blamed: instead of taking file/line information from the
innermost activation, it is taken from the closest activation which has a
``.fileName`` property.

This improves file/line blaming for the ``forEach()`` example above::

    ((o) Duktape 1.3.99 (v1.3.0-294-g386260d-dirty)
    duk> try { [1,2,3].forEach(123); } catch (e) { err = e; }
    = TypeError: function required, found 123 (stack index 0)
    duk> err.fileName
    = input
    duk> err.lineNumber
    = 1

If ``forEach()`` is assigned a filename, it will get blamed instead::

    ((o) Duktape 1.3.99 (v1.3.0-294-g386260d-dirty)
    duk> Array.prototype.forEach.fileName = 'dummyFilename.c';
    = dummyFilename.c
    duk> try { [1,2,3].forEach(123); } catch (e) { err = e; }
    = TypeError: function required, found 123 (stack index 0)
    duk> err.fileName
    = dummyFilename.c
    duk> err.lineNumber
    = 0

There's no change in behavior for errors thrown during compilation (typically
SyntaxErrors).

There's also no change for the case where the C call site is blamed, e.g. for
errors thrown explicitly using ``duk_error()``.  Because such error throws are
possible from both infrastructure code and application code, there's not yet
enough information to select the ideal file/line for such an error.

Replacing the .fileName and .lineNumber accessors
-------------------------------------------------

If the user application needs more control of file/line blaming, it's
possible to replace the inherited ``Error.prototype.fileName`` and
``Error.prototype.lineNumber`` accessors and implement whatever logic suits
the application best.  For example, the application could filter functions
based on a filename whitelist/blacklist or filename patterns.

The downside of this is that the application needs to decode ``_Tracedata``
whose format is version dependent.

Future improvements
-------------------

Control blaming of C call site
::::::::::::::::::::::::::::::

Allow C code to indicate whether the C call site of an error create/throw
should be considered relevant for file/line blaming.  This change would allow
user code to control the blaming on a per-error basis.

Duktape already does this internally by using a flag
(``DUK_ERRCODE_FLAG_NOBLAME_FILELINE``) ORed with an error code to convey the
intent.  The flag could simply be exposed in the API, but there are other API
design options too.

Control error blaming of compilation errors
:::::::::::::::::::::::::::::::::::::::::::

At the moment source text file/line is always blamed for errors thrown during
compilation (typically SyntaxErrors).

Technically there might be compilation errors inside "infrastructure code" so
that it may not always be correct to blame them.  This could be easily fixed
by adding a flag to the compilation API calls.

Control error blaming of functions
::::::::::::::::::::::::::::::::::

Allow Duktape/C and Ecmascript functions to provide a flag indicating if
the function should be considered relevant for file/line blaming.

For Duktape 1.4.0 the ``.fileName`` property of a function serves this
purpose to some extent: if a function is missing ``.fileName`` it is ignored
for file/line blaming, i.e. treated as an infrastructure function.  However,
there may be infrastructure functions which have a ``.fileName`` or
non-infrastructure functions which don't have a ``.fileName``, so being able
to control the blaming behavior explicitly would be useful.

The control flag could be implemented either as a ``duk_hobject`` flag or
an (internal or external) property.

Handling of lightfuncs
::::::::::::::::::::::

Should lightfuncs be blamed or not?  Currently they are never blamed for
file/line.

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

Overview
--------

The purpose of the ``_Tracedata`` value is to capture the relevant callstack
information very quickly before the callstack is unwound by error handling.
In many cases the traceback information is not used at all, so it should be
recorded in a compact and cheap manner.

To fulfill these requirements, the current format, described below, is a bit
arcane.  The format is version dependent, and is not intended to be accessed
directly by user code.

The ``_Tracedata`` value is a flat array, populated with values describing:
(1) a possible compilation error site, (2) a possible C call site, and (3) the
contents of the callstack, starting from the callstack top and working
downwards until either the callstack bottom or the maximum traceback depth
is reached.

The tracedata is processed only by Duktape internal functions:

* The ``Error.prototype.stack`` accessor converts tracedata into a human
  readable, printable traceback string.

* The ``Error.prototype.fileName`` and ``Error.prototype.lineNumber``
  accessors provide a file/line "blaming" for the error based on the
  tracedata.

* Currently (as of Duktape 1.4) there are no exposed helpers to decode
  tracedata in a user application.  However, user code can inspect the
  current callstack using ``Duktape.act()`` in the ``errCreate`` and
  ``errThrow`` hooks.

Example of the concrete tracedata in Duktape 1.4.0::

    ((o) Duktape 1.3.99 (v1.3.0-294-g72447fe)
    duk> try { eval('\n\nfoo='); } catch (e) { err = e; }
    = SyntaxError: parse error (line 3)
    duk> err.stack
    = SyntaxError: parse error (line 3)
            input:3
            duk_js_compiler.c:3655
            eval  native strict directeval preventsyield
            global input:1 preventsyield
    duk> Duktape.enc('jx', err[Duktape.dec('hex', 'ff') + 'Tracedata'], null, 4)
    = [
        "input",                \  compilation error site
        3,                      /
        "duk_js_compiler.c",    \  C call site
        4294970951,             /
        {_func:true},           \
        107374182400,           |  callstack entries
        {_func:true},           |
        34359738375             /
    ]

Tracedata parts
---------------

Compilation error
:::::::::::::::::

If the error is thrown during compilation (typically a SyntaxError) the
file/line in the source text is pushed to ``_Tracedata``:

* The source filename as a string.

* The offending linenumber as a number (double).

C call site
:::::::::::

If a call has a related C call site, the call site is pushed to ``_Tracedata``:

* The ``__FILE__`` value as a string.

* A number (double) containing the expression::

    (flags << 32) + (__LINE__)

  The only current flag indicates whether or not the ``__FILE__`` /
  ``__LINE__`` pair should be "blamed" as the error location when the user
  requests for a ``fileName`` or ``lineNumber`` related to the error.

Callstack entries
:::::::::::::::::

After that, for each callstack element, the array entries appended to
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
  flags allow e.g. tail calls to be noted in the traceback.

Notes
-----

* An IEEE double can hold a 53-bit integer accurately so there is space
  for plenty of flags in the current representation.  Flags must be in
  the low end of the flags field though (bit 20 or lower)

* The number of elements appended to the ``_Tracedata`` array for each
  activation does not need to constant, as long as the value can be decoded
  starting from the beginning of the array (in other words, random access is
  not important at the moment).

* The ``this`` binding, if any, is not currently recorded.

* The variable values of activation records are not recorded.  They would
  actually be available because the callstack can be inspected and register
  maps (if defined) would provide a way to map identifier names to registers.
  This is definitely future work and may be needed for better debugging
  support.

* The ``_Tracedata`` value is currently an array, but it may later be changed
  into an internal type of its own right to optimize memory usage and
  performance.  The internal type would then basically be a typed buffer
  which garbage collection would know how to visit.
