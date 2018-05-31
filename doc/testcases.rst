=========
Testcases
=========

There are two main testcase sets for Duktape:

* ECMAScript testcases (``tests/ecmascript``) for testing ECMAScript
  compliance, "real world" behavior, and Duktape specific behavior.

* API testcases (``tests/api``) for testing the Duktape specific C API.

Testcases are written using an "expected string" approach: a testcase file
describes the expected output using a custom markup (described below) and also
contains the ECMAScript or C code that is intended to produce that output.
A test runner compares actual output to expected; known issue files are used
to document "known bad" outputs.

This document describes the testcase formats and current test tools.

ECMAScript testcase format
==========================

Testcases are plain ECMAScript (``.js``) files with custom markup inside
comments for providing test metadata, expected output, and include files.
A testcase is "prepared" before execution using ``util/runtest.py``:

* Testcase metadata and expected string are parsed from inside custom markup.

* A minified prologue is injected to provide a global ``Test`` object.
  The prologue also harmonizes the execution environment so that e.g.
  ``print()`` and ``console.log()`` are available so that the prepared
  test can be executed using Duktape, V8, etc.

* Include files are located, minified, and included into the prepared test.
  All utilities included must work in both strict and non-strict contexts
  because testcases may be either strict or non-strict programs.

* A ``"use strict";`` declaration is prepended (even before the prologue)
  if test metadata indicates it is needed.  This is needed when the testcase
  is exercising strict program code behavior.

The prologue and include files are minified to one-liners so that they don't
offset the line numbers of the testcase.  This is important for tests that
exercise traceback line numbers for example.

Include files are specified using the following syntax::

  /*@include util-buffer.js@*/

Testcase metadata is provided as JSON or YAML inside a comment block.  If
multiple blocks are present they are merged, with the last occurrence of a
key overwriting previous occurrences::

  /*---
  {
      "custom": true
  }
  ---*/

  // or

  /*---
  custom: true
  ---*/

The metadata keys change over time; current keys are described below.
Metadata is optional.

Finally, the expected output is specified using the following syntax::

  /*===
  hello world
  ===*/

  print('hello world');

There's also a single-line shorthand::

  print('hello world');  //>hello world

Full testcase example::

  /*
   *  Example test.
   */

  /*@include util-foo.js@*/

  /*---
  # Optional metadata is encoded in JSON or YAML.
  slow: false
  ---*/

  /*===
  hello world
  ===*/

  if (1) {
      print("hello world");   /* automatic newline */
  } else {
      print("not quite");
  }

  /*===
  second test
  ===*/

  /* there can be multiple "expected" blocks (but only one metadata block) */
  print("second test");

  /* Shorthand can also be used. */
  print("shorthand");  //>shorthand

ECMAScript testcase metadata keys
=================================

Metadata keys are added and removed as necessary so this list may be
out-of-date; see ``util/runtest.py`` for current keys.  All keys are
optional:

+----------------------+------------------------------------------------------+
| Key                  | Description                                          |
+======================+======================================================+
| comment              | Optional string to comment on the testcase briefly.  |
+----------------------+------------------------------------------------------+
| slow                 | If true, test is (very) slow and increased time      |
|                      | limits may be necessary to avoid test timeouts.      |
+----------------------+------------------------------------------------------+
| skip                 | If true, test is skipped without causing a test      |
|                      | failure.  Useful for unfinished tests and tests      |
|                      | that need to be executed manually.                   |
+----------------------+------------------------------------------------------+
| custom               | If true, some implementation dependent behavior      |
|                      | is expected and comparison to other ECMAScript       |
|                      | engines is not relevant.  The behavior may either    |
|                      | be entirely Duktape specific (e.g. relying on JX     |
|                      | format) or specific behavior not required by the     |
|                      | ECMAScript specification (e.g. additional enumeration|
|                      | guarantees).                                         |
+----------------------+------------------------------------------------------+
| nonstandard          | If true, expected behavior is not standards          |
|                      | compliant but matches "real world" expectations.     |
+----------------------+------------------------------------------------------+
| endianness           | If set, indicates that the testcase requires a       |
|                      | specific endianness, needed for e.g. some TypedArray |
|                      | testcases.  Values: ``little``, ``big``, ``mixed``.  |
+----------------------+------------------------------------------------------+
| use_strict           | Testcase is a strict mode program.  When preparing   |
|                      | the test, prepend a ``"use strict";`` declaration as |
|                      | very first statement of the test, before the test    |
|                      | prologue.                                            |
+----------------------+------------------------------------------------------+
| intended_uncaught    | Testcase intentionally fails by throwing an uncaught |
|                      | error (which may even be a SyntaxError).  This is    |
|                      | needed to test some program level behavior.          |
+----------------------+------------------------------------------------------+

ECMAScript testcase known issues
================================

Sometimes testcases fail due to known bugs or environment specific differences
such as endianness.  Known issue files describe the "known bad" testcase
output and describes the reason for the failure.  This allows a failing test
to be flagged as a "known issue" rather than a failure.

Known issue files have a YAML metadata block, followed by ``---``, followed by
the "known bad" verbatim testcase output::

  summary: wurld is printed instead of world
  ---
  hello wurld

The "known bad" output can also be provided as an MD5 hash which is useful if
the full output is very large and uninteresting::

  summary: wurld is printed instead of world
  md5: 49a9895803ec23a6b41dd346c32203b7

Each known issue file describes a single known failure for a specific testcase.
A certain testcase may have several known issue files, for different Duktape
versions, different config options, different environments, etc.  The current
naming convention is just a numbered sequence based on the testcase name::

  # For test-dev-hello-world.js:
  test-dev-hello-world-1.txt
  test-dev-hello-world-2.txt
  test-dev-hello-world-3.txt
  ...

ECMAScript testcase best practices
==================================

Indentation
-----------

Indent with 4 spaces, no tabs.

Verifying exception type
------------------------

Since ECMAScript doesn't require specific error messages for errors
thrown, the messages should not be inspected or printed out in test
cases.  ECMAScript does require specific error types though (such as
``TypeError``.  These can be verified by printing the ``name``
property of an error object.

For instance::

  try {
      null.foo = 1;
  } catch (e) {
      print(e.name);
  }

prints::

  TypeError

When an error is not supposed to occur in a successful test run, the
exception message can (and should) be printed, as it makes it easier
to resolve a failing testcase.  This can be done most easily as::

  try {
      null.foo = 1;
  } catch (e) {
      print(e.stack || e);
  }

This is portable and prints a stack trace when available.

Printing tracebacks, pointers, etc
----------------------------------

While it should be generally avoided, in some testcases it's necessary to
print out tracebacks, JX-serialize pointers, etc.  When doing so:

* Replace filenames and line numbers in tracebacks with e.g. ``FILE:LINE``.
  Otherwise the test output will include temporary file names and it won't
  be possible to describe a stable expected output.

* Replace pointers with e.g. ``PTR``.  Pointer format is platform dependent
  and can include ``0x12345678``, ``0x123456789abcdef``, and ``12345678``.

There are utility includes to perform these replacements.

API testcase format
===================

Testcase files are C files with a ``test()`` function.  The test function
gets as its argument an already initialized ``duk_context *`` and print out
text to ``stdout``.  The testcase can assume ``duktape.h`` and common headers
like ``stdio.h`` have been included.  There are also some predefined macros
(like ``TEST_SAFE_CALL()`` and ``TEST_PCALL()``) to minimize duplication in
testcase code.

Expected output and metadata is defined as for ECMAScript testcases.  However,
the expected output shorthand syntax (``//>output``) cannot be used because
it's not portable C89.

Example::

  /*===
  Hello world from ECMAScript!
  Hello world from C!
  ===*/

  void test(duk_context *ctx) {
      duk_push_string("print('Hello world from ECMAScript!');");
      duk_eval(ctx);
      printf("Hello world from C!\n");
  }

API testcase known issues
=========================

As for ECMAScript testcases, known issues are documented using known issue
files providing the "known bad" output.  The format is the same as for
ECMAScript tests.

Test tools
==========

* ``util/runtest.py``: prepares and executes a single testcase, and prints
  out a readable result summary.  Optionally writes JSON test result file,
  prepared testcase, and various other outputs to specified files.  The tool
  can also be used to just prepare a test.  The runtest.py tool can be used
  both manually and as part of running a test suite.

* ``util/prep_test.py``: earlier version of ``runtest.py``, used by
  runtests.js and likely be to deprecated.

* ``runtests/runtests.js``: original Node.js based test runner which is
  likely to be rewritten as a Python program.

* ``testrunner/``: distributed test runner jobs for GitHub commit/pull webhook
  tests.  The testrunner client/server code is in its own repo
  https://github.com/svaarala/duktape-testrunner.

Future work
===========

* Put testcases in a directory hierarchy instead (``test/stmt/trycatch.js``),
  perhaps scales better (at the expense of adding hassle to e.g. grepping).
