=====================
API design guidelines
=====================

Introduction
============

A consistent API design makes it easier to write applications: it should be
easy to remember API call naming, argument order, and other conventions, to
make it as easy as possible to write error-free code.

API design decisions involve several important concerns:

* Consistency: easy to remember API calls and arguments.

* Versioning: easy to extend the API, avoid dead ends in design.

* Footprint: minimize code footprint of Duktape internals and application
  call site.

* Performance: some API designs allow better performance than others.

This document provides some notes on common API design issues and trade-offs.
The document is not intended to be complete, but just cover practical issues
as they come up.

Naming
======

API calls
---------

All API calls are lowercase, underscore separated, and begin with the prefix
``duk_``.  Duktape internals, not part of the public API, also have calls
matching this convention.  The public API calls are documented in the public
API header (actually a segment of ``duktape.h`` in the distributable) and the
API documentation.

Existing convention for operating on value stack items:

* ``duk_get_XXX()`` gets a value without modifying the value on the stack.
  If the value doesn't match expected type, some default value (like NULL
  or zero) is returned.

* ``duk_require_XXX()`` gets a value without modifying the value on the stack.
  If the value doesn't match expected type, an error is thrown.

* ``duk_to_XXX()`` coerces a value in-place into desired type.

Varargs API calls:

* Named with a trailing ``_va()``, for example ``duk_error_va()``.

  - Existing exception: ``duk_push_sprintf()``.

Existing convention for safe/protected calls:

* All calls are unsafe by default, i.e. may throw errors.

* ``duk_pXXX()`` calls are protected, i.e. they catch errors.  API call
  return value indicates success or error, and error is typically passed
  via the value stack.

* ``duk_safe_XXX()`` calls are safe, i.e. don't throw, but they don't
  provide a success/error indication.

  - Existing exception: ``duk_safe_call()`` doesn't currently match this
    convention: it's a protected call (catches an error and returns a
    success/failure indicator).

Arguments
---------

Lowercase, underscore separated.  No other strict guidelines, some current
names:

* ``ctx``

* ``idx``, ``obj_index``, ``from_index``, ``to_index``, ``index1``, ``index2``

* ``arr_index``

* ``flags``, ``enum_flags``

* ``len``, ``ptr``, ``key``

Avoid the argument names:

* ``index``: conflicts with a function from strings.h.

Typing
======

Return values
-------------

* ``duk_bool_t`` for true/false return values.

* ``duk_idx_t`` for value stack indices.

* ``duk_int_t`` for general return values which need to express both signed
  and unsigned values.

* ``duk_ret_t`` for Duktape/C function return values.

Arguments
---------

* ``duk_idx_t`` for value stack indices.

* ``duk_uarridx_t`` for ECMAScript array indices.

* ``duk_size_t`` for byte lengths (strings, buffers, etc).

* ``void *`` for generic void pointers.

* ``const char *`` for string data pointers.

* ``void *`` for buffer data pointers.

Varargs API calls
-----------------

Vararg API call variants are provided when it's awkward for an application to
use non-vararg API call variants.  For example, it'd be awkward to construct a
formatted string first and then push it using ``duk_push_string()``, which is
why ``duk_push_sprintf()`` is provided.

Flags vs. variants
==================

A common issue is whether there should be an API call which provides a wide
variety of behavior controlled via flags, or a corresponding set of invididual
API calls.

Individual API calls are generally favored in the API:

* They make call sites easier in applications as there's no need to remember
  and combine flag defines.

* Individual API calls can be easily mapped to either individual internal
  implementations, or to internal helpers which take flags, whichever is most
  appropriate for the internal implementation.  In contrast convering an API
  call taking flags into individual internal calls is difficult: while it's
  possible to check the flags in a macro call site, the macro will quite easily
  end up evaluating flags multiple times which violates API macro guarantees.

* They enable best performance because no flags field is built or decoded.

* Even when improved calls are added to the API, individual calls are easy to
  map to new providers, so versioning is easy even though some API clutter is
  then easily left behind.

There are situations where fewer calls with behavioral flags are preferable:

* If the API call provides a lot of functionality, so that the set of
  individual call variants would be very large, flags may be more appropriate.

* If the API call behavior is likely to evolve new control flags over time,
  flags may be more appropriate.

For example, ``duk_def_prop()`` fits both of these criteria.  The downside of
using flags is that:

* It's not easy to decode the flags in an API macro and call invididual
  internal implementations if that would otherwise be convenient.

* Passing in flags has a small performance cost, and decoding the flags in
  the call target has a relatively larger performance cost.
