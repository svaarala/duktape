==================================
Status of Emscripten compatibility
==================================

Hello world test
================

Quick hello world test::

  $ ./emcc --memory-init-file 0 \
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

Setting up fastcomp for Duktape
===============================

To build dukweb.js and to use Makefile targets like ``emscriptentest`` you
need Emscripten "fastcomp".  Example steps to setup emscripten:

* Compile fastcomp manually:

  - https://kripken.github.io/emscripten-site/docs/building_from_source/building_fastcomp_manually_from_source.html

  - The ``LLVM_ROOT`` path in the documentation seems to be outdated,
    ``.../build/bin`` should apparently be ``.../build/Release/bin``

* Checkout emscripten::

      $ cd (duktape)
      $ make emscripten  # duktape checkouts emscripten master

* Create a ``~/.emscripten`` file which uses your manually compiled fastcomp.
  You can create a default configuration as follows::

      $ cd (duktape)/emscripten
      $ ./emcc  # creates ~/.emscripten if it doesn't exist

  Then change Emscripten to use the manually compiled fastcomp by changing
  the LLVM_ROOT line in ``~/.emscripten`` to point to your fastcomp build::

      LLVM_ROOT = '/home/user/myfastcomp/emscripten-fastcomp/build/Release/bin'

You should now be able to build dukweb.js and run Emscripten Makefile
targets::

    $ make emscripteninceptiontest
    [...]
    emscripten/emcc -O2 -std=c99 -Wall --memory-init-file 0 -Idist/src dist/src/duktape.c dist/examples/hello/hello.c -o /tmp/duk-emcc-test.js
    [...]
    ./duk /tmp/duk-emcc-test-fixed.js
    Hello world!
    2+3=5
