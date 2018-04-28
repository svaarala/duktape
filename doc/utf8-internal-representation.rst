======================================
Using UTF-8 as internal representation
======================================

Some notes on using UTF-8 as internal representation for ECMAScript strings
when surrogate pairs can be combined.

Current representation
======================

Current internal representation is a union of:

* CESU-8: to support full 16-bit codepoint sequences without limitations.
  In particular, individual and unpaired surrogates must work without
  interpretation or conversion.

* UTF-8: to support non-BMP characters, if they are created from C code
  or e.g. using String.fromCharCode(0x12345).

* Extended UTF-8: to support codepoints up to U+FFFFFFFF.  This is now
  only needed by the regexp bytecode, which uses extended UTF-8 as its
  internal representation and needs to represent long offsets as
  codepoints.

C API problem with current representation
=========================================

One concrete problem with this arrangement is that non-BMP strings are
internally represented as CESU-8:

* If source code contains a non-BMP character, the ECMAScript specification
  requires that such a character is decoded into surrogates, from
  https://www.ecma-international.org/ecma-262/5.1/#sec-6:

  - If an actual source text is encoded in a form other than 16-bit code
    units it must be processed as if it was first converted to UTF-16.

* This means that ``x = '\u{12345}'`` and ``x = '\ud808\udf45'`` MUST be
  treated identically.  For example, for both inputs:

  - The string's ``.length`` must be 2.

  - ``x[0]`` must be 0xd808, and ``x[1]`` must be 0xdf45.

  - RegExps must be able to match the individual surrogates, and one must
    be able to e.g. backtrack each surrogate separately.

  - It must be possible to take a substring whose one end is between
    the surrogate codepoints.

* In the current C API such a string will appear CESU-8 encoded because
  that's the internal representation used for surrogate codepoints.

* Applications dealing natively with UTF-8 would often prefer to see UTF-8
  rather than CESU-8, thus avoiding the need to transcode CESU-8 to UTF-8.

The ECMAScript specification doesn't (and cannot) mandate any specific
internal representation, nor does it provide any requirements on how a
C API must represent strings.  The current convention of using CESU-8
for standard ECMAScript strings is thus not really mandatory.  However,
if an alternative representation is used, it MUST behave identically as
far as script code is concerned.

Automatically combining surrogates in internal representation
=============================================================

One alternative to the current internal representation is to:

* Keep the current CESU-8 + UTF-8 + extended UTF-8 as the base representation.

* When conceptual ECMAScript strings contain correctly paired surrogates,
  combine the surrogates into the actual non-BMP codepoint.  The resulting
  codepoint is then valid UTF-8 and not CESU-8.

* When a non-paired surrogate is found, encode it as CESU-8 as before.

* This process must be applied to all inputs, both script code and C code,
  so that a certain conceptual ECMAScript string has a unique duk_hstring
  representation.  (If this is not the case, string comparison using an
  interned string pointer would no longer be valid which leads to a lot of
  complications.)

This would have the upside that:

* Valid Unicode strings in UTF-8 codepoint range (U+0000 to U+10FFFF without
  surrogate range U+D800 to U+DFFF) would appear as valid UTF-8 (not CESU-8)
  in the C API.

* Pushing UTF-8 strings would produce strings that behaved like standard
  ECMAScript strings, i.e. they would conceptually have surrogate pairs in
  place of non-BMP.

And a few downsides:

* All the internal code would need to maintain an "as if" illusion: such
  strings must appear as uninterpreted 16-bit codepoint sequences, and all
  16-bit codepoint sequences must still work without difference as far as
  script code is concerned.  This is not trivial, more on this below.

* One would no longer be able to push an arbitrary byte sequence as a string
  (duk_push_string()) and then read it back as is.  The automatic surrogate
  combination would mean the output might be different, with surrogates
  represented in CESU-8 combined into UTF-8.  This is a loss of current
  functionality which has been useful for some applications; one can e.g.
  push ISO-8859-1 strings as is, and read them back.  Script code will see
  such strings as being somewhat broken, but they have previously passed
  through without modification.

Some internals where the "as if" illusion must be maintained:

* String ``.length`` must count non-BMP codepoints as 2 codepoints to get
  the standard length.

* String.charCodeAt() and all other String functions must use an index scheme
  that references the conceptual 16-bit codepoint sequence index (where each
  non-BMP counts as two indices), and allow reading, substringing, etc, both
  of the surrogate pairs individually.

* There's no longer an easy "char offset to byte offset" internal primitive.
  Currently such a conversion maps an integer to an integer (or error).  For
  non-BMP characters the result would now be a tuple: an integer pointing to
  the start of the codepoint, and a flag indicating whether we want the high
  or the low surrogate.  All places maintaining "current offset" must track
  that additional flag somehow (it could maybe be encoded as the high bit of
  a 32-bit unsigned value?).

* When doing string replacements, code must always check whether the
  replacements created valid surrogate pairs from previously unpaired
  surrogates.  They must be merged, to maintain a unique strnig representation.
  Such surrogates may appear at the edges of replacement strings.

* When combining strings, must check for previously unpaired surrogates at
  string join point.

* RegExp matching must match non-BMP codepoints as two surrogates individually
  as far as patterns are concerned.  It must be possible to capture only one
  of the surrogates, backtrack each surrogate individually, match start offset
  must try both surrogates as starting points, etc.

* RegExp /u mode would work trivially with this internal representation, as
  the codepoints are already combined.
