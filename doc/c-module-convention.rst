===========================
Duktape C module convention
===========================

Overview
========

This document provides a recommended convention for writing an init function
for a C module.  The convention allows modules to be used with both static
linking and DLL loading, and either as part of Duktape's CommonJS module
loading or outside of it.

The convention is in no way mandatory and it's perfectly fine to use a
different module loader convention for your project.  However, modules
following this convention will be easier to share between projects.

Module init function
====================

The init function for a module ``my_module`` should have the following form::

    duk_ret_t dukopen_my_module(duk_context *ctx) {
        /* Initialize module in whatever way is most appropriate.
         * Called as a Duktape/C function.
         *
         * Push the module result (e.g. an object with exported symbols or
         * a function) on top of the value stack and return 1 to indicate
         * there's a return value.  Temporary values can be left below
         * the return value like in normal Duktape/C functions.
         */

        duk_push_object(ctx);  /* module result */

        duk_put_function_list(ctx, -1, my_module_funcs);

        duk_push_int(ctx, 42);
        duk_put_prop_string(ctx, -2, "meaningOfLife");

        return 1;  /* return module value */
    }

The init function is called as a Duktape/C function.  When initializing
the module manually, you should use::

    duk_push_c_function(ctx, dukopen_my_module, 0 /*nargs*/);
    duk_call(ctx, 0);  /* or duk_pcall() if you want to catch errors */

    /* Stack top contains module value */

A DLL loader should use the same convention to call the init function
after figuring out the init function name and locating it from the DLL
symbol table.

DLL name
========

When a C module is compiled into a DLL, the DLL filename should include
the module name (``my_module`` in the running example) with any platform
specific prefix and suffix.  For example::

    my_module.so   # Linux
    my_module.dll  # Windows

A DLL loader should assume that the init function name is ``dukopen_``
followed by the module name part extracted from the DLL filename (here,
``dukopen_my_module()``).

Module name
===========

To avoid case conversion and special character issues, module names should
have the form::

    [a-zA-Z_][0-9a-zA-Z_-]*

This should minimize platform issues.

Mixed Ecmascript / C modules
============================

When a module is being initialized by a CommonJS aware module loader, the
loader can support mixed modules containing both C and Ecmascript code.
For example::

    my_module.so   # C module
    my_module.js   # Ecmascript module (CommonJS)

To support mixed modules, a Duktape 1.x ``modSearch()`` function should:

* First load the C module normally, yielding a return value RET.

* If RET is an object, copy the own properties of RET into the ``exports``
  value created by Duktape.  It should then return the source code of the
  Ecmascript module; when executed, further symbols get added to the same
  ``exports`` value.

* If RET is not an object, ignore it and load the Ecmascript module normally.
  (Alternatively, write RET to a fixed export name to make it accessible,
  e.g. ``exports.value``.)

The algorithm for Duktape 2.0 is still under design, but at a high level:

* First load the C module normally, yielding a return value RET.

* If RET is an object, use it to initialize the CommonJS ``exports`` value
  before loading the Ecmascript module.  The Ecmascript module can then
  use whatever symbols the C modules registered, and add further symbols to
  the same ``exports`` value.

* If RET is not an object, ignore it and load the Ecmascript module normally.
  (Alternatively, expose RET with a fixed name, e.g. initialize ``exports``
  as ``{ value: RET }``.)

Duktape 1.x CommonJS notes
==========================

In Duktape 1.x the module ``exports`` value is always an object created by
Duktape, and cannot be replaced by modSearch().  modSearch() can only add
symbols to the pre-created object.  This has two implications for
implementing modSearch():

- When a C module returns an object, the symbols from the object must be
  copied to the pre-created ``exports`` value manually by the modSearch()
  function.

- When a C module returns a non-object, there are several alternatives:

  + The modSearch() function can ignore the module value.  This will make
    the module value inaccessible (unless the C module init function registered
    symbols directly to the global object or similar).

  + The modSearch() function can copy the module value into a fixed name in
    the ``exports`` table.  Suggested name is ``exports.value``.

These limitations will most likely be fixed in Duktape 2.0 module loading
rework.

Limitations
===========

* The convention may not work on all platforms where Duktape itself ports to.
  For instance, a platform might have no DLL support or have filename
  restrictions that don't allow DLLs to be named as specified above.

* The convention is not "CommonJS native": a C module doesn't get an exports
  table and cannot load sub-modules (at least relative to its own CommonJS
  identifier).  This trade-off is intentional to keep the C module convention
  as simple as possible.

* Duktape 1.x CommonJS module loading doesn't support modules with a non-object
  return value (i.e. all modules return an ``exports`` table).  This module
  convention is not limited to object return values so that non-object modules
  can be supported in Duktape 2.0.
