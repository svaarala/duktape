===================
Regular expressions
===================

This document describes the Duktape ``RegExp`` built-in implementation.

Overview
========

Implementing a regular expression engine into very small space is
challenging.  See the following three excellent articles by Russ Cox
for background:

* http://swtch.com/~rsc/regexp/regexp1.html

* http://swtch.com/~rsc/regexp/regexp2.html

* http://swtch.com/~rsc/regexp/regexp3.html

Ecmascript regular expression set is described in E5 Section 15.10,
and includes:

* Disjunction

* Quantifiers, counted repetition and both greedy and minimal variants

* Assertions, negative and positive lookaheads

* Character classes, normal and inverted

* Captures and backreferences

* Unicode character support

* Unanchored matching (only) (e.g. ``/x/.exec('fooxfoo')`` matches ``'x'``)

Counted repetition quantifiers, assertions, captures, and backreferences
all complicate a non-backtracking implementation considerably.  For this
reason, the built-in regular expression implementation, described below,
uses a backtracking approach.

The two basic goals of the built-in implementation are Ecmascript compliance
and compactness.  More generally, the following prioritized requirements
should be fulfilled:

#. Ecmascript compatibility

#. Compactness

#. Avoiding deep or unbounded C recursion, and providing recursion and
   execution time sanity limits

#. Regexp execution performance

#. Regexp compilation performance

Further, it should be possible to leave out regexp support during
compilation, or to plug in a more powerful existing regexp engine should
it be needed by the user.

Architecture
============

The basic implementation approach consists of three parts:

#. A regexp tokenizer, which reuses the lexer model of the Ecmascript
   tokenizer and generates a token stream

#. A regexp compiler, which takes the token stream and produces compiled
   regexps (represented as interned strings) and *normalized* regexp
   patterns (see E5 Section 15.10.6) 

#. A regexp executor, which takes a compiled regexp and an input stream
   and produces match results

Case insensitive matching provides some surprising challenges in handling
character ranges, see discussion of canonicalization below.

Tokenizer
---------

The tokenizer is implemented in ``duk_lexer.c`` and is quite simple and
straightforward.  It shares the character decoding and character window
model of the Ecmascript tokenizer.

The two main functions are:

* ``duk_lexer.c:duk_lexer_parse_re_token()`` which parses a regexp token,
  such as character, quantifier, etc.

* ``duk_lexer.c:duk_lexer_parse_re_ranges()`` which parses character class
  ranges (a bit tricky due to canonicalization)

Quantifiers are fully parsed during tokenization, resulting in only two
types of quantifier tokens, greedy and minimal, with each having a minimum
count and a maximum count.  An unspecified maximum count (infinite) is
encoded as the maximum unsigned 32-bit value 0xffffffff, which is quite
reasonable considering that Ecmascript strings cannot be longer than that.
The quantifier maximum value 0xffffffff is treated specially by the compiler
too.

Character classes could be parsed and encoded into token values.  However,
this would mean that the token value would need to contain an arbitrary
number of character ranges.  Also, character range normalization for case
insensitive matching requires some special treatment.  For these reasons,
the lexer simply produces a start token for normal or inverted character
class (``[`` or ``[^``) and the lexer and the compiler co-operate to
process the character ranges.  See, for instance:

* ``duk_regexp_compiler.c:parse_disjunction()``, and

* ``duk_lexer.c:duk_lexer_parse_re_ranges()``.

See the detailed discussion on canonicalization and case-insensitive
matching below.

Compiler
--------

The compiler is implemented in ``duk_regexp_compiler.c``.  The main
functions are:

* ``duk_regexp_compiler.c:duk_regexp_compile()`` which provides the
  compilation wrapper (e.g. initializes the compilation context,
  including lexer state, etc) and also produces the *normalized*
  regexp source required by E5 Section 15.10.6.

* ``duk_regexp_compiler.c:parse_disjunction()`` which parses a disjunction
  (including atoms, quantifiers, and assertions) and calls itself
  recursively to implement lookaheads and capture/non-capture groups.

The code generation model is shaped by the fact that linear bytecode
generation is not possible if a regexp is parsed linearly without lookahead.
In other words, one needs to choose between non-linear bytecode generation
and non-linear parsing.  For instance, to compile ``a|b`` one would first
generate the bytecode for ``a``, and only then notice that the bytecode for
``a`` must be preceded by bytecode to handle the disjunction.  Further, the
disjunction would need to be updated for each new alternative.  Similar
problems apply to other constructs; consider, for instance, the quantifier
in ``(a|b)+``.

One common approach to deal with this problem is to first produce an
intermediate representation (e.g. a parse tree), and then perform compilation
using the more convenient intermediate representation.  However, an
intermediate representation increases code size considerably, so we try
to make do without one.

Instead, the code generation model attempts to work around these
limitations as follows:

* The regexp bytecode generation is based on a byte buffer which holds
  currently generated code.  New bytecode instructions are either appended
  to the buffer, or inserted into some earlier position e.g. to complete
  jump offsets.

* Bytecode is "PC-relative".  In particular, bytecode jump/branch offsets
  are PC-relative (relative to the first byte of the subsequent instruction,
  to be exact) which allows code blocks to be moved and copied freely
  without breaking them.  This works as long as there are no PC-relative
  jumps over the "spliced" sections.  There are a few restrictions, though,
  discussed below.

* Code generation is bottom-up: a bytecode snippet is emitted for each
  token, and these snippets are combined (concatenated, copied, etc) to
  form more complex matchers.  More complex expressions can backpatch jump
  offsets, insert new bytecode into a previous position (bumping any
  following code forwards), and clone existing bytecode snippets (e.g.
  for counted quantifiers).
  
The current model has a few drawbacks:

* Insertion into the middle of the regexp buffer requires trailing code to
  be moved (with ``memmove()``).  This can lead to quite a lot of copying
  in pathological cases.  However, regular expressions are typically so
  short that this does not really matter in practice, and keeps the
  implementation simple.

* Because the compiler works without an intermediate representation for the
  regexp, some of the back-patching required for code generation is a bit
  tricky.  This is the case especially for creating disjunction code (see
  the example below).

* Because bytecode is variable size (especially, encoded PC-relative jump
  offsets are variable size too!), back-patching jump offsets must be done
  carefully.  See comments in code, and discussion on jump offsets below.

Regular expressions are compiled into interned strings, containing both the
regexp flags and the actual regexp body bytecode.  This allows compiled
regexps to be conveniently stored and handled as an internal property of a
``RegExp`` instance.  The property is internal because the key for the
property uses a non-BMP character, which cannot be generated by standard
Ecmascript code, and cannot therefore be accessed by Ecmascript code.  See
the bytecode format details below.

Another output of regexp compilation is the *normalized* regular expression
pattern, described in E5 Section 15.10.6, which goes into the ``source``
property of a ``RegExp`` instance.  The normalized pattern is currently
formed simply as follows:

* If the input pattern is empty, output ``(?:)``.

* Else, look for any forward slash which is *not* preceded by a backslash.
  Replace all such occurrences with ``\/``.

A run-time instance of a ``RegExp`` is created with only the compiled
bytecode (string) and the normalized pattern as inputs.

Executor
--------

The executor is implemented in ``duk_regexp_executor.c``, see:

* ``duk_regexp_executor.c:duk_regexp_match()`` which initializes the regexp
  matcher context and contains most of the logic of E5 Section 15.10.6.2,
  except for the innermost match attempt (step 9.b).

* ``duk_regexp_executor.c:match_regexp()`` which does regexp bytecode
  execution starting from a certain input offset, calling itself recursively
  when necessary (see "current limitations" below).

The basic implementation approach is a recursive back-tracking matcher
which uses the C stack whenever recursion is needed, but explicitly avoids
doing so for *simple quantifiers*: see separate discussion on quantifiers
and backtracking.  Without the support for simple quantifiers, *every
character* matching the pattern ``/.*/`` would require one C recursion level
for back-tracking.

A regexp matcher context is maintained for matching to minimize C call
parameter count.  The current state includes ``PC``, the program counter
for bytecode, and ``SP``, the string pointer referring to the (immutable)
input string.  Among other book-keeping members, the context also contains
the current *saved pointers*, which are byte pointers to the (extended UTF-8
encoded) input string.

Saved pointers are used to implement capture groups.  The start and end
points of the capture are identified with saved pointers (two pointers
are needed per capture group).  A capture group is valid if *both* saved
pointers are valid; when in the middle of the capture group, the start
pointer is set but the end pointer is not.  Since the input string
is not modified during matching, even for case-insensitive matching, saved
pointers allow capturing without making explicit copies of the captured
values during matching.

Saving a pointer currently involves C recursion: when a pointer is saved,
the previous value is stored and the matcher is called recursively.  If
backtracking needs to happen, the previous value can be restored.  Saved
pointers are also wiped when a quantifier rematches a quantified containing
captures.  The previous pointers also need to be saved and restored in this
case.  (One could also try to erase saved pointers during backtracking based
on the saved pointer value: if we backtrack ``SP`` beyond the saved pointer,
the pointer is erased.)

The mapping between saved pointers are capture groups is described in
the following table:

+-------------+------------------------------------------+
| Saved index | Description                              |
+=============+==========================================+
| 0           | Start of entire matching substring       |
+-------------+------------------------------------------+
| 1           | End of entire matching substring         |
+-------------+------------------------------------------+
| 2           | Start of capture group 1                 |
+-------------+------------------------------------------+
| 3           | End of capture group 1                   |
+-------------+------------------------------------------+
| ...         |                                          |
+-------------+------------------------------------------+
| 2n+1        | Start of capture group n                 |
+-------------+------------------------------------------+
| 2n+2        | End of capture group n                   |
+-------------+------------------------------------------+

Memory allocation is generally avoided during regexp execution.
When it is necessary to allocate temporary buffers, all temporaries
are placed in the value stack for correct memory management in case
of errors.  Currently, memory allocation is needed during regexp
execution only to handle lookahead assertions, which need to make
a copy of saved pointers.

About safety: the Ecmascript executor should prevent user from reading
and replacing regexp bytecode.  Even so, the executor must validate all
memory accesses etc.  When an invalid access is detected (e.g. a 'save'
opcode to invalid, unallocated index) it must fail with an internal error
but not cause a segmentation fault.
  
Current limitations
-------------------

Regexp compiler
:::::::::::::::

C recursion depth limit
  The compiler imposes an artificial limit on C recursion depth
  (``DUK_USE_REGEXP_COMPILER_RECLIMIT``).  If the recursion limit
  is reached, regexp compilation fails with an (internal) error.

  The following constructs increase C recursion depth:

  * Negative or positive lookahead

  * Capture or non-capture group

Regexp atom copy limit
  Complex quantifiers with a non-zero minimum or a non-infinite maximum
  cause the quantified atom to be duplicated in regexp bytecode.  There
  is an artificial limit (``DUK_RE_MAX_ATOM_COPIES`` by default) on the
  number of copies the compiler is willing to create.  Some examples:

  * For ``/(?:a|b){10,20}/``, the atom code (``/(?:a|b)/``) is first
    copied 10 times to cover the quantifier minimum, and another 10
    times to cover the maximum.

  * For ``/(?:a|b){10,}/``, the atom code is first copied 10 times to
    cover the quantifier minimum, and the remaining (greedy) infinite
    match reuses the last emitted atom.

  Note that there is no such restriction for *simple quantifiers*, which
  can keep track of quantifier counts explicitly.

Regexp executor
:::::::::::::::

C recursion depth limit
  The executor imposes an artificial limit on C recursion depth
  (``DUK_USE_REGEXP_EXECUTOR_RECLIMIT``).  If the recursion limit
  is reached, regexp matching fails with an (internal) error.
  The following constructs increase C recursion depth:

  * Simple quantifier increases recursion depth by one when matching the
    sequel (but not for each atom).

  * Complex quantifier increases recursion depth for each atom matched and
    the sequel (e.g. ``/(?:x|x)+/`` causes C recursion for each ``x``
    character matched).

  * ``DUK_REOP_SAVE`` increases recursion depth by one (to provide capture
    backtracking), so each capture group increases C recursion depth by two.

  * Positive and negative lookahead increase recursion depth by one for
    matching the lookahead, and for matching the sequel (to provide capture
    backtracking).

  * Each alternative of a disjunction increases recursion depth by one,
    because disjunctions currently generate a sequence of n-1
    ``DUK_REOP_SPLIT1`` opcodes for an n-alternative disjunction, and the
    preferred execution path runs through each of these ``DUK_REOP_SPLIT1``
    opcodes on the first attempt.

Regexp opcode steps limit
  The execution imposes an artificial limit on the total number of regexp
  opcodes executed (``DUK_RE_EXECUTE_STEPS_LIMIT`` by default) to provide
  a safeguard against insane execution times.  The steps limit applies to
  total steps executed during e.g. ``exec()``.  The steps count is *not*
  zeroed for each attempt of an unanchored match.

  The steps limit provides a safety net for avoiding excessive or
  even infinite execution time.  Infinite execution time may currently
  happen for some empty quantifiers, so only the steps limit prevents
  them from executing indefinitely.

Empty quantifier bodies in complex quantifiers
  Empty quantifier bodies in complex quantifiers may cause unbounded
  matcher execution time (eventually terminated by the steps limit).
  There is no "progress" instruction or one-character lookahead to
  prevent multiple matches of the same empty atom.

  * Complex quantifier example: ``/(?:|)*x/.exec('x')`` is terminated by
    the steps limit.  The problem is that the empty group will match an
    infinite number of times, so the greedy quantifier never terminates.

  * Simple quantifiers have a workaround if the atom character length is
    zero: ``qmin`` and ``qmax`` are capped to 1.  This allows the atom
    to match once and possibly cause whatever side effects it may have
    (for instance, if we allowed captures in simple atoms, the capture
    could happen, once).  For instance, ``/(?:)*x/`` is, in effect,
    converted to ``/(?:){0,1}x/`` and ``/(?:){3,4}x/`` to
    ``/(?:){1,1}x/``.

  This problem could also be fixed for complex quantifiers, but the
  fix is not as trivial as for simple quantifiers.

Miscellaneous
:::::::::::::

Incomplete support for characters outside the BMP
  Ecmascript only mandates support for 16-bit code points, so this is
  not a compliance issue.

  The current implementation quite naturally processes code points above
  the BMP as such.  However, there is no way to express such characters
  in patterns (there is for instance no Unicode escape for code points
  higher than U+FFFF).  Also, the built-in ranges ``\d``, ``\s``, and
  ``\w`` and their inversions only cover 16-bit code points, so they
  will not currently work properly.

  This limitation has very little practical impact, because a standard
  Ecmascript program cannot construct an input string containing any
  non-BMP characters.

Compiled regexp and bytecode format
===================================

A regular expression is compiled into an "extended" UTF-8 string which is
interned into an ``duk_hstring``.  The extended UTF-8 string contains
flags, parameters, and code for the regexp body.  This simplifies handling
of compiled regexps and minimizes memory overhead.  The "extended" UTF-8
encoding also keeps the bytecode quite compact while allowing existing
helpers to deal with encoding and decoding.

Logically, a compiled regexp is a sequence of signed and unsigned integers.
Unsigned integers are encoded directly with "extended" UTF-8 which allows
codepoints of up to 36 bits, although integer values beyond 32 bits are not
used for compiled regexps.  Signed integers need special treatment because
UTF-8 does not allow encoding of negative values.  Thus, signed integers
are first converted to unsigned by doubling their absolute value and
setting the lowest bit if the number is negative; for example, ``6`` is
converted to ``2*6=12`` and ``-4`` to ``2*4+1=9``.  The unsigned result
(again at most 32 bits) is then encoded with "extended" UTF-8.  This
special treatment allows signed integers to be encoded with UTF-8 in the
first place, and further provides short encodings for small signed integers
which is useful for encoding bytecode jump distances.

The compiled regexp begins with a header, containing:

* unsigned integer: flags, any combination of ``DUK_RE_FLAG_*``

* unsigned integer: ``nsaved`` (number of save slots), which should be
  ``2n+2`` where ``n`` equals ``NCapturingParens`` (number of capture
  groups)

Regexp body bytecode then follows.  Each instruction consists of an opcode
value (``DUK_REOP_*``) (encoded as an unsigned integer) followed by a
variable number of instruction parameters.  Each opcode and parameter is
encoded (as described above) as a "code point".  When executing the
bytecode, program counter is maintained as a byte offset, not as an
instruction index, so all jump offsets are byte offsets (not instruction
offsets).

Jump targets are encoded as "skip offsets" relative to the first byte of
the instruction following the jump/branch.  Because the skip offset itself
has variable length, this needs to be handled carefully during compilation;
see discussion below.

Regexp opcodes
--------------

The following table summarizes the regexp opcodes and their parameters.
The opcode name prefix ``DUK_REOP_`` is omitted for brevity; for instance,
``DUK_REOP_MATCH`` is listed as ``MATCH``.

+--------------------------+-------------------------------------------------+
| Opcode                   | Description / parameters                        |
+==========================+=================================================+
| MATCH                    | Successful match.                               |
+--------------------------+-------------------------------------------------+
| CHAR                     | Match one character.                            |
|                          |                                                 |
|                          | * ``uint``: character codepoint                 |
+--------------------------+-------------------------------------------------+
| PERIOD                   | ``.`` (period) atom, match next character       |
|                          | against anything except a LineTerminator.       |
+--------------------------+-------------------------------------------------+
| RANGES                   | Match the next character against a set of       |
|                          | ranges; accept if in some range.                |
|                          |                                                 |
|                          | * ``uint``: ``n``, number of ranges             |
|                          |                                                 |
|                          | * ``2n * uint``: ranges, ``[r1,r2]`` encoded as |
|                          |   two unsigned integers ``r1``, ``r2``          |
+--------------------------+-------------------------------------------------+
| INVRANGES                | Match the next character against a set of       |
|                          | ranges; accept if not in any range.             |
|                          |                                                 |
|                          | * ``uint``: ``n``, number of ranges             |
|                          |                                                 |
|                          | * ``2n * uint``: ranges, ``[r1,r2]`` encoded as |
|                          |   two unsigned integers ``r1``, ``r2``          |
+--------------------------+-------------------------------------------------+
| JUMP                     | Jump to target unconditionally.                 |
|                          |                                                 |
|                          | * ``int``: ``skip``, signed byte offset for jump|
|                          |   target, relative to the start of the next     |
|                          |   instruction                                   |
+--------------------------+-------------------------------------------------+
| SPLIT1                   | Split execution.  Try direct execution first.   |
|                          | If fails, backtrack to jump target.             |
|                          |                                                 |
|                          | * ``int``: ``skip``, signed byte offset for jump|
|                          |   alternative                                   |
+--------------------------+-------------------------------------------------+
| SPLIT2                   | Split execution.  Try jump target first.        |
|                          | If fails, backtrack to direct execution.        |
|                          |                                                 |
|                          | * ``int``: ``skip``, signed byte offset for jump|
|                          |   alternative                                   |
+--------------------------+-------------------------------------------------+
| SQMINIMAL                | Simple, minimal quantifier.                     |
|                          |                                                 |
|                          | * ``uint``: ``qmin``, minimum atom match count  |
|                          |                                                 |
|                          | * ``uint``: ``qmax``, maximum atom match count  |
|                          |                                                 |
|                          | * ``skip``: signed byte offset for sequel       |
|                          |   (atom begins directly after instruction and   |
|                          |   ends in a DUK_REOP_MATCH instruction).        |
+--------------------------+-------------------------------------------------+
| SQGREEDY                 | Simple, greedy (maximal) quantifier.            |
|                          |                                                 |
|                          | * ``uint``: ``qmin``, minimum atom match count  |
|                          |                                                 |
|                          | * ``uint``: ``qmax``, maximum atom match count  |
|                          |                                                 |
|                          | * ``uint``: ``atomlen``, atom length in         |
|                          |   characters (must be known and fixed for all   |
|                          |   atom matches; needed for stateless atom       |
|                          |   backtracking)                                 |
|                          |                                                 |
|                          | * ``skip``: signed byte offset for sequel       |
|                          |   (atom begins directly after instruction and   |
|                          |   ends in a DUK_REOP_MATCH instruction).        |
+--------------------------+-------------------------------------------------+
| SAVE                     | Save ``SP`` (string pointer) to ``saved[i]``.   |
|                          |                                                 |
|                          | * ``uint``: ``i``, saved array index            |
+--------------------------+-------------------------------------------------+
| WIPERANGE                | Set saved indices at [start,start+count-1] to   |
|                          | NULL, restoring previous values if backtracking.|
|                          |                                                 |
|                          | * ``uint``: ``start``, saved array start index  |
|                          | * ``uint``: ``count`` (> 0)                     |
+--------------------------+-------------------------------------------------+
| LOOKPOS                  | Positive lookahead.                             |
|                          |                                                 |
|                          | * ``int``: ``skip``, signed byte offset for     |
|                          |   sequel (lookahead begins directly after       |
|                          |   instruction and ends in a DUK_REOP_MATCH)     |
+--------------------------+-------------------------------------------------+
| LOOKNEG                  | Negative lookahead.                             |
|                          |                                                 |
|                          | * ``int``: ``skip``, signed byte offset for     |
|                          |   sequel (lookahead begins directly after       |
|                          |   instruction and ends in a DUK_REOP_MATCH)     |
+--------------------------+-------------------------------------------------+
| BACKREFERENCE            | Match next character(s) against a capture.      |
|                          | If the capture is undefined, *always matches*.  |
|                          |                                                 |
|                          | * ``uint``: ``i``, backreference number in      |
|                          |   [1,``NCapturingParens``], refers to input     |
|                          |   string between saved indices ``i*2`` and      |
|                          |   ``i*2+1``.                                    |
+--------------------------+-------------------------------------------------+
| ASSERT_START             | ``^`` assertion.                                |
+--------------------------+-------------------------------------------------+
| ASSERT_END               | ``$`` assertion.                                |
+--------------------------+-------------------------------------------------+
| ASSERT_WORD_BOUNDARY     | ``\b`` assertion.                               |
+--------------------------+-------------------------------------------------+
| ASSERT_NOT_WORD_BOUNDARY | ``\B`` assertion.                               |
+--------------------------+-------------------------------------------------+

.. FIXME poor layout for esp. ASSERT_NOT_WORD_BOUNDARY

Jumps offsets (skips) for jumps/branches
----------------------------------------

The jump offset of a jump/branch instruction is always encoded as the last
parameter of the instruction.  The offset is relative to the first byte of
the next instruction.  This presents some challenges with variable length
encoding for negative skip offsets.

Assume that the compiler is emitting a JUMP over a 10-byte code block::

   JUMP L2
 L1:
   (10 byte code block)
 L2:

The compiler emits a ``DUK_REOP_JUMP`` opcode.  It then needs to emit
a skip offset of 10.  The offset, 10, does not need to be adjusted because
the length of the encoded skip offset does not affect the offset
(``L2 - L1``).

However, assume that the compiler is emitting a JUMP backwards over a
10-byte code block::

 L1:
   (10 byte code block)
   JUMP L1
 L2:

The compiler emits a ``DUK_REOP_JUMP`` opcode.  It then needs to emit the
negative offset ``L1 - L2``.  To do this, it needs to know the encoded
byte length for representing that *offset value in bytecode*.  The offset
thus depends on itself, and we need to find the shortest UTF-8 encoding
that can encode the skip offset successfully.  In this case the correct
final skip offset is -12 which contains 1 extra byte for ``DUK_REOP_JUMP``
and another extra byte for encoding the -12 skip offset with a one-byte
encoding.

In practice it suffices to first compute the negative offset
``L1 - L2 - 1`` (where the -1 is to account for the ``DUK_REOP_JUMP``,
which always encodes to one byte) without taking the skip parameter into
account, and figure out the length of the UTF-8 encoding of that offset,
``len1``.  Then do the same computation for the negative offset
``L1 - L2 - 1 - len1`` to get the encoded length ``len2``.
The final skip offset is ``L1 - L2 - 1 - len2``.  In some cases ``len1``
will be one byte shorter than ``len2``, but ``len2`` will be correct.

For instance, if the code block in the second example had been 1022 bytes
long:

* The first offset ``L1 - L2 - 1`` would be -1023 which is converted to
  the unsigned value ``2*1023+1 = 2047 = 0x7ff``.  This encodes to two
  UTF-8 bytes, i.e. ``len1 = 2``.

* The second offset ``L1 - L2 - 1 - 2`` would be -1025 which is converted
  to the unsigned value ``2*1025+1 = 2051 = 0x803``.  This encodes to
  *three* UTF-8 bytes, i.e. ``len2 = 3``.

* The final skip offset ``L1 - L2 - 1 - 3`` is -1026, which converts to
  the unsigned value ``2*1026+1 = 2053 = 0x805``.  This again encodes to
  three UTF-8 bytes, and is thus "self consistent".

This could also be solved into closed form directly.

Character class escape handling
-------------------------------

There are no opcodes or special constructions for character class escapes
(``\d``, ``\D``, ``\s``, ``\S``, ``\w``, ``\W``) described in E5 Section
15.10.2.12, regardless of whether they appear inside or outside a
character class.

The semantics are essentially ASCII-based except for the white space
character class which contains all characters in the E5 ``WhiteSpace`` and
``LineTerminator`` productions, resulting in a total of 11 ranges (or
individual characters).

Regardless of where they appear, character class escapes are turned into
explicit character range matches during compilation, which also allows
them to be embedded in character classes without complications (such as,
for instance, splitting the character class into a disjunction).  The
downside of this is that regular expressions making heavy use of ``\s``
or ``\S`` will result in relatively large regexp bytecode.  Another
approach would be to reuse some Unicode code points to act as special
'marker characters' for the execution engine.  Such markers would need
to be above U+FFFF because all 16-bit code points must be matchable.

.. FIXME note briefly where these ranges come from, e.g. the script
   which can be used to re-generate them

The (inclusive) ranges for positive character class escapes are:

+--------+--------+--------+
| Escape | Start  | End    |
+========+========+========+
| ``\d`` | U+0030 | U+0039 |
+--------+--------+--------+
| ``\s`` | U+0009 | U+000D |
+--------+--------+--------+
|        | U+0020 | U+0020 |
+--------+--------+--------+
|        | U+00A0 | U+00A0 |
+--------+--------+--------+
|        | U+1680 | U+1680 |
+--------+--------+--------+
|        | U+180E | U+180E |
+--------+--------+--------+
|        | U+2000 | U+200A |
+--------+--------+--------+
|        | U+2028 | U+2029 |
+--------+--------+--------+
|        | U+202F | U+202F |
+--------+--------+--------+
|        | U+205F | U+205F |
+--------+--------+--------+
|        | U+3000 | U+3000 |
+--------+--------+--------+
|        | U+FEFF | U+FEFF |
+--------+--------+--------+
| ``\w`` | U+0030 | U+0039 |
+--------+--------+--------+
|        | U+0041 | U+005A |
+--------+--------+--------+
|        | U+005F | U+005F |
+--------+--------+--------+
|        | U+0061 | U+007A |
+--------+--------+--------+

The ranges for negative character class escapes are:

+--------+--------+--------+
| Escape | Start  | End    |
+========+========+========+
| ``\D`` | U+0000 | U+002F |
+--------+--------+--------+
|        | U+003A | U+FFFF |
+--------+--------+--------+
| ``\S`` | U+0000 | U+0008 |
+--------+--------+--------+
|        | U+000E | U+001F |
+--------+--------+--------+
|        | U+0021 | U+009F |
+--------+--------+--------+
|        | U+00A1 | U+167F |
+--------+--------+--------+
|        | U+1681 | U+180D |
+--------+--------+--------+
|        | U+180F | U+1FFF |
+--------+--------+--------+
|        | U+200B | U+2027 |
+--------+--------+--------+
|        | U+202A | U+202E |
+--------+--------+--------+
|        | U+2030 | U+205E |
+--------+--------+--------+
|        | U+2060 | U+2FFF |
+--------+--------+--------+
|        | U+3001 | U+FEFE |
+--------+--------+--------+
|        | U+FF00 | U+FFFF |
+--------+--------+--------+
| ``\W`` | U+0000 | U+002F |
+--------+--------+--------+
|        | U+003A | U+0040 |
+--------+--------+--------+
|        | U+005B | U+005E |
+--------+--------+--------+
|        | U+0060 | U+0060 |
+--------+--------+--------+
|        | U+007B | U+FFFF |
+--------+--------+--------+

The ``.`` atom (period) matches everything except a ``LineTerminator`` and
behaves like a character class.  It is interpreted literally inside a
character class.  There is a separate opcode to match the ``.`` atom, 
``DUK_REOP_PERIOD`` so there is currently no need to emit ranges for the
period atom.  If it were compiled into a character range, its ranges would
be (the negative of ``.`` would not be needed):

+--------+--------+--------+
| Escape | Start  | End    |
+========+========+========+
| ``.``  | U+0000 | U+0009 |
+--------+--------+--------+
|        | U+000B | U+000C |
+--------+--------+--------+
|        | U+000E | U+2027 |
+--------+--------+--------+
|        | U+202A | U+FFFF |
+--------+--------+--------+

Each of the above range sets (including for ``.``) are affected by the
ignoreCase (``/i``) option.  However, the ranges can be emitted verbatim
without canonicalization also when case-insensitive matching is used.
This is not a trivial issue, see discussion on canonicalization below.

Misc notes
----------

There is no opcode for a non-capturing group because there is no need for
it during execution.

During regexp execution, regexp flags are kept in the regexp matching
context, and affect opcode execution as follows:

* global (``/g``): does not affect regexp execution, only the behavior of
  ``RegExp.prototype.exec()`` and ``RegExp.prototype.toString()``.

* ignoreCase (``/i``): affects all opcodes which match characters or
  character ranges, through the ``Canonicalize`` operation defined in
  E5 Section 15.10.2.8.  It also affects ``RegExp.prototype.toString()``.

* multiline (``/m``): affects the start and end assertion opcodes
  (``^`` and ``$``).  It also affects ``RegExp.prototype.toString()``.

A bytecode opcode for matching a string instead of an individual character
seems useful at first glance.  The compiler could join successive
characters into a string match (by back-patching the preceding string
match instruction, for instance).  However, this turns out to be difficult
to implement without lookahead.  Consider matching ``/xyz+/`` for instance.
The ``z`` is quantified, so the compiler would need to emit a string match
for ``xy``, followed by a quantifier with ``z`` as its quantified atom.
However, when working on the ``z`` token, the compiler does not know
whether a quantifier will follow but still needs to decide whether or not
to merge it into the previous ``xy`` matcher.  Perhaps the quantifier could
pull out the ``z`` later on, but this complicates the compiler.  Thus there
is only a character matching opcode, ``DUK_REOP_CHAR``.

Canonicalization (case conversion for ignoreCase flag)
======================================================

The ``Canonicalize`` abstract operator is described in E5 Section 15.10.2.8.
It has a pretty straightforward definition matching the behavior of
``String.prototype.toUpperCase()``, except that:

* If case conversion would turn a single codepoint character into a
  multiple codepoint character, case conversion is skipped

* If case conversion would turn a non-ASCII character (>= U+0080) into
  an ASCII character (<= U+007F), case conversion is skipped

``Canocalize`` is used for the semantics of:

* The abstract ``CharacterSetMatcher`` construct,
  E5 Section 15.10.2.8

* Atom ``PatternCharacter`` handling,
  E5 Section 15.10.2.8 (through ``CharacterSetMatcher``)

* Atom ``.`` (period) handling,
  E5 Section 15.10.2.8 (through ``CharacterSetMatcher``)

* Atom ``CharacterClass`` handling,
  E5 Section 15.10.2.8 (through ``CharacterSetMatcher``)

* Atom escape ``DecimalEscape`` handling,
  E5 Section 15.10.2.9 (through ``CharacterSetMatcher``)

* Atom escape ``CharacterEscape`` handling,
  E5 Section 15.10.2.9 (through ``CharacterSetMatcher``)

* Atom escape ``CharacterClassEscape`` handling,
  E5 Section 15.10.2.9 (through ``CharacterSetMatcher``)

* Atom escape (backreference) handling,
  E5 Section 15.10.2.9

The ``CharacterSetMatcher`` basically compares a character against all
characters in the set, and produces a match if the input character and
the target character match after canonicalization.  Matching character
ranges naively by canonicalizing the character range start and end point
and then comparing the canonicalized input character against the range
**is incorrect**, because a continuous range may turn into multiple
ranges after canonicalization.

Example: the class ``[x-{]`` is a continuous range U+0078-U+007B
(``x``, ``y``, ``z``, ``{``), but converts into two ranges after
canonicalization: U+0058-005A, U+007B (``X``, ``Y``, ``Z``, ``{``).
See test case ``test-regexp-canonicalization-js``.

The current solution has a small footprint but is expensive during
compilation: if ignoreCase (``/i``) option is given, the compiler
preprocesses all character ranges by running through all characters
in the character range, normalizing the character, and emitting output
ranges based on the normalization results.  Continuous ranges are kept
continuous, and multiple ranges are emitted if necessary.

This process is relatively simple but has a high compile time impact
(but only if ignoreCase option is specified).  Also note that the process
may result in overlapping character ranges (for instance, ``[a-zA-Z]``
results in ``[A-ZA-Z]``).  However, overlapping ranges are not eliminated
during compilation of case sensitive regular expressions either, which
wastes some bytecode space and execution time, but cause no other
complications.

Note that the resulting ranges (after canonicalization) may include or omit
all such characters whose canonicalized (uppercased) counterparts are
included in some character range of the class.  For instance, the
normalization of ``[a-z]`` is ``[A-Z]`` but ``[A-Zj]`` would also work,
although it would be sub-optimal.  The reason is that a ``j`` will never be
compared during execution, because the input character is normalized before
range comparison (into ``J``) and will thus match the canonicalized
counterpart (here contained in the range ``[A-Z]``).  The canonicalization
process could thus, for instance, simply add additional ranges but keep the
original ones too, although this particular approach would serve little
purpose.

However, this fact becomes relevant when built-in character ranges provided
by ``.``, ``\s``, ``\S``, ``\d``, ``\D``, ``\w``, and ``\W`` are considered.
In principle, the ranges they represent should be canonicalized when
ignoreCase has been specified.  However, these ranges have the following
property: if a lowercase character ``x`` is contained in the range, its
uppercase (canonicalized) counterpart is also contained in the range (see
test case ``test-misc-regexp-character-range-property.js`` for a
verification).  This property is apparent for all the ranges except for
``\w`` and ``\W``: for these ranges to have the property, the refusal of
``Canonicalize`` to canonicalize a non-ASCII character to an ASCII character
is crucial (for instance, U+0131 would map to U+0049 which would cause
problems for ``\W``).  Because of this property, the regexp compiler can use
the built-in character ranges without any normalization processing, even
when ignoreCase option has been specified: the normalized characters are
already present.

Alternative solutions to the canonicalization problem include:

* Perform a more intelligent range conversion at compile time or at regexp
  execution time.  Difficult to implement compactly.

* Preprocess all 65536 possible *input characters* during compile time, and
  match them against the character class ranges, generating optimal result
  ranges (with overlaps eliminated).  The downside include that this cannot
  be done before all the ranges are known, and that the comparison of one
  character against an (input) range is still complicated, and possibly
  requires another character loop which would result in up to 2^32
  comparisons (too high).

Compilation strategies
======================

The examples below use opcode names without the ``DUK_REOP_`` prefix, and use
symbolic labels for clarity.

PC-relative code blocks, jump patching
--------------------------------------

Because addressing of jumps and branches is PC-relative, already compiled
code blocks can be copied and removed without an effect on their validity.
Inserting code before and after code blocks is not a problem as such.

However, there are two things to watch out for:

#. Inserting or removing bytecode into an offset which is between a jump /
   branch and its target.  This breaks the jump offset.  The compiler has
   no support for 'fixing' already generated jumps (except pending jumps
   and branches which are treated specially), so this must be avoided in
   general.

#. Inserting or removing bytecode at an offset which affects a previously
   stored book-keeping offset (e.g. for a pending jump).  This is not
   necessarily a problem as long as the offset is fixed, or the order of
   patching is chosen so that offsets do not break.  See the current
   compilation strategy for an example of this.

Disjunction compilation alternatives
------------------------------------

Basic two alternative disjunction::

  /a|b/
  
        split L1
        (a)
        jump L2
    L1: (b)
    L2:

Assume this code is directly embedded in a three alternative disjunction
(original two alternative code marked with # characters)::

  /a|b|c/  ==  /(?:a|b)|c/
  
        split L3
  #     split L1
  #     (a)
  #     jump L2
  # L1: (b)
  # L2:
        jump L4
    L3: (c)
    L4: 

The "jump L2" instruction will jump directly to the "jump L4" instruction.
So, "jump L2" could be updated to "jump L4" which would not reduce bytecode
size, but would eliminate one extra jump during regexp execution::

  /a|b|c/
  
        split L3
  #     split L1
  #     (a)
  #     jump L4     <-- jump updated from L2 to L4
  # L1: (b)
        jump L4     <-- L2 label eliminated above this instruction
    L3: (c)
    L4: 

Because the compile-time overhead of manipulating code generated for
sub-expressions is quite high, currently the compiler will generate
unoptimal jumps to disjunctions.

Current disjunction compilation model
-------------------------------------

The current disjunction compilation model avoids modifying already
generated code (which is tricky with variable length bytecode) when
possible.  However, this is not entirely possible for disjunctions
compiled into a sequence of SPLIT1 opcodes as illustrated above.  The
compiler needs to track and back-patch one pending JUMP (for a previous
match) and a SPLIT1 (for a previous alternative).  This is illustrated
with an example below, for ``/a|b|c/``.

The bytecode form we create, at the end, for ``a|b|c`` is::

       split1    L2
       split1    L1
       (a)
       jump      M1
  L1:  (b)
  M1:  jump      M2
  L2:  (c)
  M2:

This is built as follows.  After parsing ``a``, a ``|`` is encountered and
the situation is, simply::

       (a)

There is no pending jump/split1 to patch in this case.  What we do in that
case is::

       split1    (empty)    <-- leave unpatched_disjunction_split
       (a)
       jump      (empty)    <-- leave unpatched_disjunction_jump
       (new atom begins here)

When ``a|b`` has been parsed, a ``|`` is encountered and the situation is::

       split1    (empty)    <-- unpatched_disjunction_split for 'a'
       (a)
       jump      (empty)    <-- unpatched_disjunction_jump for 'a'
       (b)

We first patch the pending jump to get::

       split1    (empty)    <-- unpatched_disjunction_split for 'a'
       (a)
       jump      M1
       (b)
  M1:

The pending split1 can also now be patched because the jump has its final
length now::

       split1    L1
       (a)
       jump      M1
  L1:  (b)
  M1:

We then insert a new pending jump::

       split1    L1
       (a)
       jump      M1
  L1:  (b)
  M1:  jump      (empty)    <-- unpatched_disjunction_jump for 'b'

... and a new pending split1::

       split1    (empty)    <-- unpatched_disjunction_split for 'b'
       split1    L1
       (a)
       jump      M1
  L1:  (b)
  M1:  jump      (empty)    <-- unpatched_disjunction_jump for 'b'

After finishing the parsing of ``c``, the disjunction is over and the end
of the ``parse_disjunction()`` function patches the final pending
jump/split1 similarly to what is done after ``b``.  We get::

       split1    L2
       split1    L1
       (a)
       jump      M1
  L1:  (b)
  M1:  jump      M2
  L2:  (c)
  M2:

... which is the target bytecode.

Regexp feature implications
===========================

Quantifiers with a range
------------------------

Quantifiers with a minimum-maximum range (other than the simple ``*`` and
``+`` quantifiers) cannot be implemented conveniently with a basic NFA-based
design because the NFA does not have state for keeping a count of how many
times each instance of a certain quantifier has been repeated.  This is not
trivial to fix, because a certain quantifier may be simultaneously active
multiple times with each quantifier instance having a separate, backtracked
counter.

Ranged quantifiers are not easy for backtracking matchers either.
Consider, for instance, the regexp ``/(?:x{3,4}){5}/``.  The matcher needs
to track five separate ``/x{3,4}/`` quantifiers, each of which backtracks.
Even a recursive backtracking implementation cannot easily handle such
quantifiers without resorting to some form of long jumps or continuation
passing style.  This is not apparent for simple non-hierarchical quantifier
expressions.

There are multiple ways to implement ranged quantifiers.  One can implement
the recursive backtracking engine to incorporate them into the backtracking
logic.  This seems to require a control structure that cannot consist of
simple recursion; rather, some form of long jumps or continuation passing
style seems to be required.  Another approach is to expand such quantifiers
during compile time into an explicit sequence.  For instance, ``/x{3,5}/``
would become, in effect, ``/xxx(?:x?x)?/``.  Capture groups in the
expansions need to map to the same capture group number (this cannot be
expressed in a normal regular expression, but is easy with regexp bytecode
which has a ``save i`` instruction).  This approach becomes a bit wieldy
for large numbers, e.g. for ``/x{500,10000}/``, though.

The current implementation uses the "bytecode expansion" approach to keep
the regular expression matching engine as simple as possible.  Because
bytecode uses relative offsets, and ``DUK_REOP_SAVE`` has a fixed index,
the bytecode for an "atom" may be copied without complications.

Quantifiers and backtracking, simple quantifiers
------------------------------------------------

.. FIXME there is some duplication of discussion with the above section
   on ranged quantifiers

Quantifiers (especially greedy) are problematic for a backtracking
implementation.  A simple implementation of a backtracking greedy
quantifier (or a minimal one, for that matter) will require one level
of C recursion for each atom match.  This is especially problematic
for expressions like::

  .+

The recursion is essentially unavoidable for the general case in a
backtracking implementation.  Consider, for instance::

  (?:x{4,5}){7,8}

Here, each 'instance' of the inner quantifier will individually attempt
to match either 4 or 5 ``x`` characters.  This cannot be easily
implemented without unbounded recursion in a backtracking matcher.

However, for many simple cases unbounded recursion *can* be avoided.
In this document, the term **simple quantifier** is used to refer to any
quantifier (greedy or minimal), whose atom fulfills the following property:

#. The quantifier atom has no alternatives in need of backtracking: it
   either matches once or not at all

#. The input portion matching the atom always has the same character length
   (though not necessarily the same *byte* length)

#. The quantifier atom has no captures or lookaheads

The first property eliminates the need to backtrack any matched atoms.
For instance, a minimal ``+`` quantifier can match the atom once, attempt
to match the sequel.  If the sequel match fails, it does not need to
consider an alternative match for the first atom match (there can be none).
Instead, it can simply proceed to match the atom once more, try the sequel
again, and so on.  Note that although there are no alternatives for each
atom matched, the input portion matching the atom may be different for each
atom match.  For instance, in ``.+`` the ``.`` can match a different
character each time.  The important thing is that there are no alternative
matches for a ''particular'' match, like there are in ``(?:a|b)+``.

The second property is needed for greedy matching, where the quantifier
can first match the atom as many times as possible, and then try the
sequel.  To 'undo' one atom match, we can simply rewind the input string
by the number of characters matched by the atom (which we know to be a
constant), and then try the sequel again.  For instance, the atom length
for ``.+`` is 1, and for ``(?:.x[a-f])+`` it is 3.  Because the particular
characters matching a certain atom instance may vary, we don't know the
byte length of the match in advance.  To avoid remembering backtrack
positions (input offsets after each atom match) we rewind the input by
"atom length" UTF-8-encoded code points.  This keeps a simple, greedy
quantifier stateless and avoids recursion.

The third property is needed because backtracking the ``saved`` array needs
C recursion right now.  The condition might be avoidable quite easily for a
minimal quantifier, and with some effort also for a greedy quantifier (by
rematching the atom to refresh any captures).  However, these haven't been
considered now.  The requirement to have no lookaheads has a similar
motivation: lookaheads currently require recursion for ``saved`` array
management.

Simple quantifiers are expressed with ``DUK_REOP_SQMINIMAL`` and
``DUK_REOP_SQGREEDY``.  The atom being matched *must* fulfill the conditions
described above; the compiler needs to track the simple-ness of an atom for
various nested atom expressions such as ``(?:a(?:.))[a-fA-F]``.  In theory,
the following can also be expressed as a simple quantifier: ``(?:x{3})+``,
which expands to ``(?:xxx)+``, a simple quantifier with an atom length of 3.
The compiler is not this clever, though, at least not at the time of
writing.

Any quantifiers not matching the simple quantifier properties are complex
quantifiers, and are encoded as explicit bytecode sequences using e.g.
``DUK_REOP_SPLIT1``, ``DUK_REOP_SPLIT2``, and ``DUK_REOP_JUMP``.
Counted quantifiers are expanded by the compiler into straight bytecode.
For instance, ``(?:a|b){3,5}`` is expanded into (something like)
``(?:a|b)(?:a|b)(?:a|b)(?:(?:a|b)(?:a|b)?)?``.  Capture groups inside the
atom being matched are encoded into two ``DUK_REOP_SAVE`` instructions.
The *same* save indices are used in the atom being expanded, so later atom
matches overwrite saved indices of earlier matches (which is correct
behavior).  Such expressions cannot be expressed as ordinary regexps because
the same capture group index cannot be used twice.

Future work
===========

Compiler and lexer
------------------

* E5 Section 15.10.2.5, step 4 of RepeatMatcher: is it possible that ``cap[k]``
  is defined for some ``k``, where ``k > parenCount + parenIndex``?  If so, add
  an example.  This means that we can't just clear all captures for
  ``k > parenIndex``.

* Handling empty infinite quantifiers, as in: ``/(x*)*/``.

* The regexp lexer is quite simple and could perhaps be integrated into the
  regexp compiler - at some loss of clarity but at some gain in code
  compactness.

* Add an opcode for disjunction specifically? Could this avoid the amount of
  recursion (linear to the number of alternatives) currently required by
  disjunctions?

Executor
--------

* Optimized primitive for testing a regexp (match without captures) would be
  easy by just skipping 'save' instructions but would waste space.
