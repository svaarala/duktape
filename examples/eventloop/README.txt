Eventloop examples
==================

A few examples on how an event loop can be implemented with Duktape.  There
are several approaches, here we demonstrate two main approaches:

1. Using a C eventloop which calls into Javascript.  All the event loop state
   like timers, sockets, etc, is held in C structures.

2. Using an Ecmascript eventloop which never returns.  All the event loop state
   can be managed with Ecmascript code instead of C structures.  The Ecmascript
   eventloop calls a Duktape/C helper to do the lowest level poll() call.

The event loop API provided by both examples is the same, and includes:

* Timers: setTimeout, clearTimeout, setInterval, clearInterval

* Sockets: simple network sockets

In addition there are a few synchronous API bindings which are not event loop
related:

* File I/O

* Curses integration for doing beautiful character graphics

Note that this is NOT a production quality event loop.  For example, a
production quality event loop would track its internal state (active timers
and sockets) much more efficiently.
