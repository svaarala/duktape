=========================
Duktape 2.3 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* duk_xxx_literal() API call variants which take a plain C literal argument,
  for example duk_get_prop_literal(ctx, -2, "myProperty").  The calls are
  conceptually similar to the duk_xxx_string() variants, but can take advantage
  of the fact that e.g. string length for a C literal can be determined at
  compile time using sizeof("myProperty") - 1 (the -1 is for NUL termination).
  Literal strings used by application code are also automatically pinned
  between mark-and-sweep rounds, and a small lookup cache makes mapping a C
  literal to a heap string object quite fast (almost as fast as using a heapptr).
  For now the calls are experimental.

* More ES2015 support: Symbol.hasInstance, Symbol.toStringTag,
  Symbol.isConcatSpreadable, Symbol.toPrimitive, Proxy improvements,
  Number.EPSILON, Number.MIN_SAFE_INTEGER, Number.MAX_SAFE_INTEGER,
  Number.isFinite(), Number.isNaN(), Number.isSafeInteger(),
  Number.parseInt(), Number.parseFloat().

* Other API additions: duk_random(), duk_push_new_target(),
  duk_get_global_heapptr(), duk_put_global_heapptr().

* When C++ exception support is enabled (DUK_USE_CPP_EXCEPTIONS), Duktape now
  uses a C++ exception throw also for fatal errors (e.g. uncaught error).  The
  exception thrown has the type ``duk_fatal_exception`` which inherits from
  ``std::runtime_error`` so it has a ::what() method and a useful message.

* DUK_USE_ALIGN_BY now always defaults to 8 (natural alignment) to avoid any
  potentially unsafe assumptions about compiler behavior for unaligned memory
  accesses and pointers (which may be an issue even on x86).

* A new CBOR encoder/decoder extra which may be eventually merged (in some
  form) into Duktape itself.  CBOR is a useful binary serialization format
  which is a superset of JSON and has an RFC specification.

* A Promise polyfill which will be used as a basis for the initial native
  implementation.

* Various fixes and portability improvements.

Upgrading from Duktape 2.2
==========================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v2.2.x.  Note the following:

* If you are using DUK_USE_CPP_EXCEPTIONS note that fatal errors are now
  thrown using a C++ exception of the type ``duk_fatal_exception`` which
  inherits from ``std::runtime_error`` and will be caught by a boilerplate
  ``std::exception`` catch.  In previous versions uncaught errors would
  propagate out as ``duk_internal_exception``\s, while assertions would
  default to ``abort()``.  As before, it is unsafe to continue after catching
  a ``duk_fatal_exception``.  You can override the new behavior by:

  - Providing an explicit fatal error handler in heap creation.  This affects
    heap related fatal errors (like uncaught exceptions), but won't affect
    fatal errors without a heap context (like assertions).

  - Providing an explicit default fatal error handler using the
    ``DUK_USE_FATAL_HANDLER`` config option.  This affects both types of
    fatal errors.

* If performance matters, you might consider using duk_xxx_literal() variants
  in place of duk_xxx_string() variants when the argument is a C literal.
  With literal pinning and the literal lookup cache this improves property
  access performance around 20% with minimal application changes.  Make sure
  that the arguments are C literals, e.g. duk_get_prop_literal(ctx, "mykey")
  and not pointers to strings like duk_get_prop_literal(ctx, strptr).

* If you're using the duk_xxx_heapptr() API call variants, you might consider
  switching to the duk_xxx_literal() variants.  They are less error prone, and
  with literal pinning and the literal lookup cache almost as fast as using a
  borrowed heap pointer.

* If you're working with a low memory target or any other target where memory
  usage matters, you may want to ensure UDK_USE_LITCACHE_SIZE is undefined in
  configure.py configuration (this is included in low_memory.yaml base config).

* If you're working with a low memory target or any other target where memory
  usage matters, you may want to force DUK_USE_ALIGN_BY to a lower value
  (4 or 1) to reduce memory overhead.  For most targets the memory overhead
  is not important.

* Base-64 and hex encoding/decoding support is now configurable and disabled
  by default in the example low_memory.yaml configuration.  Enable them
  manually if necessary using DUK_USE_BASE64_SUPPORT and DUK_USE_HEX_SUPPORT.

* The built-in base64 decoder is now more lenient.  If you're relying on
  strictness or specific behavior of the base64 decoder, you should use an
  external decoder with the exact behavior desired (base64 decoders differ
  quite a lot with respect to various decoding corner cases).

* Several -fsanitize=undefined warnings have been fixed in the default
  configuration using explicit checks to avoid undefined behavior.  For
  example, floating point division by zero is avoided and behavior in that
  case is implemented explicitly, at some cost in footprint and performance.
  For many compilers the undefined behavior assumptions in Duktape source
  are fine, and you can remove the extra overhead by enabling the
  DUK_USE_ALLOW_UNDEFINED_BEHAVIOR option in configure.py (this option is
  enabled in the low_memory.yaml and performance_sensitive.yaml example
  configurations).  Note, however, that recent gcc/clang versions are
  optimizing around undefined behavior so while relying on undefined behavior
  may work in one version, it may break with newer compiler versions.

* The console extra (extras/console) now by default uses ``stdout`` for log
  levels up to info, and ``stderr`` for warn and above.  To restore previous
  behavior (stdout only) use the ``DUK_CONSOLE_STDOUT_ONLY`` flag in
  ``duk_console_init()``.
