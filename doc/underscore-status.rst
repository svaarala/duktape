==============================
Status of underscore testcases
==============================

Overview
========

Underscore testcases exercise some core language features but also contain
some browser specific stuff.  There are also some asynchronous tests which
cannot be executed directly.  Underscore uses a unit test framework which
needs to be emulated, see ``underscore-test-shim.js``.

This document summarizes the currently failing testcases and why they fail.

Summary of failure reasons
==========================

* Because asyncTest() is unimplemented, all async tests are skipped now
  (such tests are used in the Functions module).

* Several template testcases fail (see below), not diagnosed yet.  The same
  errors occur with Rhino, so the culprit is probably the shim or the test
  cases themselves.

* Interpolate bug, see below.

Individual errors
=================

_.template provides the generated function source, when a SyntaxError occurs
----------------------------------------------------------------------------

In module utility::

  *** _.template provides the generated function source, when a SyntaxError occurs
  FAILURE undefined

_.template handles \u2028 & \u2029
----------------------------------

In module utility::

  *** _.template handles \u2028 & \u2029
  FAILURE undefined

result calls functions and returns primitives
---------------------------------------------

In module utility::

  *** result calls functions and returns primitives
  *** _.templateSettings.variable
  FAILURE undefined
  FAILURE undefined

#547 - _.templateSettings is unchanged by custom settings.
----------------------------------------------------------

In module utility::

  *** #547 - _.templateSettings is unchanged by custom settings.
  FAILURE undefined
  FAILURE undefined

#556 - undefined template variables.
------------------------------------

In module utility::

  *** #556 - undefined template variables.
  FAILURE undefined
  FAILURE undefined
  FAILURE undefined
  FAILURE undefined
  FAILURE undefined
  FAILURE undefined
  FAILURE undefined
  FAILURE undefined

interpolate evaluates code only once.
-------------------------------------

In module utility::

  *** interpolate evaluates code only once.
  TEST CASE FAILED: assert count mismatch (0 vs 2)
