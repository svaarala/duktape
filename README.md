Duktape
=======

Introduction
------------

[Duktape](http://www.duktape.org/) is an **embeddable Javascript** engine,
with a focus on **portability** and **compact** footprint.

Duktape is easy to integrate into a C/C++ project: add `duktape.c` and
`duktape.h` to your build, and use the Duktape API to call Ecmascript
functions from C code and vice versa.

Main features:

* Embeddable, portable, compact
* Ecmascript E5/E5.1 compliant
* Built-in regular expression engine
* Built-in Unicode support
* Minimal platform dependencies
* Combined reference counting and mark-and-sweep garbage collection with finalization
* Liberal license

See [duktape.org](http://www.duktape.org/) for packaged end-user downloads
and documentation.

Have fun!

About this repository
---------------------

This repository is **intended for Duktape developers only**, and contains
Duktape internals: test cases, internal documentation, sources for the
duktape.org web site, etc.

Getting started: end user
-------------------------

When embedding Duktape in your application you should use the packaged source
distributables available from [duktape.org/download.html](http://www.duktape.org/download.html).

However, if you really want to use a bleeding edge version:

    $ git clone https://github.com/svaarala/duktape.git
    $ cd duktape
    $ make dist-src

Then use `duktape-<version>.tar.xz` like a normal source distributable.

Getting started: developing Duktape
-----------------------------------

If you intend to change Duktape internals, run test cases, etc:

    # Install required packages
    $ sudo apt-get install nodejs npm perl openjdk-7-jre

    # Compile the command line tool ('duk')
    $ git clone https://github.com/svaarala/duktape.git
    $ cd duktape
    $ make

    # Run Ecmascript and API testcases, and some other tests
    $ make ecmatest
    $ make apitest
    $ make regfuzztest
    $ make underscoretest    # see doc/underscore-status.txt
    $ make test262test       # see doc/test262-status.txt
    $ make emscriptentest    # see doc/emscripten-status.txt
    $ make jsinterpretertest
    $ make luajstest
    $ make dukwebtest        # then browse to file:///tmp/dukweb/dukweb.html
    $ make xmldoctest

**Note: the repo Makefile is intended for Linux developer use**, it is not a
multi-platform "end user" Makefile.  In particular, the Makefile is not
intended to work on e.g. OSX or Windows.  The source distributable has more
user-friendly Makefile examples, but you should normally simply write your
own Makefile when integrating Duktape into your program.

Contributing
------------

See [CONTRIBUTING.md](https://github.com/svaarala/duktape/blob/master/CONTRIBUTING.md).

Copyright and license
---------------------

See [AUTHORS.txt](https://github.com/svaarala/duktape/blob/master/AUTHORS.txt)
and [LICENSE.txt](https://github.com/svaarala/duktape/blob/master/LICENSE.txt).
