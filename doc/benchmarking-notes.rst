==================
Benchmarking notes
==================

This document provides some notes on how to benchmark performance or memory
consumption so that you'll get the most relevant results for an actual
target device.

Memory benchmarking
===================

Enable Duktape low memory options
---------------------------------

Enable Duktape low memory config options for benchmarking if the target
would actually be running with these options enabled.  For example, if
the target has 128-256 kB of system RAM, low memory options would be very
recommended:

* doc/low-memory.rst

* config/examples/low_memory.yaml

Enable DUK_USE_GC_TORTURE to see actual hard memory usage
---------------------------------------------------------

Enable ``DUK_USE_GC_TORTURE`` for testing so that you'll be measuring actual
memory, i.e. actually **reachable objects** which won't be collected when
an emergency garbage collection would take place.  This config option causes
a full mark-and-sweep garbage collection pass for every allocation so that
only actually reachable memory will remain use at any time.

Apparent memory usage seen without this option enabled may differ quite a lot
from what's actually needed.  That difference is usually irrelevant: if memory
were to run out, emergency garbage collection would be able free the
non-reachable objects.

The difference may matter in practice if some *other* component in the system
is out of memory, as it usually cannot trigger an emergency garbage
collection which would free up memory.  However, when using a pool allocator
for Duktape this is not an issue: all Duktape allocations will be contained
in the pre-allocated pool.

Measure usage using a pool allocator if target uses one
-------------------------------------------------------

Measurements using ``valgrind --tool=massif`` are relatively accurate (when
GC torture is enabled) but will include allocation overhead not present when
a pool allocator is used.  Pool allocators are recommended for low memory
targets to reduce overhead and heap fragmentation.

If the actual target uses a pool allocator, benchmarking should be done
against that allocator, with the pool entry sizes optimized for the actual
application code to be executed.  The difference between valgrind massif
reported usage and actual pool allocator usage can be quite large.  However,
when the pool configuration is poorly optimized, memory allocation overhead
caused by wasted pool entry bytes can also be significant.

Measurements using e.g. process RSS are very inaccurate and should be avoided
if possible as they don't accurately reflect the actual memory usage
achievable.  When measuring without a pool allocator, valgrind massif,
combined with enabling GC torture, is a much better option.

Example of DUK_USE_GC_TORTURE measurement impact
------------------------------------------------

Let's take an example program which involves creating a lot of anonymous
function instances, quite typical in callback oriented code::

    function test() {
        for (var i = 0; i < 10000; i++) {
            var ignored = function () {};
        }
    }
    test();

Because each such anonymous function is in a reference loop with its default
``.prototype`` object (which points back to the function using ``.constructor``
reference), the functions won't be collected by reference counting and will
be freed by mark-and-sweep.  Mark-and-sweep runs periodically but an emergency
mark-and-sweep is also triggered when an allocation attempt fails.

Compiling Duktape for defaults on x64 (without any low memory options, ROM
builtins, etc) shows the following memory usage in valgrind massif::

    ...
        KB
    539.2^                                                                      :
         |         #       :      ::      :              :      :@       :      :
         |         #       :      :      ::      :      ::      :@      @:     ::
         |         #      ::      :      ::      :      ::      :@     :@:    @::
         |        @#      ::     ::      ::      :      ::     ::@     :@:    @::
         |        @#     :::     ::     :::    :::     :::     ::@    ::@:    @::
         |        @#     :::    :::     :::    : :     :::    :::@    ::@:   :@::
         |       @@#    @:::    :::    ::::    : :     :::    :::@    ::@:   :@::
         |       @@#    @:::   @:::    ::::   :: :    ::::    :::@   :::@:  ::@::
         |      @@@#   :@:::   @:::   :::::   :: :    ::::   ::::@  ::::@:  ::@::
         |      @@@#   :@:::  :@:::   :::::   :: :    ::::  :::::@  ::::@:  ::@::
         |     @@@@#   :@:::  :@:::  ::::::  ::: :   :::::  :::::@  ::::@: :::@::
         |  @  @@@@#  ::@:::  :@:::  ::::::  ::: :   :::::  :::::@ :::::@: :::@::
         |  @  @@@@#  ::@:::  :@:::  :::::: :::: :  :::::: ::::::@ :::::@: :::@::
         |  @::@@@@#  ::@::: ::@:::  :::::: :::: :  :::::: ::::::@::::::@:::::@::
         | @@: @@@@#  ::@::::::@::: ::::::: :::: :  :::::: @:::::@::::::@:::::@:::
         | @@: @@@@#::::@::::::@::: :::::::::::: ::::::::::@:::::@::::::@:::::@:::
         | @@: @@@@#: ::@::::::@::: :::::::::::: :: :::::::@:::::@::::::@:::::@:::
         | @@: @@@@#: ::@::::::@::: :::::::::::: :: :::::::@:::::@::::::@:::::@:::
         | @@: @@@@#: ::@::::::@::: :::::::::::: :: :::::::@:::::@::::::@:::::@:::
       0 +----------------------------------------------------------------------->Mi
         0                                                                   138.9

From this it would appear the program is using ~540 kB of memory.  This is very
misleading because almost all of that usage is actually collectable garbage which
is periodically collected by mark-and-sweep (as seen above as "spiking").  In
particular, if memory were to run out (in concrete terms, an attempt to allocate
memory would fail), an emergency mark-and-sweep pass would free that memory which
would then be available for other use.

Enabling ``DUK_USE_GC_TORTURE`` we get a very different result::

    ...
        KB
    118.2^#
         |#
         |#:@@:@::::::::    :     :::             @::  :: : : : : ::
         |#:@ :@:: : : ::::::::::::::::::::@@:::::@: ::: :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
         |#:@ :@:: : : :: : ::::: :::::::::@ : :: @: : : :::::::::: :::@:::::::@::
       0 +----------------------------------------------------------------------->Gi
         0                                                                   20.29

The actual "hard" memory usage is ~120kB, only about 22% of the apparent memory
usage as seen by valgrind.  This hard memory usage is what really matters, i.e.
determines whether an application will be able to allocate more memory or not.

Performance benchmarking
========================

Enable Duktape performance options
----------------------------------

Unless you're running on a memory constrained device and prefer performance
over e.g. code footprint, you should enable Duktape performance options.
For more information, see:

* doc/performance-sensitive.rst

* config/examples/performance_sensitive.yaml

As with memory, it's important to measure with options relevant to the actual
target.  It's possible to enable most low memory options and performance options
at the same time (which makes sense if there's relatively little RAM but code
ROM footprint is not an issue).  Duktape low memory options may have an effect
on performance; in particular, heap pointer compression has a relatively large
performance impact which is important to account for, depending on whether the
eventual target will use heap pointer compression or not.

Test using function code by default
-----------------------------------

Global code (program code) and eval code have important semantic differences
to function code, i.e. statements residing inside a ``function () { ... }``
expression.  For Duktape the performance difference between these two kinds
of compiled code is very large.  The concrete difference is that for global
and eval code there are no local variables but instead all variable accesses
go through an internal slow path and are actually property reads and writes
on the global object.

As a concrete example, empty loop inside a function::

    $ cat test.js
    function test() {
        for (var i = 0; i < 1e7; i++) {
        }
    }
    test();

    $ time ./duk.O2.140 test.js
    real   0m0.256s
    user   0m0.256s
    sys    0m0.000s

Empty loop outside a function::

    $ cat test.js
    // Note that 'i' is actually a property of the global object.
    for (var i = 0; i < 1e7; i++) {
    }

    $ time ./duk.O2.140 _test.js
    real   0m4.325s
    user   0m4.319s
    sys    0m0.004s

The loop in global code runs ~20x slower than inside a function.  The
performance difference for practical code depends on how many variable
accesses are done.

In most programs the majority of actually performance relevant code is inside
functions.  In particular, all CommonJS modules are inside anonymous wrapper
functions automatically, so all module code will run using the fast path.
For benchmarking the best default, usually matching actually executing code
on the target, is to measure performance critical code by placing it inside
a function.

However, if the target will actually be running performance relevant code
in the global or eval context (which is quite possible for specific applications)
then it is of course prudent to measure that code outside a function.
