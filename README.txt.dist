=======
Duktape
=======

Duktape is a small and portable Ecmascript E5/E5.1 implementation.  It is
intended to be easily embeddable into C programs, with a C API similar in
spirit to Lua's.

Duktape supports the full E5/E5.1 feature set including errors, Unicode
strings, and regular expressions, as well as a subset of E6 features (e.g.
Proxy objects).  Duktape also provides a number of custom features such as
error tracebacks, additional data types for better C integration, combined
reference counting and mark-and sweep garbage collector, object finalizers,
co-operative threads a.k.a. coroutines, tail calls, built-in logging and
module frameworks, and so on.

You can browse Duktape programmer's API and other documentation at::

  http://duktape.org/

In particular, you should read the getting started section::

  http://duktape.org/guide.html#gettingstarted

Building and integrating Duktape into your project is very straightforward.
See Makefile.example for an example::

  $ cd <dist_root>
  $ make -f Makefile.example
  [...]
  $ ./hello
  Hello world!
  2+3=5

To build an example command line tool, use the following::

  $ cd <dist_root>
  $ make -f Makefile.cmdline
  [...]
  $ ./duk
  ((o) Duktape
  duk> print('Hello world!');
  Hello world!
  = undefined

There are further examples in the ``examples/`` directory.  Although
Duktape itself is widely portable, some of the examples are Linux only.
For instance the ``eventloop`` example illustrates how ``setTimeout()``
and other standard timer functions could be implemented on Unix/Linux.

The ``polyfills/`` directory provides a few replacement suggestions for
non-standard Javascript functions provided by other implementations.

This distributable contains Duktape version @DUK_VERSION_FORMATTED@, created from git
commit @GIT_COMMIT@ (@GIT_DESCRIBE@).

Duktape is copyrighted by its authors (see ``AUTHORS.txt``) and licensed
under the MIT license (see ``LICENSE.txt``).  MurmurHash2 is used internally;
it is also under the MIT license.

Have fun!

Sami Vaarala (sami.vaarala@iki.fi)
