=======
Duktape
=======

Duktape is a small and portable Ecmascript E5/E5.1 implementation.  It is
intended to be easily embeddable into C programs, with a C API similar in
spirit to Lua's.

Duktape supports the full E5/E5.1 feature set including errors, Unicode
strings, and regular expressions, a subset of E6 features (e.g. Proxy
objects), Khronos/ES6 ArrayBuffer/TypedView, and Node.js Buffer bindings.

Duktape also provides a number of custom features such as error tracebacks,
additional data types for better C integration, combined reference counting
and mark-and sweep garbage collector, object finalizers, co-operative
threads a.k.a. coroutines, tail calls, built-in logging and module frameworks,
a built-in debugger protocol, function bytecode dump/load, and so on.

You can browse Duktape programmer's API and other documentation at:

* http://duktape.org/

In particular, you should read the getting started section:

* http://duktape.org/guide.html#gettingstarted

More examples and how-to articles are in the Duktape Wiki:

* http://wiki.duktape.org/

Building and integrating Duktape into your project is very straightforward:

* http://duktape.org/guide.html#compiling

See Makefile.hello for a concrete example::

  $ cd <dist_root>
  $ make -f Makefile.hello
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

  $ ./duk mandel.js
  [...]

This distributable contains:

* ``src/``: main Duktape library in a "single source file" format (duktape.c,
  duktape.h, and duk_config.h).

* ``src-noline/``: contains a variant of ``src/duktape.c`` with no ``#line``
  directives which is preferable for some users.  See discussion in
  https://github.com/svaarala/duktape/pull/363.

* ``src-separate/``: main Duktape library in multiple files format.

* ``config/``: genconfig utility for creating duk_config.h configuration
  files, see: http://wiki.duktape.org/Configuring.html.

* ``examples/``: further examples for using Duktape.  Although Duktape
  itself is widely portable, some of the examples are Linux only.
  For instance the ``eventloop`` example illustrates how ``setTimeout()``
  and other standard timer functions could be implemented on Unix/Linux.

* ``extras/``: utilities and modules which don't comfortably fit into the
  main Duktape library because of footprint or portability concerns.
  Extras are maintained and bug fixed code, but don't have the same version
  guarantees as the main Duktape library.

* ``polyfills/``: a few replacement suggestions for non-standard Javascript
  functions provided by other implementations.

* ``debugger/``: a debugger with a web UI, see ``debugger/README.rst`` and
  https://github.com/svaarala/duktape/blob/master/doc/debugger.rst for
  details on Duktape debugger support.

* ``licenses/``: licensing information.

You can find release notes at:

* https://github.com/svaarala/duktape/blob/master/RELEASES.rst

This distributable contains Duktape version @DUK_VERSION_FORMATTED@, created from git
commit @GIT_COMMIT@ (@GIT_DESCRIBE@).

Duktape is copyrighted by its authors (see ``AUTHORS.rst``) and licensed
under the MIT license (see ``LICENSE.txt``).  String hashing algorithms are
based on the algorithm from Lua (MIT license), djb2 hash, and Murmurhash2
(MIT license).  Duktape module loader is based on the CommonJS module
loading specification (without sharing any code), CommonJS is under the
MIT license.

Have fun!

Sami Vaarala (sami.vaarala@iki.fi)
