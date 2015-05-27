==========
Test cases
==========

Introduction
============

There are two separate test case sets for Duktape:

1. Ecmascript test cases for testing Ecmascript compliance

2. Duktape API test cases for testing that the exposed user API works

Ecmascript test cases
=====================

How to test?
------------

There are many unit testing frameworks for Ecmascript such as `Jasmine`_
(see also `List of unit testing frameworks`_).  However, when testing an
Ecmascript *implementation*, a testing framework cannot always assume
that even simple language features like functions or exceptions work
correctly.

How to do automatic testing then?

.. _Jasmine: http://pivotal.github.com/jasmine/
.. _List of unit testing frameworks: http://en.wikipedia.org/wiki/List_of_unit_testing_frameworks#JavaScript

The current solution is to run an Ecmascript test case file with a command
line interpreter and compare the resulting ``stdout`` text to expected.
Control information, including expected ``stdout`` results, are embedded
into Ecmascript comments which the test runner parses.

The intent of the test cases is to test various features of the implementation
against the specification *and real world behavior*.  Thus, the tests are
*not* intended to be strict conformance tests: implementation specific
features and internal behavior are also covered by tests.  However, whenever
possible, test output can be compared to output of other Ecmascript engines,
currently: Rhino, NodeJS (V8), and Smjs.

Test case scripts write their output using the ``print()`` function.  If
``print()`` is not available for a particular interpretation (as is the case
with NodeJS), a prologue defining it is injected.

Test case format
----------------

Test cases are plain Ecmascript files ending with the extension ``.js`` with
special markup inside comments.

Example::

  /*
   *  Example test.
   *
   *  Expected result is delimited as follows; the expected response
   *  here is "hello world\n".
   */

  /*@include util-foo.js@*/

  /*---
  {
     "slow": false,
     "_comment": "optional metadata is encoded as a single JSON object"
  }
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

The ``/*@include ... @*/`` line is replaced with another Ecmascript file which
is automatically minified to a one-liner.  This ensures that utilities can be
included without changing line numbers in the testcase, which is important for
tests checking for linenumbers in stack traces etc.  The utilities may be used
both in strict and non-strict programs so they must be compatible with both
(it's probably easiest to make helpers 'use strict' so they'll be strict in
both cases).

The metadata block and all metadata keys are optional.  Boolean flags
default to false if metadata block or the key is not present.  Current
metadata keys:

* ``slow``: if true, test is slow and increased timelimits are applied
  to avoid incorrect timeout errors.

* ``skip``: if true, test is not finished yet, and a failure is not
  counted towards failcount.

* ``custom``: if true, some implementation dependent features are tested,
  and comparison to other Ecmascript engines is not relevant.

* ``nonstandard``: if true, expected behavior is not standards conformant
  but matches real world expectations (different from a purely Duktape
  specific features).

* ``endianness``: if set, indicates that testcase requires a specific
  endianness.  Needed for e.g. some TypedArray testcases.  Values:
  "little", "big", "mixed".

* ``comment``: optional string to comment on testcase very shortly.

Testcases used to be executable directly prior to Duktape 1.1.  With the
include mechanism testcases need to be "prepared" before execution, but it
can be done with a simple utility and the resulting prepped testcases are
independent of the rest of the testing framework.

Known issues
------------

There are several testcases that won't pass at a given time but with the
bugs considered known issues (perhaps even permanently).  Earlier these
were part of the testcase metadata, but since the known issues status is
not really testcase related, it is now stored in a separate testcase status
file, ``doc/testcase-known-issues.yaml``.

The YAML file is a list of items with the keys:

* ``test``: testcase name, e.g. ``test-dev-mandel2-func.js``.

* ``specialoptions``: if true, expected behavior requires special feature
  options to be given and the test case is expected to fail without them.
  If set to a string, the string also indicates that special feature options
  are needed, and can explain the options.

* ``knownissue``: if true, the test is categorized as failure but is shown
  as a known issue which does not need immediate fixing (i.e. they are not
  release blockers).  Bugs marked as known issues may be difficult to fix,
  may be low priority, or it may uncertain what the desired behavior is.
  If set to a string, the string can summarize the issue and is shown in
  test case output.

* ``comment``: optional string to comment on testcase status, failure
  reason, etc.

Practices
---------

Indentation
:::::::::::

Indent with space, 4 spaces.

Verifying exception type
::::::::::::::::::::::::

Since Ecmascript doesn't require specific error messages for errors
thrown, the messages should not be inspected or printed out in test
cases.  Ecmascript does require specific error types though (such as
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
to resolve a failing test case.  This can be done most easily as::

  try {
      null.foo = 1;
  } catch (e) {
      print(e.stack || e);
  }

This is portable and prints a stack trace when available.

Test cases
----------

Test cases filenames consist of lowercase words delimited by dashes, e.g.::

  test-stmt-trycatch.js

The first part of each test case is ``test``.  The second part indicates a
major test category.  The test categories are not very strictly defined, and
there is currently no tracking of specification coverage.

For example, the following prefix are used:

* ``test-dev-``: development time test cases which demonstrate a particular
  issue and may not be very well documented.

* ``test-bug-``: illustrate a particular development time bug which has usually
  already been fixed.

* ``test-bi-xxx-``: builtin tests for "xxx", e.g. ``test-bi-string-`` prefix
  is for String built-in tests.

Duktape API test cases
======================

Test case format
----------------

Test case files are C files with a ``test()`` function.  The test function
gets as its argument an already initialized ``duk_context *`` and print out
text to ``stdout``.  The test case can assume ``duktape.h`` and common headers
like ``stdio.h`` have been included.  There are also some predefined macros
(like ``TEST_SAFE_CALL()`` and ``TEST_PCALL()``) to minimize duplication in
test case code.

Expected output and metadata is defined as for Ecmascript test cases.

Example::

  /*===
  Hello world from Ecmascript!
  Hello world from C!
  ===*/

  void test(duk_context *ctx) {
      duk_push_string("print('Hello world from Ecmascript!');");
      duk_eval(ctx);
      printf("Hello world from C!\n");
  }

Known issues
------------

As for Ecmascript testcases, known issue status is stored in
``doc/testcase-known-issues.yaml``.

Test runner
===========

The current test runner is a NodeJS program which handles both Ecmascript
and API testcases.  See ``runtests/runtests.js``.

Remote tests can be executed with a shell script wrapper which copies the
test case over with scp and then executes it with ssh.  For instance::

  #!/bin/sh

  TESTCASE=""
  while [ "$1" ]
  do
      # Parse whatever options may be present, assume last argument is
      # testcase.
      case "$1" in
          --restrict-memory)
              shift
              ;;
          *)
              TESTCASE=$1
              shift
              ;;
      esac
  done

  scp "$TESTCASE" user@192.168.100.20:/tmp >/dev/null
  ssh user@192.168.100.20 "./duk /tmp/`basename $TESTCASE`"

Exit code should be passed through if possible.  Be careful about normalizing
newlines (plain LF) if something in the process uses CR LF.

Future work
===========

* Put test cases in a directory hierarchy instead (``test/stmt/trycatch.js``),
  perhaps scales better (at the expense of adding hassle to e.g. grepping).
