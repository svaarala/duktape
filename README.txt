=======
Duktape
=======

Duktape is a small and portable Ecmascript E5/E5.1 implementation.
It is intended to be easily embeddable into C programs, with a C API
similar in spirit to Lua's.

The goal is to support the full E5 feature set like Unicode strings
and Perl 5 -like regular expressions.  Other feature highlights:

  * Custom types (like pointers and buffers) for better C integration

  * Reference counting and mark-and-sweep garbage collection, with
    finalizer support

  * Co-operative threads, a.k.a. coroutines

  * Tail call support

For building, see::

  doc/building.txt

For an API description for calling code, see::

  doc/api.txt

The internal design is described in a bunch of separate documents.
You should begin with the overview in::

  doc/design.txt

Duktape is licensed under the MIT license (see ``LICENSE.txt``).

Have fun!

