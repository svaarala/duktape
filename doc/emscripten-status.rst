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

Setting up fastcomp for Duktape
===============================

To build dukweb.js and to use Makefile targets like ``emscriptentest`` you
need Emscripten "fastcomp".  Example steps to setup emscripten:

* Compile fastcomp manually:

  - https://kripken.github.io/emscripten-site/docs/building_from_source/building_fastcomp_manually_from_source.html

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
