==================================
Status of Emscripten compatibility
==================================

Hello world test
================

Quick hello world test::

  $ ./emcc --memory-init-file 0 tests/hello_world.cpp -o /tmp/test.js
  $ python $DUKTAPE/util/fix_emscripten.py < /tmp/test.js > /tmp/test-fixed.js
  $ duk /tmp/test-fixed.js

Tweaks needed:

* ``--memory-init-file 0``: don't use an external memory file.

* Some RegExps need to be fixed, see ``util/fix_emscripten.py``.

Normally this suffices.  If you're running Duktape with a small amount of
memory (e.g. when running the Duktape command line tool with the ``-r``
option) you may need to reduce Emscription "virtual memory" size with the
following additional options:

* ``-s TOTAL_MEMORY=2097152``: reduce total memory size to avoid running
  out of memory.

* ``-s TOTAL_STACK=524288``: reduce total stack size to fit it into the
  reduced memory size.

Since Duktape 1.3 there is support for Khronos/ES6 TypedArrays which allow
Emscripten to run better than with Duktape 1.2, and also allows use of
Emscripten fastcomp.
