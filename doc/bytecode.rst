=======================
Duktape bytecode format
=======================

Overview
========

Duktape has API functions to dump a compiled function into bytecode and load
(reinstantiate) a function from a bytecode dump.  Bytecode dump/load allows
code to be compiled offline, compiled code to be cached and reused, compiled
code to be moved from one Duktape heap to another, etc.  However, Duktape
bytecode format is version specific so it is *not* a version neutral code
distribution format like Java bytecode.  (The term "bytecode" is used here
and in other Duktape documentation even though it's a bit inaccurate: the
serialization format includes many other fields besides bytecode
instructions.)

Duktape bytecode is **version specific** and (potentially) **config option
specific**, and may change arbitrarily even in minor releases (but is
guaranteed not to change in a patch release, as long as config options are
kept the same).  In other words, the bytecode format is not part of the
ordinary versioning guarantees.  If you compile code into bytecode offline,
you must ensure such code is recompiled whenever Duktape source is updated.
In this sense Duktape bytecode differs fundamentally from e.g. Java bytecode
which is used as a version neutral distribution format.

Duktape bytecode is **unvalidated** which means that loading untrusted or
broken bytecode may cause a crash or other memory unsafe behavior, leading
to potentially exploitable vulnerabilities.  Calling code is responsible for
ensuring that bytecode for a different Duktape version is not loaded, and that
the bytecode input is not truncated or corrupted.  (Validating bytecode is
quite difficult, because one would also need to validate the actual bytecode
which might otherwise refer to non-existent registers or constants, jump out
of bounds, etc.)

The bytecode format is **platform neutral**, so that it's possible to compile
the bytecode on one platform and load it on another, even if the platforms
have different byte order.  This is useful to support offline compilation in
cross compilation.

There are a few limitations on what kind of functions can be dumped into
bytecode, and what information is lost in the process.  See separate section
on limitations below.  The following API test case provides concrete examples
on usage and current limitations:

* ``api-testcases/test-dump-load-basic.c``

Working with bytecode
=====================

The ``duk_dump_function()`` API call is used to convert a function into a
buffer containing bytecode::

    duk_eval_string(ctx, "(function myfunc() { print('hello world'); })");
    duk_dump_function(ctx);
    /* -> stack top contains bytecode for 'myfunc' */

The ``duk_load_function()`` API call does the reverse, converting a buffer
containing bytecode into a function object::

    /* ... push bytecode to value stack top */
    duk_load_function(ctx);
    /* -> stack top contains function */

The Duktape command line tool "duk" can also be used to compile a file
into bytecode::

    ./duk -c /tmp/program.bin program.js

The input source is compiled as an ECMAScript program and the bytecode
will be for the "program function".  The command line tool doesn't support
compiling individual functions, and is mostly useful for playing with
bytecode.

The command line tool can also execute bytecode functions; it will just load
a function and call it without arguments, as if a program function was being
executed::

    ./duk -b /tmp/program.bin

When to use bytecode dump/load
==============================

Generally speaking, there are two main motivations for using a bytecode
dump/load mechanism:

* Performance

* Obfuscation

Duktape's bytecode format improves performance compared to compilation, but
is not ideal for obfuscation as discussed in more detail below.

Performance
-----------

Whenever compilation performance is *not* an issue, it is nearly always
preferable to compile functions from source rather than using bytecode
dump/load.  Compiling from source is memory safe, version compatible,
and has no semantic limitations like bytecode.

There are some applications where compilation is a performance issue.
For example, a certain function may be compiled and executed over and
over again in short lived Duktape global contexts or even separate
Duktape heaps (which prevents reusing a single function object).  Caching
the compiled function bytecode and instantiating the function by loading
the bytecode is much faster than recompiling it for every execution.

Obfuscation
-----------

Obfuscation is another common reason to use bytecode: it's more difficult
to reverse engineer source code from bytecode than e.g. minified code.
However, when doing so, you should note the following:

* Some minifiers support obfuscation which may be good enough and avoids
  the bytecode limitations and downsides.

* For some targets source code encryption may be a better option than
  relying on bytecode for obfuscation.

* Although Duktape bytecode doesn't currently store source code, it does
  store all variable names (``_Varmap``) and formal argument names
  (``_Formals``) which are needed in some functions.  It may also be
  possible source code is included in bytecode at some point to support
  debugging.  In other words, **obfuscation is not a design goal for the
  bytecode format**.

That said, concrete issues to consider when using bytecode for obfuscation:

* Variable names in the ``_Varmap`` property: this cannot be easily avoided
  in general but a minifier may be able to rename variables.

* Function name in the ``name`` property: this can be deleted or changed
  before dumping a function, but note that some functions (such as
  self-recursive functions) may depend on the property being present and
  correct.

* Function filename in the ``fileName`` property: this can also be deleted
  or changed before dumping a function.  You can avoid introducing a filename
  at all by using ``duk_compile()`` (rather than e.g. ``duk_eval_string()``)
  to compile the function.

* Line number information in the ``_Pc2line`` property: this can be deleted or
  changed, or you can configure Duktape not to store this information in the
  first place (using option ``DUK_USE_PC2LINE``).  Without line information
  tracebacks will of course be less useful.

When not to use bytecode dump/load
==================================

Duktape bytecode is **not** a good match for:

* Distributing code

* Minimizing code size

Distributing code
-----------------

It's awkward to use a version specific bytecode format for code distribution.
This is especially true for ECMAScript, because the language itself is
otherwise well suited for writing backwards compatible code, detecting
features at run-time, etc.

It's also awkward for code distribution that the bytecode load operation
relies on calling code to ensure the loaded bytecode is trustworthy and
uncorrupted.  In practice this means e.g. cryptographic signatures are
needed to avoid tampering.

Minimizing code size
--------------------

The bytecode format is designed to be fast to dump and load, while still
being platform neutral.  It is *not* designed to be compact (and indeed
is not).

For example, for a simple Mandelbrot function (``mandel()`` in
``dist-files/mandel.js``):

+---------------------------+----------------+----------------------+
| Format                    | Size (bytes)   | Gzipped size (bytes) |
+===========================+================+======================+
| Original source           | 884            | 371                  |
+---------------------------+----------------+----------------------+
| Bytecode dump             | 809            | 504                  |
+---------------------------+----------------+----------------------+
| UglifyJS2-minified source | 364            | 267                  |
+---------------------------+----------------+----------------------+

For minimizing code size, using a minifier and ordinary compression is
a much better idea than relying on compressed or uncompressed bytecode.

Bytecode limitations
====================

Function lexical environment is lost
------------------------------------

A function loaded from bytecode always works as if it was defined in the
global environment so that any variable lookups not bound in the function
itself will be resolved through the global object.  If you serialize ``bar``
created as::

    function foo() {
        var myValue = 123;

        function bar() {
            // myValue will be 123, looked up from 'foo' scope

            print(myValue);
        }

        return bar;
    }

and then load it back, it will behave as if it was originally created as::

    function bar() {
        // myValue will be read from global object

        print(myValue);
    }

If the original function was established using a function declaration,
the declaration itself is not restored when a function is loaded.  This
may be confusing.  For example, if you serialize ``foo`` declared as::

    function foo() {
        // Prints 'function' before dump/load; 'foo' is looked up from
        // the global object.

        print(typeof foo);
    }

and then load it back, it will behave as::

    var loadedFunc = (function() {
        // Prints 'undefined' after dump/load; 'foo' is looked up from
        // the global object.  Workaround is to assign loadedFunc to
        // globalObject.foo manually before calling to simulate declaration.

        print(typeof foo);
    });

No function name binding for function declarations
--------------------------------------------------

Function name binding for function expressions is supported, e.g. the
following function would work::

    // Can dump and load this function, the reference to 'count' will
    // be resolved using the automatic function name lexical binding
    // provided for function expressions.

    var func = function count(n) { print(n); if (n > 0) { count(n - 1); } };

However, for technical reasons functions that are established as global
declarations work a bit differently::

    // Can dump and load this function, but the reference to 'count'
    // will lookup globalObject.count instead of automatically
    // referencing the function itself.  Workaround is to assign
    // the function to globalObject.count after loading.

    function count(n) { print(n); if (n > 0) { count(n - 1); } };

(The NAMEBINDING flag controls creation of a lexical environment which
contains the function expression name binding.  In Duktape 1.2 the flag
is only set for function templates, not function instances; this was
changed for Duktape 1.3 so that the NAMEBINDING flag could be detected
when loading bytecode, and a lexical environment can then be created
based on the flag.)

Custom internal prototype is lost
---------------------------------

A custom internal prototype is lost, and ``Function.prototype`` is used
on bytecode load.

Custom external prototype is lost
---------------------------------

A custom external prototype (``.prototype`` property) is lost, and a
default empty prototype is created on bytecode load.

Finalizer on the function is lost
---------------------------------

A finalizer on the function being serialized is lost, no finalizer will
exist after a bytecode load.

Only specific function object properties are kept
-------------------------------------------------

Only specific function object properties, i.e. those needed to correctly
revive a function, are kept.  These properties have type and value
limitations:

* .length: uint32, non-number values replaced by 0

* .name: string required, non-string values replaced by empty string

* .fileName: string required, non-string values replaced by empty string

* ._Formals: internal property, value is an array of strings

* ._Varmap: internal property, value is an object mapping identifier
  names to register numbers

Bound functions are not supported
---------------------------------

Currently a ``TypeError`` is thrown when trying to serialize a bound function
object.

CommonJS modules don't work well with bytecode dump/load
--------------------------------------------------------

CommonJS modules cannot be trivially serialized because they're normally
evaluated by embedding the module source code inside a temporary function
wrapper (see ``modules.rst`` for details).  User code does not have access
to the temporary wrapped function.  This means that:

* If you compile and serialize the module source, the module will
  have incorrect scope semantics.

* You could add the function wrapper and compile the wrapped function
  instead.

* Module support for bytecode dump/load will probably need future work.

Bytecode format
===============

A function is serialized into a platform neutral byte stream.  Multibyte
values are in network order (big endian), and don't have any alignment
guarantees.

Because the exact format is version specific, it's not documented in full
detail here.  Doing so would mean tedious documentation updates whenever
bytecode was changed, and documentation would then easily fall out of date.
The exact format is ultimately defined by the source code, see:

* ``src-input/duk_api_bytecode.c``

* ``tools/dump_bytecode.py``

As a simplified summary of the bytecode format:

* A single 0xBF marker byte which never occurs in a valid extended UTF-8
  string.  (A version byte used to follow the marker; it was removed in
  Duktape 2.2 because it hadn't been bumped and because it really provided
  no version guarantees.)

* The marker is followed by a serialized function.  The function may contain
  inner functions which are serialized recursively (without duplicating the
  two-byte header).

The function serialization format is tedious and best looked up directly from
source code.

NOTE: The top level function is a function instance, but all inner functions
are function templates.  There are some difference between the two which must
be taken into account in bytecode serialization code.

Security and memory safety
==========================

Duktape bytecode must only be loaded from a trusted source: loading broken
or maliciously crafted bytecode may lead to memory unsafe behavior, even
exploitable behavior.

Because bytecode is version specific, it is generally unsafe to load bytecode
provided by a network peer -- unless you can somehow be certain the bytecode
is specifically compiled for your Duktape version.

Design notes
============

Eval and program code
---------------------

ECMAScript specification recognizes three different types of code: program
code, eval code, and function code, with slightly different scope and variable
binding semantics.  The serialization mechanism supports all three types.

Version specific vs. version neutral
------------------------------------

Duktape bytecode instruction format is already version specific and can change
between even minor releases, so it's quite natural for the serialization
format to also be version specific.

Providing a version neutral format would be possible when Duktape bytecode no
longer changes in minor versions (not easy to see when this would be the case)
or by doing some kind of recompilation for bytecode.

Config option specific
----------------------

Some Duktape options may affect what function metadata is available.  E.g. you
can disable line number information (pc2line) which might then be left out of
the bytecode dump altogether.  Attempting to load such a dump in a Duktape
environment compiled with line number information enabled might then fail due
to a format error.

(In the initial master merge there are no config option specific format
differences, but there may be such differences in later Duktape versions
if it's convenient to do so.)

Endianness
----------

Network endian was chosen because it's also used elsewhere in Duktape (e.g.
the debugger protocol) as the default, portable endianness.

Faster bytecode dump/load could be achieved by using native endianness and
(if necessary) padding to achieve proper alignment.  This additional speed
improvement was considered less important than portability.

Platform neutrality
-------------------

Supporting cross compilation is a useful feature so that bytecode generated on
one platform can be loaded on another, as long as they run the same Duktape
version.

The cost of being platform neutral is rather small.  The essential features
are normalizing endianness and avoiding alignment assumptions.  Both can be
quite easily accommodated with relatively little run-time cost.

Bytecode header
---------------

The initial 0xBF byte is used because it can never appear in valid UTF-8
(even extended UTF-8) so that using a random string accidentally as bytecode
input will fail.

Memory safety and bytecode validation
-------------------------------------

The bytecode load primitive is memory unsafe, to the extent that trying to
load corrupted (truncated and/or modified) bytecode may lead to memory unsafe
behavior (even exploitable behavior).  To keep bytecode loading fast and simple,
there are even no bounds checks when parsing the input bytecode.

This might seem strange but is intentional: while it would be easy to do basic
syntax validation for the serialized data when it is loaded, it still wouldn't
guarantee memory safety.  To do so one would also need to validate the bytecode
opcodes, otherwise memory unsafe behavior may happen at run time.

Consider the following example: a function being loaded has ``nregs`` 100, so
that 100 slots are allocated from the value stack for the function.  If the
function bytecode then executed::

    LDREG 1, 999   ; read reg 999, out of bounds
    STREG 1, 999   ; write reg 999, out of bounds

Similar issues exist for constants; if the function has 100 constants::

    LDCONST 1, 999 ; read constant 999, out of bounds

In addition to direct out-of-bounds references there are also "indirect"
opcodes which e.g. load a register index from another register.  Validating
these would be a lot more difficult and would need some basic control flow
algorithm, etc.

Overall it would be quite difficult to implement bytecode validation that
would correctly catch broken and perhaps maliciously crafted bytecode, and
it's not very useful to have a partial solution in place.

Even so there is a very simple header signature for bytecode which ensures
that obviously incorrect values are rejected early.  The signature ensures
that no ordinary string data can accidentally be loaded as byte code
(the initial byte 0xBF is invalid extended UTF-8).  Any bytes beyond the
marker are unvalidated.

Future work
===========

Full value serialization
------------------------

Bytecode dump/load is restricted to a subset of function values.  It would be
more elegant to support generic value dump/load.  However, there are several
practical issues:

* Arbitrary object graphs would need to be supported, which is quite
  challenging.

* There'd have to be some mechanism to "revive" any native values on
  load.  For example, for a native object representing an open file,
  the revive operation would reopen the file and perhaps seek the file
  to the correct offset.

Support bound functions
-----------------------

Currently a TypeError is thrown for bound functions.  As a first step, it's
probably better to follow the bound chain and serialize the final target
function instead, i.e. bound status would be lost during serialization.
This is more in line with serializing with loss of some metadata rather than
throwing an error.

As the second step, it would be nice to serialize the bound ``this`` and
argument values.  However, proper generic value serialization would be needed
to do that.

Caching CommonJS modules
------------------------

Caching CommonJS modules would be very useful.  Figure out how to do that
when reworking the module mechanism.

Figure out debugger overlap
---------------------------

The debugger protocol has its own value serialization format (with somewhat
different goals):

- Would it be sensible to share value serialization format between dump/load
  and debugger protocol?

- Should function values be serialized in the debugger protocol in the
  bytecode dump/load format?  Would that be useful for the debugger (not
  immediately apparent why)?
