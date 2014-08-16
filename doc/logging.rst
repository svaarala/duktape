=========================
Duktape logging framework
=========================

Introduction
============

Duktape contains a built-in very minimal logging framework which has a
small footprint (around 1kB), reasonably high performance, and makes it
easy to change both the frontend and the backend of logging.  It is easy
to write log entries from both C and Ecmascript, and then redirect all
the log output to a custom backend.  Lazy formatting is also possible.

The framework focuses on how logger objects are created and what the
logger interface looks like.  Other features are quite barebones; for
example the default backend simply writes to the stderr and there is no
advanced backend configuration like multiple log outputs.  The user can
easily replace the frontend and the backend functions to extend the
basic feature set in a transparent manner.

The logging framework also provides API calls to write log entries from C
code.  This is very convenient, as these log entries will then nicely
interleave with log entries written from Ecmascript code.

The logger object API is close to log4javascript except that there is no
special handling for an error object as the last call argument.  See:

* http://log4javascript.org/docs/quickstart.html

Writing log entries from Ecmascript
===================================

A logger object is first created::

  // Without arguments, the logger defaults to the fileName of the call site
  var logger = new Duktape.Logger();

  // Explicitly name
  var logger = new Duktape.Logger('myLogger');

  // Explicitly unnamed (any non-string argument, null probably best)
  var logger = new Duktape.Logger(undefined);
  var logger = new Duktape.Logger(null);

After a logger has been created, log entries are written with the exposed
log writing calls inherited from the logger prototype.  There are six
log levels, each with its own frontend function.  Each log level also has
a number (0 to 5) which is used e.g. to control log level, and a 3-letter
abbreviation (like INF) used in the log prefix.  Some example log calls::

  logger.trace('loop iteration:', i, 'out of', n);   // level 0 (TRC)
  logger.debug('objects:', obj1, obj2, obj3);        // level 1 (DBG)
  logger.info('normal log message');                 // level 2 (INF)
  logger.warn('exceptional condition');              // level 3 (WRN)
  logger.error('something went wrong');              // level 4 (ERR)
  logger.fatal('something went really wrong');       // level 5 (FTL)

The logger functions make a logging decision based on log level.  If the
entry gets logged, call arguments are formatted into strings, concatenated
with spaces, and a prefix is added.  The prefix contains a timestamp, the
log level, and the logger name.  Example::

  duk> logger.info('test', 123)
  2014-03-19T02:42:20.425Z INF myLogger: test 123

Each argument is formatted separately, and if an error is thrown during
formatting, the argument is replaced with string coercion of the error.
Non-object values are formatted in pure C by the default logger functions
to minimize unnecessary calls.  Object values are formatted by calling
``fmt()`` method of the logger, usually inherited from
``Duktape.Logger.prototype.fmt()``.

The default object formatting function calls the ``toLogString()`` function
of the object if it has one, else it simply coerces with ``String(val)``.
The ``toLogString()`` function is a Duktape custom function which allows the
user to control how an object should appear in logs by default.

Although arguments whose formatting fails get replaced by an error, the
logger API does **not** guarantee that no errors can be thrown.  For example,
if formatting fails and also string coercing the formatting error fails,
the latter error will propagate out of the logger.  As always, internal errors
like out-of-memory or out-of-stack can occur at any time.

The final log message is passed as a buffer to the logging backend, provided
by ``Duktape.Logger.prototype.raw()``.  The default implementation writes the
log message to ``stderr`` and appends a newline.  The log message is provided
as a buffer (instead of a string) to avoid interning the message unnecessarily.
For short messages, a single dynamic buffer is reused over and over by modifying
the visible size of the buffer (the dynamic buffer is not reallocated), to
eliminate memory churn.

Logger related properties like the logger name (``p``), log level (``l``),
and the ``fmt()`` and ``raw()`` functions are all looked up through the
logger instance so that they can overridden either in the prototype, or on
a per-logger basis.

Writing log entries from C
==========================

The Duktape API provides a snprintf-like log call which allows C code to log
to the same backend as Ecmascript code::

  duk_log(ctx, DUK_LOG_INFO, "return value: %d", rc);

Note that if you just want to format a plain string, you *must* use a ``%s``
format to avoid security holes (your string might contain a ``%`` which would
cause uncontrolled memory accesses)::

  const char *my_plain_string = "beware of the %s";
  duk_log(ctx, DUK_LOG_WARN, "%s", my_plain_string);

Log writes from C code use a representative logger instance stored in
``Duktape.Logger.clog``.  You can manipulate or replace this logger to
control C log writes more explicitly.

Logger objects
==============

Each logger is an object with a few possible properties:

* ``n``: name of the logger, added to log lines.  If not given, defaults
  to the ``fileName`` of the function where to logger is created.  If even
  that is missing, the value will be missing from the object, and a default
  value is inherited.

* ``l``: minimum log level of the logger.  Log writes below this level
  are dropped.  If missing, a default value is inherited.

Typically, if log levels are not altered, a logger object only contains the
``n`` property.  Loggers are compacted at creation to ensure a minimal
footprint (they very rarely change state).

Each logger object has as its internal prototype ``Duktape.Logger.prototype``.
The prototype provides:

* ``n``: a default name ('anon')

* ``l``: a default log level (2, info level)

* log writing methods for each level

Lazy formatting
===============

Lazy formatting is useful when formatting the log arguments is costly and
the log line is normally filtered by the log level.  This is often the case
when debug logging complex values like deep serializations of internal state
objects.

Lazy formatting is easily achievable by using the ``toLogString()`` method.
The simplest but not very efficient approach is::

  function lazyJx1(obj) {
    return { toLogString: function() { return Duktape.enc('jx', obj); } };
  }

  logger.debug('complex object:', lazyJx1(obj));

One can use ``bind()`` for the same effect (in this particular case)::

  function lazyJx2(obj) {
    return { toLogString: Duktape.bind(null, 'jx', obj) };
  }

  logger.debug('complex object:', lazyJx2(obj));

Creating a function instance per lazy-logged value is quite expensive.
Because the ``toLogString()`` is called as a method, lazy values can
inherit from a prototype which is reasonably efficient::

  function LazyValue(val) {
    this.v = val;
  }
  LazyValue.prototype.toLogString = function () {
    return Duktape.enc('jx', this.v);
  }
  function lazyJx3(val) {
    // Per lazy value creation, only creates an object with one property.
    return new LazyValue(val);
  }

  logger.debug('complex object:', lazyJx3(obj));

Lazy formatting can also be done inline, though not very readably::

  logger.debug('data:', { toLogString: function() { return Duktape.enc('jx', data); } });

Customizing logging
===================

Some options:

* Add a ``toLogString()`` method to the prototype of interesting objects
  to control how they are serialized into strings by the default formatter
  ``Duktape.Logger.prototype.fmt()``.  For instance, you can add the method
  to ``Object.prototype`` to provide better logging for all object values.

* Replace ``Duktape.Logger.prototype.fmt()`` for custom formatting of
  object values.

* Replace ``Duktape.Logger.prototype.raw()`` for redirecting formatted
  log lines to an alternate destination.  Be careful to avoid unnecessary
  memory and string table churn.

* Replace the frontend functions (``Duktape.Logger.prototype.info()``
  etc) for custom formatting of log lines.  You may also choose not to
  call ``Duktape.Logger.prototype.raw()`` for emitting the formatted
  log line, but rather interface with your custom backend directly.

* Replace the entire ``Duktape.Logger`` constructor and prototype object
  for full control over logging.

* Of course, you can also use an external logging framework.

Limitations
===========

The built-in logging mechanism has several limitations.  Most of them are
intentional to keep the logger footprint small:

* Currently a new logger is created regardless of whether or not a previous
  logger exists with the same name.  Sometimes it might desirable to return
  the same logger instance in this case, so that e.g. the log level can be
  controlled by finding a logger and operating on it.  You can implement this
  by overriding the constructor.

* There is no way to modify the built-in line format except by overriding
  the frontend functions (``Logger.prototype.info()`` etc).  This is
  intentional, as having a fixed format makes it easier to log faster and
  reduce memory churn caused by logging.

* There is no concept of a logging context for C code.  Instead, all log
  writes go through a single logger instance.  If multiple global objects
  exist in the Duktape heap, each global context (or more specifically
  ``Duktape.Logger`` instance) will have its own logger object.  Logging
  from C is usually less of a priority so the logging C api is kept very
  minimal on purpose.

Existing frameworks and related links
=====================================

* http://ajaxpatterns.org/Javascript_Logging_Frameworks

* http://getfirebug.com/logging

* http://log4javascript.org/docs/quickstart.html

* http://log4js.berlios.de/

* http://benalman.com/projects/javascript-debug-console-log/

Future work
===========

Format all value types in a useful manner by default
----------------------------------------------------

Like JX, the logger should write useful log entries for all available value
types by default.  Currently this is not the case for e.g. buffer values.

Reduce memory churn
-------------------

Memory churn can be reduced considerably by string coercing all primitive
types (or at least undefined, null, boolean, integer numbers) without going
through string interning.

Better multiline support
------------------------

Perhaps duplicate the prefix but perhaps change the final colon to indicate
continuation, e.g.::

  <timestamp> INF myLogger: multi
  <timestamp> INF myLogger| line

Or perhaps::

  <timestamp> INF myLogger: multi
                          | line

ASCII sanitization
------------------

It would be nice if logger output would be guaranteed to be printable ASCII
only.  This needs handling either in the frontend functions (e.g. for strings)
or the final writer function.

Buffer formatting
-----------------

Buffer data should maybe be formatted in hex encoded form (like JX does).
Since buffers are plain objects, they don't currently go through the formatter,
but that would be easy to change.

__FILE__ and __LINE__ for C log writes
--------------------------------------

Include __FILE__ and __LINE__ automatically in C log writes somehow?
