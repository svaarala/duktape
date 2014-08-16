=========================
URI encoding and decoding
=========================

Specification notes
===================

URI syntax
----------

E5.1 Annex F::

  15.1.3: Added notes clarifying that ECMAScript‘s URI syntax is based upon
  RFC 2396 and not the newer RFC 3986. In the algorithm for Decode, a step
  was removed that immediately preceded the current step 4.d.vii.10.a
  because it tested for a condition that cannot occur.

Changes from RFC 2396 to RFC 3986 are summarized in RFC 3986:

* http://tools.ietf.org/html/rfc3986#appendix-D

Changes relevant to Ecmascript include:

* Additional characters in "reserved" set.

  - RFC 2396::

     reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
                     "$" | ","

     ; / ? : @ & = + $ ,

  - RFC 3986::

     reserved      = gen-delims / sub-delims
     gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
     sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                   / "*" / "+" / "," / ";" / "="

     : / ? # [ ] @ ! $ & ' ( ) * + , ; =

  - New characters in RFC 3986 are::

     # [ ] ! ' ( ) *

Effect on decoding: don't decode hex escapes into reserved characters.
However, RFC 3986 additional characters should be decoded normally
because they're not supported in Ecmascript.  Thus::

  decodeURI("%23%5B%5D%21%27%28%29%2A") -> "%23[]!'()*"

The '#' character is explicitly added to the reserved set by the
decodeURI() algorithm in E5.1 Section 15.1.3.1.

Effect on encoding: don't encode into hex escapes.  However, RFC 3986
additional characters should be escaped normally because they're not
supported::

  encodeURI("#[]!'()*") -> "#%5B%5D!'()*"

The '#' character is explicitly added to the reserved set by the
decodeURI() algorithm in E5.1 Section 15.1.3.1.  The characters
``!'()*`` are already part of the uriMark production which goes into
uriUnescaped.  Brackets are not included so they get escaped in
Ecmascript.

Reserved set / unescaped set
----------------------------

The "unescaped set" for encoding and the "reserved set" for decoding always
consist of only ASCII codepoints.  Thus comparing codepoints against the sets
should only be necessary when processing ASCII range characters.

When encoding, step 4.c will catch characters in the "unescaped set" and
encode them as-is into the output.  Note that these can only be single-byte
ASCII characters.  If we go to step 4.d, the codepoint may either be ASCII
or non-ASCII, and will be escaped regardless.

When decoding percent escaped codepoints, one-byte encoded codepoints (i.e.
ASCII) are checked in step 4.d.vi; multi-byte encoded codepoints in the BMP
range are checked in step 4.d.vii but codepoints above BMP are not checked.

Apparently the idea here is to ensure no characters in the reserved set are
decoded from percent escapes even if invalid UTF-8 (non-shortest) encodings
are allowed.  Because characters above BMP are encoded with surrogate pairs,
the formula for surrogate pairs ensures that the codepoint cannot be below
U+00010000 (0x10000 is added to the surrogate pair bits), and thus no check
against the "reserved set" is needed.

However, at the end of Section 15.1.3:

  RFC 3629 prohibits the decoding of invalid UTF-8 octet sequences. For
  example, the invalid sequence C0 80 must not decode into the character
  U+0000. Implementations of the Decode algorithm are required to throw a
  URIError when encountering such invalid sequences.

Because "reserved set" / "unescaped set" always consists of only ASCII
codepoints, the check in step 4.d.vii should not be necessary.  The UTF-8
validity check happens in step 4.d.vii.8.

Decoding characters outside BMP
-------------------------------

The URI decoding algorithm requires that UTF-8 encoded codepoints consisting
of more than 4 encoded bytes are rejected.  4 byte encoding contains 21 bits,
so the maximum codepoint which can be expressed is U+1FFFFF.  However, since
the bytes must also be valid UTF-8 (step 4.d.vii.8) the highest allowed
codepoint is actually U+10FFFF.

It would be nice to be able to:

* decode higher codepoints because Duktape can represent them

* decode codepoints up to U+10FFFF without surrogate pairs

Because the API requirements are strict, these cannot be added to the standard
API without breaking compliance.  Custom URI encoding/decoding functions could
provide these extended semantics.
