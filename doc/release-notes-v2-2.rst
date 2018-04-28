=========================
Duktape 2.2 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* Internal reworking of call handling for better performance and code sharing.
  Coroutine yield and tail call restrictions removed when using new Foo(),
  .call(), .apply(), Reflect.apply(), and Reflect.construct().  Maximum call
  argument count increased to ~64k.

* C API additions: duk_seal(), duk_freeze(), duk_is_constructable(),
  duk_require_object(), duk_push_proxy(), macros for creating symbol
  literals in C code (DUK_HIDDEN_SYMBOL("myValue") etc), and more
  duk_def_prop() convenience flags.  The 0xFF byte prefix is now reserved
  entirely for user hidden Symbols, so there are no longer restrictions in
  what follows the prefix.

* More ES2015 features: Math.clz32(), Math.imul(), Math.sign(),
  Object.prototype.{__defineGetter__,__defineSetter__},
  Object.prototype.{__lookupGetter__,_lookupSetter__}, Proxy 'apply' and
  'construct' traps, minimal new.target, and fixed string/symbol key sorting.

* Performance.now() binding and a monotonic time provider.

* Case insensitive RegExp character class canonicalization performance has
  improved by ~50x using a small lookup table (256 bytes, total footprint
  impact is ~300-400 bytes).

* Performance, footprint, and portability improvements.  Also improvements
  to error messages, value summaries, and assertion coverage.

Upgrading from Duktape 2.1
==========================

No action (other than recompiling) should be needed for most users to upgrade
from Duktape v2.1.x.  Note the following:

* There are public API macros to create different Symbol types as C literals.
  For example, DUK_HIDDEN_SYMBOL("myPointer") can now be used instead of
  manually creating the internal representation ("\xFF" "myPointer").

* Bytecode dump format has been changed slightly: initial byte is now 0xBF
  (previously 0xFF) to avoid potential confusion with Symbol strings, and
  serialization version byte (in practice unused) was removed from the format.
  If any application code checks for bytecode files using a 0xFF initial byte
  check, it will need to be updated to check for 0xBF.

* The typedef for duk_bool_t was changed from duk_small_int_t (typically
  'int') to duk_small_uint_t (typically 'unsigned int').  API constants for
  DUK_TYPE_xxx, DUK_TYPE_MASK_xxx, flags, etc were changed to unsigned
  (e.g. '(1U << 3)) to match their C type.  These changes may cause some
  sign conversion warnings in application call sites.

* duk_safe_call() no longer automatically extends the value stack to ensure
  there's space for 'nrets' return values.  This was not guaranteed by the
  API and the check is mostly unnecessary overhead.  If a duk_safe_call()
  call site now fails due to this change, simply ``duk_require_stack()``
  to ensure that there's reserve for ``nargs - nrets`` more elements
  (conceptually 'nargs' arguments are consumed, and 'nrets' result values
  pushed).

* Call stack (including both activation records and catcher records) is no
  longer a resized monolithic allocation which improves memory behavior for
  very low memory targets.  If you're using a pool allocator, you may need to
  measure and adjust pool sizes/counts.

* Function.prototype.call(), Function.prototype.apply(), Reflect.apply(),
  new Xyz(), duk_new(), and Reflect.construct() are now handled inline in call
  handling.  As a result, they are not part of the call stack, are absent in
  tracebacks, don't consume native stack for ECMAScript-to-ECMAScript calls,
  no longer prevent a coroutine yield, and can be used in tail call positions
  (e.g. 'return func.call(null, 1, 2);').

* Functions pushed using duk_push_c_function() and duk_push_c_lightfunc() now
  inherit from an intermediate prototype (func -> %NativeFunctionPrototype%
  -> Function.prototype) which provides ``.name`` and ``.length`` getters.
  The setters are intentionally missing so direct writes for these properties
  fail, but you can write them using ``Object.defineProperty()`` or
  ``duk_def_prop()``.  The inherited getters can also be replaced if necessary.
  The intermediate prototype doesn't have a named global binding, but you can
  access it by reading the prototype of a pushed function.

* The bound 'this', bound arguments, and target of a duk_hboundfunc are no
  longer internal properties (but duk_hboundfunc struct members).  The 'this'
  binding, target, and bound argument count are now visible as artificial
  properties; the bound argument values are not visible in the debugger
  protocol for now.

* The Proxy target and handler references are no longer internal properties
  (but duk_hproxy struct members), and are not visible in the debugger
  protocol for now.

* DUK_USE_DATE_GET_NOW() is now allowed to return fractions.  The fractions
  won't be available through the Date built-in (this is forbidden by the
  ECMAScript specification) but are available through the duk_get_now() C
  API call.  The default POSIX and Windows Date providers now return fractions,
  so duk_get_now() call sites may now get fractional millisecond timestamps
  even in default configuration.

* Debugger StepInto, StepOver, and StepOut are now accepted also when the
  current function doesn't have line information (i.e. it is native).  The
  step commands will still pause on function entry/exit as appropriate; for
  example, StepInto will pause on function entry or exit (or an error throw).

* Case insensitive RegExps now perform better by default, with the small
  canonicalization lookup (~300-400 bytes, DUK_USE_REGEXP_CANON_BITMAP)
  enabled by default.  The small lookup still performs slower than
  DUK_USE_REGEXP_CANON_WORKAROUND but the difference is smaller, and you
  may be able to turn off the workaround option whose main downside is a
  relatively large footprint impact (128kB).

* When an Error instance is being constructed and Duktape.errCreate() is
  called for the constructor return value, the call stack seen by errCreate()
  now includes the constructor call (previously it was unwound before calling
  errCreate()).  This affects e.g. any Duktape.act() calls in errCreate().
