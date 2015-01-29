====================
Duktape debug client
====================

Overview
========

**Highly experimental!**

Debugger web UI which connects to the Duktape command line tool or any other
target supporting the example TCP transport (``examples/debug-trans-socket``).

For detailed documentation of the debugger internals, see `debugger.rst`__.

__ https://github.com/svaarala/duktape/blob/master/doc/debugger.rst

Usage
=====

Some prerequisites:

* You'll need Node.js v0.10.x or newer.  Older Node.js versions don't support
  the required packages.

Compile Duktape command line tool with debugger support (for further options
see ``doc/feature-options.rst``):

* ``DUK_OPT_DEBUGGER_SUPPORT``

* ``DUK_OPT_INTERRUPT_COUNTER``

* ``DUK_CMDLINE_DEBUGGER_SUPPORT``

The source distributable contains a Makefile to build a "duk" command with
debugger support::

    $ cd <duktape dist directory>
    $ make -f Makefile.dukdebug

The Duktape Git repo "duk" target has debugger support enabled by default::

    $ make clean duk

Start Duktape command line tool so that it waits for a debugger connection::

    # For now we need to be in the directory containing the source files
    # executed so that the 'fileName' properties of functions will match
    # that on the debug client.

    # Using source distributable
    $ cd <duktape dist directory>
    $ ./duk --debugger mandel.js

    # Using Duktape Git repo
    $ cd <duktape checkout>/ecmascript-testcases/
    $ ../duk --debugger test-dev-mandel2-func.js

Start the web UI::

    # Must be in 'debugger' directory.

    $ cd debugger/
    $ make  # runs 'node duk_debug.js'

Once the required packages are installed, the NodeJS debug client will be
up and running.  Open the following in your browser and start debugging:

* http://localhost:9092/

The debug client automatically attaches to the debug target on startup.
If you start the debug target later, you'll need to click "Attach" in the
web UI.

Source search path
==================

The NodeJS debug client needs to be able to find source code files matching
code running on the target ("duk" command line).  **The filenames used on the
target and on the debug client must match exactly**, because e.g. breakpoints
are targeted based on the 'fileName' property of Function objects.

The search path can be set using the ``--source-dirs`` option given to
``duk_debug.js``, with the default search paths including only
``../ecmascript-testcases``.

The default search path means that if a function on the target has fileName
``foo/bar.js`` it would be loaded from (relative to the duk_debug.js working
directory, ``debugger/``)::

    ../ecmascript-testcases/foo/bar.js

Similarly, if the filesystem contained::

    ../ecmascript-testcases/baz/quux.js

the web UI dropdown would show ``baz/quux.js``.  If you selected that file
and added a breakpoint, the breakpoint fileName sent to the debug target
would be ``baz/quux.js``.

.. note:: There's much to improve in the search path.  For instance, it'd
          be nice to add a certain path to search but exclude files based
          on paths and patterns, etc.

Architecture
============

::

    +-------------------+
    | Web browser       |  [debug UI]
    +-------------------+
          |
          | http (port 9092)
          | socket.io
          v
    +-------------------+
    | duk_debug.js      |  [debug client]
    +-------------------+
          |          /\
          |          ||
          +----------||---- [example tcp transport] (port 9091)
          |          ||     (application provides concrete transport)
          |          ||
          |          ||---- [debug protocol stream]
          |          ||     (between debug client and Duktape)
          |          ||
    + - - | - - - - -|| - - +
    :     v          ||     :
    :  +-------------||-+   :  [target]
    :  | application || |   :
    :  +-------------||-+   :
    :     ^          ||     :
    :     |          ||     :   [debug API]
    :     +----------||-------- debug transport callbacks
    :     |          ||     :   (read, write, peek, read/write flush)
    :     |          ||     :   implemented by application
    :     |          \/     :
    :  +----------------+   :
    :  | Duktape        |   :
    :  +----------------+   :
    + - - - - - - - - - - - +

The debug transport is application specific:

* Duktape command line ("duk") and this debug client use an **example** TCP
  transport as a concrete example.

* It is entirely up to the application to come up with the most suitable
  transport for its environment.  Different mechanisms will be needed for
  Wi-Fi, serial, etc.

The debug protocol running inside the transport is transport independent:

* The debug protocol is documented in ``doc/debugger.rst``.

* This debug client provides further concrete examples and clarifications
  on how the protocol can be used.

Using a custom transport
========================

Quite possibly your target device cannot use the example TCP transport and
you need to implement your own transport.  You'll need to implement your
custom transport both for the target device and for the debug client.

Target device
-------------

Implement the debug transport callbacks needed by ``duk_debugger_attach()``.

See ``doc/debugger.rst`` for details and ``examples/debug-trans-socket``
for example running code for a TCP transport.

Debug client alternative 1: TCP proxy
-------------------------------------

If you don't want to change ``duk_debug.js`` you can implement a TCP proxy
which accepts a TCP connection from ``duk_debug.js`` and then uses your
custom transport to talk to the target::

   +--------------+   TCP   +-------+   custom   +--------+
   | duk_debug.js | ------> | proxy | ---------> | target |
   +--------------+         +-------+            +--------+

This is a straightforward option and a proxy can be used with other debug
clients too (perhaps custom scripts talking to the target etc).

You could also use netcat and implement your proxy so that it talks to
``duk_debug.js`` using stdin/stdout.

Debug client alternative 2: NodeJS stream
-----------------------------------------

To make ``duk_debug.js`` use a custom transport you need to:

* Implement your own transport as NodeJS stream.  You can add it directly to
  ``duk_debug.js`` but it's probably easiest to use a separate module so that
  the diff to ``duk_debug.js`` stays minimal.

* Change ``duk_debug.js`` to use the custom transport instead of a TCP
  stream.  Search for "CUSTOMTRANSPORT" in ``duk_debug.js``.

See:

* http://nodejs.org/api/stream.html

* https://github.com/substack/stream-handbook

Debug client alternative 3: custom debug client
-----------------------------------------------

You can also implement your own debug client and debug UI with support for
your custom transport.

You'll also need to implement the client part of the Duktape debugger
protocol.  See ``doc/debugger.rst`` for the specification and ``duk_debug.js``
for example running code which should illustrate the protocol in more detail.
