==================================
Status of Emscripten compatibility
==================================

Hello world test
================

Quick hello world test::

  $ emcc --memory-init-file 0 \
          -s WASM=0 -s POLYFILL_OLD_MATH_FUNCTIONS=1 \
          tests/hello_world.cpp -o /tmp/test.js
  $ duk /tmp/test.js

Tweaks needed:

* ``--memory-init-file 0``: don't use an external memory file.

* ``-s WASM=0``: disable WebAssembly support, as Duktape doesn't support it.

* ``-s POLYFILL_OLD_MATH_FUNCTIONS``: don't assume post-ES5.1 Math functions
  like Math.fround().

Normally this suffices.  If you're running Duktape with a small amount of
memory (e.g. when running the Duktape command line tool with the ``-r``
option) you may need to reduce Emscription "virtual memory" size with the
following additional options:

* ``-s TOTAL_MEMORY=2097152``: reduce total memory size to avoid running
  out of memory.

* ``-s TOTAL_STACK=524288``: reduce total stack size to fit it into the
  reduced memory size.

Changes in Duktape versions:

* Since Duktape 1.3 there is support for ES2015 TypedArrays which allow
  Emscripten to run better than with Duktape 1.2, and also allows use of
  Emscripten fastcomp.

* Since Duktape 1.5 no fixups are needed to run Emscripten-generated code:
  Duktape now accepts non-standard unescaped curly braces in regexps, and
  the Function ``.toString()`` output was changed to be acceptable to the
  Emscripten regexps.  Earlier a fixup script was needed::

      $ python $DUKTAPE/util/fix_emscripten.py < /tmp/test.js > /tmp/test-fixed.js

Setting up emcc for Duktape
===========================

See docker/ for Docker image files which set up a working ``emcc``.

See also
========

* https://github.com/kripken/emscripten/wiki/Optimizing-Code

* http://mozakai.blogspot.fi/2013/08/outlining-workaround-for-jits-and-big.html
