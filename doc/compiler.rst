======================
Duktape ES5.1 compiler
======================

Introduction
============

This document provides an overview of the Duktape ES5.1 compiler structure,
the basic compilation process, and notes on difficult-to-compile constructs.
Assorted lexer and compiler design notes are covered in separate sections in
no specific order (these are raw notes collected on the first implementation
pass).  Future work lists known areas for improvement (not exhaustively).

The document does not cover the changes needed to support new ES6 constructs
like destructuring assignment; the main change will be the addition of some
form of an intermediate representation (IR), perhaps a statement level
expression tree or a full expression tree.  Having an IR will also enable
several optimizations.

This document is a snapshot of design issues and will not be kept exactly
up-to-date with the compiler.  The document was originally written for Duktape
1.0 so some parts may be out-of-date.

Compilation overview
====================

Basics
------

The compiler converts source code containing global code, eval code, or
function code into an executable form.  The most important parts of that
form are bytecode, constants, function metadata, and inner function
templates.  Compilation can also end in a ``SyntaxError``, which are
mandated to be "early errors" in Ecmascript, or some internal error, such
as out of memory error.

The end result of compilation is more specifically a *function template*.
A function template does not yet have a lexical environment, it merely
refers to symbols outside of its own scope by name.  A function template
is instantiated into a closure which supplies the missing lexical environment.
Inner functions are also function templates, and multiple closures may
be created from a certain template::

  // For each invocation of f(), a separate closure of g() is created
  function f(x) {
    function g() {
      print(x);
    }
    return g;
  }

Compilation depends on two major components:

* A *lexer*, which generates tokens from an underlying input stream on request.
  The lexer supports backtracking and reparsing to allow multi-pass parsing,
  at the cost of not currently supporting streamed parsing (adding support
  for chunked streaming would be possible, see future work).

* A *compiler*, which generates bytecode from a token stream.  The compiler
  uses two (or more) passes over a function, and generates bytecode directly,
  avoiding an explicit intermediate representation.  An *intermediate value*
  (ivalue) concept is used instead for managing expression values.

Code and memory footprint concern originally lead to the decision to avoid
a comprehensive intermediate representation in the compiler; a limited
intermediate value concept is used instead.  This complicates compilation
somewhat but reduces memory footprint.  A more comprehensive intermediate
representation will be needed in future versions to fully support ES6.

Lexer
-----

The lexer has a straightforward design:

* Source code is decoded from UTF-8 into a window of Unicode codepoints,
  with an infinite number of virtual EOF codepoints following the end-of-input.
  The decoder supports rewinding: the current position in the source stream
  can be saved and restored at a later time.

* The main lexing function decodes individual tokens like keywords, numbers,
  identifiers, etc, using the codepoint window for a safe lookup.  The lexing
  function keeps track of token line number and source byte offset information,
  and the eligibility of tokens for automatic semicolon insertion.

The upside of the codepoint window is that character encoding is effectively
hidden and normalized in the process, and that the main lexing function can
safely do forward lookups up to a certain limit without explicit end-of-input
checks.  It would also be possible to support encodings other than UTF-8 quite
easily.  The downside is a small performance impact over hardcoding the lexer
for UTF-8 input, which would also be a viable design.

Compiler
--------

The compiler component uses a hand crafted recursive descent statement parser,
with somewhat tricky handling of a few Ecmascript constructs (like the
"for/for-in" statement).  In contrast, top-down operator parsing is used for
parsing expressions (see http://javascript.crockford.com/tdop/tdop.html)
which is nice in that it allows single pass parsing while generating mostly
nice bytecode.

Compilation is triggered by Duktape C API compilation/eval calls, ``eval()``
calls, or ``Function`` constructor calls.  The calling context provides the
necessary flags to indicate whether global code, eval code, or function code
is being compiled.  The global compiler state is set up, and a "current
function" is set up with matching the requested compilation context.  The
function body is then parsed with a multi-pass parser, resulting in final
bytecode, constants, inner functions, and a lot of other state (for example,
flags indicating whether the function accesses ``argument`` or ``eval``;
this state is needed to optimize functions).  Finally, the "current function"
is converted from the compiler representation to the run-time function
(template) format used by the executor.

Statement parsing is straightforward: a function body is parsed as a sequence
of "source elements" which are otherwise the same as statements but function
declarations are also allowed (Duktape allows function declarations anywhere
in a function for compatibility with existing code and other engines, however).
For global code and eval code there's a need to support an "implicit return
value" which requires tracking of statement values; implicit return value is
not needed for function code.

Expressions are parsed into *intermediate values* (ivalues) which are one
step from being concrete, fully referenced values.  This missing step allows
ivalues to be used both as left-hand-side values (assignment targets) and
right-hand-side values (evaluated values), and allows limited constant folding
at compile time.  An ivalue may, for example, represent:

* A constant value which is not yet registered to the function constant table
  (e.g. the string ``"foo"``).

* A constant registered to the function constant table (denoted e.g. ``C18``
  for constant 18).

* A register in the function register frame (denoted e.g. ``R23`` for register
  23).

* A variable access using a variable name.

* A property access with a base value and a key, with the base value and key
  being registers or constants.

* A unary or binary arithmetic operation.

One way to characterize ivalues is that instead of using a full expression
tree as an intermediate representation, the compiler uses small, local
fragments of that tree without ever constructing the full tree.  Each ivalue
is either a leaf node or an internal node with two leaf node children.

When parsing an expression the compiler typically creates an ivalue to
represent the value of the expression; some bytecode may be emitted in the
process to prepare registers/constants for the ivalue but this is not always
the case.  The ivalue may then be used as an input to another expression,
converted from one form to another if necessary.  This conversion process
may allocate new constants or registers, and emit bytecode as necessary.
For example, the result of a previously parsed ivalue representing an addition
operation may be needed in a single register/constant.  The compiler converts
the ivalue by allocating a temporary register and emitting the ADD opcode to
compute the result.  The temporary register can then be used as an input in
another ivalue as needed.

Creating ivalues for expressions and converting ivalues from one form to
another drives much of the code generation process.  The ivalue conversion
helpers can also perform limited optimization, such as constant folding for
numbers and strings.

Bytecode emission is conceptually quite simple: expression and ivalue handling
code simply request opcodes to be emitted as needed.  However, the bytecode
emission functions transparently handle *register shuffling* to extend the
range of addressable registers.  For example, the binary ``ADD X, Y, Z``
opcodes can directly only address an 8-bit target register (X) and two 8-bit
source registers or constants (Y and Z).  If any arguments exceed their
allowed range, the bytecode emission functions emit the necessary opcodes
to shuffle source and/or target values through temporary registers.  While
such code is not optimal, it is necessary to support very large functions
(for example those produced by Emscripten).

Two (or more) passes are made over every parsed function.  On the first pass
we don't know which variables and inner functions the function will declare,
as such declarations are conceptually "hoisted" to the top of the function.
One purpose of the first pass is to gather this information for the second
pass.  Even so, to keep the code simple, the first pass also generates "broken"
throw-away bytecode so that the same parsing code can be used for all passes.

On the second pass all the necessary information has been gathered and actual
bytecode can be emitted.  A simple in-place peephole optimizer is applied to
the bytecode before generating final bytecode.  The peephole optimizer
currently only straightens out JUMPs (JUMP->JUMP->X is converted to JUMP->X).

The temporary registers needed for shuffling are only allocated when they're
actually needed.  Typically this is noticed on the first pass, but in some
cases it is only detected on the second pass; in such cases a third pass is
needed to generate the final bytecode.

This multi-pass approach has several downsides: (1) it requires a lexer which
can backtrack to the beginning of the function; and (2) time is wasted in
lexing and compiling the function twice (in an initial design inner functions
would also get parsed *four times* in total, their inner functions
*eight times* in total, etc, but there's a specific solution to this problem
in the current compiler).  The upside of multi-pass parsing is that there is
no need for an intermediate representation which saves on memory footprint.

The statement parser keeps track of a "statement number" within the
current function.  This is not needed for any critical purpose, but it
allows the first compilation pass to stash information related to a
certain statement for the second pass, perhaps allowing more optimal
code generation.  For instance, the first pass could note that a loop
statement has no relevant break/continue statements, so a label site is
not actually needed.  Similar expression counts, token counts, or source
offsets could be used to address constructs to help in multi-pass parsing.
However, no such optimizations are currently used by the compiler.

Recursive expression parsing, statement parsing, and function parsing may
happen during parsing; for example, a function expression may appear almost
anywhere and trigger recursive function compilation.  To fully support
recursion in function parsing, all compilation state is kept in the "current
function" state rather than the global compiler state.

Both the lexer and compiler need to deal with the fact that garbage collection
may occur almost anywhere (which may even lead to nested compilation if a
finalizer is invoked), errors may be thrown almost anywhere, and so on.  All
resources must thus be visible to the garbage collector and correctly reference
counted at nearly all times.  The current approach to deal with this is to use
the current thread's value stack to stash token values, intermediate values,
identifier names, etc.  Slots are allocated from the value stack as necessary.
This is a bit complicated but the alternatives are not simple either.  (One
alternative would be to make the compiler state a traversable object type for
the garbage collector.)

Ivalue example
--------------

Expression parsing and ivalue manipulation drives most of the code generation
process.  Let's look at a concrete example how these work together to generate
bytecode.

Consider the statement::

    x.y.z = 1 + 2;

The steps taken to compile the statement are roughly:

* The "x" expression generates ivalue I1 of type "variable access" with the
  variable name "x", which is not yet allocated a constant identifier.  No
  code is emitted.

* The ".y" part generates ivalue I2 of type "property access":

  - The base value (I1) needs to be a register or a constant, so a constant C0
    is allocated for the variable name (``"x"``) and a temporary register R0
    for the value, and bytecode to read the variable is emitted (``GETVAR
    R0, C0``).

  - The key needs to be a register or constant, so a constant C1 is allocated
    for the key (``"y"``).  No bytecode needs to be emitted.

  - I2 base value is R0, key is C1.

* The ".z" part generates ivalue I3 of type "property access":

  - The base value (I2) is coerced into a new temporary register R1 by
    emitting bytecode for the property load (``GETPROP R1, R0, C1``).

  - A constant C2 is allocated for the key (``"z"``).

  - I3 base value is R1, key is C2.

* The compiler notices an assignment operator and parses the right side.
  The constants 1 and 2 are compiled into ivalues I4 and I5 initially, and
  the combined into an ivalue I6 representing the addition of two constants.
  No code is emitted for the addition yet.

* To perform the assignment the right-hand side (I6) needs to be coerced into
  a single register/constant.  For this specific ivalue the compiler notices
  that two integer constants are being added so constant folding is used.
  The compiler allocates a temporary register R2 and emits bytecode to load
  the integer (``LDINT R2, 3``).  The ivalue I7 represents the result in R2.
  (The compiler could also register a new constant instead of using an integer
  load, but (some) integers are more efficiently handled using direct integer
  loads.)

* Finally, the assignment operation uses I3 as its target and I7 as its
  source, emitting a property write (``PUTPROP R1, C2, R2``).  Here I3 is
  used as a left-hand side value (write target) rather than as a right-hand
  side value.

While there are multiple steps and ivalues, the bytecode emitted from this
process is relatively short (the opcodes here are for illustration only and
don't match 1:1 with the actual opcodes used by Duktape)::

    ; Constant C0: "x"
    ; Constant C1: "y"
    ; Constant C2: "z"

    GETVAR R0, C0        ; read variable "x" to R0
    GETPROP R1, R0, C1   ; read property R0["y"] (= x.y) to R1
    LDINT R2, 3          ; load integer 3 to R2
    PUTPROP R1, C2, R2   ; write property R1["z"] (= x.y.z), value R2 (integer 3)

As can be seen from the example, ivalues are convenient in that the final
result of a property expression has a single format (an ivalue) which is
one step removed from the final value.  This allows them to be used both as
left-hand-side and right-hand-side values; the decision is made by the caller
in the final conversion.  Optimizations are also possible when converting
ivalues from one form to the next.

Ivalue conversion also provides a lot of flexibility: if the result of a
previous expression isn't directly compatible with the needs of the expression
being parsed, ivalues can be converted to the required form.  Because ivalues
are one step away from being completed, inefficient conversions are mostly
(but certainly not always) avoided.  For example, an ivalue representing an
integer can be converted either to a register or a constant, with the
necessary bytecode only emitted when it's known which one is preferred.

Several details are omitted from this description; for example:

* The compiler tries to reuse temporary registers where possible to reduce the
  number of temporaries needed.

* Local variables (including arguments) are assigned to registers and are
  accessed directly without an explicit variable read/write operation (GETVAR
  or PUTVAR).

* Register shuffling might be needed; it is currently handled transparently
  by the bytecode emission functions.

Bytecode
--------

The bytecode opcodes used by Duktape are chosen simply to work well for both
compilation and execution.  The bytecode is not version compatible, and may
change arbitrarily in even minor versions.  The role of Duktape bytecode is
not to be a code distribution format like e.g. Java bytecode.

The bytecode executor is the best source for documentation on exact bytecode
semantics at any given time.  Opcode information must be sync in:

* ``src/duk_js_bytecode.h`` defines opcode names and various constants

* ``src/duk_js_compiler.c`` emits bytecode

* ``src/duk_js_executor.c`` interprets bytecode

* ``debugger/duk_opcodes.yaml`` provides opcode metadata in a programmatic
  format, used by the debugger Web UI for bytecode dumping

Code organization
-----------------

The main entry point to compilation is ``duk_js_compile()`` in
``duk_js_compiler.c``.

``duk_lexer.c`` and ``duk_lexer.h`` contain the entire lexer implementation.
Tokens are represented by ``duk_token``.  Two slots are reserved from the
value stack for token values (regexp literals need two slots: pattern and
flags) to keep the values reference counted.

``duk_js_compiler.c`` and ``duk_js_compiler.h`` contain the entire compiler
implementation: function, statement and expression parsers, bytecode emission,
ivalue manipulation, and assorted support functionality like label and constant
management.  The compiler was originally written as a single file for efficient
inlining, before source files were combined into a single file in the dist
process.

Compilation state is encapsulated into ``duk_compiler_ctx``, which includes:

* Tokenization state

* Control structure for the current function being compiled; the function
  structure includes:

  - Code generation state: bytecode, identifier bindings, constants,
    temporary register state, label state, etc

  - Control variables for the current expression being parsed

* Various control flags which operate at the entry point level

Intermediate values are represented by ``duk_ivalue`` and ``duk_ispec``.
These need value value stack slots for storing values such as strings.

A function being compiled is represented by the inner representation
``duk_compiler_func`` which is converted into an actual function object
(a template) once compilation is finished.  The intermediate function
refers to a number of allocated value stack locations for storing
compilation data such as label information, known identifiers, bytecode
emitted, etc.  There are also support state and structures like
``duk_labelinfo``.

Bytecode is generated as a sequence of ``duk_compiler_instr`` structs.
These contain an actual instruction (``duk_instr_r``) and line information.
Line information is compressed into a compact bit-packed run-time format
(pc2line) at the end of function compilation.

General design notes
====================

This section lists miscellaneous issues affecting lexer and compiler design.

C recursion depth
-----------------

C recursion depth or C stack size needs to be reasonably limited for
compatibility with some embedded environments with small stacks.

Avoiding memory churn
---------------------

Minimizing the number of alloc/realloc/free operations is desirable for
all environments.  Memory churn has a performance impact and also increases
the chance that memory gets fragmented which is an issue for some (but not
all) allocators.

A few examples on how to avoid memory churn:

* Use fixed size buffers when possible, e.g. for codepoint decode window.

* Use a shared temporary buffer for parsing string valued tokens, reusing
  the buffer.  Most keywords and literal strings will easily fit into a
  few hundred without ever needing to resize the temporary buffer.

* Minimize resizes of the bytecode emission buffer.  For example, when
  starting second compilation pass, keep the current bytecode buffer
  without resizing it to a smaller size.

Memory usage patterns for pooled allocators
-------------------------------------------

For low memory environments using pool allocation, any large allocations that
grow without bounds are awkward to handle because selecting the pool sizes
becomes difficult.  It is preferable to do a lot of smaller allocations with
a bounded size instead; typical pool configurations provide a lot of small
buffers from 4 to 64 bytes, and a reasonable number of buffers up to 256
bytes.  Above that buffer counts are smaller and tightly reserved.

There are a few unbounded allocations in the current solution, such as current
bytecode being emitted.

Lexer design notes
==================

This section has small lexer design notes in no specific order.  Larger
issues are covered in dedicated sections below.

Tokenization is stateful
------------------------

Tokenization is affected by:

* Strictness of the current context, which affects the set of recognized
  keywords (reserved words, to be more precise).

* Regexp mode, i.e. whether a literal regexp is allowed in the current
  context.  This is the case because regexp literals use the forward slash
  which is easily confused with a division expression.  Currently handled
  by having a table indicating which tokens may not be followed by a
  RegExp literal.

* In some contexts reserved words are recognized but in others they must
  be interpreted as identifiers: an ``Identifier`` production accepts
  any ``IdentifierName`` except for ``ReservedWord``.  Both ``Identifier``
  and ``IdentifierName`` appear in constructs.  The current approach is
  to supply both the raw identifier name and a possible reserved word in
  ``duk_token``.  The caller can then decide which is appropriate in the
  caller's context.

Source code encoding is not specified
-------------------------------------

The E5.1 specification does not mandate any specific source code encoding.
Instead, source code is assumed to be a 16-bit codepoint sequence for
specification purposes (E5.1 Section 6).  Current choice is for the source
code to be decoded in UTF-8.  Changing the supported encoding(s) would be
easy because of the codepoint decoding window approach, but it's preferred
that calling code transcode non-UTF-8 inputs into UTF-8.

Source code may contain non-BMP characters but Ecmascript does not
support such characters directly.  For instance, if codepoint U+12345 would
appear (without escaping) inside a string constant, it would need to be
interpreted as two 16-bit codepoint surrogate codepoints (surrogate pair),
if such characters are supported at all.

Duktape strings support non-BMP characters though, but they cannot be created
using source literals.

Use strict directive
--------------------

The "use strict" and other directives have somewhat odd semantics (see E5.1
Section 14.1):

* ``"use strict"`` is a valid "use strict directive" and triggers strict mode.

* ``"use\u0020strict"`` is a valid directive but **not** a "use strict
  directive".

* ``("use strict")`` is not a valid directive.

The lexer and the expression parser coordinate to provide enough information
(character escaping, expression "depth") to allow these cases to be
distinguished properly.

Compiler design notes
=====================

This section has small compiler design notes in no specific order.  Larger
issues are covered in dedicated sections below.

Expression parsing algorithm
----------------------------

The expression parsing algorithm is based on:

* http://javascript.crockford.com/tdop/tdop.html

* http://effbot.org/zone/simple-top-down-parsing.htm

* http://effbot.org/zone/tdop-index.htm

The ``nud()`` function considers a token as a "value" token.  It also parses
unary expressions (such as ``!x``).

The ``led()`` function considers a token as an "operator" token, which
operates on a preceding value.

Some tokens operate in both roles but with different semantics.  For instance,
opening bracket (``[``) may either begin an array literal in ``nud()``, or a
property access in ``led()``.

The simplified algorithm is as follows.  The 'rbp' argument defines "right
binding power", which governs when the expression is considered to be
finished.  The 'lbp()' value provides token binding power, "left binding
power".  The higher 'rbp' is, the more tightly bound expression we're parsing::

  nud()                ; parse current token as "value"
  while rbp < lbp():   ; while token binds more tightly than rbp...
    led()              ; combine previous value with operator

The ``led()`` function may parse an expression recursively,
with a higher 'rbp', i.e. a more tightly bound expression.

In addition to this basic algorithm, some special features are needed:

* Keep track of led() and nud() counts.  This allows directives in a
  function "directive prologue" (E5.1 Section 14.1) to be detected correctly.
  For instance::

    function f() {
      'use strict';       // valid directive for strict mode
      'use\u0020strict';  // valid directive, but not for strict mode (!)
      ('use strict');     // not a valid directive, terminates directive prologue

* Keep track of parenthesis nesting count during expression parsing.  This
  allows "top level" to be distinguished from nested levels.

* Keep track of whether the expression is a valid LeftHandSideExpression, i.e.
  the top level contains only LeftHandSideExpression level operators.

* Allow a caller to specify that expression parsing should be terminated at
  a top-level ``in`` token.  This is needed for the "NoIn" variants, which are
  used in for/for-in statements.

* Allow a caller to specify whether or not an empty expression is allowed.

The expression parses uses both the "previous token" and "current token"
in making parsing decisions.  Which token is considered at each point is
not always trivial, and the responsibilities between compiler internal
helper functions are not always obvious; token state assumptions are thus
documented in most functions.

Parsing statements
------------------

Statement parsing is a traditional top-down recursive process which is
relatively straightforward.  Some complicated issues are:

* Specific statement types which are difficult to parse without lookahead

* Label site handling

* Tail calls

* Implicit return values

Parsing functions
-----------------

At the end of function parsing, the compiler needs to determine what
flags to set for the function.  Some flags have an important performance
impact.  In particular, the creation of an ``arguments`` object can be
skipped if the compiler can guarantee that it will never be accessed.

This is not trivial because e.g. the presence of a direct ``eval()``
call may allow indirect access to ``arguments``.  The compiler must always
make a conservative choice to ensure compliance and safety.

Distinguishing for/for-in
-------------------------

There are a total of four ``for`` / ``for-in`` statement variants.  Each
variant requires slightly different bytecode output.  Detecting the correct
variant is difficult, but possible, without multiple passes or arbitrary
length token lookup.  See separate discussion below.

Expressions involving "new"
---------------------------

Expression involving ``new`` are not trivial to parse without lookahead.
The grammar rules for ``LeftHandSideExpression``, ``CallExpression``,
``NewExpression``, and ``MemberExpression`` are a bit awkward.  See separate
discussion below.

Directive detection
-------------------

The "use strict" and other directives are part of a directive prologue which
is the sequence of initial ExpressionStatements producing only a string
literal (E5.1 Section 14.1).

The expression parser provides a nud/led call count which allows the
statement parser to determine that an expression is a valid directive.
The first non-valid directive terminates the directive prologue, and
no more directives are processed.  The lexer provides character escape
metadata in token information to allow "use strict" to be detected correctly.

The transition to strict mode occurs in the directive prologue of the
first compilation pass.  Function strictness is already known at the
beginning of the second pass.  This is important because strict mode
affects function argument parsing, for instance, so it must be known
before parsing the function body.

Declaration "hoisting"
----------------------

Variable and function declarations affect code generation even before the
declarations themselves appear in the source code: in effect, declarations
are "hoisted" to the top of the function.  To be able to generate reasonable
code, compile-time identifier resolution requires multi-pass parsing or some
intermediate representation.  Current solution is multi-pass function parsing.

Some token lookahead is needed
------------------------------

Because we need some lookahead, the compiler currently keeps track of two
tokens at a time, a "current token" and a "previous token".

Implicit return values
----------------------

Global and eval code have an implicit return value, see separate section
below.

Guaranteed side effects
-----------------------

Sometimes code must be generated even when it might seem intuitive it is not
necessary.  For example, the argument to a ``void`` operator must be coerced to
a "plain" register/constant so that any side effects are generated.  Side effects
might be caused by e.g. getter calls::

  // If foo.x is an accessor, it must be called
  void foo.x

Evaluation order requirements
-----------------------------

Evaluation order requirements complicate one-pass code generation somewhat
because there's little leeway in reordering emitted bytecode without a
larger IR.

Dynamic lexical contexts
------------------------

Ecmascript lexical contexts can be dynamically altered even after a function
call exits.  For example, if a function makes a direct ``eval()`` call with
a variable argument, it is possible to declare new variables when the function
is called::

    var foo = 123;
    var myfunc;

    function f(x) {
        eval(x);

        return function () { print(foo); }
    }

    // declare 'foo' in f(), returned closure sees this 'foo' instead
    // of the global one

    myfunc = f('var foo = 321');
    myfunc();  // prints 321, not 123

    // don't declare 'foo' in f(), returned closure sees the global 'foo'
    // instead of the global one

    myfunc = f('var quux = 432');
    myfunc();  // prints 123

For execution efficiency we should, for example, avoid creation of environment
records and the ``arguments`` object.  The compiler thus needs to
conservatively estimate what optimizations are possible.

Compilation may trigger a GC or recursive compilation
-----------------------------------------------------

At first glance it might seem that the compiler cannot be invoked recursively.
This is not the case however: the compiler may trigger a garbage collection
or a refzero, which triggers a finalizer execution, which in turn can use e.g.
``eval()`` to cause a recursive Ecmascript compilation.  Compiler recursion is
not a problem as such, as it is a normal recursive C call which respects value
stack policy.

There are a few practical issues to note with regards to GC and recursion:

* All heap values must be correctly reference counted and reachable.  The
  compiler needs heap values to represent token values, compiler intermediate
  values, etc.  All such values must be reachable through the valstack, a
  temporary object, or GC must explicitly support compiler state.

* There should be no global (heap- or thread-wide) compiler state that would
  get clobbered by a recursive compilation call.  If there is such state, it
  must be saved and restored by the compiler.

* At the moment there is a "current compiler context" variable in ``duk_hthread``
  which is used to augment SyntaxErrors with a line number.  This state is saved
  and restored in recursive compilation to avoid clobbering.

Unary minus and plus
--------------------

Quite interestingly, the minus sign in ``-123`` is **not** a part of the
number token in Ecmascript syntax.  Instead, ``-123`` is parsed as a unary
minus followed by a number literal.

The current compiler follows this required syntax, but constant folding
ensures no extra code or constants are generated for unary minus or unary
plus.

Compile-time vs. run-time errors
--------------------------------

Compilation may fail with an error only if the cause is an "early
error", specified in E5.1 Section 16, or an internal error such as
out of memory occurs.  Other errors must only occur when the result
of the compilation is executed.  Sometimes this includes constructs
that we know can never be executed without an error (such as a
function call being in a left-hand-side position of an assignment),
but perhaps that code is never reached or the error is intentional.

Label statement handling
------------------------

Label statements essentially prefix actual statements::

  mylabel:
    while (true) { ... }

Labels are currently handled directly by the internal function which
parses a single statement.  This is useful because all labels preceding
an actual statement are coalesced into a single "label site".  All labels,
including an implicit empty label for iteration statements, point to the
same label site::

  // only a single label site is established for labels:
  // "label1", "label2", ""
  label1:
  label2:
    for (;;) { ... }

Technically, a label looks like an expression statement initially, as a
label begins with an identifier.  The current parsing approach avoids
backtracking by parsing an expression statement normally, and then
noticing that (1) it consisted of a single identifier token, and (2)
is followed by a colon.

No code is emitted by the expression parser for such a terminal single
token expression (an intermediate value is generated, but it is not
coerced to any code yet), so this works without emitting any invalid
code.

Note that some labels cannot accept break or continue (e.g. label for
an expression statement), some can accept a break only (switch) while
others can accept both (iteration statements: do-while, for, while).
All the label names are registered while processing explicit labels,
and an empty label is registered for an iteration/switch statement.
When the final statement type is known, all labels in the set of labels
are updated to indicate whether they accept break and/or continue.

Backtracking
------------

There is currently only a need to backtrack at the function level, to
restart function compilation when moving from one parsing pass to the next.
The "current function" state needs to be carefully reinitialized during
this transition.

More fine-grained backtracking is not needed right now, but would involve
resetting:

* Emitted bytecode

* Highest used (temp) register

* Emitted constants and inner functions

* Active label set

Temporary register allocation
-----------------------------

Temporary registers are allocated as a strictly increasing sequence from a
specified starting register.  The "next temp" is reset back to a smaller
value whenever we know that none of the higher temp values are no longer
needed.  This can be done safely because temporaries are always allocated
with a strict stack discipline, and any fixed identifier-to-register
bindings are below the initial temp reg.

The current expression parsing code does not always produce optimal
register allocations.  It would be preferable for expression result values
to be in as low register numbers as possible, which maximizes the amount
of temporaries available for later expression code.  This is currently
done on a case-by-case basis as need arises.

The backstop is at the statement level: after every statement is complete,
the "next temp" can be reset to the same value it was before parsing
the statement.  However, it's beneficial to reset "next temp" to a smaller
value whenever possible (inside expression parsing), to minimize function
register count and avoid running out of temp registers.

Unused temporary registers are not set to undefined, and are reachable for
garbage collection.  Unless they're overwritten by temporary values needed
by another expression, they may cause a "silent leak".  This is usually not
a concrete concern because a function exit will always decref all such
temporaries.  This may be an issue for forever-running functions though.

Register shuffling
------------------

The compiler needs to handle the case where it runs out of "easy access"
registers or constants (usually 256 or 512 registers/constants).  Either
this needs to be handled correctly in one pass, or the compiler must
fall back to a different strategy.  Current solution is to use register
shuffling through temporary registers.  Shuffling is handled by the bytecode
emitters.

Pc2line debug data creation
---------------------------

The "pc2line" debug data is a bit-packed format for converting a bytecode
PC into an approximate source line number at run time.

Although almost all of the bytecode is emitted in a linear fashion (appending
to earlier code), some tricky structures insert bytecode instructions in the
middle of already emitted bytecode.  These insertions prohibit the emission
of debug data in a streaming fashion during code emission.  Instead, it needs
to be implemented as a post-step.  This unfortunately doubles the memory
footprint of bytecode during compilation.

The current solution is to keep track of (instruction, line number) pairs
for each bytecode instruction during compile time.  When the intermediate
representation of the compiled function is converted to an actual run-time
representation, this representation is converted into a plain opcode list
and bit-packed pc2line data.

There is currently some inaccuracy in the line numbers assigned to opcodes:
the bytecode emitter associates the line number of the previous token because
this matches how expression parsing consumes tokens.  However, in some other
call sites the relevant line number would be in the current token.  Fixing
this needs a bit more internal book-keeping.

Peephole optimization
---------------------

Currently a simple in-place peephole optimizer is applied at the end of
function compilation to straighten out jumps.  Consider for instance::

   a:
     JUMP c      -.
   b:             |      <--.
     JUMP d       |   -.    |
   c:          <--'    |    |
     JUMP b            |   -'
   d:               <--'

The peephole optimizer runs over the bytecode looking for JUMP-to-JUMP
cases until the bytecode no longer changes.  On the first peephole pass
these jumps are straightened to::

   a:
     JUMP b      -.
   b:          <--'
     JUMP d           -.
   c:                  |
     JUMP d            |  -.
   d:               <--' <-'

(The JUMPs are modified in place, so some changes may be visible to later
jumps on the same pass.)

On the next pass this is further optimized to::

   a:
     JUMP d      -.
   b:             |
     JUMP d       |   -.
   c:             |    |
     JUMP d       |    |  -.
   d:          <--' <--' <-'

The peephole pass doesn't eliminate any instructions, but it makes some
JUMP chains a bit faster.  JUMP chains are generated by the current compiler
in many cases, so this simple pass cheaply improves generated code slightly.

Avoiding C recursion
--------------------

C recursion happens in multiple ways.  These should suffice to control it:

* Recursive expression parsing

* Recursive statement parsing (e.g. ``if`` statement parses another statement)

* Recursive function parsing (e.g. function expression or function declaration
  inside another function)

Recursion controls placed in these key functions should suffice to guarantee
an upper limit on C recursion, although it is difficult to estimate how much
stack is consumed before the limit is reached.

ES6 constructs need an intermediate representation
--------------------------------------------------

ES6 constructs such as destructuring assignment will need an intermediate
representation (or at least a much larger fragment of the expression tree)
to compile in a reasonable manner.

Operator precedences (binding powers)
=====================================

Operator precedences (binding powers) are required by the expression parser
for tokens acting as "operators" for ``led()`` calls.  This includes tokens
for binary operators (such as ``+`` and ``instanceof``).

A higher binding power binds more strongly, e.g. ``*`` has a higher binding
power than ``+``.  The binding power of operators can be determined from the
syntax.  Operators of different precedence are apparent from production
nesting level; outer productions have lower binding power.

Operators at the same level have the same binding power if left-associative.
A production can be determined to be left-associative by its production.
For instance::

  AdditiveExpression:
      MultiplicativeExpression
      AdditiveExpression '+' MultiplicativeExpression
      AdditiveExpression '-' MultiplicativeExpression

Abbreviated::

  AE:
      ME
      AE '+' ME
      AE '-' ME

The expression ``1 + 2 + 3 + 4 * 5`` would be derived as (with parentheses for
emphasizing order)::

  AE -> AE '+' ME
     -> (AE '+' ME) '+' ME
     -> ((AE '+' ME) '+' ME) '+' ME
     -> ((ME '+' ME) '+' ME) '+' ME
     -> ((1 '+' 2) '+' 3) '+' (4 '*' 5)

Operators at the same level which are right-associative can be determined
from its production.  For instance::

  AssignmentExpression:
      ConditionalExpression
      LeftHandSideExpression '=' AssignmentExpression
      LeftHandSideExpression AssignmentOperator AssignmentExpression

  AssignmentOperator:
      '*='
      (... others omitted)

Abbreviated::

  AE:
      CE
      LE '=' AE
      LE AO AE

  AO:
      '*='

The expression ``a = b = c *= 4`` would be produced as (using parentheses for
emphasis)::

  AE -> LE '=' AE
     -> LE '=' (LE '=' AE)
     -> LE '=' (LE '=' (LE '*=' AE)
     -> LE '=' (LE '=' (LE '*=' CE)
     -> a '=' (b '=' (c '*=' 4))

Right associative productions are parsed by using a tweaked 'rbp' argument
to the recursive expression parsing.  For the example above:

* ``a`` is parsed with ``nud()`` and evaluates into a variable reference.

* The first ``=`` operator is parsed with ``led()``, which calls the
  expression parser recursively, with a 'rbp' argument which causes
  the recursive call to consume all further assignment operations.

What is a proper 'rbp' for the recursive ``led()`` call?  It must be
lower than the binding power for the ``=`` operator, but higher or equal
than any operator whose binding power is less than that of ``=``.  For
example, if the binding power of ``=`` was 10, the 'rbp' used could be 9.
The current compiler uses multiples of 2 for binding powers so that
subtracting 1 from the binding power of an operator results in a binding
power below the current operator but never equal to any other operator.
Technically this is not necessary, because it's OK for the 'rbp' to be
equal to a lower binding operator.

In addition to binary operators, binding powers need to be assigned to:

* Unary operators

* Some tokens which are not strictly operators.  For example, ``(``, ``[``,
  and ``{`` which begin certain expressions (function calls, property
  access, and object

Token precedences for ``lbp()``, from highest (most tightly bound) to lowest
are summarized in the list below.  Operators of equal binding power are on
the same line.  The list is determined based on looking at the ``Expression``
production.  Operators are left associative unless indicated otherwise:

* (IdentifierName, literals, ``this``, etc.  Parsed by ``nud()``
  and don't need binding powers.)

* ``.`` ``[``
  (Note: MemberExpression parsed by ``led``.)

* ``new``
  (Note: unary expression parsed by ``nud()``.  Right-associative.)

* ``(``
  (Note: CallExpression parsed by ``led()``.)

* ``++`` ``--``
  (Note: postfix expressions which are parsed by ``led()`` but which are
  "unary like".  The expression always terminates in such a case.)

* ``delete`` ``void`` ``typeof`` ``++`` ``--`` ``+`` ``-`` ``~`` ``!``
  (Note: unary expressions which are parsed by ``nud()`` and don't thus
  actually need a binding power.  All of these are also right-associative.
  ``++`` and ``--`` are preincrement here; ``+`` and ``-`` are unary plus
  and minus.)

* ``*`` ``/`` ``%``

* ``+`` ``-``

* ``<<`` ``>>`` ``>>>``

* ``<`` ``>`` ``<=`` ``>=`` ``instanceof`` ``in``

* ``==`` ``!=`` ``===`` ``!==``

* ``&``

* ``^``

* ``|``

* ``&&``

* ``||``

* ``?``
  (Note: starting a "a ? b : c" expression)

* ``=`` ``*=`` ``/=`` ``%=`` ``+=`` ``-=`` ``<<=`` ``>>=`` ``>>>=`` ``&=`` ``^=`` ``|=``
  (Note: right associative.)

* ``,``

* ``)`` ``]``
  (Note: when parsed with ``led()``; see below.)

* EOF
  (Note: when parsed with ``led()``; see below.)

The precedence list is clear starting from the lowest binding up to binary
``+`` and ``-``.  Binding powers higher than that get a bit tricky because
some of them are unary (parsed by ``nud()``) and some or parsed by ``led()``
but are not binary operators.

When parsing an expression beginning with ``(`` using ``nud()``, the
remainder of the expression is parsed with a recursive call to the expression
parser and a 'rbp' which guarantees that parsing stops at the closing ``)``.
The ``rbp`` here must NOT stop at the comma operator (``,``) so technically
``)`` is considered to have a binding power lower than comma.  The same applies
to ``]``.  Similarly, EOF is considered to have a binding power lowest of all.
These have been appended to the list above.

Parsing RegExp literals
=======================

The Ecmacsript lexer has two goal symbols for its lexical grammar:
``InputElementDiv`` and ``InputElementRegExp``.  The former is used in
all lexical contexts where a division (``/``) or a division-assignment
(``/=``) is allowed; the latter is used elsewhere.  The E5.1 specification
does not really say anything else on the matter (see E5.1 Section 7, 2nd
paragraph).

In the implementation of the compiler, the ``advance()`` set of helpers
knows the current token, and consults a token table which indicates
whether a regexp literal is prohibited after the current token.  Thus,
this detail is hidden from ordinary parsing code.

The ``advance()`` helper knows the current token type and consults a
token table which has a flag indicating whether or not a RegExp can ever
follow that particular token.  Unfortunately parsing Identifier (which
prohibits keywords) vs. IdentifierName (which allows them) is context
sensitive.  The current lexer handles this by providing a token type for
both interpretations: ``t`` indicates token type with reserved words
being recognized (e.g. "return" yields a token type DUK_TOK_RETURN)
while ``t_nores`` indicates token type ignoring reserved words (e.g.
"return" yields a token type DUK_TOK_IDENTIFIER).

``IdentifierName`` occurs only in::

  PropertyName -> IdentifierName   (object literal)
  MemberExpression -> MemberExpression '.' IdentifierName
  CallExpression -> CallExpression '.' IdentifierName

Using ``t_nores`` for determing whether or not a RegExp is allowed does
not work.  For instance, ``return`` statement allows a return value so
a RegExp must be allowed to follow::

  return /foo/;

On the other hand, a RegExp cannot follow ``return`` here::

  t = foo.return/2;

Using ``t`` has the inverse problem; if DUK_TOK_RETURN allows a RegExp
to follow, this parses correctly::

  return /foo/;

but this will fail::

    t = foo.return/2;

The IdentifierName cases require special handling:

* The ``PropertyName`` in object literal is not really an issue.  It cannot
  be followed by either a division or a RegExp literal.

* The ``MemberExpression`` case: a RegExp can never follow.  A special one-time
  flag can be used to reject RegExp literals on the next ``advance()`` call.

* The ``CallExpression`` case: can be handled similarly.

Currently this special handling is implemented using the ``reject_regexp_in_adv``
flag in the current compiler function state.  It is only set when handling
``DUK_TOK_PERIOD`` in ``expr_led()``, and is automatically cleared by the next
``advance()`` call.

See test case: ``test-dev-regexp-parse.js``.

Automatic semicolon insertion
=============================

Semicolons need to be automatically inserted at certain points of the
token stream.  Only the parser/compiler can handle automatic semicolon
insertion, because automatic semicolons are only allowed in certain
contexts.  Only some statement types have a terminating semicolon and
thus participate in automatic semicolon insertion.

Automatic semicolon insertion is implemented almost completely at the
statement parsing level, the only exception being handling of
post-increment/decrement.

After the longest valid statement (usually containing an expression) has
been parsed, the statement is either terminated by an explicit semicolon
or is followed by an offending token which permits automatic semicolon
insertion.  In other words, the offending token is preceded by a newline,
or is either the EOF or the ``}`` token, whichever is appropriate for the
statement list in question.  The actual specification for "longest valid
statement" is that an automatic semicolon can only be inserted if a parse
error would otherwise occur.

Some statements also have grammar which prohibits automatic semicolon insertion
in certain places, such as: ``return [no LineTerminator here] Expression;``.
These need to be handled specially.

Some statements have a semicolon terminator while others do not.  Automatic
semicolons are naturally only processed for statements with a semicolon
terminator.

The current implementation:

* The statement list parser parses statements.

* Individual statement type parsers need to have a capability of parsing
  until an offending token is encountered (either a semicolon, or some
  other unexpected token), and to indicate whether that specific statement
  type requires a semicolon terminator.

* The general statement parsing wrapper then checks whether a semicolon
  termination is needed, and if so, whether an explicit semicolon or an
  automatically inserted semicolon terminates the statement.

* Statements which prohibit line terminators in some cases have a special
  check in the parsing code for that statement type.  If the token following
  the restriction has a "lineterm" flag set, the token is considered
  offending and the statement is terminated.  For instance, "return\\n1;"
  is parsed as an empty return because the token ``"1"`` has a lineterm
  preceding it.  The ``duk_token`` struct has a flag indicating whether
  the token was preceded by whitespace which included one or more line
  terminators.

* Checking whether an automatic semicolon is allowed depends on a token
  which is potentially part of the next statement (the first token of
  the next statement).  In the current implementation the statement
  parsing function is expected to "pull in" the token *following* the
  statement into the "current token" slot anyway, so the token can be
  examined for automatic semicolon insertion without backtracking.

* Post-increment/decrement has a restriction on LineTerminator occurring
  between the preceding expression and the ``++``/``--`` token (note that
  pre-increment/decrement has no such restriction).  This is currently
  handled by ``expr_lbp()`` which will return an artificially low binding
  power if a ``++``/``--`` occurs in a post-increment/decrement position
  (which is always the case if they're encountered on the ``expr_led()``
  context) and the token was preceded by a line terminator.  This
  effectively terminates the preceding expression, treating e.g.
  "a+b\\n++" as "a+b;++;" which causes a SyntaxError.

There is a custom hack for an errata related to a statement like::

  do{print('loop')}while(false)false

Strictly speaking this is a syntax error, but is allowed by most
implementations in the field.  A specific hack is needed to handle
this case.  See ``test-stmt-dowhile-bug.js``.

Implicit return value of global code and eval code
==================================================

Global code and eval code have an "implicit return value" which comes from
the last non-empty statement executed.  Function code has no implicit return
value.  Statements returning a completion with type "empty" do not change
the implicit return value.  For instance: ``eval("1;;var x=2;")`` returns
``1`` because the empty statement and the ``var`` statement have an empty
completion.  This affects code generation, which is a bit different at the
statement level for global/eval code and function code.

When in a context requiring an implicit return value (eval code or global
code), a register is allocated for the last non-empty statement value.
When such a statement is parsed, its value is coerced to the allocated
register.  Other statements are coerced into a plain value (which is then
ignored) which ensures all side effects have been generated (e.g. property
access is generated for the expression statement ``x.y;``) without affecting
the implicit return value.

Statement types generating an empty value directly:

* Empty statement (12.3)

* Debugger statement (12.15)

Statement types generating an empty value indirectly:

* Block statement (12.1): may generate an empty statement indirectly if all
  statements inside the block are empty.

* ``if`` statement (12.5): may generate empty statement either if a clause has
  an empty value (e.g. ``eval("if (true) {} else {1}")`` returns ``undefined``)
  or a clause is missing (e.g. ``eval("if (false) {1}")`` returns ``undefined``).

* ``do-while``, ``while``, ``for``, ``for in`` statements (12.6): statement value
  is the value of last non-empty statement executed within loop body; may be empty
  if only empty statements or no statements are executed.

* ``continue`` and ``break`` statements (12.7, 12.8): have an empty value but
  ``continue`` and ``break`` are handled by their catching iteration statement,
  so they are a bit special.

* ``with`` statement (12.10): like block statements

* ``switch`` statement (12.11): return value is the value of the last non-empty
  statement executed (in whichever clause).

* Labelled statement (12.12): returns whatever the statement following them
  returns.

* ``try`` statement (12.14): return value is the value of the last non-empty
  statement executed in try and/or catch blocks.

Some examples:

+--------------------+-------------+-------------------------------------------------------------------------+
| Eval argument      | Eval result | Notes                                                                   |
+====================+=============+=========================================================================+
| "1+2;"             | 3           | Normal case, expression statement generates implicit return value.      |
+--------------------+-------------+-------------------------------------------------------------------------+
| "1+2;;"            | 3           | An empty statement generates an empty value.                            |
+--------------------+-------------+-------------------------------------------------------------------------+
| "1+2; var a;"      | 3           | A variable declaration generates an empty value.                        |
+--------------------+-------------+-------------------------------------------------------------------------+
| "1+2; var a=5;"    | 3           | A variable declaration, even with assignment, generates an empty value. |
+--------------------+-------------+-------------------------------------------------------------------------+
| "1+2; a=5;         | 5           | A normal assignment generates a value.                                  |
+--------------------+-------------+-------------------------------------------------------------------------+

Tail call detection and handling
================================

A tail call can be used when:

1. the value of a CALL would become the argument for an explicit
   ``return`` statement or an implicit return value (for global or
   eval code); and

2. there are no active TCF catchers between the return and the
   function entrypoint.

A trivial example is::

  function f(x) {
    return f(x+1);
  }

The generated code would look something like::

  CSREG r0, c0    ; c0 = 'f'
  GETVAR r1, c1   ; c1 = 'x'
  ADD r0, r1, c2  ; c2 = 1
  CALL r0, 2      ; TAILCALL flag not set
  RETURN r0       ;

This could be emitted as a tail call instead::

  CSREG r0, c0    ; c0 = 'f'
  GETVAR r1, c1   ; c1 = 'x'
  ADD r0, r1, c2  ; c2 = 1
  CALL r0, 2      ; TAILCALL flag set
  RETURN r0       ; kept in case tail call isn't allowed at run time

There are more complex cases, like::

  function f(x) {
    return (g(x) ? f(x+1) : f(x-1));
  }

Here, just before executing a RETURN, both paths of execution end up with
a function call.  Both calls can be converted to tail calls.

The following is not a candidate for a tail call because of a catcher::

  function f(x) {
    try {
      return f(x+1);
    } finally {
      print('cleaning up...');
    }
  }

Detecting anything other than the very basic case is probably not worth the
complexity, especially because E5.1 does not require efficient tail calls at
all (in fact, as of this writing, neither V8 nor Rhino support tail calls).
ES6 *does* require tail calls and provides specific guarantees for them.
Adding support for ES6 tail calls will require compiler changes.

The current approach is very simplistic and only detects the most common
cases.  First, it is only applied to compiling function code, not global or
eval code, which restricts consideration to explicit ``return`` statements
only.  When parsing a ``return`` statement:

* First request the expression parser to parse the expression for the return
  value normally.

* If the last bytecode instruction generated by the expression parser is a
  CALL whose value would then become the RETURN argument and there is nothing
  preventing a tail call (such as TCF catchers), convert the last CALL to a
  tail call.  (There are a few more details to this; see ``duk_js_compiler.c``
  for comments.)

* The RETURN opcode is kept in case the tail call is not allowed at run time.
  This is possible e.g. if the call target is a native function (which are
  never tail called) or has a ``"use duk notail"`` directive.

* Note that active label sites are not a barrier to tail calls; they are
  unwound by the tail call logic.

See ``test-dev-tail-recursion.js``.

Parsing CallExpression / NewExpression / MemberExpression
=========================================================

The grammar for ``CallExpression``, ``NewExpression``, and ``MemberExpression``
is interesting; they're not in a strict binding power sequence.  Instead,
there is a branch, starting from LeftHandSideExpression::

   LeftHandSideExpression
           |               .--.
           |               v  |
           |  .---> NewExpression ----.
           |  |                       |
           `--+                       +---> MemberExpression
              |                       |
              `---> CallExpression ---'
                           ^  |
                           `--'

Both NewExpression and CallExpression contain productions containing themselves
and MemberExpressions.  However, a NewExpression never produces a CallExpression
and vice versa.

This is unfortunately difficult to parse.  For instance, both productions
(CallExpression and NewExpression) may begin with a 'new' token, so without
lookahead we don't know which we're parsing.

Consider the two productions::

   Production 1:

     LeftHandSideExpression -> NewExpression
                            -> 'new' MemberExpression
                            -> 'new' 'Foo'

   Production 2:

     LeftHandSideExpression -> CallExpression
                            -> MemberExpression Arguments
                            -> 'new' 'Foo' '(' ')'

These two are syntactically different but semantically identical: they both
cause a constructor call with no arguments.  However, they derive through
different productions.

Miscellaneous notes:

* A NewExpression is the only production capable of generating "unbalanced"
  'new' tokens, i.e. 'new' tokens without an argument list.  A NewExpression
  essentially generates 0...N 'new' tokens before generating a MemberExpression.

* A MemberExpression can generate a "'new' MemberExpression Arguments"
  production.  These can nest, generating e.g. "new new Foo () ()" which
  parses as "(new (new Foo ()) ())".

* If a LeftHandSideExpression generates a NewExpression, it is no longer
  possible to generate more argument lists (open and close parenthesis)
  than there are 'new' tokens.  However, it is possible to generate more
  'new' tokens than argument lists.

* If a LeftHandSideExpression generates a CallExpression, it is no longer
  possible to generate 'new' tokens without argument list (MemberExpression
  only allows 'new' with argument list).  However, it is possible to
  generate more argument lists than 'new' tokens; any argument lists not
  matching a 'new' token are for function calls generated by CallExpression.
  For instance (with angle brackets for illustration)::

    new new Foo () () () == <(new <new Foo ()> ()> ()

  where the last parenthesis are for a function call.

* Parentheses match innermost 'new' expressions generated by MemberExpression,
  innermost first.  There can then be either additional 'new' tokens on the
  left or additional argument lists on the right, but not both.

  Any additional 'new' tokens on the left are generated by NewExpression.
  Any additional argument lists on the right are generated by CallExpression.

For instance::

   new new new new Foo () ()

parses as (with angle brackets used for illustration)::

   new new new <new Foo ()> ()
   new new <new <new Foo ()> ()>
   new <new <new <new Foo ()> ()>>

whereas::

   new new Foo () () () ()

parses as (with angle brackets used for illustration)::

   <<<new <new Foo ()> ()> ()> ()>
   :::    |==========|   :   :   :
   :::    constr. call   :   :   :
   :::                   :   :   :
   ::|====================   :   :
   ::     constr. call       :   :
   ::                        :   :
   :|========================|   :
   :      function call          :
   :                             :
   |=============================|
          function call

Current parsing approach:

* When a 'new' token is encountered by ``nud()``, eat the 'new' token.

* Parse a MemberExpression to get the call target.  This expression parsing
  must terminate if a left parenthesis '(' is encountered.  The expression
  parsing must not terminate if a property access is encountered (i.e. the
  ``.`` or ``[`` token in ``led()``).  This is achieved by a suitable binding
  power given to expression parser.

* Finally, look ahead to see whether the next token is a left parenthesis ('(').
  If so, the 'new' token has an argument list; parse the argument list.
  If the next token is not a left parenthesis, the 'new' expression is complete,
  and ``nud()`` can return.

* There are many tests in ``test-dev-new.js`` which attempt to cover the
  different cases.

Compiling "try-catch-finally" statements
========================================

Compiling the try-catch-statement statement is not very complicated.
However, what happens during execution is relatively complex:

* The catch stack is involved with a "TCF catcher".

* A new declarative environment record, containing the "catch variable",
  may need to be used during the catch part.

The execution control flow is described in ``execution.rst``.

The catch variable has a local scope ("let" scope) which differs from the
way variables are normally declared -- they are usually "hoisted" to the top
level of the function.

Implementing the local scope in the general case requires the creation of
a declarative lexical environment which only maps the catch variable and
uses the previous lexical environment as its parent.  This has the effect
of temporarily "masking" a variable of the same name, e.g.::

  var e = "foo"; print(e);

  try { throw new Error("error"); }
  catch (e) { print(e); }

  print(e);

prints::

  foo
  Error: error
  foo

We would like to avoid emitting code for creating and tearing down such
an environment, as it is very often not needed at all.  Instead, the
error caught can be bound to a register (only) at compile time.

To do so, the compiler would need to record some information about the
contents of the catch clause in pass 1, so that the compiler would know
in pass 2 if the environment record will be needed and emit the necessary
opcodes only when necessary.  (The "statement number" would be enough to
identify the statement on the second pass.)

The current compiler does not have the necessary intelligence to avoid
creating a lexical environment, so the environment is currently always
established when the catch-clause activates.

There is a small footprint impact in having the declarative environment
established for the duration of the catch clause.  The TRYCATCH flags
indicate that the environment is needed, and supplies the variable
name through a constant.  There is a run-time penalty for this to
(1) establish the lexical environment and associated book-keeping, and
(2) access to the variable within the catch clause will happen through
the slow path primitives (GETVAR, PUTVAR, etc).  The latter is a limitation
in the current lexical environment model, where an identifier is either
bound as a normal property of the lexical environment object, or is bound
to a *function-wide* register.  (This will need to change anyway for ES6
where "let" statements are supported.)

Compiling "with" statements
===========================

A ``with`` statement requires that an object environment record is
established on entry, and cleaned up on exit.

There is no separate catch stack entry for handling ``with`` statements.
Instead, the "TCF" catcher (which implements try-catch-finally) has
enough functionality to implement the semantics of ``with`` statement,
including the automatic handling of the object environment record.

For example::

  with (A)
    B

Generates code::

   (code for A, loading result to rX)

   TRYCATCH reg_catch=rN
            var_name=none
            with_object=rX
            have_catch=false
            have_finally=false
            catch_binding=false
            with_binding=true
   INVALID
   JUMP done

   (code for B)
   ENDTRY

 done:

Note that neither a "catch" nor a "finally" part is needed: all the
cleanup handles either when the catcher is unwound by an error, or
by ENDTRY (which of course performs an unwind).

Compiling "for"/"for-in" statements
===================================

Four variants
-------------

Parsing a for/for-in statement is a bit complicated because there are
four variants which need different code generation:

1. for (ExpressionNoIn_opt; Expression_opt; Expression_opt) Statement

2. for (var VariableDeclarationListNoIn; Expression_opt; Expression_opt) Statement

3. for (LeftHandSideExpression in Expression) Statement

4. for (var VariableDeclarationNoIn in Expression) Statement

Distinguishing the variants from each other is not easy without
back-tracking.  If back-tracking is avoided, any code generated before
the variant is determined needs to be valid for all potential variants
being considered.  Also, no SyntaxErrors can be thrown in cases where
one variant would parse correctly.

There are also tricky control flow issues related to each variant.
Because code is generated while parsing, control flow often needs to
be implemented rather awkwardly.

Note that the ``in`` token serves two independent roles in Ecmascript:
(1) as a membership test in ``"foo" in y`` and (2) as part of the for-in
iterator syntax.  These two uses have entirely different semantics and
compile entirely different code.

Semantics notes on variant 1
----------------------------

Nothing special.

Semantics notes on variant 2
----------------------------

Like all Ecmascript variable declarations, the declaration is "hoisted" to
the top of the function while a possible initializer assignment only happens
when related code is executed.

There can be multiple variable declarations variant 2, but only one in
variant 4.

Semantics notes on variant 3
----------------------------

Variants 1 and 3 cannot be trivially distinguished by looking ahead a
fixed number of tokens, which seems counterintuitive at first.  This is
the case because a LeftHandSideExpression production in E5.1 allows for
e.g. function calls, 'new' expressions, and parenthesized arbitrary
expressions.

Although pure E5.1 functions cannot return left-hand-side values, native
functions are allowed to do so if the implementation wishes to support
it.  Hence the syntax supports such cases, e.g.::

  for (new Foo().bar() in quux) { ... }

This MUST NOT cause a SyntaxError during parsing, but rather a
ReferenceError at runtime.

A valid left-hand-side expression (such as an identifier) may also be
wrapped in one or more parentheses (i.e., an arbitrary number of tokens)::

  for ( (((i))) in [ 'foo', 'bar' ] ) { }
  print(i);

  // -> prints 1

The comma expression semantics requires that every comma expression
part is coerced with ``GetValue()``, hence a comma expression is *not*
normally a valid left-hand-side expression::

  for ( ("foo", i) in [ 'foo', 'bar' ] ) { }

  // -> ReferenceError (not a SyntaxError, though)

Again, if a native function is allowed to return a Reference, a comma
expression could be a valid left-hand-side expression, but we don't
support that.

A valid left-hand-side expression may also involve multiple property
reference steps with side effects.  The E5.1 specification allows some
leeway in implementing such expressions.  Consider, e.g.::

  y = { "z": null };
  x = {
    get y() { print("getter"); return y; }
  }
  for (x.y.z in [0,1]) {}

Such an expression may (apparently) print "getter" either once or multiple
times: see E5.1 Section 12.6.4, step 6.b which states that the left-hand-side
expression "may be evaluated repeatedly".  This probably also implies that
"getter" can also be printed zero times, if the loop body is executed zero
times.  At least V8 and Rhino both print "getter" two times for the example
above, indicating that the full code for the left-hand-side expression (if
it requires any code emission beyond a property/variable assignment) is
evaluated on every loop.

Another example of the evaluation order for a "for-in" statement::

  function f() { throw new Error("me first"); }
  for ("foo" in f()) {}

The code must throw the "me first" Error before the ReferenceError related
to an invalid left-hand-side.

A valid left-hand-side expression must ultimately be either a variable or a
property reference.  Because we don't allow functions to return references,
any left-hand-side expression involving a function call or a 'new' expression
should cause a ReferenceError (but not a compile time SyntaxError).  In fact,
the only acceptable productions for LeftHandSideExpression are::

  LeftHandSideExpression -> NewExpression
  NewExpression          -> MemberExpression
  MemberExpression       -> MemberExpression [ Expression ]
                          | MemberExpression . Expression
                          | PrimaryExpression
  PrimaryExpression      -> this
                          | Identifier
                          | ( Expression )

Actual implementations seem to vary with respect to checking the syntax
validity of the LeftHandSideExpression.  For instance, V8 accepts an
Expression which is not necessarily a valid LeftHandSideExpression
without throwing a SyntaxError, but then throws a ReferenceError at
run time::

  > function f() { for (a+b in [0,1]) {} }
  undefined
  > f()
  ReferenceError: Invalid left-hand side in for-in

This is technically incorrect.

Rhino gives a SyntaxError::

  js> function f() { for (a+b in [0,1]) {} }
  js: line 1: Invalid left-hand side of for..in loop.

So, a passable loose implementation is to parse the LeftHandSideExpression
as just a normal expression, and then check the final intermediate value.
If it is a property or variable reference, generate the respective iteration
code.  Otherwise generate a fixed ReferenceError throw.

Semantics notes on variant 4
----------------------------

There can be only one declared variable.  However, the variable may have
an initializer::

  for (var i = 8 in [ 0, 1 ]) { ... }

The initializer cannot be safely omitted.  There may be side effects and
the initialized value *can* be accessed in some cases, e.g.::

  function f() {
    function g() { print(i); return [0,1] };
    for (var i = 8 in g()) { print(i); }
  }

  f();  // -> prints 8, 0, 1

Control flow for variant 1
--------------------------

Control flow for ``for (A; B; C) D``::

    LABEL N
    JUMP L4  ; break
    JUMP L2  ; continue

    (code for A)

  L1:
    (code for B)
    (if ToBoolean(B) is false, jump to L4)
    JUMP L3

  L2:
    (code for C)
    JUMP L1

  L3:
    (code for D)
    JUMP L2

  L4:
    ; finished

If A is an empty expression, no code is omitted.  If B is an empty expression,
it is considered "true" for loop termination (i.e. don't terminate loop) and
can be omitted ("JUMP L3" will occur at L1).  If C is empty it can be omitted
("JUMP L1" will occur at L2); more optimally, the "JUMP L2" after L3 can be
changed to a direct "JUMP L1".

Control flow for variant 2
--------------------------

Control flow for variant 2 is the same as for variant 1: "code for A" is
replaced by the variable list assignment code for 1 or more variables.

Control flow for variant 3
--------------------------

Control flow for ``for (A in C) D``::

    ; Allocate Rx as temporary register for loop value
    ; Allocate Re as enumerator register

    JUMP L2

  L1:
    (code for A)
    (assign Rx to the variable/property of the left-hand-side expression A)
    JUMP L3

  L2:
    (code for C)
    (initializer enumerator for value of C into Re)
    JUMP L4

  L3:
    (code for D)

  L4:
    (if enumerator Re is finished, JUMP to L5)
    (else load next enumerated value to Rx)
    JUMP L1

  L5:
    ; finished

Control flow for variant 4
--------------------------

Control flow for ``for (var A = B in C) D`` is similar to that of variant 3.
If the variable declaration has an initializer (B), it needs to be evaluated
before the enumerator target expression (C) is evaluated::

    ; Allocate Rx as temporary register for loop value
    ; Allocate Re as enumerator register

    (code for B)
    (code for assigning result of B to variable A)
    JUMP L2

  L1:
    (assign Rx to the variable A)
    JUMP L3

  L2:
    (code for C)
    (initializer enumerator for value of C into Re)
    JUMP L4

  L3:
    (code for D)

  L4:
    (if enumerator Re is finished, JUMP to L5)
    (else load next enumerated value to Rx)
    JUMP L1

  L5:
    ; finished

Compiling without backtracking
------------------------------

The first token after the left parenthesis determines whether we're parsing
variant 1/3 or variant 2/4: a ``var`` token can never begin an expression.

Parsing variant 2/4 without backtracking:

* Parse ``var``

* Parse identifier name

* Check whether next token is the equal sign; if so:

  - Parse equal sign

  - Parse assignment value expression as AssignmentExpressionNoIn:
    terminate parsing if ``in`` encountered, and use the "rbp" argument
    to start parsing at the "AssignmentExpression" binding power level

* If the next token is ``in``, we're dealing with variant 4:

  - The code emitted for the variable assignment is proper for variant 4

  - The variable identifier should be used for the loop iteration

* Else we're dealing with variant 2.

  - The code emitted for the variable assignment is proper for variant 2

  - There may be further variable declarations in the declaration list.

Parsing variant 1/3 without backtracking is a bit more complicated.  An
important observation is that:

* The first expression (ExpressionNoIn_opt) before semicolon in variant 1
  cannot contain a top-level ``in`` token

* The expression (LeftHandSideExpression) before ``in`` also cannot contain
  a top-level ``in`` token

This observation allows the following compilation strategy:

* Parse an Expression, prohibiting a top-level ``in`` token and keeping
  track whether the expression conforms to LeftHandSideExpression.
  Any code generated during this parsing is correct for both variant 1
  and variant 3.

* After Expression parsing, check the next token; if the next token is
  an ``in``, parse the remainder of the statement as variant 3.

* Else, if the next token is a semicolon, parse the remainder of the
  statement as variant 1.

* Else, SyntaxError.

Note that if the E5.1 syntax allowed a top-level ``in`` for variant 1,
this approach would not work.

Compiling "do-while" statements
===============================

There is a bug filed at:

* https://bugs.ecmascript.org/show_bug.cgi?id=8

The bug is about the expression::

  do{;}while(false)false

which is prohibited in the specification but allowed in actual implementations.
The syntax error is that a ``do`` statement is supposed to be followed by a
semicolon and since there is no newline following the right parenthesis, an
automatic semicolon should not be allowed.

The workaround in the current implementation is a special flag for automatic
semicolon insertion (ALLOW_AUTO_SEMI_ALWAYS).  If the flag is set, automatic
semicolon insertion is allowed even when no lineterm is not present before the
next token.

Compiling "switch" statements
=============================

Compiling switch statements is not complicated as such, but switch statement
has a bit tricky control flow.  Essentially there are two control paths:
the "search" code path which looks for the first matching case (or the
default case), and the "case" code path which executes the case statements
starting from the first match, falling through where appropriate.

The code generated for this matching model is quite heavy in JUMPs.
It would be preferable to structure the code differently, e.g. first emit
all checks, and then emit all statement code.  Intermediate jumps would
not be required at least in the statement code in this case.  However, this
would require multi-pass parsing or construction of an intermediate
representation, which the current multi-pass model explicitly avoids.

The algorithm in E5.1 Section 12.11 seems to contain some ambiguity,
e.g. for a switch statement with a default clause, what B statements
are iterated in step 9 in each case?  The intent seems clear though,
although the text is not.  See:

* https://bugs.ecmascript.org/show_bug.cgi?id=345

See ``test-dev-switch*.js``.

Sometimes switch-case statements are used with a large number of integer
case values.  For example, a processor simulator would commonly have such
a switch for decoding opcodes::

    switch (opcode) {
    case 0: /* ... */
    case 1: /* ... */
    case 2: /* ... */
    /* ... */
    case 255: /* ... */
    }

It would be nice to detect such structures and handle it using some sort
of switch value indexed jump table.  Doing so would need more state than is
currently available for the compiler, so switch-case statements like this
generate quite suboptimal bytecode at present.  This is definite future work.

Compiling "break"/"continue" (fast and slow)
============================================

A "fast" break/continue jumps directly to the appropriate jump slot
of the matching LABEL instruction.  The jump slot then jumps to the
correct place; in case of BREAK, the jump slot jumps directly to
ENDLABEL.  The peephole optimizer then optimizes the extra jump,
creating a direct jump to the desired location.

A "fast" break/continue cannot cross a TCF catcher (i.e. a 'try'
statement or a 'with' statement), and the matching label must be
the innermost label (otherwise a LABEL catcher would be bypassed).

A "slow" break/continue uses a ``longjmp()`` and falls back to the
generic, always correct longjmp handler.

Compiling "return"
==================

Compiling a ``return`` statement is mostly trivial, but tail calls pose
some interesting problems.

If the return value is generated by a preceding ``CALL`` opcode, the call
can be flagged a tail call.  The ``RETURN`` opcode is still emitted just
in case, if there's some feature preventing the tail call from happening
at run time -- for example, the call target may be a native function (which
are never tail called) or have a ``use duk notail`` directive which
prevents tail calling the function.

Compiling "throw" statements
============================

A ``throw`` is never "fast"; we always use the longjmp handler to
process them.

Compiling logical expressions
=============================

Ecmascript has three logical operators: binary operators ``&&`` and ``||``,
and a unary operator ``!``.  The unary logical NOT operator coerces its
argument to a boolean value and negates the result (E5.1 Section 11.4.9).
The binary AND and OR operator employ ordered, short circuit evaluation
semantics, and the result of a binary operation is one of its arguments,
which is **not** coerced to a boolean value (E5.1 Section 11.11).

The Ecmascript ``ToBoolean()`` specification function is used to coerce
values into booleans (E5.1 Section 9.2) for comparison purposes.  The
following values are coerced to ``false``: ``undefined``, ``null``,
``false``, ``+0``, ``-0``, ``NaN``, ``""``.  All other values are coerced
to ``true``.  Note that the ``ToBoolean`` operation is side-effect free,
and cannot throw an error.

Evaluation ordering and short circuiting example using Rhino::

  js> function f(x,y) { print("f called for:", y); return x; }
  js> function g(x,y) { print("g called for:", y); throw new Error("" + x); }
  js>
  js> // Illustration of short circuit evaluation and evaluation order
  js> // (0/0 results in NaN)
  js> var a = f(1,"first (t)") && f(0,"second (f)") || f(0/0,"third (f)") && g(0,"fourth (err)");
  f called for: first (t)
  f called for: second (f)
  f called for: third (f)
  js> print(a);
  NaN

The first expression is evaluated, coerced to boolean, and since it coerces
to ``true``, move on to evaluate the second expression.  That coerces to
``false``, so the first AND expression returns the number value ``0``, i.e.
the value of the second expression (which coerced to ``false`` for comparison).
Because the first part of the OR coerces to ``false``, the second part is
evaluated starting from the third expression (``NaN``).  Since ``NaN`` coerces
to ``false``, the fourth expression is never evaluated.  The result of the
latter AND expression is ``NaN``, which also becomes the final value of the
outer OR expression.

Code generation must respect the ordering and short circuiting semantics of
Ecmascript boolean expressions.  In particular, short circuiting means that
binary logical operations are not simply operations on values, but must rather
be control flow instructions.  Code generation must emit "skip jumps" when
generating expression code, and these jumps must be back-patched later.  It
would be nice to generate a minimum amount of jumps (e.g. when an AND
expression is contained by a logical NOT).

Logical expressions can be used in deciding the control flow path in a
control flow statement such as ``if`` or ``do-while``, but the expression
result can also be used and e.g. assigned to a variable.  For optimal code
generation the context where a logical expression occurs matters; for example,
often we don't need the final evaluation result but only its "truthiness".
The current compiler doesn't take advantage of this potential because there's
not enough state information to do so.

Let's look at the code generation issues for the following::

  if (!((A && B) || (C && D && E) || F)) {
    print("true");
  } else {
    print("false");
  }

One code sequence for this would be::

  start:
        (t0 <- evaluate A)
        IF        t0, 1            ; skip if (coerces to) true
        JUMP      skip_and1        ; AND is done, result in t0 (= A)
        (t0 <- evaluate B)
        IF        t0, 1            ; skip if (coerces to) true
        JUMP      skip_and1        ; AND is done, result in t0 (= B)
        ; first AND evaluates to true, result in t0 (= B)
        JUMP        do_lnot

  skip_and1:
        (t0 <- evaluate C)
        IF        t0, 1
        JUMP      skip_and2
        (t0 <- evaluate D)
        IF        t0, 1
        JUMP      skip_and2
        (t0 <- evaluate E)
        IF        t0, 1
        JUMP      skip_and3
        ; second AND evaluates to true, result in t0 (= E)
        JUMP      do_lnot

  skip_and2:
        (t0 <- evaluate F)
        IF        t0, 1
        JUMP      skip_and3
        ; third AND evaluates to true, result in t0 (= F)
        JUMP      do_lnot

  skip_and3:
        ; the OR sequence resulted in a value (in t0) which
        ; coerces to false.

        ; fall through to do_lnot

  do_lnot:
        ; the AND/OR part is done, with result in t0.  Note that
        ; all code paths must provide the result value in the same
        ; temporary register.

        LNOT      t0, t0           ; coerce and negate
        IF        t0, 1            ; skip if true
        JUMP      false_path

  true_path:
        (code for print("true"))
        JUMP      done

  false_path:
        (code for print("false"))
        ; fall through

  done:
        ; "if" is done

Because the result of the logical NOT is not actually needed, other than to
decide which branch of the if statement to execute, some extra jumps can be
eliminated::

  start:
        (t0 <- evaluate A)
        IF        t0, 1            ; skip if (coerces to) true
        JUMP      skip_and1        ; AND is done, result in t0 (= A)
        (t0 <- evaluate B)
        IF        t0, 1            ; skip if (coerces to) true
        JUMP      skip_and1        ; AND is done, result in t0 (= B)
        JUMP      false_path

  skip_and1:
        (t0 <- evaluate C)
        IF        t0, 1
        JUMP      skip_and2
        (t0 <- evaluate D)
        IF        t0, 1
        JUMP      skip_and2
        (t0 <- evaluate E)
        IF        t0, 1
        JUMP      skip_and3
        JUMP      false_path

  skip_and2:
        (t0 <- evaluate F)
        IF        t0, 1
        JUMP      skip_and3
        JUMP      false_path

  skip_and3:
        ; the expression inside LNOT evaluated to false, so LNOT would
        ; yield true, and we fall through to the true path

  true_path:
        (code for print("true"))
        JUMP      done

  false_path:
        (code for print("false"))
        ; fall through

  done:
        ; "if" is done

Which can be further refined to::

  start:
        (t0 <- evaluate A)
        IF        t0, 1            ; skip if (coerces to) true
        JUMP      skip_and1        ; AND is done, result in t0 (= A)
        (t0 <- evaluate B)
        IF        t0, 0            ; skip if (coerces to) false (-> skip_and1)
        JUMP      false_path

  skip_and1:
        (t0 <- evaluate C)
        IF        t0, 1
        JUMP      skip_and2
        (t0 <- evaluate D)
        IF        t0, 1
        JUMP      skip_and2
        (t0 <- evaluate E)
        IF        t0, 0            ; -> skip_and2
        JUMP      false_path

  skip_and2:
        (t0 <- evaluate F)
        IF        t0, 0            ; -> skip_and3
        JUMP      false_path

  skip_and3:
        ; the expression inside LNOT evaluated to false, so LNOT would
        ; yield true, and we fall through to the true path

  true_path:
        (code for print("true"))
        JUMP      done

  false_path:
        (code for print("false"))
        ; fall through

  done:
        ; "if" is done

The current compilation model for logical AND and OR is quite simple.
It avoids the need for explicit back-patching (all back-patching state
is kept in C stack), and allows generation of code on-the-fly.  Although
logical AND and OR expressions are syntactically *left-associative*, they
are parsed and evaluated in a *right-associate* manner.

For instance, ``A && B && CC`` is evaluated as ``A && (B && C)``, which
allows the which processes the first logical AND to generate the code
for the latter part ``B && C`` recursively, and then back-patch a skip
jump over the entire latter part (= short circuiting the evaluation).

Unnecessary jumps are still generate between boundaries of AND and OR
expressions (e.g. in ``A && B || C && D``).  These jumps are usually
"straightened out" by the final peephole pass, possibly leaving unneeded
instructions in bytecode, but generating more or less optimal run-time
jumps.

Note that there are no opcodes for logical AND and logical OR.  They would
not be useful because short-circuit evaluation requires them to be control
flow instructions rather than logical ones.

Compiling function calls; direct eval
=====================================

Ecmascript E5.1 handles **direct** ``eval`` calls differently from other
``eval`` calls.  For instance, direct ``eval`` calls may declare new
variables in the calling lexical scope, while variable declarations in
non-direct ``eval`` calls will go into the global object.  See:

* E5.1 Section 10.4.2: Entering Eval Code

* E5.1 Section 15.1.2.1.1: Direct Call to Eval

E5.1 Section 15.1.2.1.1 states that:

  A direct call to the eval function is one that is expressed as a
  CallExpression that meets the following two conditions:

  The Reference that is the result of evaluating the MemberExpression
  in the CallExpression has an environment record as its base value
  and its reference name is "eval".

  The result of calling the abstract operation GetValue with that
  Reference as the argument is the standard built-in function defined
  in 15.1.2.1.

Note that it is *not* required that the binding be actually found in
the global object, a local variable with the name ``eval`` and with
the standard built-in ``eval()`` function as its value is also a
direct eval call.

Direct ``eval`` calls cannot be fully detected at compile time, as
we cannot always know the contents of the environment records outside
the current function.  The situation can even change at run time.
See ``test-dev-direct-eval.js`` for an illustration using an
intercepting ``with`` environment.

On the other hand, partial information can be deduced; in particular:

* If a function never performs a function call with the identifier name
  ``eval``, we *can* be sure that there are no direct eval calls, as the
  condition for the identifier name is never fulfilled.

The current approach is quite conservative, favoring correctness and
simple compilation over performing complicated analysis.  The current
approach to handle a function call made using the identifier ``eval``
as follows:

* Flag the function as "tainted" by eval, which turns off most function
  optimizations to ensure semantic correctness.  For example, the varmap
  is needed and the ``arguments`` object must be created on function entry
  in case eval code accesses it.

* Call setup is made normally, it doesn't matter whether ``eval`` is bound
  to a register or accessed using ``GETVAR``.  It is perfectly fine for a
  direct eval to happen through a local variable.

* Set the ``DUK_BC_CALL_FLAG_EVALCALL`` flag for the CALL bytecode
  instruction to indicate that the call was made using the identifier
  ``"eval"``.

Then at run time:

* ``CALL`` handler notices that ``DUK_BC_CALL_FLAG_EVALCALL`` is set.  It
  then checks if the target function is the built-in eval function, and if
  so, triggers direct eval behavior.

Identifier-to-register bindings
===============================

Varmap, fast path and slow path
-------------------------------

Identifiers local to a function are (1) arguments, (2) variables, (3) function
declarations, and (4) dynamic bindings like "catch" or "let" bindings.  Local
identifiers are handled in one of two ways:

* An identifier can be bound to a fixed register in the value stack frame
  allocated to the function.  For example, an identifier named ``"foo"``
  might be bound to register 7 (R7).  This is possible when the identifier
  is known at compile time, a suitable register is available, and when the
  identifier binding is not deletable (which is usually, but not always,
  the case).

* An identifier can be always accessed explicitly by name, and its value
  will be stored in an explicit environment record object.  This is possible
  in all cases, including dynamically established and non-deletable bindings.

Only function code identifiers can be register mapped.  For global code
declarations are mapped to the global object (an "object binding").  For
non-strict eval code the situation is a bit different: a variable declaration
inside a direct eval call will declare new variable to the *containing scope*.
Such bindings are also deletable whereas local declarations in a function are
not.

An example of a function and identifier binding::

  function f(x, y) {
    // Arguments 'x' and 'y' can be mapped to registers R0 and R1.

    // Local variable can be mapped to register R2.
    var a = 123;

    // Dynamically declared variable is created in an explicit environment
    // record and is not register mapped.
    eval('var b = 321');
  }

When the compiler encounters an identifier access in the local function it
looks through the variable map ("varmap") which records identifier names and
their associated registers.  If the identifier is found in the varmap, it is
safe to access the identifier with a direct register reference which is called
a "fast path" access.  This is safe because only non-deletable bindings are
register mapped, so there's no way that the binding would later be removed e.g.
by uncontrolled eval() calls.  There's also nothing that could come in the way
to capture the reference.  For example, the Ecmascript statement::

  a += 1;

could be compiled to the following when "a" is in the varmap and mapped to R2::

  INC R2

When the identifier is not in the varmap, the compiler uses the "slow path"
which means addressing identifers by name.  For example, the Ecmascript
statement::

  b += 1;

could be compiled to the following when "b" is *not* in the varmap::

  ; c3 = 'b'
  ; r4 = temp reg

  GETVAR r4, c3  ; read 'b' to r4
  INC r4
  PUTVAR r4, c3  ; write r4 to 'b'

The GETVAR and PUTVAR opcodes (and other slow path opcodes) are handled by
the executor by looking up the variable name through explicit environment
record objects, which is more or less equivalent to a property lookup
through an object's prototype chain.  The slow path is available at any
time for looking up any identifier, including a register mapped one.

When a function call exits, the executor copies any register mapped values
from the value stack frame into an environment record object so that any
inner functions which are still active can continue to access values held
by the outer function.  An example of inner functions accessing a "closed"
outer function::

  function outer(val) {
    var foo = 'bar';
    return function inner() {
      print(val);
      print(foo);
    }
  }

  // Once outer() returns, 'fn' refers to a function which can still see
  // into the variables held in outer().
  var fn = outer(123);

  fn();  // prints 123, "bar"

Basic optimizations
-------------------

A few optimizations are applied to the conceptual model described above:

* Creation of a lexical environment object is delayed for a function call
  when possible, so that an actual object is only created when necessary.
  Most functions don't establish new local bindings so there's no need to
  create an explicit lexical environment object for every function call.

* When a function exits, identifier values are copied from registers to a
  lexical environment object only when necessary -- e.g. when the function
  has inner functions or eval calls.  The compiler makes a conservative
  estimate when this step can be omitted for better performance.

  Here's an example when an eval() is enough to access function bindings
  after function exit::

      duk> function f(x) { var foo=123; return eval(x); }
      = undefined
      duk> g = f('(function myfunc() { print(foo); })');
      = function myfunc() {/* ecmascript */}
      duk> g()
      123
      = undefined

* When there is no possibility of slow path accesses to identifiers nor any
  constructs which might otherwise access the varmap (direct eval calls,
  inner functions, etc), the compiler can omit the "varmap" from the final
  function template.  However, when debugger support is enabled, varmap
  is always kept so that the debugger can inspect variable names for all
  functions.

Arguments object
----------------

The ``arguments`` object is special and quite expensive to create when
calling a function.  The need to create an arguments objects is recorded
into the final function template with the ``DUK_HOBJECT_FLAG_CREATEARGS``
flag which is checked in call handling.

The compiler can omit argument object creation only when it's absolutely
certain it won't be needed.  For example the following will now cause the
arguments object to be created on function entry (sometimes unnecessarily):

* If there's an ``eval`` anywhere in the function there's a risk it will
  access the arguments object.

* If there's an identifier reference using the name ``arguments`` which
  is not shadowed the arguments object may be referenced.

Delaying arguments object creation to the point of an actual access is not
trivial because argument values may have already been mutated and they affect
arguments object creation.

Current approach
----------------

* The ``varmap`` keeps track of identifier-to-register bindings.  In the
  first pass the ``varmap`` is empty; the ``varmap`` is populated before
  the second pass.  First pass gathers argument names, variable declarations,
  and inner function declarations.

* After first pass but before second pass the effects of declaration
  binding instantiation (E5.1 Section 10.5) are considered and a ``varmap``
  is built.  The varmap contains all known identifiers, and their names
  are mapped either to an integer (= register number) or ``null``
  (identifier is declared but not register mapped).  The rather complex
  shadowing rules for arguments, variable declarations, and inner function
  declarations are handled in this step.

* ``catch`` clause bindings: handled at runtime by the try-catch-finally
  opcodes by creating an explicit lexical scope with the catch variable
  binding.  All code accessing the catch variable name inside the catch
  clause uses slow path lookups; this leaves room for future work to
  handle catch bindings better.

* ``with`` statements: handled at runtime by try-catch-finally opcodes by
  creating an explicit lexical scope indicating an "object binding".  The
  ``with_depth``, the number of nested ``with`` statements, is tracked during
  compilation.  A non-zero with_depth prevents fast path variable accesses
  entirely because potentially any identifier access is captured by the object
  binding.

* After second pass, when creating the final function template, the ``varmap``
  is cleaned up: ``null`` entries are removed and the map is compacted.

Future work
===========

Some future work (not a comprehensive list by any means), in no particular
order.

Better handling of "catch" variables, "let" bindings
----------------------------------------------------

Current handling for "catch" variables creates an explicit lexical environment
object and uses slow path for accessing the variable.  This is far from optimal
but requires more compiler state to be solved better.

Similarly the ES6 "let" binding needs efficient support to be useful.

Improve line number assignment
------------------------------

Current compiler associates opcode line numbers with the "previous token"
which is always not correct.  Add the necessary plumbing to associate opcode
line numbers more accurately.

Partial copy of variables when closing a function scope
-------------------------------------------------------

As of Duktape 1.3 when an outer function containing inner functions exits,
its lexical scope is closed with variable values copied from VM registers
(value stack frame) into an explicit scope object.  This works correctly
but causes a reference to be held for all variables in the outer scope, even
those that are *never* accessed by any inner function, see:
https://github.com/svaarala/duktape/issues/229.

This could be fixed by improving the compiler a bit:

* For every variable in the varmap, track the variable's current register
  mapping and a flag indicating if it has been referenced by an inner
  function ("keep on close").

* Whenever a function dereferences a variable not defined in the function
  itself, scan outer lexical scopes for matching variables.  If so, mark
  that variable in the outer function as being referenced by an inner
  function.  (Note that if any involved function has an eval(), all bets
  are off and conservative code must be generated, as eval() may introduce
  new bindings at run time.)

* Encode that "keep on close" flags to the final compilation result (the
  function template).  If eval()s are involved, mark all variables as
  "keep on close".

* At run time, when a function exits, copy only "keep on close" variables
  into the explicit scope object.  Other variables are then decref'd and
  finalized if appropriate.

Make ivalue manipulation shuffling aware
----------------------------------------

Current ivalue manipulation is not aware of register shuffling.  Instead,
ivalue manipulation relies on bytecode emission helpers to handle shuffling
as necessary.  Sometimes this results in sub-optimal opcode sequences (e.g.
the result of an operation shuffled to a high register and then immediately
needed in a subsequent operation).  Code quality could be improved by making
ivalue manipulation shuffling aware.

Improve support for large functions
-----------------------------------

Large functions don't produce very good code with the current compiler:

* The method of binding identifiers to registers consumes a lot of useful
  low registers which can be directly addressed by all opcodes.  It might
  be better to reserve identifiers in a non-continuous fashion so that a
  reasonable number of temporary registers could also be guaranteed to be
  in the low register range.

* The method of allocating temporaries may reserve low registers as
  temporaries which are then not available for inner expressions which
  are often more important for performance (think outer loop vs. inner
  loop).

These are not fundamental limitations of the compiler, but there's been
little effort to improve support for large functions so far, other than to
ensure they work correctly.

Chunked stream parsing with rewind
----------------------------------

For low memory environments it would be useful to be able to stream source
code off e.g. flash memory.  Because Duktape decodes the source code into
a codepoint window anyway, hiding the streaming process would be relatively
straightforward.

Adding support for streaming would involve using a callback (perhaps a pure
C callback or even an actual Duktape/C or Ecmascript callback) for providing
a chunk of source code for Duktape to decode from.  Another callback would be
needed to rewind to a specified position.  Another approach is to provide a
callback to provide at most N bytes starting from a specified offset, and let
the callback optimize for continuous reads if that's helpful.

Allowing source code compression is also preferable.  It's possible to use
an ordinary stateful compression algorithm (like deflate) for the source code,
but in a naive implementation any rewind operation means that the decompression
must restart from the beginning of the entire source text.  A more practical
approach is to use chunked compression so that semi-random access is possible
and reasonably efficient.

One more design alternative is to model the source input as a sequence of
Unicode codepoints instead of bytes, so that Duktape would just request a
sequence of codepoints starting from a certain *codepoint* offset and then
put them into the codepoint window.  The user callback would handle character
encoding as needed, which would simultaneously add support for custom source
encodings.  The downside of this approach is that the user callback needs the
ability to map an arbitrary codepoint offset to a byte offset which is an
awkward requirement for multibyte character encodings.

Context aware compilation of logical expressions
------------------------------------------------

When a logical expression occurs in an "if" statement, the final result of the
expression is not actually needed (only its truthiness matters).  Further,
the "if" statement only needs to decide between two alternative jumps, so that
the short circuit handling used by the logical expression could just jump to
those targets directly.

Improve pool allocator compatibility
------------------------------------

A small improvement would be to track opcodes and line numbers in separate
buffers rather than a single buffer with ``duk_compiler_instr`` entries.

Split compiler into multiple files
----------------------------------

Example:

* Bytecode emission

* Ivalue handling

* Expression parser

* Statement parser and entry point

Using some "memory" between pass 1 and pass 2
---------------------------------------------

The multi-pass compilation approach allows us to build some "memory"
to help in code generation.  In fact, pass 1 is now used to discover
variable declarations, which is already a sort of memory which affects
code generation later.

These would help, for example:

* Avoiding LABEL sites for iteration structures not requiring them.
  For instance, an iteration statement without an explicit label
  and with no "break" or "continue" statement inside the iteration
  construct does not need a LABEL site.

* More simply, one could simply record all label sites created in
  pass 1, and note whether any break/continue targeted the label
  site in question.  On pass 2, this state could be consulted to
  skip emitting label sites.

Because the source is identical when reparsed, it is possible to address
such "memory" using e.g. statement numbering, expression numbering, or
token numbering, where the numbers are assigned from start of the function
(the rewind point).

Compile time lookups for non-mutable constants
----------------------------------------------

Variable lookups are represented by ivalues which identify a variable by name.
Eventually they get converted to concrete code which reads a variable either
directly from a register, or using a slow path GETVAR lookup.

This could be improved in several ways.  For example, if support for ``const``
was added, the ivalue conversion could detect that the variable maps to a
constant in the current function or an outer function (with the necessary
checks to ensure no "capturing" bindings can be established by e.g. an eval).
The ivalue could then be coerced into a registered constant, copying the value
of the constant declaration.

Slow path record skip count
---------------------------

When a slow path access is made, some environment record lookups can be
skipped if the records belong to functions which have no potential for
dynamically introduced bindings.  For example::

  var foo = 123;  // global

  function func1() {
      var foo = 321;

      function func2() {
          var bar = 432;

          function func3() {
              var quux = 543;

              // The slow path lookup for 'hello' can skip func3, func2, and
              // func1 entirely because it will never match there.  In other
              // words, we could look up 'hello' directly from the global object.
              print('hello');

              // The slow path lookup for 'foo' could bypass func3 and func2,
              // and begin from func1.
              print(foo);
          }
      }
  }

Any function with an ``eval()`` will potentially contain any binding, "with"
statements must be handled correctly, etc.

This optimization would be nice for looking up global bindings like ``print``,
``Math``, ``Array``, etc.

The technical change would be for e.g. GETVAR to get an integer argument
indicating how many prototype levels to skip when looking up the binding.

Slow path non-configurable, non-writable bindings
-------------------------------------------------

When a slow path access is certain to map to a non-configurable, non-writable
value, the value could be copied into the function's constant table and used
directly without an actual slow path lookup at run time.  There are a few
problems with this:

* At the moment constants can only be numbers and strings, and this affects
  bytecode dump/load.  If a constant were e.g. a function reference, bytecode
  dump/load wouldn't be able to handle it without some backing information to
  reconstruct the reference on bytecode load.

* Even though a binding is non-writable and non-configurable, it can still be
  changed by C code with ``duk_def_prop()``.  This is intentional so that C
  code has more freedom for sandboxing and such.  For such environments this
  optimization might not always be appropriate.

Better handling of shared constant detection
--------------------------------------------

When a new constant is introduced, the current implementation linearly
walks through existing constants to see if one can be reused.  This
walk is capped to ensure reasonable compilation times even for functions
with a large number of constants.

A better solution would be to use a faster search structure for detecting
shared constants, e.g. a hash map with more flexible keys than in Ecmascript
objects (perhaps one of the ES6 maps).

Better switch-case handling
---------------------------

It would be nice to support at least dense integer ranges and use a jump table
to handle them.  This is important, for example, if a switch-case implements
some kind of integer-dependent dispatch such as an opcode decoder.
