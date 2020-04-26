===========
Code issues
===========

This document covers C coding issues related to Duktape implementation
such as:

* Conventions
* Portability concerns
* Specific platforms and compilers
* Size and performance optimization issues

Some code conventions are checked by the ``make codepolicycheck`` target.

Basic conventions
=================

Indentation, naming, etc.
-------------------------

Indent with tab.  On continuation lines indent with tab to shared indent
depth and then indent with spaces.  For example, denoting tab indent with
colon and space indent with period::

  ::::::::snprintf(buf,
  ::::::::.........sizeof(buf), 
  ::::::::........."%d",
  ::::::::.........123);

Names are lowercase, underscore separated::

  void duk_func(void) {
          /* ... */
  }

Local functions, arrays, structs, typedefs, etc. have a double underscore
after "duk"::

  typedef int duk__temptype;

  static void duk__frobnicate_helper(void) {
          /* ... */
  }

The prefix is useful when looking at object files to clearly identify an
internal symbol as originating from Duktape.  It will also show up in
debugger tracebacks and such.

Macros are uppercase, underscore separated::

  #define DUK_MACRO(x)  /* ... */

Macro names must not begin with an underscore.  Macros which are of local
interest only can have a local name or have a double underscore after "DUK"::

  /* 'foo' alternatives, not to be used directly */
  #define DUK__FOO_ALT1  /* ... */
  #define DUK__FOO_ALT2  /* ... */

  /* select DUK_FOO provider */
  #define DUK_FOO  DUK_FOO_ALT2

There is only one space after a ``#define``, ``#if``, etc., but there may be
multiple spaces between the a macro name and its definition.  There is no
strict rule on the alignment of a macro value; successive definitions usually
keep values in the same column.

Comments are always traditional C comments, never ``//``, as they are not
portable to older compilers::

  /* always used traditional C comments */

Opening brace on the same line as the start of the construct, even
for functions::

  void func(int x) {
          if (x) {
                  /* ... */
          } else {
                  /* ... */
          }
  }

The case-statements of a switch are at the same level as the switch
to reduce indent.  If case clauses have their own blocks, this leads
to a confusing closing brace, so a comment for that may be in order::

  switch (x) {
  case A: {
          /* ... */
          break;
  }
  case B: {
          /* ... */
          break;
  }
  default: {
  }
  }  /* switch */

Space after ``if``, ``switch``, etc::

  if (x) { ... }   /* correct */
  if(x) { ... }    /* incorrect */

  switch (x) { ... }  /* correct */
  switch(x) { ... }   /* incorrect */

Use of goto for error cleanup and shared error handling is not only
allowed but encouraged.  Some goto notes:

* Avoid goto to an inner block as that might have portability impact.

* Jumping to skip blocks should be used only when it saves considerable
  indentation.

No naked statements in e.g. ``if-then-else``, always use a block.
This is more macro compatible.  Example::

  if (x) {
          return 1;  /* correct */
  }

  if (x)
          return 1;  /* incorrect */

Multi-statement macros should use a ``do-while(0)`` construct::

  #define FROBNICATE(x,y)  do { \
                  x = x * x; \
                  y = y * y; \
          } while (0)

When the body of a macro is sometimes empty, use an empty do-while so that
the macro still yields a statement::

  #if defined(DUK_USE_FROB)
  #define FROBNICATE(x,y)  do { \
                  x = x * x; \
                  y = y * y; \
          } while (0)
  #else
  #define FROBNICATE(x,y)  do { } while (0)
  #endif

Use parentheses when referring to macro arguments and the final macro
result to minimize error proneness::

  #define MULTIPLY(a,b)  ((a) * (b))

  /* Now MULTIPLY(1 + 2, 3) expands to ((1 + 2) * (3)) == 9, not
   * 1 + 2 * 3 == 7.  Parentheses are used around macro result for
   * similar reasons.
   */

Labels are intended by one space relative to the parent tab depth::

  DUK_LOCAL void duk__helper(duk_hthread *thr) {
          if (!thr) {
                  DUK_D(DUK_DPRINT("thr is NULL"));
                  goto fail;
          }

          return;

   fail:
          DUK_D(DUK_DPRINT("failed, detaching"));
  }

Comment styles
--------------

A block or "banner" comment is used in file headers and to distinguish logical
sections containing (typically) multiple functions, definitions, variables, etc.::

    /*
     *  First line is empty and there are two spaces between the star
     *  characters and text.
     *
     *  There are two spaces after a period ending a sentence.  This is
     *  used throughout the Duktape code base and documentation.
     */

A compact comment is typically used to describe a single function/variable,
or a sequence of small defines grouped together::

    /* Text starts on the first line with a capital letter.  There's only
     * one space between a star and the text.  Text ends with a period.
     */

    /* Can also be a single line. */
    static void duk__helper(void) {
            /* ... */
    }

A compact comment may also appear intended inside a function.  The style is
the same::

    static void duk__helper(char *values, int count) {
            int i;

            /* Frobnicate all the elements in the user supplied
             * list of values.
             */
            for (i = 0; i < count; i++) {
                /* ... */
            }
    }

If a comment doesn't begin with a capital letter, it also doesn't have an
ending period (i.e. the text is not a sentence)::

    static void duk__helper(char *values, int count) {
            int i;

            /* frobnicate values */
            for (i = 0; i < count; i++) {
                /* ... */
            }
    }

A comment on the same line as a statement is separate by two spaces.  Don't
use C++ style comments, as they're not portable::

    static void duk__helper(char *values, int count) {
            int i;  /* loop counter */

            /* ... */

            return;  /* No return value. */
    }

The text in the comment may be a sentence (``/* No return value. */``, ends
in a period) or not (``/* no return value */``, no period).

Local variable declarations
---------------------------

C variables should only be declared in the beginning of the block.  Although
this is usually not a portability concern, some older still compilers require
it.  In particular, MSVC (at least Visual Studio 2010 Express) seems to
require this.

Be careful especially of assertions, debug prints, and other macros::

  int x, y;
  DUK_UNREF(y);
  int flags = 0;  /* problem: DUK_UNREF() */

Note that even **disabled** debug prints break the variable declaration
part because disabled debug prints are replaced with ``do {} while (0)``
(this is intentional to flush out this kind of errors even in release
builds)::

  {
          int x;

          DUK_DDD(DUK_DDDPRINT("debug print"));

          int y;  /* error here */

          x = 123;
          ...
  }

The fix is::

  {
          int x;
          int y;

          DUK_DDD(DUK_DDDPRINT("debug print"));

          x = 123;
          ...
  }

Local variable naming
---------------------

Variables are generally lowercase and underscore separated, but no strict
guidelines otherwise.

Avoid local variable names which might shadow with global symbols defined in
platform headers (not just one platform but potentially any platform).  For
example, using ``alloc`` would be a bad idea, and ``index`` also causes
concrete problems with some GCC versions.  There are a few blacklisted
identifiers in the code policy check.

Other variable declarations
---------------------------

Use symbol visibility macros throughout.

For DUK_INTERNAL_DECL macro use a DUK_SINGLE_FILE wrapper check to avoid
both declaring and defining a static variable (see GH-63)::

  /* Header: declare internal variable visible across files. */
  #if !defined(DUK_SINGLE_FILE)
  DUK_INTERNAL_DECL int duk_internal_foo;
  #endif  /* !DUK_SINGLE_FILE */

  /* Source: define the variable. */
  DUK_INTERNAL int duk_internal_foo;

Function declarations and definitions
-------------------------------------

For functions with a small number of arguments::

  DUK_INTERNAL_DECL void foo(duk_hthread *thr, duk_idx_t idx);

In definition opening brace on same line::

  DUK_INTERNAL void foo(duk_hthread *thr, duk_idx_t idx) {
          /* ... */
  }

If there are too many arguments to fit one line comfortably, symbol
visibility macro (and other macros) on a separate line, arguments
aligned with spaces::

  DUK_INTERNAL_DECL
  void foo(duk_hthread *thr,
           duk_idx_t idx,
           duk_uint_t foo,
           duk_uint_t bar,
           duk_uint_t quux,
           duk_uint_t baz);,

Again opening brace on the same line::

  DUK_INTERNAL
  void foo(duk_hthread *thr,
           duk_idx_t idx,
           duk_uint_t foo,
           duk_uint_t bar,
           duk_uint_t quux,
           duk_uint_t baz) {
          /* ... */
  }

Function calls with many difficult-to-identify arguments
--------------------------------------------------------

Example helper::

  duk_bool_t frob(duk_hthread *thr, int allow_foo, int allow_bar, int allow_quux);

Such helpers lead to call sites which are difficult to read::

  duk_bool_t rc = frob(thr, 1, 0, 1);

In such cases, inline comments can be used to clarify the argument names::

  duk_bool_t rc = frob(thr, 1 /*allow_foo*/, 0 /*allow_bar*/, 1 /*allow_quux*/);

Include guards
--------------

There are several popular include guard conventions.  Leading underscores
are reserved and should be avoided in user code.  The current include guard
convention is::

  /* duk_foo.h */

  #if !defined(DUK_FOO_H_INCLUDED)
  #define DUK_FOO_H_INCLUDED

  ...

  #endif  /* DUK_FOO_H_INCLUDED */

See:

* http://en.wikipedia.org/wiki/Include_guard

``#pragma once`` is not portable, and is not used.

Preprocessor value comparisons with empty arguments must be avoided
-------------------------------------------------------------------

This will cause a compile error even with newer compilers::

  /* FOO and BAR are defined, BAR is defined with an empty value. */
  #define FOO 123
  #define BAR

  #if defined(FOO) && defined(BAR) && (FOO == BAR)
  /* ... */
  #endif

It doesn't help to guard the comparison because the root cause is the
comparison having an empty argument::

  #define FOO 123
  #define BAR

  #if defined(FOO) && defined(BAR)  /* will match */
  #if (FOO == BAR)  /* still fails */
  /* ... */
  #endif
  #endif

The "guarded" form above is still preferred because it works also with
compilers which fail a comparison with an undefined value.

Explicitly detecting an empty value seems difficult to do properly, so
there doesn't seem to be an easy way to avoid this:

* http://stackoverflow.com/questions/3781520/how-to-test-if-preprocessor-symbol-is-defined-but-has-no-value

The comparison is not an issue in Duktape internals when comparing against
**required config options**.  This is safe, for example::

  #if (DUK_USE_ALIGN_BY == 8)
  /* ... */
  #endif

The comparison is a concrete issue in ``duk_config.h`` where the defines
provided by the environment vary a great deal.  See for example:

* https://github.com/judofyr/duktape.rb/pull/33#issuecomment-159488580

Preprocessor ifdef vs. if defined
---------------------------------

This form is preferred::

  #if defined(FROB)
  ...
  #endif

instead of::

  #ifdef FROB
  ...
  #endif

FIXME, TODO, XXX, NOTE, etc markers
-----------------------------------

The following markers are used inside comments:

FIXME:
  Issue should be fixed before a stable release.  Does not block
  an intermediate release.

TODO:
  Issue should be fixed but does not block a release (even a stable
  one).

XXX:
  Like TODO, but it may be unclear what the proper fix is.

NOTE:
  Noteworthy issue important for e.g. maintenance, but no action needed.

SCANBUILD:
  Scan-build note: describe why a warning is produced for warnings that
  cannot be easily fixed or silenced.

The markers must appear verbatim and be followed by a colon without
any space in between.  This is important so that the markers can be
grep'd.  Example::

  /* FIXME: foo should have a different type */

Unused variables
----------------

Suppressing unused variable warnings use the following macro::

  DUK_UNREF(my_unused_var);

Internally, this currently uses the form::

  (void) my_unused_var;  /* suppress warning */

This seems to work with both GCC and Clang.  The form::

  my_unused_var = my_unused_var;  /* suppress warning */

works with GCC but not with Clang.

Unreachable code and "noreturn" functions
-----------------------------------------

Noreturn functions must have a void return type and are declared as::

  DUK_NORETURN(void myfunc(void));

The macro style is awkward but is not easy to implement in another way.

Unreachable points in code are declared as::

  DUK_UNREACHABLE();

Likely/unlikely comparisons
---------------------------

Providing "branch hints" may provide benefits on some platforms but not on
others.  ``DUK_LIKELY()`` and ``DUK_UNLIKELY()`` can always be used in code,
and will be defined as a no-op if using branch hints on the target platform
is not possible or useful.

``DUK_UNLIKELY()`` should be used at least for conditions which are almost
never true, like invalid API call arguments, string size overflows, etc::

  if (DUK_UNLIKELY(ptr == NULL)) {
          /* ... */
  }

Similarly, ``DUK_LIKELY()`` should be used for conditions which are almost
always true::

  if (DUK_LIKELY(ptr != NULL)) {
          /* ... */
  }

The argument to these macros must be an integer::

  /* correct */
  if (DUK_LIKELY(ptr != NULL)) {
          /* ... */
  }

  /* incorrect */
  if (DUK_LIKELY(ptr)) {
          /* ... */
  }

Inlining control
----------------

For the vast majority of functions it's unnecessary to force a specific
inlining behavior (which is compiler specific).  There are a few inlining
control macros that can be applied when necessary for performance or code
size.

Inline control macros are applied to function definition, not declaration::

    /* Declaration */
    DUK_INTERNAL_DECL duk_foo(...);

    /* Definition */
    DUK_INTERNAL DUK_ALWAYS_INLINE duk_foo(...) {
            ...
    }

Applying inline control in the declaration causes issues with e.g. gcc.

C++ compatibility
-----------------

The source code is meant to be C++ compatible so that you can both:

1. Compile Duktape with C but use it from C++.

2. Compile Duktape with C++ and use it from C++ (preferred when
   using C++).

To achieve this:

* Avoid variable names conflicting with C++ keywords (``throw``,
  ``class``, ``this``, etc).

* Use explicit casts for all pointer conversions.

* Make sure there are no ``static`` forward declarations for *data symbols*,
  see symbol visibility section.

Debug macros
------------

Debug macros unfortunately need double wrapping to deal with lack of variadic
macros on pre-C99 platforms::

  DUK_D(DUK_DPRINT("foo"));
  DUK_DD(DUK_DDPRINT("bar"));
  DUK_DDD(DUK_DDDPRINT("quux"));

The outer and inner defines must match in their debug level.  On non-C99
platforms the outer macro allows a debug log write to be omitted entirely.
If the log writes are not omitted, the workaround for lack of variadic
macros causes a lot of warnings with some compilers.  With this wrapping,
at least the non-debug build will be clean on non-C99 compilers.

Gcc/clang -Wcast-align
----------------------

When casting from e.g. a ``duk_uint8_t *`` to a struct pointer clang will
emit a warning when ``-Wcast-align`` is used; see ``misc/clang_cast_align.c``
and https://github.com/svaarala/duktape/issues/270.

One fix is to change the original pointer being cast into a ``void *`` from
a char/byte-based pointer (e.g. ``duk_uint8_t *``)::

  void *p = DUK_FICTIONAL_GET_BUFFER_BASE(...);
  struct dummy *dummy = (struct dummy *) p;

However, this doesn't work well when pointer arithmetic on the pointer is
needed; pointer arithmetic on a void pointer works on many compilers but
is non-standard, non-portable behavior.  Instead, raw (byte-based) pointer
arithmetic should be done on a char/byte pointer (e.g. ``duk_uint8_t *``).
In such situations casting through a ``void *`` avoids the warning::

  duk_uint8_t *p = DUK_FICTIONAL_GET_BUFFER_BASE(...);
  struct dummy *dummy = (struct dummy *) (void *) (p + 16);

Code doing casts like this must of course be aware of actual target
alignment requirements and respect them properly.

Gcc/clang -Wcast-qual
---------------------

As a general rule casting from e.g. ``const char *`` to ``char *``
should be avoided by reworking code structure.  Sometimes this can't
be avoided though; for example, ``duk_push_pointer()`` takes a ``void *``
argument and if the source pointer is ``const char *`` a cast may be
necessary.

There doesn't seem to be a nice portable approach:

* Casting through a ``void *`` is not enough to silence the warning.

* Casting through an integer (e.g. ``(void *) (duk_uintptr_t) const_ptr``)
  works but assumes that pointers can be safely cast through an integer.
  This is not necessarily portable to platforms with segmented pointers.
  Also, ``(u)intptr_t`` is an optional type in C99.

If a const-losing cast is required internally, the following macro is used
to cast an arbitrary const pointer into a ``void *``::

  const my_type *src;

  dst = (char *) DUK_LOSE_CONST(src);

It is defined in ``duk_config.h`` so that it can be hacked if necessary.
If nothing else, it signals the intent of the call site.

A similar issue exists for volatile pointers.  Technically casting from a
volatile pointer to a non-volatile pointer and then using the non-volatile
pointer has "undefined behavior".  In practice the compiler may generate code
which conflicts with assumed behavior, e.g. not reading or writing the value
behind the pointer every time.  Rework the code to avoid the cast.  For
example::

  void write_something(int *target);

  void test(void) {
      volatile int x = 123;

      write_something((int *) &x);
  }

can be reworked to::

  void write_something(int *target);

  void test(void) {
      volatile int x = 123;
      int tmp;

      write_something(&tmp);
      x = tmp;
  }

For volatile byte arrays a workaround is awkward because you can't use a
non-volatile temporary and then ``memcpy()`` from the temporary into the
volatile buffer: a volatile-to-non-volatile cast would happen for the
``memcpy()`` call.  You'd need to copy the bytes one by one manually or
use an external helper which accepts a volatile source and a non-volatile
destination.

Gcc/clang -Wfloat-equal
-----------------------

When comparing floats for equality (``==``) or inequality (``!=``) there
are subtle portability issues.  For example, with x87 the compiler may use
extended precision (80 bits) even when the arguments are nominally IEEE
doubles.  Gcc/clang warn about such comparisons when ``-Wfloat-equal`` is
used.  Useful discussion:
https://randomascii.wordpress.com/2012/03/21/intermediate-floating-point-precision/

Duktape needs to do float equality comparisons in some cases, and when the
comparisons are done properly they're not an actual portability issue.
Unfortunately there doesn't seem to be an idiom which avoids the warning,
see: https://github.com/svaarala/duktape/issues/234.  So at present Duktape
code base is not ``-Wfloat-equal`` clean.

One workaround would be to implement all comparisons by looking at the IEEE
byte representation directly (using a union with double and byte array).
This is a rather heavy workaround though.

Avoid (u)intptr_t arithmetic
----------------------------

The ``(u)intptr_t`` types are optional in C99 so it's best to avoid using
them whenever possible.  Duktape provides ``duk_(u)intptr_t`` even when
they're missing.

Platforms/compilers with exotic pointer models may have unexpected behavior
when a pointer is cast to ``(u)intptr_t`` and then used in arithmetic or
binary operations.  For more details, see:

* https://github.com/svaarala/duktape/issues/530#issuecomment-171654860

* https://github.com/svaarala/duktape/issues/530#issuecomment-171697759

Arithmetic on integer cast pointer values may be needed e.g. for alignment::

    /* Align to 4. */
    while (((duk_size_t) (p)) & 0x03) {
        p++;
    }

**Don't** use ``duk_(u)intptr_t`` in such cases to avoid portability issues
with exotic pointer models::

    /* AVOID THIS */
    while (((duk_uintptr_t) (p)) & 0x03) {
        p++;
    }

Argument order
==============

Having a consistent argument order makes it easier to read and maintain code.
Also when the argument position of functions match it often saves some move
instructions in the compiled code.

Current conventions:

* The ``thr`` or ``heap`` argument, if present, is always first.

* For callbacks, a userdata argument follows ``thr`` or ``heap``; if neither
  is present, the userdata argument is first.  Same applies to user-defined
  macros which accept a userdata argument (e.g. pointer compression macros).

* When registering a callback, the userdata argument to be given in later
  callbacks is immediately after the callback(s) related to the userdata.

* Flags fields are typically last.

duk_context vs. duk_hthread
===========================

Both ``duk_context`` and ``duk_hthread`` typedefs resolve to
``struct duk_hthread`` so they're interchangeable.  Guidelines:

* Use ``duk_context *ctx`` only in public API declarations.

* Use ``duk_hthread *thr`` everywhere else, in all internals,
  including the definitions of public API functions.

Symbol visibility
=================

Symbol visibility issues
------------------------

There are several issues related to symbol visibility:

* Minimality: Duktape should only expose the function and data symbols that
  are used by calling programs.  This is a hygiene issue but also affects
  compiler optimization: if a function is internal, it doesn't need to conform
  to a rigid ABI, which allows some optimizations.  See e.g.
  https://gcc.gnu.org/wiki/Visibility.

* Single file vs. separate files: symbols need to be declared differently
  depending on whether Duktape is compiled from a single file source or
  multiple source files.

* Compiling Duktape vs. compiling application: some compiler attributes need
  to be set differently when compiling Duktape vs. compiling the application
  (see MSVC below).

* Compiler dependency: controlling link visibility of non-static symbols
  requires compiler specific mechanisms.

Symbol visibility macros
------------------------

All Duktape symbols are declared with one of the following prefix macros:

* ``DUK_EXTERNAL_DECL`` and ``DUK_EXTERNAL``: symbol is exposed to calling
  application.  May require compiler specific link specification.

* ``DUK_INTERNAL_DECL`` and ``DUK_INTERNAL``: symbol is internal to Duktape,
  but may reside in another compilation unit.  May require compiler specific
  link specification.

* ``DUK_LOCAL_DECL`` and ``DUK_LOCAL``: symbol is file local.  This maps to
  ``static`` and currently requires no compiler specific treatment.

As usual, ``duk_config.h`` defines these visibility symbols as appropriate,
taking into account both the compiler and whether Duktape is being compiled
from a single or multiple files.

Missing a visibility macro is not critical on GCC: it will just pollute
the symbol table.  On MSVC it can make break a DLL build of Duktape.

Avoid "static" forward declarations for data symbols
----------------------------------------------------

C++ does not allow a ``static`` variable to be both forward declared and
defined (see GH-63 for more discussion).  It's also not ideal for C and
is a potential portability issue.  This issue is avoided by:

* Not using ``DUK_LOCAL_DECL`` for local data symbols: it would always map
  to a ``static`` data declaration.

* Not using ``DUK_INTERNAL_DECL`` for data symbols when compiling from the
  single file distribution: such data symbols would map to ``static`` in
  the single file distribution (but not in the multiple files distribution
  where the declarations are needed).

The ``DUK_INTERNAL_DECL`` idiom is::

  #if !defined(DUK_SINGLE_FILE)
  DUK_INTERNAL_DECL const char *duk_str_not_object;
  #endif  /* !DUK_SINGLE_FILE */

For this to work in the single file case, ``tools/combine_src.py`` must
ensure that the symbol definition appears before its use.  This is currently
handled via manual file reordering.

Concrete example
----------------

As a concrete example, this is how these defines work with GCC 4.x.x.
For function declaration in header::

    /* Header file */
    DUK_EXTERNAL_DECL void foo(void);
    DUK_INTERNAL_DECL void foo(void);
    DUK_LOCAL_DECL void foo(void);

    /* Single file */
    __attribute__ ((visibility("default"))) extern void foo(void);
    static void foo(void);
    static void foo(void);

    /* Separate files */
    __attribute__ ((visibility("default"))) extern void foo(void);
    __attribute__ ((visibility("hidden"))) extern void foo(void);
    static void foo(void);

For the actual function declaration::

    /* Source file */
    DUK_EXTERNAL void foo(void) { ... }
    DUK_INTERNAL void foo(void) { ... }
    DUK_LOCAL void foo(void) { ... }

    /* Single file */
    __attribute__ ((visibility("default"))) void foo(void) { ... }
    static void foo(void) { ... }
    static void foo(void) { ... }

    /* Separate files */
    __attribute__ ((visibility("default"))) void foo(void) { ... }
    __attribute__ ((visibility("hidden"))) void foo(void) { ... }
    static void foo(void) { ... }

As seen from this example, different outcomes are needed for forward
declaring a symbol and actually defining the symbol.  For now, the same
macros work for function and data symbols.

MSVC DLL import/export
----------------------

For MSVC, DLL import/export attributes are needed to build as a DLL.
When compiling Duktape, public symbols should be declared as "dllexport"
in both header files and the actual declarations.  When compiling a
user application, the same header symbols must be declared as "dllimport".
The compilation context is available through ``DUK_COMPILING_DUKTAPE``.
For more on MSVC dllimport/dllexport, see:

* http://msdn.microsoft.com/en-us/library/y4h7bcy6.aspx

Shared strings
==============

Sharing of constant internal strings has multiple considerations:

* Some very old compilers won't share the same string value for multiple
  occurrences of the same literal string; newer compilers will treat such
  strings as ``const`` and share them.

* If strings are declared with explicit symbols which are referred to from
  code (explicit sharing), sharing is guaranteed but such strings may end
  up in a symbol table without some kind of compiler specific "linker script"
  (although for a combined duktape.c/duktape.h the strings can be declared
  static)::

    const char *shared_string = "shared string;

    /* foo.c */
    duk_push_sprintf(thr, "%s", shared_string);

    /* bar.c */
    sprintf(buf, "%s: %d", shared_string, 123);

* In low memory environments it may be desirable to simplify or shorten
  messages, or perhaps merge multiple strings into a more generic shared
  message (e.g. "parse error: invalid token", "parse error: expect lparen"
  could be mapped to "parse error").

The current approach for shared strings is as follows:

* Shared strings are referred to using macros in Duktape internals.  The
  macros begin with a ``DUK_STR_`` prefix::

    DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, DUK_STR_PARSE_ERROR);

* ``duk_strings.h`` provides the necessary macros and decides what string
  each macro maps to (depending on e.g. memory footprint target).  In case
  string literals are automatically shared by the compiler, the preferred
  definition may be e.g.::

    #define DUK_STR_PARSE_ERROR "parse error"

  If not, an explicit shared string may be better::

    /* Note: the extern should be rewritten to "static" in a single
     * file distributable.
     */

    #define DUK_STR_PARSE_ERROR duk_str_parse_error
    extern const char *duk_str_parse_error;

* ``duk_strings.c`` contains the actual shared string values required by
  the macros (assuming the macros don't provide the strings directly).

The upsides include:

* Call sites are relatively clean.

* Footprint tuning is quite flexible.

* Message consistency is easier to achieve than by having strings in the
  call sites.

* Non-ASCII (EBCDIC) portability may be easier to achieve.

The downsides include:

* Conditional strings need to be conditional in ``duk_strings.c`` too.
  This easily becomes messy and easy to get wrong.  Unused strings are
  difficult to detect.  By using literal strings directly in ``duk_strings.h``
  this is not an issue (but requires a compiler that shares string
  constants).

* Format strings don't abstract entirely.  The arguments of a formatted
  call must match the format string, so whatever footprint variants are
  used, they must have the same argument list.  For example::

    "parse error, got: %d"

  cannot be replaced with a shared::

    "parse error"

  for this reason.

* Indirection obscures the strings emitted from each call site a bit, and
  makes the code less modular.

Portability concerns
====================

No variadic macros
------------------

Lack of variadic macros can be worked around by using comma expressions.
The ``duk_push_error_object()`` API call is a good example.  It needs to
capture the call site's ``__FILE__`` and ``__LINE__`` which needs some
macro expansions to be included in the function call arguments.

Without variadic macros it's defined as::

    DUK_EXTERNAL_DECL duk_idx_t duk_push_error_object_stash(duk_hthread *thr, duk_errcode_t err_code, const char *fmt, ...);
    /* Note: parentheses are required so that the comma expression works in assignments. */
    #define duk_push_error_object  \
            (duk_api_global_filename = __FILE__, \
             duk_api_global_line = (duk_int_t) (__LINE__), \
             duk_push_error_object_stash)  /* last value is func pointer, arguments follow in parens */

When you call it as::

    int err_idx = duk_push_error_object(thr, 123, "foo %s", "bar");

It gets expanded to::

    int err_idx = (duk_api_global_filename = __FILE__, \
                   duk_api_global_line = (duk_int_t) (__LINE__), \
                   duk_push_error_object_stash) (thr, 123, "foo %s", "bar");

The comma expression is evaluated in order performing the stash assignments.
The final expression is a function pointer (``duk_push_error_object_stash``),
and the parenthesized argument list is used to call the function.

Note that the parentheses around the comma expression are required.  This would
not work::

    int err_idx = duk_api_global_filename = __FILE__, \
                  duk_api_global_line = (duk_int_t) (__LINE__), \
                  duk_push_error_object_stash (thr, 123, "foo %s", "bar");

The problem is that ``__FILE__`` gets assigned to err_idx.

The limitation in this technique is the need to "stash" the file/line
information temporarily which is not thread safe unless the stash is
located e.g. in the ``duk_hthread`` or ``duk_heap`` structure.  (At least
up to Duktape 1.4.x the stashes for file/line are global and thus not
thread safe; the potential issues don't compromise memory safety though.)

Missing or broken platform functions
------------------------------------

Sometimes platform functions are missing, even when they're supposed to be
present.  For instance, a compiler might advertise as being C99 compliant
but lack some mandatory functions.

Sometimes platform functions may be present but broken.  For instance,
some old uclibc versions have a broken ``memcpy()`` but a working
``memmove()``.

Platform functions which cannot be referred to using function pointers
----------------------------------------------------------------------

On some platforms built-in functions may be defined as inline functions or
macros.  Any code which assumes that built-in functions can be used as
function pointers will then break.  There are some platform "polyfills"
which use macros in this way, and it seems that Microsoft VS2013 may behave
like this at least with some options.

This problem can be avoided by using explicit function wrappers when a
function pointer is needed::

  double duk__acos(double x) {
          return acos(x);
  }

  /* ... use duk__acos as a function pointer */

va_copy
-------

Duktape needs ``va_copy()`` to implement ``duk_push_sprintf()`` which needs
trial printing of a formatted string into a buffer whose required size is
not known beforehand.

Most vararg macros are C89 but ``va_copy()`` is C99 / C++11, so a replacement
is needed for older environments.  This replacement is difficult to implement
in a portable fashion because the type of ``va_list`` varies a lot.

Strict aliasing rules
---------------------

Strict aliasing rules and prohibition of dereferencing type-punned pointers
are good for portability so the implementation should adhere to the common
rules, e.g. use a union to convert between types.  Sometimes this is not
straightforward.  For instance, the indirect realloc approach currently in
use needs a getter callback to avoid type-punning.

Current goal is to compile and work without warnings even with strict
aliasing rules enforced.

Numeric types
-------------

This is a complicated topic covered in a separate section below.

Numeric constants
-----------------

For the most part the rules are simple:

* For signed values, use "L" if the value is at most 32 bits wide and "LL"
  if at most 64 bits wide (keeping in mind that 64-bit constants are not
  always available).

* For unsigned values, use "UL" and "ULL", similarly.

There is an interesting corner case when trying to define minimum signed
integer value constants.  For instance, trying to define a constant for
the minimum 32-bit signed integer as follows is non-portable::

  #define MIN_VALUE  (-0x80000000L)

Apparently the compiler will first evaluate "0x80000000L" and, despite being
a signed constant, determine that it won't fit into a signed integer so it
must be an unsigned value.  Applying a unary minus to this unsigned value
may then cause a warning and cause the negated value to be 0x80000000, i.e.
a positive value (this happens on at least 64-bit VS2010).

This may then result in very unintuitive behavior.  For instance::

  /* 'd' is an input double to be clamped */
  if (d < (double) MIN_VALUE) {
          return (duk_int_t) MIN_VALUE;
  }

The compiler will actually end up doing::

  if (d < (double) 0x80000000) {  /* positive! */
          return (duk_int_t) 0x80000000;
  }

Given zero as an input, the comparison will match (which is undesired), and
the return statement will also contain a positive constant which is coerced
to a signed integer.  Although the input to the coercion is unsigned, the
final result is -0x80000000.  So, zero would "clip" to -0x80000000.  This
actually caused a non-trivial lexer bug in practice.

There seem to be only bad alternatives for defining signed integer minimum
constants:

* ``(-0x7fffffffL - 1L)``: works, but constant will be computed and the
  C preprocessor won't necessarily be able to compare against it.

* ``((int) -2147483648.0)``: same problem as above

* ``(-0x80000000LL)``: works if 64-bit constants are available, but since
  this is not always the case, not really an option

Linux ``stdint.h`` seems to be using the first option::

  # define INT8_MIN               (-128)
  # define INT16_MIN              (-32767-1)
  # define INT32_MIN              (-2147483647-1)
  # define INT64_MIN              (-__INT64_C(9223372036854775807)-1)

The fix should be applied to at least 32-bit and 64-bit constants, but the
``stdint.h`` header also applies to 16-bit constants.

For now:

* Use a computed value for minimum signed int value for 16, 32, and 64 bit
  constants.

Also see:

* http://stackoverflow.com/questions/6728900/hexadecimal-constant-in-c-is-unsigned-even-though-i-used-the-l-suffix

Alignment
---------

Platforms vary in their alignment requirements:

* Some platforms cause an error ("bus error") when alignment requirements
  are violated.  Such platforms may have unaligned access instructions but
  unaligned accesses may need to be flagged to the compiler.

* Some platforms have slower unaligned accesses but which behave externally
  just like aligned accesses.  "Slower" may mean that an interrupt / trap
  handler is invoked, at a considerable penalty.

* Some platforms support aligned and unaligned accesses with more or less
  the same performance.

Alignment level may also vary, e.g. platform may require 4-byte alignment
for both 32-bit integers and IEEE doubles, or it may require 4-byte alignment
for 32-bit integers but 8-byte alignment for doubles, etc.

The user provided allocation functions are required to return memory aligned
in a way which matches platform requirements.  In particular, if the platform
requires 8-byte alignment for doubles, returned memory is required to be 8-byte
aligned (at least if the allocation size is 8 bytes or more).  This ensures
that single allocated structures are properly allocated by default.  It also
ensures that arrays of structures are properly aligned.  The C compiler will
pad a structure to ensure that proper alignment is kept in arrays too.  For
instance, if the platform requires 8-byte alignment and a struct contains a
double (8 bytes) and a 32-bit integer (4 bytes), the struct will be padded
from 12 bytes to 16 bytes to ensure that arrays of such structures work as
expected.

There are a few places in Duktape where alignment may still be broken.  They
are related to "byte packing tricks" which are necessary to maintain a small
footprint:

* Object property table must ensure that duk_tval values and pointer values
  are properly aligned.   This is a particular issue with duk_tval values on
  platforms which require 8-byte alignment.

* Buffer data after the ``duk_hbuffer_fixed`` header must be properly aligned.
  The ``duk_hbuffer_fixed`` structure always contains 4-byte elements but not
  necessarily 8-byte elements, so data following the structure is 4-byte aligned
  but not automatically 8-byte aligned.

* The ``duk_hstring`` struct contains 4-byte values so it guarantees 4-byte
  alignment for string data, but there is no guarantee of an 8-byte alignment.
  This is not necessary, as strings don't need a specific alignment on any
  known platform.

Forcing a struct size to a multiple of 4 or 8 can be done in a compiler
specific manner with pragmas or struct attributes.  The only somewhat
portable solution is to add a suitably sized dummy member to the end of
the struct (e.g. a ``duk_uint64_t`` to force the struct size to be a
multiple of 8) or somewhere inside the struct.  See ``duk_hbuffer.h`` for
a concrete example.

64-bit arithmetic
-----------------

Some compilers on 32-bit platforms may have 64-bit arithmetic problems
(this seems to be the case with VBCC for example).  There are also older
compiles with no 64-bit support at all.

Duktape must compile with only 32-bit operations if necessary, so
replacements are needed in the few places where 32 bits are not enough.

Array indexing
--------------

This is a common 64-bit portability bug::

  char *buf = /*something*/;
  uint32_t idx = /*something*/
  char *p;

  p = &buf[idx - 1];

The index computation happens using unsigned integers, so with ``idx == 0``
the index becomes 0xffffffffUL.  With 32-bit pointers adding this value to
the base (``buf``) is the same as subtracting one from the base.  But with
64-bit pointers, these two operations are not the same.

A safer expression, preferred in Duktape internals, is::

  p = buf + idx - 1;

See ``misc/arridx_unsigned.c`` for more concrete examples.

Integer overflows
-----------------

Signed integer overflows are undefined behavior:

* https://www.securecoding.cert.org/confluence/display/seccode/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow?showComments=false

At least unsigned overflow handling is important, as it is needed to make
"add with carry" etc convenient.

Detecting overflow in simple addition is straightforward when unsigned
integer type bit size is exact::

  duk_uint32_t x, y, z;
  /* ... */
  z = x + y;
  if (z < x) {
          /* Overflow: (z < x) or equivalently (z < y) cannot be true unless
           * overflow occurs.  This relies on unsigned overflow behavior and
           * an exact bit size for the type.
           */
  }

Detecting overflow in multiplication is a bit trickier.  This comes up
e.g. in array join/concat helper when it computes the combined size of
separators (separator_size times separator_count).  The check is easy
if a larger type is available::

  duk_uint32_t x, y, z;
  duk_uint64_t t;

  t = (duk_uint64_t) x * (duk_uint64_t) y;
  if (t >= (duk_uint64_t) LIMIT) {
          /* Overflow. */
  }
  z = (duk_uint32_t) t;

However, for portability a 64-bit type cannot (for instance) be assumed.
The following approach works without a larger temporary type, but is
conservative and may indicate overflow even when one wouldn't occur::

  /*
   * Basic idea:
   *
   *      x * y > limit     // limit is e.g. 2**32-1
   * <=>  x > limit / y     // y != 0
   * <=>  y > limit / x     // equivalent, x != 0
   *
   * When a truncating division is used on the right size, the result
   * is no longer equivalent:
   *
   *      x > floor(limit / y)  <==  x > limit / y   // not ==>
   *
   * Limit must fit into the integer type.
   */

  duk_uint32_t x, y, z;

  if (y != 0 && x > (duk_uint32_t) 0xffffffffU / y) {
          /* Probable overflow. */
  }
  z = x * y;

One can also simply test by division (but careful for division-by-zero)::

  z = x * y;
  if (x != 0 && z / x != y) {
          /* Overflow. */
  }

For 32-bit types the check is actually exact, see test in::

  misc/c_overflow_test.py 

Shifting
--------

With 32-bit integers the following may cause warnings on some compilers
when the value is used in conjunction with unsigned values (see
``duk_hobject.h``)::

  #define FOO(v) ((v) << 24)

Suppose ``v`` is 0x80 (signed constant).  The result of the shift now has
the highest bit (bit 31) set which causes the result to become unsigned.
This can be fixed e.g. as::

  #define FOO(v) (((duk_uint_t) (v)) << 24)

On a more general note, suppose a macro does::

  #define BAR(v) ((v) << N)

What is a plain value coerced to during shifting?  If the platform has 16-bit
integers, can it be coerced to a 16-bit integer, with the left shift then
overflowing?  If so, all such shifts would need to be replaced with::

  #define BAR(v) (((duk_uint_t) (v)) << N)

**This is not done now for shifts (as of Duktape 0.11.0).**

Switch statement
----------------

Any integral type should work as a switch argument, so avoid casting it.

Differences in malloc(), realloc(), and free()
----------------------------------------------

For malloc(), zero size is special:

* A malloc() with zero size cannot fail, but may return either NULL or a
  non-NULL pointer which must be free()'d.

* Safest strategy is to:

  - Consider malloc() failed if return value is NULL *and* requested size != 0.
    Otherwise consider successful.

  - If successful, store result pointer in internal data structures.  Ensure
    that eventually free() is called for the pointer; free(NULL) is acceptable
    so no guard is needed.

For free():

* free(NULL) is allowed and guaranteed to be a no-op.  Duktape relies on this.

For realloc(), zero size is special as well:

* A realloc() with a zero size cannot fail, but may return either NULL or a
  non-NULL pointer which must be free()'d.

* Safest strategy is to:

  - Consider realloc() failed if return value is NULL *and* requested size != 0.
    Otherwise consider successful.

  - If successful, store result pointer in internal data structures (replacing
    a previous value).  Ensure that eventually free() is called for the
    pointer; free(NULL) is acceptable so no guard is needed.

String handling
---------------

snprintf buffer size
::::::::::::::::::::

NUL terminator behavior for snprintf() (and its friends) is inconsistent
across implementations.  Some ensure a NUL terminator added when truncated
(unless of course the buffer size is zero) while others do not.
The most portable way seems to be to::
  
  char buf[256];
  snprintf(buf, sizeof(buf), "format", args);
  buf[sizeof(buf) - 1] = (char) 0;
  
Using sizeof(buf) - 1 for size may cause a NUL terminator to appear at
the second to last character of buf in some implementations.

Examples of snprintf() calls which don't NUL terminate on truncation:

* Windows ``_snprintf()``: http://msdn.microsoft.com/en-us/library/2ts7cx93.aspx

s(n)printf %s and NULL value
::::::::::::::::::::::::::::

Giving a NULL argument to ``%s`` format string may cause a segfault in some
old compilers.  Avoid NULL values for ``%s``.

Use of sprintf vs. snprintf
:::::::::::::::::::::::::::

Use snprintf instead of sprintf by default, even when legal output size is
known beforehand.  There can always be bugs in the underlying standard library
implementation.  Sometimes the output size is known to be limited because
input values are known to be constrained (e.g. year values are kept between
[-999999,999999]).  However, if there is a bug, it's better to corrupt a
printed output value than to cause a memory error.

EBCDIC
------

See separate section below.

Setjmp, longjmp, and volatile
=============================

Volatile variables
------------------

When a local variable in the function containing a ``setjmp()`` gets changed
between ``setjmp()`` and ``longjmp()`` there is no guarantee that the change
is visible after a ``longjmp()`` unless the variable is declared volatile.
It should be safe to:

* Use non-volatile variables that are written before ``setjmp()`` and then
  only read.

* Use volatile variables which can be read and written at any point.

When pointer values are changed, be careful with placement of "volatile"::

  /* Non-volatile pointer, which points to a volatile integer. */
  volatile int *ptr_x;

  /* Volatile pointer, which points to a non-volatile integer. */
  int * volatile x;

When a pointer itself may be reassigned, the latter is correct, e.g.::

  duk_hthread * volatile curr_thread;

  curr_thread = thr;

In practice it seems that some compilers have trouble guaranteeing these
semantics for variables that are assigned to before ``setjmp()`` and not
changed before ``longjmp()``.  For instance, there are crashes on macOS when
using ``_setjmp()`` in such cases.  These crashes can be eliminated by
declaring the variables volatile.  (It might be that adding the "volatile"
changes the compiler output enough to mask a different bug though.)

Optimizations may also cause odd situations, see e.g.:

* http://blog.sam.liddicott.com/2013/09/why-are-setjmp-volatile-hacks-still.html

With Emscripten a function containing ``setjmp()`` executes much more slowly
than a function without it.  For example, for the bytecode executor the speed
improvement of refactoring ``setjmp()`` out of the main executor function was
around 25%:

* https://github.com/svaarala/duktape/pull/370

Some compilers generate incorrect code with setjmp.  Some workarounds may be
needed (e.g. optimizations may need to be disabled completely) for functions
containing a setjmp:

* https://github.com/svaarala/duktape/issues/369

To minimize the chances of the compiler handling setjmp/longjmp incorrectly,
the cleanest approach would probable be to:

* Minimize the size of functions containing a ``setjmp()``; use a wrapper
  with just the ``setjmp()`` and an inner function with the rest of the
  function when that's possible.

* Declare all variables used in the ``setjmp()`` non-zero return case (when
  called through ``longjmp()``) as volatile, so that we don't ever rely on
  non-volatile variable values in that code path.

Because volatile variables are slow (explicit read/write operations are
generated for each access) it may be more practical to use explicit "save"
variables, e.g.::

  volatile int save_x;
  int x;

  if (setjmp(...)) {
          x = save_x;
          /* use 'x' normally */
          return;
  }

  /* Assume foo(), bar(), quux() never longjmp(). */
  x = foo();
  x += bar();
  x += quux();
  save_x = x;  /* Save before any potential longjmp(). */

  /* ... */

(As of Duktape 1.3 this has not yet been done for all setjmp/longjmp
functions.  Rather, volatile declarations have been added where they
seem to be needed in practice.)

Limitations in setjmp() call site
---------------------------------

There are limitations to what a ``setjmp()`` call site can look like,
see e.g.:

- https://www.securecoding.cert.org/confluence/display/c/MSC22-C.+Use+the+setjmp%28%29,+longjmp%28%29+facility+securely

This is fine for example::

  if (DUK_SETJMP(jb) == 0) {
          /* ... */
  }

But this is not::

  /* NOT OK */
  if (DUK_LIKELY(DUK_SETJMP(jb) == 0)) {
          /* ... */
  }

Setjmp and floating points
--------------------------

There may be limitations on what floating point registers or state is
actually saved and restored, see e.g.:

- http://www-personal.umich.edu/~williams/archive/computation/setjmp-fpmode.html

To minimize portability issues, floating point variables used in the setjmp
longjmp path should be volatile so that they won't be stored in registers.

Numeric types
=============

C data types, especially integer types, are a bit of a hassle: the best choice
of types depends on the platform and the compiler, and also the C specification
version.  Types also affect e.g. printf() and scanf() format specifiers which
are, of course, potentially compiler specific.  To remain portable, (almost)
all C types are wrapped behind a typedef.

The ``duktape.h`` header handles all platform and feature detection and provides
all necessary type wrappers, both for the public API and for internal use.

Preferred integer type with at least 32 bits
--------------------------------------------

A large amount of code needs an integer type which is convenient for the CPU
but still guaranteed to be 32 bits or more.  The ``int`` type is NOT a good
choice because it may be 16 bits even on platforms with a 32-bit type and
even 32-bit registers (e.g. PureC on M68K).  The ``long`` type is also not a
good choice as it may be too wide (e.g. GCC on x86-64, int is 32 bits while
long is 64 bits).

For this use, there are two typedefs:

* ``duk_int_t``: an integer convenient on the target, but always guaranteed
  to be 32 bits or more.  This may be mapped to ``int`` if it's large enough,
  or possibly ``int_fast32_t``, or something else depending on the target.

* ``duk_uint_t``: same but unsigned.

There are also typedefs for the case where a 32 bits or more are needed but
the types also need to be fastest for the CPU.  This is useful for true fast
paths like executor loops and such:

* ``duk_int_fast_t``: an integer fastest on the target, but always guaranteed
  to be 32 bits or more.  This is usually mapped to ``int_fast32_t`` when C99
  types are available.

* ``duk_uint_fast_t``: same but unsigned.

For cases where 16 bits are enough, the following wrapped types are provided
(they are essentially ``int`` and ``unsigned int`` but wrapped for consistency):

* ``duk_small_int_t``: an integer convenient on the target, guaranteed to be
  16 bits or more.

* ``duk_small_uint_t``: same but unsigned.

For these, too, there are fast variants:

* ``duk_small_int_fast_t``: an integer fastest of the target, guaranteed to be
  16 bits or more, usually mapped to ``int_fast16_t`` when C99 types are
  available.

* ``duk_small_uint_fast_t``: same but unsigned.

Exact 32-bit types are needed in some cases e.g. for ECMAScript semantics and
or guaranteeing portable overflow / underflow handling.  Also, 64-bit
arithmetic emulation (implemented on 32 bit types) relies on exact unsigned
overflows / underflows.  The wrapped C99 types are used in these cases.

Format specifiers
-----------------

Format specifiers are more or less standardized, e.g. ``%d`` is used to format
an ``int`` in decimal, but:

* When typedef wrappers are used, how can calling code know the correct format
  specifier for the wrapped type?  The target type may be differ between
  platforms.  In practice there are two reasonable strategies:

  1. Define preprocessor macros for the format specifiers (C99 uses this approach,
     e.g. ``PRId32``).

  2. Cast upwards to a reasonable guess, e.g. all signed integers to ``long``
     or (if C99 can be assumed) ``maxint_t`` (``unsigned long`` and ``umaxint_t``
     for unsigned integers) and use a known format specifier.

* There are separate format codes for ``printf()`` and ``scanf()``.  They are
  sometimes different.  As a concrete example, the proper print format code for
  an IEEE double is ``%f`` while the scan format code is ``%lf``.

  - Inside Duktape code, use ``%lf`` for the print format code: it's
    also an acceptable format and perhaps more clear

* Some useful portable format codes:

  - ``%s``: string, use ``(const char *)`` cast
  - ``%p``: pointer, use ``(void *)`` cast
  - ``%d``: int, use ``(int)`` cast
  - ``%u``: unsigned int, use ``(unsigned int)`` cast
  - ``%ld``: long, use ``(long)`` cast
  - ``%lu``: unsigned long, use ``(unsigned long)`` cast

* These are useful but unfortunately C99 (C++11):

  - ``%zu``: size_t (C99), use ``%lu`` and ``(unsigned long)`` cast instead
  - ``%jd``: maxint_t (C99), use ``%lu`` and ``(unsigned long)`` cast instead

* Format argument types, see e.g.:

  - http://www.gnu.org/software/libc/manual/html_node/Formatted-Output.html#Formatted-Output
  - http://www.gnu.org/software/libc/manual/html_node/Other-Output-Conversions.html#Other-Output-Conversions
  - http://www.gnu.org/software/libc/manual/html_node/Integer-Conversions.html#Integer-Conversions

Types used inside Duktape
-------------------------

* ``duktape.h`` performs all the detection needed and provide typedefs for
  types used in the public API and inside Duktape.

* C99 types are **not** used directly, wrapper types are used instead.  For
  instance, use ``duk_uint32_t`` instead of ``uint32_t``.  Wrapper types are
  used because we don't want to rely on C99 types or define them if they are
  missing.

* Only use ``duk_XXX_t`` typedefs for integer types unless there is a special
  reason not to.  For instance, if a platform API requires a specific type,
  that type must of course be used (or casted to).

* Integer constants should generally use ``L`` or ``UL`` suffix, i.e.
  makes them ``long int`` or ``unsigned long int``, and they are
  guaranteed to be 32 bits or more.  Without a suffix integer constants
  may be only 16 bits.  64-bit constants need ``LL`` or ``ULL`` suffix.
  Small constants (16 bits or less) don't need a suffix and are still
  portable.  This is convenient for codepoint constants and such.
  Note the absurd corner case when trying to represent the smallest signed
  integer value for 32 and 64 bits (see separate section).

* Integer constant sign should match the type the constant is related to.
  For instance, ``duk_codepoint_t`` is a signed type, so a signed constant
  should be used.  This is more than a style issue: suppose signed codepoint
  ``cp`` had value ``-1``.  The comparison ``(cp < 0x7fL)`` is true while
  the comparison ``(cp < 0x7fUL)`` is false because of C coercion rules.

* Value stack indices which are relative to the current activation use
  ``duk_idx_t``.  Value stack sizes and value stack indices related to the
  entire value stack are ``duk_size_t``.  In principle the value stack could
  be larger than 32 bits while individual activations could be limited to
  a signed 32 bit index space.

Formatting considerations
-------------------------

* Use standard format specifiers (``%d``, ``%p``, ``%s``, etc) instead of
  relying on compiler specific or C99 format specifiers: they may not be
  available on all platforms.

* Select a standard specifier which is guaranteed to be wide enough for
  the argument type and cast the argument explicitly to a matching type.

  - Casting all arguments explicitly is a compromise: an explicit cast removes
    some useful warnings but also removes some pointless warnings.  Since type
    detection ends up with different typing across platforms, the only way to
    format portably is to use a portable format specifier and an explicit cast;
    the format specifier/type must be chosen to be wide enough to match all
    possible type detection results.

* For integers, use ``long`` variants by default because it is guaranteed
  to be 32 bits or more:

  - ``%ld`` with ``(long)`` cast

  - ``%lu`` with ``(unsigned long)`` cast

  - ``%lx`` with ``(unsigned long)`` cast; there seems to be some variance
    whether a signed or unsigned cast should be used, GCC seems to expect
    an unsigned argument:

    + http://www.gnu.org/software/libc/manual/html_node/Integer-Conversions.html#Integer-Conversions

* For debug code, use ``long`` variants for all integers for simplicity,
  even for short fields like booleans.

* For release code using ``int`` variants (``%d``, ``%u``, ``%x``) is OK
  if a 16-bit range suffices.  It's probably nice to mention this in code
  so that there is no doubt.

* Selecting signed/unsigned variant for debug logs is not that critical, as most
  values don't use the full range.  The current code base contains both signed
  and unsigned formatting for e.g. lengths (which are never negative).

* Use ``%lf`` for IEEE doubles; ``%f`` is the other alternative.

* When using ``%c``, cast the argument explicitly with ``(int)`` (not ``(char)``).
  This is the "promoted type" expected, see e.g.:

  - http://www.gnu.org/software/libc/manual/html_node/Formatted-Output.html#Formatted-Output

* When using hexadecimal formats ``%lx`` (or ``%x``), cast the argument to an
  unsigned type (``unsigned long`` or ``unsigned int``).  There seems to be
  some variation between compilers whether they expect a signed or an unsigned
  argument.  GCC seems to expect an unsigned argument.

* Don't rely on ``%s`` accepting a NULL pointer, this breaks on some
  platforms.  Check pointer before formatting; if the string argument
  is obtained with Duktape API without an explicit NULL check (which is
  mostly preferable), use ``duk_require_string()`` instead of
  ``duk_get_string()``.

* For debug prints, the debug formatter special cases ``%s`` so that the
  platform never sees a NULL pointer with ``%s``.  NULL pointers can thus
  be safely debug logged with ``%s``.

* For debug custom formatting, use the following casts:

  - ``%!T`` and variants: ``(duk_tval *)``
  - ``%!O`` and variants: ``(duk_heaphdr *)``

duk_size_t
::::::::::

Use ``duk_size_t`` for internal uses of ``size_t``.  Coerce it explicitly
to ``size_t`` for library API calls.

duk_double_t
::::::::::::

Use ``duk_double_t`` for IEEE double precision float.  This is slight
paranoia but may be handy if e.g. built-in soft float library is introduced.

void
::::

The ``void`` type is used as is, cannot imagine a reason why it would need
to be reassigned for portability.

duk_int_t
:::::::::

Use ``duk_int_t`` as an ``int`` replacement; it behaves like an ``int`` but,
unlike ``int``, is guaranteed to be at least 32 bits wide.  Similarly
``duk_uint_t`` should be used as an ``unsigned int`` replacement.

duk_int_fast_t
::::::::::::::

This is a type at least the size of ``duk_int_t`` but which is guaranteed to
be a "fast" variant if that distinction matters for the CPU.  This type is
mainly used in the executor where performance really matters.  ``duk_uint_fast_t``
is used similarly.

duk_small_int_t
:::::::::::::::

The ``duk_small_int_t`` should be used in internal code e.g. for flags.
It is guaranteed to be 16 bits or more.  Similarly ``duk_small_uint_t``.

duk_small_int_fast_t
::::::::::::::::::::

Same as ``duk_small_int_t`` but guaranteed to be a fast variant.  Used mainly
for fast paths like the executor.  Similarly for ``duk_small_uint_fast_t``.

duk_bool_t
::::::::::

The ``duk_bool_t`` should be used for boolean values.  It must be wide
enough to accommodate results from C comparisons (e.g. ``x == y``).  In
practice it's defined as an ``int``.  (Currently some internal code uses
``duk_small_int_t`` for booleans, but this will be fixed.)

duk_uint8_t
:::::::::::

``duk_uint8_t`` should be used as a replacement for ``unsigned char`` and
often for ``char`` too.  Since ``char`` may be signed, it is often a
problematic type when comparing ranges, indexing lookup tables, etc, so
a ``char`` or a ``signed char`` is often not the best type.  Note that
proper string comparison of UTF-8 character strings, for instance, relies
on unsigned byte comparisons.

duk_idx_t
:::::::::

``duk_idx_t`` is used for value stack indices.

duk_arridx_t
::::::::::::

``duk_arridx_t`` is used for array indices.

Portability issues on very old compilers
========================================

Initialization of auto arrays
-----------------------------

Some old compilers (such as bcc) refuse to compile the following (error
message is something along the lines of: initialization of auto arrays
is illegal)::

  int myarray[] = { 123, 234 };

or even::

  int myarray[2] = { 123, 234 };

Apparently the following would be legal::

  static int myarray[2] = { 123, 234 };

The workaround is to use a static array or initialize explicitly::

  int myarray[2];

  myarray[0] = 123;
  myarray[1] = 234;

Initializer is too complicated (bcc)
------------------------------------

BCC complains about "initializer is too complicated" when a function pointer
array contains casts::

  ...
  (duk_c_function) my_function,
  ...

This works::

  ...
  my_function,
  ...

Non-integral selector in switch (bcc)
-------------------------------------

For some reason BCC fails to compile switch statements where the value is
obtained with a macro such as::

  switch (DUK_DEC_OP(ins)) {
          ...
  }

This is probably caused by the fact that ``DUK_DEC_OP(ins)`` is a 32-bit value
while BCC's integer type is 16 bits.  Switch argument needs to be ``int``, so
one needs to::

  switch ((int) DUK_DEC_OP(ins)) {
          ...
  }

Or perhaps (using type wrappers)::

  switch ((duk_small_int_t) DUK_DEC_OP(ins)) {
          ...
  }

Division by zero is a compile error
-----------------------------------

Attempting to create NaN or infinity values with expressions like ``0/0`` and
``1/0`` are treated as compile errors by some compilers (such as BCC) while
others will just replace them with an incorrect value (e.g. VBCC replaces them
with zero).  Run-time computed NaN / infinity values are needed on such platforms.

ULL integer constants may cause an error
----------------------------------------

The following may cause a compilation error (e.g. BCC)::

  #if defined(ULONG_MAX) && (ULONG_MAX == 18446744073709551615ULL)

The error happens even if ``ULONG_MAX`` is not defined.  Instead, this needs
to be restructured in one of several ways.  For instance, old compilers can be
rejected explicitly::

  #if defined(DUK_F_BCC)
  /* cannot check ULONG_MAX */
  #else
  #if defined(ULONG_MAX) && (ULONG_MAX == 18446744073709551615ULL)
  /* ... */
  #endif
  #endif

The important point is that the old compiler cannot process the preprocessor
line containing the integer constant; if it processes even part of the line,
it may choke on a syntax error.

Comments inside macro arguments may cause an error (BCC)
--------------------------------------------------------

The following causes an error on BCC::

  DUK_ASSERT(FOO ||   /* foo happens */
             BAR);

The comment causes BCC to produce an error like "incorrect number of macro
arguments".  The fix is to remove the comment from inside the macro::

  DUK_ASSERT(FOO ||
             BAR);

Character values in char literals and strings, EBCDIC
=====================================================

**FIXME: under work, while some other projects do support EBCDIC,
EBCDIC may not be a useful portability target for Duktape.**

Overview
--------

Character constants in C code are integers whose value depends on the
platform.  On the vast majority of platforms the constants are ASCII but
there are also e.g. EBCDIC platforms:

* http://en.wikipedia.org/wiki/EBCDIC#Codepage_layout

If you read a character value from a platform specific text file, then
code such as the following would be appropriate::

  if (c == 'x') {
          ...
  }

However, if you have a character value which must be interpreted as ASCII,
then the above would not be portable because ``'x'`` would not necessarily
have the value 120 ('x' in ASCII) but might have the value 167 ('x' in
EBCDIC).  To correctly compare the value as ASCII::

  if (c == 120) {
          ...
  }

The same applies to string constants, this would be unportable::

  const char *msg = "hello there";  /* content bytes depend on platform */

In practice the string terminator (NUL) seems to be guaranteed to have
a zero integer value.

In Duktape code we always deal with (extended) UTF-8 data, so we never have
the need to use platform specific character constants.  In other words, we
want the ASCII constant values.

Character literals
------------------

You should never use a character constant in Duktape code (e.g. ``'x'``).
Its value is not portable.  Use either an integer, or more preferably,
character constants (``DUK_ASC_xxx``) defined in Duktape internal headers.

String literals
---------------

C strings which end up visible to user code (either through ECMAScript
or through the C API) must be converted to UTF-8 at some point.

Ideally the strings would be written directly in UTF-8 (ASCII in practice)
format, but this would be very awkward.  The next best thing would be to
translate the strings with some sort of macro which would be a no-op on
ASCII platforms, e.g. ``DUK_STR("hello there")``.  This approach doesn't
work well: a buffer would need to be allocated (and freed) or some maximum
size imposed silently.

These rules are very inconvenient, but unfortunately the only portable choice.

**FIXME: exact code rules to be defined.**

Testing
-------

The Hercules emulator together with IBM zLinux provides an EBCDIC
platform where you can test this particular portability issue.

GCC can also be used to play with EBCDIC portability to some extent,
but because libc will be ASCII oriented, the tests will not match
an actual EBCDIC platform.  See ``misc/ebcdic_test.c``.

Calling platform functions
==========================

All platform function calls (ANSI C and other) are wrapped through macros
defined in ``duk_config.h``.  For example, ``fwrite()`` calls are made using
``DUK_FWRITE()``.

Many of these wrappers are not currently needed but some are, so it's simplest
to wrap just about everything in case something needs to be tweaked.  As an
example, on some old uclibc versions ``memcpy()`` is broken and can be
replaced with ``memmove()`` in ``duk_config.h``.

The only exception is platform specific Date built-in code.  As this code is
always platform specific and contained to the Date code, wrapping them is not
necessary or useful.  Any tweaks can be more comfortably applied directly in
the Date code.

The following can be used to find "leaks", accidental unwrapped calls::

  $ python util/find_func_calls.py src-input/*.c src-input/*.h | \
    grep -v -i -P ^duk_ | grep -v -P '^(sizeof|va_start|va_end|va_arg)' | \
    sort | uniq

Other considerations
====================

Const qualifiers for tables
---------------------------

Using ``const`` for tables allows tables to compiled into the text section.
This is important on some embedded platforms where RAM is tight but there
is more space for code and fixed data.

Config options
==============

All feature detection is concentrated into ``duk_config.h`` which detects
the compiler, platform, and architecture via preprocessor defines.

As a result, ``duk_config.h`` defines ``DUK_USE_xxx`` macros which enable
and disable specific features and provide parameter values (such as traceback
depth).  These are the **only** feature defines which should be used in
internal Duktape code.  The ``duk_config.h`` defines, especially typedefs,
are also visible for the public API header.

When adding specific hacks and workarounds which might not be of interest
to all users, add a ``DUK_USE_xxx`` flag metadata into the build.

Platforms and compilers
=======================

VBCC
----

Even in C99 mode VBCC 0.9b:

* Does not have ``inttypes.h``.

* Does not have ``fpclassify()`` and friends.

* Does not have ``NAN`` or ``INFINITY``.

* The expression ``0.0 / 0.0`` causes a warning and results in ``0.0``
  instead of ``NaN`` as expected.

* The expression ``1.0 / 0.0`` causes a warning and results in ``0.0``
  instead of infinity as expected.

The following program demonstrates the NaN issue::

  #include <stdio.h>

  void main(void) {
          double z = 0.0;
          double t;
          volatile union {
                  double d;
                  unsigned char b[8];
          } u;
          int i;
  
          /* this results in 0.0 */
          t = 0.0 / 0.0;
          printf("result: %lf\n", t);
  
          /* this results in NaN */
          t = z / z;
          printf("result: %lf\n", t);
      
          u.d = t;
          for (i = 0; i < 8; i++) {
              printf("%02x\n", u.b[i]);
          }
  }

To work with compiler optimization, the above approach needs to have the
``double`` values in ``volatile`` variables.  Otherwise VBCC will end up
replacing the result with zero.  So something like this is probably safest::

  volatile double a = 0.0;
  volatile double b = 0.0;
  double t = a / b;  /* -> NaN */

tcc
---

Tcc has trouble with negative zeroes.  See ``misc/tcc_zerosign1.c``.  For
instance:

* Assign d = 0.0

* Assign d = -d

* Now d should be a negative zero, but in tcc (with default options) it
  has not changed sign: the memory dump verified this, signbit() returns
  zero, etc.

This happens at least in tcc versions 0.9.25, 0.9.26.

clang
-----

Clang has some issues with union aliasing.  See ``misc/clang_aliasing.c``.

bcc
---

BCC is not a realistic compilation target at the moment but serves as a nice
"torture target".  Various issues have been documented above in portability
issues.

Resources
=========

* http://graphics.stanford.edu/~seander/bithacks.html
