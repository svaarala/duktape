=======
Duktape
=======

Duktape is a small and portable Ecmascript E5/E5.1 implementation.
It is intended to be easily embeddable into C programs, with a C API
similar in spirit to Lua's.

The goal is to support the full E5 feature set like Unicode strings
and regular expressions.  Other feature highlights include:

  * Custom types (like pointers and buffers) for C integration

  * Reference counting and mark-and-sweep garbage collection
    (with finalizer support)

  * Co-operative threads, a.k.a. coroutines

  * Tail call support

This is an early development version which is not intended for actual use.
Most basic things work:

  * Ecmascript compiler and bytecode executor

  * Regexp compiler and regexp bytecode executor

  * Garbage collection (reference counting and mark-and-sweep)

  * Object, array, function, identifier access etc basic semantics

However, some built-ins are missing and there are (known and unknown) bugs
here and there (for instance, number parsing is incomplete).  There is also
minimal documentation in this release.  API and internal documentation will
be included in future releases.

To build (only Linux at the moment)::

  $ scons -s -j 8

To test the command line version::

  $ build/400/duk.400
  duk> print('Hello world!');
  [bytecode length 6 opcodes, registers 4, constants 2, inner functions 0]
  Hello world!
  = undefined

To run the current test suite, install node.js and then::

  $ cd runtests/
  $ npm install   # installs dependencies
  $ cd ..  
  $ node runtests/runtests.js --run-duk --cmd-duk=build/400/duk.400 \
        --num-threads 8 --log-file=/tmp/log.txt testcases/

The source code compiles on Darwin (and maybe OSX), but you need to
change the following in src/SConscript::

  -                   LIBS=['m', 'rt', 'readline', 'ncursesw'])
  +                   LIBS=['m', 'rt', 'readline', 'ncurses'])

Have fun!

-- 
Sami Vaarala
sami.vaarala@iki.fi

