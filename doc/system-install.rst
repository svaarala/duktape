======================================
Installing Duktape as a system library
======================================

Duktape can be installed as a system library, e.g. as part of a Unix
distribution, as follows:

* Run ``tools/configure.py`` to create prepared Duktape header and source
  files with options suitable for the target system.  As a general rule
  you should use the defaults unless they conflict with the target system.

  - On some systems you may need to override byte order, alignment requirements,
    Date built-in provider, etc.

  - When compiling as a Windows DLL use the ``--dll`` option which ensures
    symbol visibility declspec attributes are used correctly.

  - See http://wiki.duktape.org/Configuring.html for more details and examples.

* Build a system specific installation package with:

  - ``duktape.c`` compiled into a static/dynamic library, installed into
    system library paths.

  - ``duktape.h`` and ``duk_config.h``, installed into system include paths.

* A user application then simply ensures that:

  - The system include path containing ``duktape.h`` and ``duk_config.h``
    is in the include path.

  - The system library path containing the Duktape static/dynamic library
    is in the library path.

  - Link with the the static/dynamic Duktape library.

  - Ensure that the standard math library is linked (``-lm``).

There a few limitations in this approach:

* A user application cannot use custom config options because the active
  options are encapsulated in ``duk_config.h`` and affect the compilation
  of the shared Duktape library.  This issue exists with all libraries with
  custom features affecting library compilation but may be emphasized for
  Duktape because there are a lot of config options.

* Technically the Duktape library and the user application should be compiled
  with the same compiler.  When using different compilers, basic types or
  struct alignment rules (among other things) may differ.  In practice it's
  unlikely you'll run into problems, at least when using mainline compilers
  like gcc and clang.
