===============
Unicode support
===============

Overview
========

ECMAScript E5 requires quite extensive Unicode support, which is difficult to
implement in a very compact fashion.  The subsections below discuss Unicode
handling in various parts of the E5 standard.  Below, the terms "character"
and "codepoint" are used interchangeably.

The general principles for implementation are:

* Operations on ASCII characters and ASCII strings should have a fast path
  which avoids expensive scanning of conversion tables etc.

* Simple run-time operations on non-ASCII characters like string
  concatenation, character lookups etc, should be reasonably fast (e.g.,
  avoid a scan of Unicode character information ranges).

* Complex run-time operations on non-ASCII characters like case conversion
  can have a performance penalty in exchange for small size.

* Compile-time operations on non-ASCII characters can have a performance
  penalty in exchange for small size.

Handling Unicode case conversion, character classes, etc. in a compact code
size is bit challenging.  The current solution is to fast path ASCII
characters and to use a bit-packed format for encoding case conversion
rules (e.g. range mappings).  The rules are created by build-time Python
scripts (see ``tools/`` directory) and decoded by run-time code such as the
parser with the help of ``duk_bitdecoder_ctx`` and ``duk_bd_decode()``.

.. note:: There are many Unicode specifications, and I'm not sure
   which ones apply to E5.  For instance, which specification governs the
   'end of word' behavior for the Final_Sigma context?  Is it #29 or
   something else?

Useful background information:

* Unicode Standard Annex #44: UNICODE CHARACTER DATABASE:
  http://unicode.org/reports/tr44/#Casemapping

* CLDR - Unicode Common Locale Data Repository:
  http://cldr.unicode.org/

* Unicode Technical Standard #35: UNICODE LOCALE DATA MARKUP LANGUAGE (LDML):
  http://www.unicode.org/reports/tr35/tr35-21.html

* Unicode Standard Annex #29: UNICODE TEXT SEGMENTATION:
  http://www.unicode.org/reports/tr29/

Unicode data:

* http://unicode.org/Public/UNIDATA/

* UnicodeData.txt: http://unicode.org/Public/UNIDATA/UnicodeData.txt

* SpecialCasing.txt: http://unicode.org/Public/UNIDATA/SpecialCasing.txt

Source text
===========

The ``IdentifierStart`` and ``IdentifierPart`` codepoint sets are rather
complex.  They are currently encoded into about 1.5 kilobytes of bit-packed
match data by ``extract_chars.py``.

Regular expression
==================

The ``Canonicalize()`` abstract operation described in E5 Section 15.10.2.8
shares the case conversion of ``String.prototype.toUpperCase()`` with a few
exceptions.  The conversion tables can be shared so no additional tables are
needed.

String case conversion
======================

ECMAScript E5 requires case conversion for 16-bit Unicode characters with the
``String.prototype`` functions ``toLowerCase()``, ``toLocaleLowerCase()``,
``toUpperCase()``, and ``toLocaleUpperCase()``, see E5 Sections 15.5.4.16 to
15.5.4.19.  Titlecase conversion is not required by ECMAScript E5.  Regular
expression abstract ``Canonicalize()`` operation also borrows the case
conversion rules (though only for 1:1 conversions), see E5 Section 15.10.2.8.

Unicode data files describe case conversion rules in two parts:

1. ``UnicodeData.txt`` describes simple 1:1 mappings for lowercase, uppercase,
   and titlecase.  The titlecase mapping, if missing, defaults to uppercase
   mapping.

2. ``SpecialCasing.txt`` describes complex 1:many mappings for case conversion,
   which are also required by E5.  These mappings may be locale sensitive (e.g.
   apply only to a certain language) and/or context sensitive (e.g. apply only
   if a character is preceded or followed by certain codepoints).

UnicodeData.txt lists all Unicode codepoints and optionally gives case
conversion rules for each.  Titlecase conversion defaults to uppercase
conversion, and if no conversion is given, the character is assumed to remain
the same unless SpecialCasing.txt has an overriding rule.  The actual case
conversion rules are not random, but in many cases continuous ranges are
shifted to another position in the codepoint space; the ranges may be fully
continuous or have a "skip", e.g. apply to every other character.

SpecialCasing.txt provides additional rules particularly for handling cases
where the case conversion is not 1:1.  For instance, "ß" converted to
uppercase is "SS".  There are slightly over 100 such rules, almost entirely
for uppercase and titlecase conversion.  The special casing rules can convert
an input codepoint into 1-3 result codepoints (the ligature U+FB03 uppercases
to "FFI", for instance).  Some special casing rules are context and/or locale
sensitive.  *Context sensitivity* means that a rule only applies when a
codepoint is (or is not) surrounded by certain other codepoints, which means
that characters cannot be case converted individually.  *Locale sensitivity*
means that a rule might only apply for a certain language.

The Python script ``extract_caseconv.py`` reads in UnicodeData.txt and
SpecialCasing.txt, extracts the appropriate case conversion rules, scans the
conversion rules to generate a compact rules database (really just a list of
rules), and encodes the rules into a bit packed format.  The bit packed rule
format has been developed experimentally to minimize data and code space, by
looking at the case conversion data and first detecting simple rules (ranges
which are either continuous or have a certain "skip"), and then looking at
what remains.

Currently the encoded format consists of three parts:

1. range mappings with a "skip" of 1...6;

2. simple 1:1 character mappings which are not covered by the range rules;

3. complex 1:n character mappings.

There's probably some room for improvement in optimizing the encoding further;
currently it takes almost 2 KiB for uppercase and lowercase rules combined.

.. note:: Context or locale specific rules are not processed now.  This
   violates E5 requirements for both context and locale support.

See also:

* http://www.unicode.org/faq/casemap_charprop.html.

* ``misc/CaseConversion.java`` which allows easy testing of what Java does

Context and locale sensitive rules
==================================

The following context and locale sensitive rules exist in SpecialCasing.txt
with md5sum of 5cea3d079e2b6c6c3babb0726e47e1db.

Useful background:

* Unicode Standard Annex #44: UNICODE CHARACTER DATABASE, Section 5.6:
  http://unicode.org/reports/tr44/#Casemapping

  - Clarifies that contexts are not formal character properties

* CLDR - Unicode Common Locale Data Repository: http://cldr.unicode.org/

* http://unicode.org/reports/tr44/#General_Category_Values

Final sigma (all languages)
---------------------------

::

  # Special case for final form of sigma

  03A3; 03C2; 03A3; 03A3; Final_Sigma; # GREEK CAPITAL LETTER SIGMA

The lowercase conversion of U+03A3: GREEK CAPITAL LETTER SIGMA depends
on context as follows:

* Final_Sigma: lowercase is U+03C2: GREEK SMALL LETTER FINAL SIGMA

* Otherwise: lowercase is U+03C3: GREEK SMALL LETTER SIGMA

Other conversions (uppercase or titlecase conversions, or lowercase
conversions of other sigma characters) are not context sensitive.
In particular, codepoints U+03C2 and U+03C3 lowercase to themselves.

What is the definition for "Final_Sigma"?  Not quite sure, see:

* http://www.unicode.org/faq/greek.html#5

* "Unicode demystified" link below seems to indicate that:

  - Let p = previous codepoint (if exists)
  - Let n = next codepoint (if exists)
  - Then final_sigma = (p exists) and (p is a letter) and
    ((n does not exist) or (n is not a letter))
  - The meaning of a "letter" is not clear

See also:

* http://unicode.org/faq/greek.html#5
* http://en.wikipedia.org/wiki/Sigma
* http://www.unicode.org/reports/tr29/#Word_Boundaries
* http://books.google.fi/books?id=wn5sXG8bEAcC&pg=PA169&lpg=PA169&dq=%22Final_Sigma%22&source=bl&ots=J07ysYPbVD&sig=tGhPz1VFpi-KE1InQPsjX2diVlg&hl=fi&ei=XHswTqmrA4aSOrSf3X4&sa=X&oi=book_result&ct=result&resnum=5&ved=0CDYQ6AEwBA#v=onepage&q=%22Final_Sigma%22&f=false

Lithuanian (lt)
---------------

::

  # Lithuanian retains the dot in a lowercase i when followed by accents.

  # Remove DOT ABOVE after "i" with upper or titlecase

  0307; 0307; ; ; lt After_Soft_Dotted; # COMBINING DOT ABOVE

::

  # Introduce an explicit dot above when lowercasing capital I's and J's
  # whenever there are more accents above.
  # (of the accents used in Lithuanian: grave, acute, tilde above, and ogonek)

  0049; 0069 0307; 0049; 0049; lt More_Above; # LATIN CAPITAL LETTER I
  004A; 006A 0307; 004A; 004A; lt More_Above; # LATIN CAPITAL LETTER J
  012E; 012F 0307; 012E; 012E; lt More_Above; # LATIN CAPITAL LETTER I WITH OGONEK
  00CC; 0069 0307 0300; 00CC; 00CC; lt; # LATIN CAPITAL LETTER I WITH GRAVE
  00CD; 0069 0307 0301; 00CD; 00CD; lt; # LATIN CAPITAL LETTER I WITH ACUTE
  0128; 0069 0307 0303; 0128; 0128; lt; # LATIN CAPITAL LETTER I WITH TILDE

Turkish and Azeri (tr and az)
-----------------------------

::

  # I and i-dotless; I-dot and i are case pairs in Turkish and Azeri
  # The following rules handle those cases.

  0130; 0069; 0130; 0130; tr; # LATIN CAPITAL LETTER I WITH DOT ABOVE
  0130; 0069; 0130; 0130; az; # LATIN CAPITAL LETTER I WITH DOT ABOVE

::

  # When lowercasing, remove dot_above in the sequence I + dot_above, which will turn into i.
  # This matches the behavior of the canonically equivalent I-dot_above

  0307; ; 0307; 0307; tr After_I; # COMBINING DOT ABOVE
  0307; ; 0307; 0307; az After_I; # COMBINING DOT ABOVE

::

  # When lowercasing, unless an I is before a dot_above, it turns into a dotless i.

  0049; 0131; 0049; 0049; tr Not_Before_Dot; # LATIN CAPITAL LETTER I
  0049; 0131; 0049; 0049; az Not_Before_Dot; # LATIN CAPITAL LETTER I

::

  # When uppercasing, i turns into a dotted capital I

  0069; 0069; 0130; 0130; tr; # LATIN SMALL LETTER I
  0069; 0069; 0130; 0130; az; # LATIN SMALL LETTER I

Various 'i' characters
----------------------

Case conversion rules for various 'i' characters are particularly fun.
There are four separate 'i'-characters:

* U+0049: LATIN CAPITAL LETTER I
* U+0069: LATIN SMALL LETTER I
* U+0130: LATIN CAPITAL LETTER I WITH DOT ABOVE
* U+0131: LATIN SMALL LETTER DOTLESS I

Case conversion rules for these characters are locale and context dependent and differ
from standard conversions at least for Lithuanian (lt), Turkish (tr), and Azeri (az)
as follows (ignoring context dependent rules):

+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Input  | uc/lt    | uc/tr    | uc/az    | uc/other | lc/lt    | lc/tr    | lc/az    | lc/other |
+========+==========+==========+==========+==========+==========+==========+==========+==========+
| U+0049 |          |          |          |          |          |          |          |          |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| U+0069 |          |          |          |          |          |          |          |          |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| U+0130 |          |          |          |          |          |          |          |          |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| U+0131 |          |          |          |          |          |          |          |          |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+

**FIXME: FILL**

Java behavior:

+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Input  | uc/lt    | uc/tr    | uc/az    | uc/other | lc/lt    | lc/tr    | lc/az    | lc/other |
+========+==========+==========+==========+==========+==========+==========+==========+==========+
| U+0049 |  U+0049  |  U+0049  |  U+0049  |  U+0049  |  U+0069  |**U+0131**|**U+0131**|  U+0069  |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| U+0069 |  U+0049  |**U+0130**|**U+0130**|  U+0049  |  U+0069  |  U+0069  |  U+0069  |  U+0069  |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| U+0130 |  U+0130  |  U+0130  |  U+0130  |  U+0130  |  U+0069  |  U+0069  |  U+0069  |  U+0069  |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| U+0131 |  U+0049  |  U+0049  |  U+0049  |  U+0049  |  U+0131  |  U+0131  |  U+0131  |  U+0131  |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
