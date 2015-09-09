=======================
Low memory environments
=======================

Overview
========

This document describes suggested feature options for reducing Duktape
memory usage for memory-constrained environments, which are one important
portability target for Duktape.

The default Duktape options are quite memory conservative, and significant
Ecmascript programs can be executed with e.g. 1MB of memory.  Currently
realistic memory targets are roughly:

* 256-384kB flash memory (code) and 256kB system RAM

  - Duktape compiled with default options is feasible

  - Duktape compiles to around 200-210kB of code (x86), so 256kB is
    technically feasible but leaves little space for user bindings,
    hardware initialization, communications, etc; 384kB is a more
    realistic flash target

* 256-384kB flash memory (code) and 128kB system RAM

  - Duktape feature options are needed to reduce memory usage

  - A custom pool-based memory allocation with manually tuned pools
    may be required

  - Aggressive measures like lightweight functions, 16-bit fields for
    various internal structures (strings, buffers, objects), pointer
    compression, external strings, etc may need to be used

* 256kB flash memory (code) and 96kB system RAM

  - Requires a bare metal system, possibly a custom C library, etc.

  - http://pt.slideshare.net/seoyounghwang77/js-onmicrocontrollers

There are four basic goals for low memory optimization:

1. Reduce Duktape code (flash) footprint.  This is currently a low priority
   item because flash size doesn't seem to be a bottleneck for most users.

2. Reduce initial memory usage of a Duktape heap.  This provides a baseline
   for memory usage which won't be available for user code (technically some
   memory can be reclaimed by deleting some built-ins after heap creation).

3. Minimize the growth of the Duktape heap relative to the scope and
   complexity of user code, so that as large programs as possible can be
   compiled and executed in a given space.  Important contributing factors
   include the footprint of user-defined Ecmascript and Duktape/C functions,
   the size of compiled bytecode, etc.

4. Make remaining memory allocations as friendly as possible for the memory
   allocator, especially a pool-based memory allocator.  Concretely, prefer
   small chunks over large contiguous allocations.

The following genconfig option file template enables most low memory related
option: ``config/examples/low_memory.yaml``.  It doesn't enable pointer
compression because that always requires some application specific code.

Suggested feature options
=========================

* Use the default memory management settings: although reference counting
  increases heap header size, it also reduces memory usage fluctuation
  which is often more important than absolute footprint.

* If the target has a shallow C stack, you may want to limit C stack
  recursion, see:

  - ``config/examples/shallow_c_stack.yaml``

* Reduce error handling footprint with one or more of:

  - ``DUK_OPT_NO_AUGMENT_ERRORS``

  - ``DUK_OPT_NO_TRACEBACKS``

  - ``DUK_OPT_NO_VERBOSE_ERRORS``

  - ``DUK_OPT_NO_PC2LINE``

* Use slower but more compact lexer algorithm (saves on code footprint):

  - ``#undef DUK_USE_LEXER_SLIDING_WINDOW``

* Disable JSON fast paths (saves on code footprint):

  - ``#undef DUK_USE_JSON_STRINGIFY_FASTPATH``

  - ``#undef DUK_USE_JSON_QUOTESTRING_FASTPATH``

  - ``#undef DUK_USE_JSON_DECSTRING_FASTPATH``

  - ``#undef DUK_USE_JSON_DECNUMBER_FASTPATH``

  - ``#undef DUK_USE_JSON_EATWHITE_FASTPATH``

* If you don't need Node.js Buffer and Khronos/ES6 typed array support, use:

  - ``DUK_OPT_NO_BUFFEROBJECT_SUPPORT``

  - ``#undef DUK_USE_BUFFEROBJECT_SUPPORT``

* If you don't need the Duktape-specific additional JX/JC formats, use:

  - ``DUK_OPT_NO_JX``

  - ``DUK_OPT_NO_JC``

* Features borrowed from Ecmascript E6 can usually be disabled:

  - ``DUK_OPT_NO_ES6_OBJECT_SETPROTOTYPEOF``

  - ``DUK_OPT_NO_ES6_OBJECT_PROTO_PROPERTY``

  - ``DUK_OPT_NO_ES6_PROXY``

* If you don't need regexp support, use:

  - ``DUK_OPT_NO_REGEXP_SUPPORT``

* Disable unnecessary parts of the C API:

  - ``DUK_OPT_NO_BYTECODE_DUMP_SUPPORT``

  - ``#undef DUK_USE_BYTECODE_DUMP_SUPPORT``

* Duktape debug code uses a large, static temporary buffer for formatting
  debug log lines.  If you're running with debugging enabled, use e.g.
  the following to reduce this overhead:

  - ``-DDUK_OPT_DEBUG_BUFSIZE=2048``

More aggressive options
=======================

**These options are experimental in Duktape 1.1, i.e. the may change
in an incompatible manner in Duktape 1.2.**

The following may be needed for very low memory environments (e.g. 128kB
system RAM):

* Consider using lightweight functions for your Duktape/C bindings and to
  force Duktape built-ins to be lightweight functions:

  - ``DUK_OPT_LIGHTFUNC_BUILTINS``

* If code footprint is a significant issue, disabling reference counting
  reduces code footprint by several kilobytes at the cost of more RAM
  fluctuation:

  - ``DUK_OPT_NO_REFERENCE_COUNTING``

  - ``#undef DUK_USE_REFERENCE_COUNTING``

  - ``#undef DUK_USE_DOUBLE_LINKED_LIST``

* Enable other 16-bit fields to reduce header size; these are typically
  used together (all or none):

  - ``DUK_OPT_REFCOUNT16``

  - ``DUK_OPT_STRHASH16``

  - ``DUK_OPT_STRLEN16``

  - ``DUK_OPT_BUFLEN16``

  - ``DUK_OPT_OBJSIZES16``

* Enable heap pointer compression, assuming pointers provided by your allocator
  can be packed into 16 bits:

  - ``DUK_OPT_HEAPPTR16``

  - ``DUK_OPT_HEAPPTR_ENC16(udata,p)``

  - ``DUK_OPT_HEAPPTR_DEC16(udata,x)``

  - Note: you cannot currently enable Duktape debug prints (DUK_OPT_DEBUG and
    DUK_OPT_DPRINT etc) when heap pointer compression is enabled.

* Enable data pointer compression if possible.  Note that these pointers can
  point to arbitrary memory locations (outside Duktape heap) so this may not
  be possible even if Duktape heap pointers can be compressed:

  - ``DUK_OPT_DATAPTR16``

  - ``DUK_OPT_DATAPTR_ENC16(udata,p)``

  - ``DUK_OPT_DATAPTR_DEC16(udata,x)``

  - **UNIMPLEMENTED AT THE MOMENT**

* Enable C function pointer compression if possible.  Duktape compiles to
  around 200kB of code, so assuming an alignment of 4 this may only be
  possible if there is less than 56kB of user code:

  - ``DUK_OPT_FUNCPTR16``

  - ``DUK_OPT_FUNCPTR_ENC16(udata,p)``

  - ``DUK_OPT_FUNCPTR_DEC16(udata,x)``

  - **UNIMPLEMENTED AT THE MOMENT**

* Enable a low memory optimized string table variant which uses a fixed size
  top level hash table and array chaining to resolve collisions.  This makes
  memory behavior more predictable and avoids a large continuous allocation
  used by the default string table:

  - ``DUK_OPT_STRTAB_CHAIN``

  - ``DUK_OPT_STRTAB_CHAIN_SIZE=128`` (other values possible also)

* Use "external" strings to allocate most strings from flash (there are
  multiple strategies for this, see separate section):

  - ``DUK_OPT_EXTERNAL_STRINGS``

  - ``DUK_OPT_EXTSTR_INTERN_CHECK(udata,ptr,len)``

  - ``DUK_OPT_EXTSTR_FREE(udata,ptr)``

* Enable struct packing in compiler options if your platform doesn't have
  strict alignment requirements, e.g. on gcc/x86 you can:

  - ``-fpack-struct=1`` or ``-fpack-struct=2``

Notes on pointer compression
============================

Pointer compression can be applied throughout (where it matters) for three
pointer types:

* Compressed 16-bit Duktape heap pointers, assuming Duktape heap pointers
  can fit into 16 bits, e.g. max 256kB memory pool with 4-byte alignment

* Compressed 16-bit function pointers, assuming C function pointers can
  fit into 16 bits

* Compressed 16-bit non-Duktape-heap data pointers, assuming C data
  pointers can fit into 16 bits

Pointer compression can be quite slow because often memory mappings are not
linear, so the required operations are not trivial.  NULL also needs specific
handling.

External string strategies (DUK_OPT_EXTSTR_INTERN_CHECK)
========================================================

The feature can be used in two basic ways:

* You can anticipate a set of common strings, perhaps extracted by parsing
  source code, and build them statically into your program.  The strings will
  then be available in the "text" section of your program.  This works well
  if the set of common strings can be estimated well, e.g. if the program
  code you will run is mostly known in advance.

* You can write strings to memory mapped flash when the hook is called.
  This is less portable but can be effective when the program you will run
  is not known in advance.

Note that:

* Using an external string pointer for short strings (e.g. 3 chars or less)
  is counterproductive because the external pointer takes more room than the
  character data.

The Duktape built-in strings are available from build metadata:

* ``dist/duk_build_meta.json``, the ``builtin_strings_base64`` contains
  the byte exact strings used, encoded with base-64.

Strings used by application C and Ecmascript code can be extracted with
various methods.  The Duktape main repo contains an example script for
scraping strings from C and Ecmascript code using regexps:

* ``util/scan_strings.py``

There are concrete examples for some external string strategies in:

* ``dist/examples/cmdline/duk_cmdline_ajduk.c``

Tuning pool sizes for a pool-based memory allocator
===================================================

The memory allocations used by Duktape depend on the architecture and
especially the low memory options used.  So, the safest approach is to
select the options you want to use and then measure actual allocation
patterns of various programs.

The memory allocations needed by Duktape fall into two basic categories:

* A lot of small allocations (roughly between 16 and 128 bytes) are needed
  for strings, buffers, objects, object property tables, etc.  These
  allocation sizes constitute most of the allocation activity, i.e. allocs,
  reallocs, and frees.  There's a lot churn (memory being allocated and
  freed) even when memory usage is nearly constant.

* Much fewer larger allocations with much less activity are needed for
  Ecmascript function bytecode, large strings and buffers, value stacks,
  the global string table, and the Duktape heap object.

The ``examples/alloc-logging`` memory allocator can be used to write out
an allocation log file.  The log file contains every alloc, realloc, and
free, and will record both new and old sizes for realloc.  This allows you
to replay the allocation sequence so that you can simulate the behavior of
pool sizes offline.

The ``examples/allog-logging/pool_simulator.py`` simulates pool allocator
behavior for a given allocation log, and provides a lot of detailed graphs
of pool usage, allocated bytes, waste bytes, etc.  It also provides some
tools to optimize pool counts for one or multiple application "profiles".
See detailed description below.

You can also get a dump of Duktape's internal struct sizes by enabling
``DUK_OPT_DPRINT``; Duktape will debug print struct sizes when a heap is
created.  The struct sizes will give away the minimum size needed by strings,
buffers, objects, etc.  They will also give you ``sizeof(duk_heap)`` which
is a large allocation that you should handle explicitly in pool tuning.

Finally, you can look at existing projects and what kind of pool tuning
they do.  AllJoyn.js has a manually tuned pool allocator which may be a
useful starting point:

* https://git.allseenalliance.org/cgit/core/alljoyn-js.git/

Tuning pool sizes using pool_simulator.py
=========================================

Overview
--------

The pool simulator replays allocation logs, simulates the behavior of a
pool-based memory allocator, and provides several useful commands:

* Replay an allocation log and provide statistics and graphs for the pool
  performance: used bytes, wasted bytes, by-pool breakdowns, etc.

* Optimize pool counts based on a high-water-mark measurement, when given
  pool byte sizes (a base pool configuration) and an allocation log.

* Optimize pool counts based on a more complex algorithm which takes pool
  borrowing into account (see discussion below).

* Generate a pool configuration for a given total memory target, given the
  tight pool configuration for Duktape and a set of representative
  applications.

These operations are discussed in more detail below.

Important notes
---------------

* Before optimizing pools, you should select Duktape feature options
  (especially low memory options) carefully.

* It may be useful to use DUK_OPT_GC_TORTURE to ensure that there is no
  slack in memory allocations; reference counting frees unreachable values
  but does not handle loops.  When GC torture is enabled, Duktape will run
  a mark-and-sweep for every memory allocation.  High-water-mark values
  will then reflect the memory usage achievable in an emergency garbage
  collect.

* The pool simulator provides pool allocator behavior matching AllJoyn.js's
  ajs_heap.c allocator.  If your pool allocator has different basic features
  (for example, splitting and merging of chunks) you'll need to tweak the
  pool simulator to get useful results.

Basics
------

The Duktape command line tool writes out an allocation log when requested::

  # Log written to /tmp/duk-alloc-log.txt
  $ make clean duk
  $ ./duk --alloc-logging tests/ecmascript/test-dev-mandel2-func.js

The "ajduk" command line tool is a variant with AllJoyn.js pool allocator,
and a host of low memory optimizations.  It represents a low memory target
quite well and it can also be requested to write out an allocation log::

  # Log written to /tmp/ajduk-alloc-log.txt
  $ make clean ajduk
  $ ./ajduk --ajsheap-log tests/ecmascript/test-dev-mandel2-func.js

Allocation logs are represented in examples/alloc-logging format::

  ...
  A 0xf7541c38 16
  R 0xf754128c -1 0xf754125c 6
  A 0xf7541c24 16
  ...

The pool simulator doesn't need to know the "previous size" for a realloc
entry, so it can be written out as -1 (like ajduk does).

Pool configurations are expressed in JSON::

  {
    "pools": [
      { "size": 8, "count": 10, "borrow": true },
      { "size": 12, "count": 10, "borrow": true },
      { "size": 16, "count": 200, "borrow": true },
      ...
    ]
  }

The "size" (entry size, byte size) of a pool is the byte-size of individual
chunks in that pool.  The "count" (entry count) is the number of chunks
preallocated for that pool.  Above, the second pool has entry size of 12
bytes and a count of 10, for a total of 120 bytes.

The pool simulator matches AllJoyn.js ajs_heap.c behavior:

* Allocations are taken from smallest matching pool.  Borrowing is enabled
  or disabled for each pool individually.

* Reallocation tries to shrink the allocation to a previous pool size if
  possible.

"High-water-mark" (hwm) over an entire allocation log means simulating the
allocation log against a certain pool configuration, and recording the
highest number of used entries for each pool.  There are two variants for
this measurement:

* Without borrowing: ignore the "count" for each pool in the configuration
  and autoextend the pool as needed.  This provides a high-water-mark
  without a need to borrow from larger pools.

* With borrowing: respect the "count" in the pool configuration and borrow
  as needed.

Tight pool counts using high water mark (hwm)
---------------------------------------------

To find out the high water mark for each pool size without borrowing::

  $ rm -rf /tmp/out; mkdir /tmp/out
  $ python examples/alloc-logging/pool_simulator.py \
      --out-dir /tmp/out \
      --alloc-log /tmp/duk-alloc-log.txt \
      --pool-config examples/alloc-logging/pool_config_1.json \
      --out-pool-config /tmp/tight_noborrow.json \
      tight_counts_noborrow

The hwm records the maximum count for each pool size::

  ^ pool entry count
  |
  |   ##
  |  #####
  | ######
  | ######
  | ########
  +---------> pool entry size

As described above, this command ignores the pool counts in the pool config
and autoextends each pool to find its hwm.  The resulting pool configuration
with updated counts is written out.

Tight pool counts taking borrowing into account
-----------------------------------------------

The high water marks for each pool entry size don't necessarily happen
at the same time.  Let's use the example above::

  ^ pool entry count
  |
  |   ##
  |  #####
  | ######
  | ######
  | ########
  +---------> pool entry size

As an example, when the hwm for the third pool size (highlighted below)
happens, the allocation state might be::

  ^ pool entry count
  |
  |   #
  |  :#
  | ::#::
  | ::#:::
  | ::#:::::
  +---------> pool entry size

This means that we can often reduce the hwm-based pool counts and still
allow the application to run; the application will be able to borrow
allocations from larger pool entry sizes.

As an extreme example, if Duktape were to allocate and free one entry
from each pool entry size (but so that only one allocation would be
active at a time), the hwm counts would look like::

  ^ pool entry count
  |
  |
  |
  |
  |
  | ########
  +---------> pool entry size

However, the allocations can all be satisfied by having just one pool
entry of the largest allocated size: all other allocation requests
will just borrow from that (assuming borrowing is allowed)::

  ^ pool entry count
  |
  |
  |
  |
  |
  |        #
  +---------> pool entry size

The pool simulator optimizes for tight pool counts with borrowing effects
taken into account using a pretty simple brute force algorithm:

* Get the basic hwm profile with no borrowing.

* Start from the largest pool entry size and loop downwards:

  - Reduce pool entry count for that pool entry size in question and rerun
    the allocation log.

  - If allocation requests can be still satisfied through borrowing, continue
    to reduce the allocation.

  - When the pool entry count can no longer be reduced, move on to the next
    pool size.

The basic observation in the algorithm is as follows:

* The pool entry counts above the current one are optimal: they can't be
  reduced further.

* The pool entry counts below the current one never borrow from any of the
  higher pool counts (yet) because they were optimized for their hwm.

* We reduce the current pool entry count, hoping that some of the allocations
  needed for its hwm can be borrowed from the larger pool entry sizes.  This
  is possible if the hwm of the current pool entry size doesn't coincide with
  the hwm of the larger pool entry sizes.

This algorithm leads to reasonable pool entry counts, but:

* The counts may not be an optimal balance for other applications.

* The pool entry sizes are assumed to be given and are not optimized for
  automatically.

Use the following command to run the optimization::

  $ rm -rf /tmp/out; mkdir /tmp/out
  $ python examples/alloc-logging/pool_simulator.py \
      --out-dir /tmp/out \
      --alloc-log /tmp/duk-alloc-log.txt \
      --pool-config examples/alloc-logging/pool_config_1.json \
      --out-pool-config /tmp/tight_borrow.json \
      tight_counts_borrow

This may take a lot of time, so be patient.

As a concrete example, for test-dev-mandel2-func.js on x86 with low memory
optimizations, the tight pool configuration based on hwm is::

  total 31564:
  8=91 12=25 16=373 20=56 24=2 28=58 32=1 40=32 48=4 52=27 56=1 60=5 64=0
  128=20 256=9 512=8 1024=4 1360=1 2048=2 4096=0 8192=0 16384=0 32768=0

and after borrow optimization::

  total 28532:
  8=91 12=20 16=370 20=53 24=2 28=58 32=0 40=10 48=3 52=26 56=1 60=4 64=0
  128=16 256=8 512=8 1024=3 1360=1 2048=2 4096=0 8192=0 16384=0 32768=0

The more dynamic an application's memory usage is, the more potential there
is for borrowing.

Optimizing for multiple application profiles
--------------------------------------------

Run hello world with alloc logging for Duktape baseline::

  # Using "duk", writes log to /tmp/duk-alloc-log.txt
  $ ./duk --alloc-logging tests/ecmascript/test-dev-hello-world.js

  # Using "ajduk", writes log to /tmp/ajduk-alloc-log.txt
  $ ./ajduk --ajsheap-log tests/ecmascript/test-dev-hello-world.js

Extract a "tight" pool configuration for the hello world baseline,
pool entry sizes (but not counts) need to be known in advance::

  $ rm -rf /tmp/out; mkdir /tmp/out
  $ python examples/alloc-logging/pool_simulator.py \
      --out-dir /tmp/out \
      --alloc-log /tmp/duk-alloc-log.txt \
      --pool-config examples/alloc-logging/pool_config1.json \
      --out-pool-config /tmp/config_tight_duktape.json \
      tight_counts_borrow

Run multiple test applications and extract tight pool configurations for
each (includes Duktape baseline but that is subtracted later) using the
same method::

  $ ./duk --alloc-logging tests/ecmascript/test-dev-mandel2-func.js
  $ rm -rf /tmp/out; mkdir /tmp/out
  $ python examples/alloc-logging/pool_simulator.py \
      --out-dir /tmp/out \
      --alloc-log /tmp/duk-alloc-log.txt \
      --pool-config examples/alloc-logging/pool_config1.json \
      --out-pool-config /tmp/config_tight_app1.json \
      tight_counts_borrow

  $ ./duk --alloc-logging tests/ecmascript/test-bi-array-proto-push.js
  $ rm -rf /tmp/out; mkdir /tmp/out
  $ python examples/alloc-logging/pool_simulator.py \
      --out-dir /tmp/out \
      --alloc-log /tmp/duk-alloc-log.txt \
      --pool-config examples/alloc-logging/pool_config1.json \
      --out-pool-config /tmp/config_tight_app2.json \
      tight_counts_borrow

  # ...

Select a target memory amount (here 200kB) and optimize pool entry
counts for that amount::

  $ python examples/alloc-logging/pool_simulator.py \
      --out-pool-config /tmp/config_200kb.json \
      --out-ajsheap-config /tmp/ajsheap_200kb.c \
      pool_counts_for_memory \
      204800 \
      /tmp/config_tight_duktape.json \
      /tmp/config_tight_app1.json \
      /tmp/config_tight_app2.json \
      ... \
      /tmp/config_tight_appN.json

  # /tmp/config_200kb.json is the pool config in JSON

  # /tmp/ajsheap_200kb.c is the pool config as an ajs_heap.c initializer

The optimization algorithm is based on the following basic idea:

* Pool entry byte sizes are kept fixed throughout the process.

* Application pool counts are normalized by subtracting Duktape baseline
  pool counts, yielding application memory usage on top of Duktape.  These
  pool counts can be scaled meaningfully to estimate memory demand if the
  "application size" (function count, statement count, etc) were to grow
  or shrink.

* The resulting pool count profiles are normalized to a fixed total memory
  usage (any value will do, 1MB is used now).  The resulting pool counts
  are fractional.

* A pool count profile representing all the applications is computed as
  follows.  For each pool entry size, take the maximum of the normalized,
  scaled pool counts over the application profiles.  This profile represents
  the the memory usage of a mix of applications.

* Allocate pool counts for Duktape baseline.  This allocation is independent
  of application code and doesn't grow in relation to application memory
  usage.

* Scale the representative pool count profile to fit the remaining memory,
  using fractional counts.

* Round pool counts into integers, ensuring the total memory usage is as
  close to the target (without exceeding it).

Summary of potential measures
=============================

Heap headers
------------

* Compressed 16-bit heap pointers

* 16-bit field for refcount

* Move one struct specific field (e.g. 16-bit string length) into the unused
  bits of the ``duk_heaphdr`` 32-bit flags field

Objects
-------

* Tweak growth factors to keep objects always or nearly always compact

* 16-bit field for property count, array size, etc.

* Drop hash part entirely: it's rarely needed in low memory environments
  and hash part size won't need to be tracked

* Compressed pointers

Strings
-------

* Use an indirect string type which stores string data behind a pointer
  (same as dynamic buffer); allow user code to indicate which C strings
  are immutable and can be used in this way

* Allow user code to move a string to e.g. memory-mapped flash when it
  is interned or when the compiler interns its constants (this is referred
  to as "static strings" or "external strings")

* Memory map built-in strings (about 2kB bit packed) directly from flash

* 16-bit fields for string char and byte length

* 16-bit string hash

* Rework string table to avoid current issues: (1) large reallocations,
  (2) rehashing needs both old and new string table as it's not in-place.
  Multiple options, including:

  - Separate chaining (open hashing, closed addressing) with a fixed or
    bounded top level hash table

  - Various tree structures like red-black trees

* Compressed pointers

Duktape/C function footprint
----------------------------

* Lightweight functions, converting built-ins into lightweight functions

* Lightweight functions for user Duktape/C binding functions

* Magic value to share native code cheaply for multiple function objects

* Compressed pointers

Ecmascript function footprint
-----------------------------

* Motivation

  - Small lexically nested callbacks are often used in Ecmascript code,
    so it's important to keep their size small

* Reduce property count:

  - _pc2line: can be dropped, lose line numbers in tracebacks

  - _formals: can be dropped for most functions (affects debugging)

  - _varmap: can be dropped for most functions (affects debugging)

* Reduce compile-time maximum alloc size for bytecode: currently each
  instruction takes 8 bytes, 4 bytes for the instruction itself and 4 bytes
  for line number.  Change this into two allocations so that the maximum
  allocation size is not double that of final bytecode, as that is awkward
  for pool allocators.

* Improve property format, e.g. ``_formals`` is now a regular array which
  is quite wasteful; it could be converted to a ``\xFF`` separated string
  for instance.

* Spawn ``.prototype`` on demand to eliminate one unnecessary object per
  function

* Use virtual properties when possible, e.g. if ``nargs`` equals desired
  ``length``, use virtual property for it (either non-writable or create
  concrete property when written)

* Write bytecode and pc2line to flash during compilation

* Compressed pointers

Contiguous allocations
----------------------

Unbounded contiguous allocations are a problem for pool allocators.  There
are at least the following sources for these:

* Large user strings and buffers.  Not much can be done about these without
  a full rework of the Duktape C programming model (which assumes string and
  buffer data is available as plain ``const char *``).

* Bytecode/const buffer for long Ecmascript functions:

  - Bytecode and constants can be placed in separate buffers.

  - Bytecode could be "segmented" so that bytecode would be stored in chunks
    (e.g. 64 opcodes = 256 bytes).  An explicit JUMP to jump from page to page
    could make the executor impact minimal.

  - During compilation Duktape uses a single buffer to track bytecode
    instructions and their line numbers.  This takes 8 bytes per instruction
    while the final bytecode takes 4 bytes per instruction.  This is easy to
    fix by using two separate buffers.

* Value stacks of Duktape threads.  Start from 1kB and grow without
  (practical) bound depending on call nesting.

* Catch and call stacks of Duktape threads.  Also contiguous but since these
  are much smaller, they're unlikely to be a problem before the value stack
  becomes one.

Notes on function memory footprint
==================================

Normal function representation
------------------------------

In Duktape 1.0.0 functions are represented as:

* A ``duk_hcompiledfunction`` (a superset of ``duk_hobject``): represents
  an Ecmascript function which may have a set of properties, and points to
  the function's data area (bytecode, constants, inner function refs).

* A ``duk_hnativefunction`` (a superset of ``duk_hobject``): represents
  a Duktape/C function which may also have a set of properties.  A pointer
  to the C function is inside the ``duk_hnativefunction`` structure.

In Duktape 1.1.0 a lightfunc type is available:

* A lightfunc is an 8-byte ``duk_tval`` with no heap allocations, and
  provides a cheap way to represent many Duktape/C functions.

RAM footprints for each type are discussed below.

Ecmascript functions
--------------------

An ordinary Ecmascript function takes around 300-500 bytes of RAM.  There are
three objects involved:

- a function template
- a function instance (multiple instances can be created from one template)
- automatic prototype object allocated for the function instance

The function template is used to instantiate a function.  The resulting
function is not dependent on the template after creation, so that the
template can be garbage collected.  However, the template often remains
reachable in callback style programming, through the enclosing function's
inner function templates table.

The function instance contains a ``.prototype`` property while the prototype
contains a ``.constructor`` property, so that both functions require a
property table.  This is the case even for the majority of user functions
which will never be used as constructors; built-in functions are oddly exempt
from having an automatic prototype.

Duktape/C functions
-------------------

A Duktape/C function takes about 70-80 bytes of RAM.  Unlike Ecmascript
functions, Duktape/C function are already stripped of unnecessary properties
and don't have an automatic prototype object.

Even so, there are close to 200 built-in functions, so the footprint of
the ``duk_hnativefunction`` objects is around 14-16kB, not taking into account
allocator overhead.

Duktape/C lightfuncs
--------------------

Lightfuncs require only a ``duk_tval``, 8 bytes.  There are no additional heap
allocations.
