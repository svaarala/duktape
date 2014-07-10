==================================
Status of Emscripten compatibility
==================================

Hello world test
================

Quick hello world test::

  $ EMCC_FAST_COMPILER=0 ./emcc -s USE_TYPED_ARRAYS=0 \
           tests/hello_world.cpp -o /tmp/duk-emcc-test.js

Tweaks needed:

* ``-s USE_TYPED_ARRAYS=0``: needed because Duktape does not yet support
  Javascript typed arrays.  Without this the Emscripten won't be able to
  create an array for simulating memory.  Note that without typed arrays,
  Emscripten code will run very slow and be very memory inefficient.

* ``EMCC_FAST_COMPILER=0``: needed (in the env) because without this more
  recent Emscripten versions will require typed arrays:

  - https://github.com/kripken/emscripten/wiki/LLVM-Backend

Normally this suffices.  If you're running Duktape with a small amount of
memory (e.g. when running the Duktape command line tool with the ``-r``
option) you may need to reduce Emscription "virtual memory" size with the
following additional options:

* ``-s TOTAL_MEMORY=2097152``: reduce total memory size to avoid running
  out of memory.

* ``-s TOTAL_STACK=524288``: reduce total stack size to fit it into the
  reduced memory size.

There used to be an invalid RegExp expression in the Emscripten output:
a few curly braces were used as plain literals, which is technically a
SyntaxError although it is accepted by several engines.  This has now been
fixed in the Emscripten repository.  For other Emscripten issues, see
``fix_emscripten.py`` for details.
