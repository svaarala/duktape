===========================
Duktape C module convention
===========================

Overview
========

This document provides a recommended convention for writing an init function
for a C module.  The convention allows a module to be initialized manually
when using static linking, or as part of loading the module from a DLL.
Modules can be initialized either as part of CommonJS module loading or
outside of it.

The convention is in no way mandatory and it's perfectly fine to use different
module loader conventions.  However, modules following this convention will be
easier to share between projects.

Module init function
====================

The init function for a module ``my_module`` should contain an initialization
function of the following form::

    duk_ret_t dukopen_my_module(duk_context *ctx) {
        /* Initialize module in whatever way is most appropriate.
         * Called as a Duktape/C function.
         *
         * Push the module result (e.g. an object with methods) on
         * top of the value stack and return 1 to indicate there's
         * a return value.  Temporary values can be left below the
         * return value like in normal Duktape/C functions.
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
loader should support mixed modules containing both C and Ecmascript code.
For example::

    my_module.so   # C module
    my_module.js   # Ecmascript module (CommonJS)

Such a combined module should be loaded as follows:

* First load the C module normally, yielding a return value RET.

* If RET is an object, use it to initialize the CommonJS ``exports`` value
  before loading the Ecmascript module.  The Ecmascript module can then
  use whatever symbols the C modules registered, and add further symbols to
  the same exports value.

* If RET is not an object, ignore it and load the Ecmascript module normally.

**FIXME: at the moment the module loader cannot replace the ``exports``
value, so it needs copy symbols from RET into ``exports`` one by one.**

Limitations
===========

* This convention may not work on all platforms where Duktape itself ports to.
  For instance, a platform might have no DLL support or have filename
  restrictions that don't allow DLLs to be named as specified above.

* The convention is not "CommonJS native": a C module doesn't get an exports
  table and cannot load sub-modules (at least relative to its own CommonJS
  identifier).  This is intentional to keep the C module convention as simple
  as possible.
