=========================
Duktape 3.0 release notes
=========================

Release overview
================

Main changes in this release (see RELEASES.rst for full details):

* Switch to WTF-8 internal string representation.  Matching surrogate pairs,
  both inside ECMAScript strings and in CESU-8 format provided by native code,
  are automatically combined to non-BMP UTF-8 codepoints; unpaired surrogates
  remain in CESU-8 style (= WTF-8).  Invalid WTF-8 byte sequences are replaced
  with U+FFFD replacements to guarantee interned strings are now always valid
  WTF-8. ECMAScript code will still see standard ECMAScript strings where surrogates
  are individual characters.  However, native code now sees standard UTF-8
  strings whenever possible (and WTF-8 otherwise), simplifying integration with
  other UTF-8 based APIs.

* TBD.

Upgrading from Duktape 2.x
==========================

* If your application deals with UTF-8 and/or CESU-8 string representations,
  the WTF-8 string representation may change or eliminate some conversion needs.

* Source variants have been removed from the distributable, with the
  ``src/`` directory containing combined sources with default options and
  *without* ``#line`` directives (previously ``src-noline/``).  You can
  recreate the removed variants using ``tools/configure.py``.

* TBD.

More details on WTF-8 changes
=============================

* WTF-8 sanitization, i.e. combining of paired surrogates and using U+FFFD
  replacements for invalid sequences, is applied during the string intern check
  which ensures all string data created by any means goes through the
  sanitization.

* WTF-8 sanitization uses U+FFFD replacements for invalid byte sequences.
  The replacement policy matches http://unicode.org/review/pr-121.html
  (same as TextDecoder).

* When native code pushes strings, non-BMP codepoints encoded in UTF-8 style
  (one combined codepoint) and CESU-8 style (surrogate pair encoded as two
  codepoints) now both result in UTF-8 style being used in the internal string
  representation.  Previously one could push strings in either form and they
  would behave slightly differently for both native and ECMAScript code.

* ECMAScript now always sees standard strings consisting of 16-bit codepoints,
  extended codepoint support previously available to ECMAScript code has been
  removed.

* String.fromCharCode() now always uses standard ToUint16() coercion.
  The option ``DUK_USE_NONSTD_STRING_FROMCHARCODE_32BIT`` has been
  removed (it allowed non-standard ToUint32() codepoint coercion).
  The standard function String.fromCodePoint() already allows the
  creation of non-BMP characters which now appear as UTF-8 to native code.
