=============
JSON built-in
=============

This document describes the Duktape ``JSON`` built-in implementation which
provides:

* The standard, very strict JSON encoding and decoding required by the
  Ecmascript standard.

* An extended custom format (JX) which encodes all value types and is
  optimized for readability.  The custom encodings parse back into proper
  values (except for function values).  This format is most useful for
  dumping values, logging, and the like.  The format is not JSON compatible
  but rather JSON-like.

* An extended compatible format (JC) which also encodes all value types
  into standard JSON.  A standard JSON parser can parse the result but
  special values need to be revived manually.  The result is not as
  readable as JX, but can be parsed by other JSON implementations.

Overview of JSON
================

JSON_ (JavaScript Object Notation) is a format originally based on a subset of
Ecmascript E3, but which is now used by multiple languages and implementations
and defined in `RFC 4627`_.  The E5/E5.1 specification has its own more or
less compatible definition of JSON.  The syntax and processing requirements in
Section 15.12 form the basis for implementing the JSON built-in.  Note that
unlike the RFC 4627 interpretation, E5/E5.1 JSON interpretation is very strict;
E5.1 Section 15.12 states:

  Conforming implementations of JSON.parse and JSON.stringify must support
  the exact interchange format described in this specification without any
  deletions or extensions to the format. This differs from RFC 4627 which
  permits a JSON parser to accept non-JSON forms and extensions.

.. _JSON: http://en.wikipedia.org/wiki/JSON
.. _`RFC 4627`: http://www.ietf.org/rfc/rfc4627.txt

JSON only supports nulls, booleans, numbers, strings, objects, and arrays.
Non-finite numbers (i.e. NaN and +/- Infinity) are encoded as "null" values
while "undefined" values and function objects are skipped entirely.

Ecmascript JSON only supports 16-bit Unicode codepoints because it operates
on Ecmascript strings (which are sequences of 16-bit codepoints).  Full
transparency for all 16-bit codepoints is required; in particular, even
invalid surrogate pairs must be supported.  Because Duktape supports codepoints
above the 16-bit BMP, support for these will necessarily be non-standard.
Such codepoints are now encoded and decoded "as is", so they pass through
encoding and decoding without problems.  There is currently no escape syntax
for expressing them in escaped form.

Duktape also has custom types not supported by Ecmascript: buffers and
pointers.  These are now skipped when encoding (just like function objects).
There is currently no syntax for expressing them for parsing.

The custom JX and JC formats provide support for encoding and decoding
"undefined" values, function values, special numbers like NaN, buffer values,
and pointer values.  Separate API entrypoints are used for JX and JC
because JSON.parse() and JSON.stringify() are intended to be strict interfaces.

See also:

* http://json.org/

* http://bolinfest.com/essays/json.html

Notes on stringify()
====================

Basic approach
--------------

Stringify uses a single context structure (``duk_json_enc_ctx``) whose pointer
is passed around the helpers to minimize C call argument counts and also to
minimize C stack frame sizes.  The encoding context contains a ``thr`` pointer,
flags, and various (borrowed) value stack references and indices.

Encoded JSON output is appended to a growing buffer which is converted to the
final string at the end.  This differs from the specification algorithm which
basically concatenates strings in pieces.  Unfortunately this concatenation
process is out of order for encoding object key/value pairs: JO() will first
call Str() and only then decide whether to serialize the property at all (so
that key and colon are only emitted if the Str(value) call does not return
undefined).  Side effects prevent a two-pass "dry run" approach.

This problem is now avoided by splitting Str() into two separate algorithms.
The first algorithm performs all the conversions and coercions, and causes
all side effects to take place, and then indicates whether the final result
would be undefined or not.  The caller can then take appropriate action before
anything needs to be written to the buffer.  The second algorithm performs
the final output generation, assuming that all the coercions etc have already
been done.

In addition to standard requirements, a C recursion limit is imposed to
safeguard against overrunning the stack in stack limited environments.

Loop detection
--------------

The specification uses a stack for loop detection in the beginning of the
JO() and JA() algorithms.  If the value is already present, an error is thrown;
else the value is added to the stack.  At the end of JO() and JA() the stack
is popped.  Note that:

* The stack order does not matter; it is simply used to check whether a
  value is present anywhere in the stack.

* Only objects and arrays (i.e., heap objects) are ever present in the stack.
  A single object/array can only be present at most once.

* The maximum stack depth matches object recursion depth.  Even for very
  large JSON documents the maximum stack depth is not necessarily very high.

The current implementation uses a tracking object instead of a stack.  The
keys of the tracking object are heap pointers formatted with sprintf()
``%p`` formatter.  Because heap objects have stable pointers in Duktape,
this approach is reliable.  The upside of this approach is that we don't
need to maintain yet another growing data structure for the stack, and don't
need to do linear stack scans to detect loops.  The downside is relatively
large memory footprint and lots of additional string table operations.

Another approach would be to accept a performance penalty for highly nested
objects and user a linear C array for the heap object stack.

This should be improved in the future if possible.  Except for really
heavily nested cases, a linear array scan of heap pointers would probably
be a better approach.

PropertyList
------------

When a PropertyList is used, the serialization becomes quite awkward, and
requires a linear scan of the PropertyList over and over again. PropertyList
is used in the JO() algorithm:

* If PropertyList is defined, K is set to PropertyList.

* If PropertyList is undefined, K is set to a list of property names of
  the object's own enumerable properties, in the normal enumeration order.

* The list K is iterated, and non-undefined values are serialized.

When PropertyList is undefined, the algorithm is clear: simply enumerate
the object in the normal way.  When PropertyList is not undefined, even
non-enumerable properties can be serialized, and serialization order is
dictated by PropertyList.

It might be tempting to serialize the object by going through its properties
and then checking against the PropertyList (which would be converted into a
hash map for better performance).  However, this would be incorrect, as the
specification requires that the key serialization order be dictated by
PropertyList, not the object's enumeration order.

Note that even if serialization could be done by iterating the object keys,
it's not obvious which of the following would be faster:

* Iterate over object properties and compare them against PropertyList
  (assuming this would be allowed)

* Iterate over the PropertyList, and checking the object for properties

If the object has only a few properties but PropertyList is long, the
former would be faster (if it were allowed); if the object has a lot of
properties but PropertyList is short, the latter would be faster.

Further complications

* PropertyList may contain the same property name multiple times.  The
  specification requires that this be detected and duplicate occurrences
  ignores.  The current implementation doesn't do this::

    JSON.stringify({ foo:1, bar:2 }, [ 'foo', 'bar', 'foo', 'foo' ]);
    --> {"foo":1,"bar":2,"foo":1,"foo":1}

* PropertyList may be sparse which may also cause its natural enumeration
  order to differ from an increasing array index order, mandated by the
  E5.1 specification for PropertyList.  Currently we just use the natural
  enumeration order which is correct for non-sparse arrays.

Handling codepoints above U+FFFF
--------------------------------

Codepoints above U+FFFF don't occur in standard Ecmascript string values,
so there is no mandatory behavior when they are encountered during JSON
serialization.  The current solution is to encode them into plain string
data (this matches JC behavior)::

  "foo bar: U+12345678"

Handling invalid UTF-8/CESU-8 data
----------------------------------

Standard Ecmascript values are always valid CESU-8 data internally, so
handling invalid UTF-8/CESU-8 data has no mandatory behavior.  The current
solution is:

* If UTF-8/CESU-8 decoding fails, treat the initial byte as a codepoint
  value directly (interpreting it as an 8-bit unsigned value) and advance
  by one byte in the input stream.  The replacement codepoint is encoded
  into the output value.

* The current UTF-8/CESU-8 decoding is not strict, so this is mainly
  triggered for invalid initial bytes (0xFF) or when a codepoint has been
  truncated (end of buffer).

This is by no means an optimal solution and produces quite interesting
results at times.

Miscellaneous
-------------

* It would be nice to change the standard algorithm to be based around
  a "serializeValue()" primitive.  However, the standard algorithm provides
  access to the "holder" of the value, especially in E5 Section 15.12.3,
  Str() algorithm, step 3.a: the holder is passed to the ReplacerFunction.
  This exposes the holder to user code.

* Similarly, serialization of a value 'val' begins from a dummy wrapper
  object: ``{ "": val }``.  This seems to be quite awkward and unnecessary.
  However, the wrapper object is accessible to the ReplacerFunction, so
  it cannot be omitted, at least when a replacer function has been given.

* String serialization should be fast for pure ASCII strings as they
  are very common.  Unfortunately we may still need to escape characters
  in them, so there is no explicit fast path now.  We could use ordinary
  character lookups during serialization (note that ASCII string lookups
  would not affect the stringcache).  This would be quite slow, so we
  decode the extended UTF-8 directly instead, with a fast path for ASCII.

* The implementation uses an "unbalanced value stack" here and there.  In
  other words, the value stack at a certain point in code may contain a
  varying amount and type of elements, depending on which code path was
  taken to arrive there.  This is useful in many cases, but care must be
  taken to use proper indices to manipulate the value stack, and to restore
  the value stack state when unwinding.

Notes on parse()
================

Basic approach
--------------

Like stringify(), parse() uses a single context structure (``duk_json_dec_ctx``).

An important question in JSON parsing is how to implement the lexer component.
One could reuse the Ecmascript lexer (with behavior flags); however, this is
not trivial because the JSON productions, though close, contain many variances
to similar Ecmascript productions (see below for discussion).  The current
approach is to use a custom JSON lexer.  It would be nice if some shared code
could be used in future versions.

Parsing is otherwise quite straightforward: parsed values are pushed to the
value stack and added piece by piece into container objects (arrays and
objects).  String data is x-UTF-8-decoded on-the-fly, with ASCII codepoints
avoiding an actual decode call (note that all JSON punctuators are ASCII
characters).  Non-ASCII characters will be decoded and re-encoded.
Currently no byte/character lookahead is necessary.

Once basic parsing is complete, a possible recursive "reviver" walk is
performed.

A C recursion limit is imposed for parse(), just like stringify().

Comparison of JSON and Ecmascript syntax
----------------------------------------

JSONWhiteSpace
::::::::::::::

JSONWhiteSpace does not have a direct Ecmascript syntax equivalent.

JSONWhiteSpace is defined as::

  JSONWhiteSpace::
      <TAB>
      <CR>
      <LF>
      <SP>

whereas Ecmascript WhiteSpace and LineTerminator are::

  WhiteSpace::
      <TAB>
      <VT>
      <FF>
      <SP>
      <NBSP>
      <BOM>
      <USP>

  LineTerminator::
      <LF>
      <CR>
      <LS>
      <PS>

Because JSONWhiteSpace includes line terminators, the closest Ecmascript
equivalent is WhiteSpace + LineTerminator.  However, that includes several
additional characters.

JSONString
::::::::::

JSONString is defined as::

  JSONString::
      " JSONStringCharacters_opt "

  JSONStringCharacters::
      JSONStringCharacter JSONStringCharacters_opt

  JSONStringCharacter::
      SourceCharacter but not one of " or \ or U+0000 through U+001F
      \ JSONEscapeSequence

  JSONEscapeSequence ::
      JSONEscapeCharacter
      UnicodeEscapeSequence

  JSONEscapeCharacter :: one of
      " / \ b f n r t

The closest equivalent is Ecmascript StringLiteral with only the double
quote version accepted::

  StringLiteral::
      " DoubleStringCharacters_opt "
      ' SingleStringCharacters_opt '

  DoubleStringCharacters::
      DoubleStringCharacter DoubleStringCharacters_opt

  DoubleStringCharacter::
      SourceCharacter but not one of " or \ or LineTerminator
      \ EscapeSequence
      LineContinuation

  SourceCharacter: any Unicode code unit

Other differences include:

* Ecmascript DoubleStringCharacter accepts source characters between
  U+0000 and U+001F (except U+000A and U+000D, which are part of
  LineTerminator).  JSONStringCharacter does not.

* Ecmascript DoubleStringCharacter accepts LineContinuation,
  JSONStringCharacter does not.

* Ecmascript DoubleStringCharacter accepts and parses broken escapes
  as single-character identity escapes, e.g. the string "\\u123" is
  parsed as "u123".  This happens because EscapeSequence contains a
  NonEscapeCharacter production which acts as an "escape hatch" for
  such cases.  JSONStringCharacter is strict and will cause a SyntaxError
  for such escapes.

* Ecmascript EscapeSequence accepts single quote escape ("\\'"),
  JSONEscapeSequence does not.

* Ecmascript EscapeSequence accepts zero escape ("\\0"), JSONEscapeSequence
  does not.

* Ecmascript EscapeSequence accepts hex escapes ("\\xf7"),
  JSONEscapeSequence does not.

* JSONEscapeSquence accepts forward slash escape ("\\/").  Ecmascript
  EscapeSequence has no explicit support for it, but it is accepted through
  the NonEscapeCharacter production.

Note that JSONEscapeSequence is a proper subset of EscapeSequence.

JSONNumber
::::::::::

JSONNumber is defined as::

  JSONNumber::
      -_opt DecimalIntegerLiteral JSONFraction_opt ExponentPart_opt

Ecmascript NumericLiteral and DecimalLiteral::

  NumericLiteral::
      DecimalLiteral | HexIntegerLiteral

  DecimalLiteral::
      DecimalIntegerLiteral . DecimalDigits_opt ExponentPart_opt
      . DecimalDigits ExponentPart_opt
      DecimalIntegerLiteral ExponentPart_opt

  ...

Another close match would be StrDecimalLiteral::

  StrDecimalLiteral::
      StrUnsignedDecimalLiteral
      + StrUnsignedDecimalLiteral
      - StrUnsignedDecimalLiteral

  StrUnsignedDecimalLiteral::
      Infinity
      DecimalDigits . DecimalDigits_opt ExponentPart_opt
      . DecimalDigits ExponentPart_opt

Some differences between JSONNumber and DecimalLiteral:

* NumericLiteral allows either DecimalLiteral (which is closest to JSONNumber)
  and HexIntegerLiteral.  JSON does not allow hex literals.

* JSONNumber is a *almost* proper subset of DecimalLiteral:

  - DecimalLiteral allows period without fractions (e.g. "1." === "1"),
    JSONNumber does not.

  - DecimalLiteral allows a number to begin with a period without a leading
    zero (e.g. ".123"), JSONNumber does not.

  - DecimalLiteral does not allow leading zeros (although many implementations
    allow them and may parse them as octal; e.g. V8 will parse "077" as octal
    and "099" as decimal).  JSONNumber does not allow octals, and given that
    JSON is a strict syntax in nature, parsing octals or leading zeroes should
    not be allowed.

  - However, JSONNumber allows a leading minus sign, DecimalLiteral does not.
    For Ecmascript code, the leading minus sign is an unary minus operator,
    and it not part of the literal.

* There are no NaN or infinity literals.  There are no such literals for
  Ecmascript either but they become identifier references and *usually*
  evaluate to useful constants.

JSONNullLiteral
:::::::::::::::

Trivially the same as NullLiteral.

JSONBooleanLiteral
::::::::::::::::::

Trivially the same as BooleanLiteral.

Extended custom encoding (JX)
=============================

The extended custom encoding format (JX, controlled by the define
``DUK_USE_JX``) extends the JSON syntax in an incompatible way, with
the goal of serializing as many values as faithfully and readably as
possible, with as many values as possible parsing back into an accurate
representation of the original value.  All results are printable ASCII
to be maximally useful in embedded environments.

Undefined
---------

The ``undefined`` value is encoded as::

  undefined

String values
-------------

Unicode codepoints above U+FFFF are escaped with an escape format borrowed
from Python::

  "\U12345678"

For codepoints between U+0080 and U+00FF a short escape format is used::

  "\xfc"

When encoding, the shortest escape format is used.  When decoding input
values, any escape formats are allowed, i.e. all of the following are
equivalent::

  "\U000000fc"
  "\u00fc"
  "\xfc"

Number values
-------------

Special numbers are serialized in their natural Ecmascript form::

  NaN
  Infinity
  -Infinity

Function values
---------------

Function values are serialized as::

  {_func:true}

Function values do not survive an encoding round trip.  The decode result
will be an object which has a ``_func`` key.

Buffer values
-------------

Plain buffer values and Buffer object values are serialized in hex form::

  |deadbeef|

Pointer values
--------------

Plain pointer values and Pointer object values are serialized in a platform
specific form, using the format ``(%p)``, e.g.::

  (0x1ff0e10)          // 32-bit Linux
  (000FEFF8)           // 32-bit Windows
  (000000000026A8A0)   // 64-bit Windows

A pointer value parses back correctly when serialized and parsed by the same
program.  Other than that there is no guarantee that a pointer value can be
parsed back across different Duktape builds.  Note that pointer format may
differ between compilers even on the same platform.

If the pointer value doesn't parse back, with ``sscanf()`` and ``%p``
format applied to the value between the parentheses, the value is replaced by
a NULL pointer during parsing.  This is probably more useful than throwing
an error.

``NULL`` pointers are serialized in a platform independent way as::

  (null)

ASCII only output
-----------------

The output for JX encoding is always ASCII only.  The standard Ecmascript
JSON encoding retains Unicode characters outside the ASCII range as is
(deviating from this would be non-compliant) which is often awkward in
embedded environments.

The codepoint U+007F, normally not escaped by Ecmascript JSON functions,
is also escaped for better compatibility.

Avoiding key quotes
-------------------

Key quotes are omitted for keys which are ASCII and match Ecmascript
identifier requirements be encoded without quotes, e.g.::

  { my_value: 123 }

When the key doesn't fit the requirements, the key is quoted as
usual::

  { "my value": 123 }

The empty string is intentionally not encoded or accepted without
quotes (although the encoding would be unambiguous)::

  { "": 123 }

The ASCII identifier format (a subset of the Ecmascript identifier
format which also allows non-ASCII characters) is::

  [a-zA-Z$_][0-9a-zA-Z$_]*

This matches almost all commonly used keys in data formats and such,
improving readability a great deal.

When parsing, keys matching the identifier format are of course accepted
both with and without quotes.

Compatible custom encoding (JC)
===============================

The compatible custom encoding format (JC, controlled by the define
``DUK_USE_JC``) is intended to provide a JSON interface which is more
useful than the standard Ecmascript one, while producing JSON values
compatible with the Ecmascript and other JSON parsers.

As a general rule, all values which are not ordinarily handled by standard
Ecmascript JSON are encoded as object values with a special "marker" key
beginning with underscore.  Such values decode back as objects and don't
round trip in the strict sense, but are nevertheless detectable and even
(manually) revivable to some extent.

Undefined
---------

The ``undefined`` value is encoded as::

  {"_undef":true}

String values
-------------

Unicode codepoints above U+FFFF are escaped into plain text as follows::

  "U+12345678"

This is not ideal, but retains at least some of the original information
and is Ecmascript compatible.

BMP codepoints are encoded as in standard JSON.

Number values
-------------

Special numbers are serialized as follows::

  {"_nan":true}
  {"_inf":true}
  {"_ninf":true}

Function values
---------------

Function values are serialized as::

  {"_func":true}

Like other special values, function values do not survive an encoding round trip.

Buffer values
-------------

Plain buffer values and Buffer object values are serialized in hex form::

  {"_buf":"deadbeef"}

Pointer values
--------------

Plain pointer values and Pointer object values are serialized in a platform
specific form, using the format ``%p``, but wrapped in a marker table::

  {"_ptr":"0x1ff0e10"}

``NULL`` pointers are serialized in a platform independent way as::

  {"_ptr":"null"}

Note that compared to JX, the difference is that there are no surrounding
parentheses outside the pointer value.

ASCII only output
-----------------

Like JX, the output for JC encoding is always ASCII only, and the codepoint
U+007F is also escaped.

Key quoting
-----------

Unlike JX, keys are always quoted to remain compatible with standard JSON.

Custom formats used by other implementations
============================================

(This is quite incomplete.)

Python
------

Python uses the following NaN and infinity serializations
(http://docs.python.org/2/library/json.html)::

  $ python
  Python 2.7.3 (default, Aug  1 2012, 05:14:39) 
  [GCC 4.6.3] on linux2
  Type "help", "copyright", "credits" or "license" for more information.
  >>> import numpy
  >>> import json
  >>> print(json.dumps({ 'k_nan': numpy.nan, 'k_posinf': numpy.inf, 'k_neginf': -numpy.inf }))
  {"k_posinf": Infinity, "k_nan": NaN, "k_neginf": -Infinity}

Proto buffer JSON serialization
-------------------------------

Protocol buffers have a JSON serialization; does not seem relevant:

* http://code.google.com/p/protobuf-json/source/checkout

Dojox/json/ref
--------------

Dojox/json/ref supports object graphs, and refers to objects using a marker
object with a special key, ``$ref``.

* http://dojotoolkit.org/reference-guide/1.8/dojox/json/ref.html

Using keys starting with ``$`` may be a good candidate for custom types, as
it is rarely used for property names.

AWS CloudFormation
------------------

Base64 encoding through a "function" syntax:

* http://docs.aws.amazon.com/AWSCloudFormation/latest/UserGuide/resources-section-structure.html

Rationale for custom formats
============================

Security and eval()
-------------------

One apparent goal of JSON is to produce string representations which can be
safely parsed with ``eval()``.  When using custom syntax this property may
be lost.  For instance, if one uses the custom Python encoding of using
``NaN`` to represent a NaN, this ``eval()``\ s incorrectly if there is a
conflicting definition for ``NaN`` in the current scope (note that e.g.
"NaN" and "undefined" are *not* Ecmascript literals, but rather normal
global identifiers).

ASCII only serialization
------------------------

ASCII only serialization is a useful feature in many embedded applications,
as ASCII is a very compatible subset.  Unfortunately there is no standard way
of guaranteeing an ASCII-only result: the ``Quote()`` algorithm will encode
all non-ASCII characters as-is.

Further, the standard Ecmascript JSON interface does not escape U+007F, which
is usually considered a "dangerous" character.

Buffer representation
---------------------

Base64 would be a more compact and often used format for representing binary
data.  However, base64 data does not allow a programmer to easily parse the
binary data (which often represents some structured data, such as a C struct).

Function representation
-----------------------

It would be possible to serialize a function into actual Ecmascript function
syntax.  This has several problems.  First, sometimes the function source may
not be available; perhaps the build strips source code from function instances
to save space, or perhaps the function is a native one.  Second, the result is
costly to parse back safely.  Third, although seemingly compatible with
``eval()``\ ing the result, the function will not retain its lexical environment
and will thus not always work properly.

Future work
===========

Hex constants
-------------

Parse hex constants in JX::

  { foo: 0x1234 }

This is useful for e.g. config files containing binary flags, RGB color
values, etc.

Comments
--------

Allow ``//`` and/or ``/* */`` comment style.  This is very useful for
config files and such and allowed by several other JSON parsers.

Trailing commas in objects and arrays
-------------------------------------

Allow commas in objects and arrays.  Again, useful for config files and
such, and also supported by other JSON parsers.

Serialization depth limit
-------------------------

Allow caller to impose a serialization depth limit.  Attempt to go too
deep into object structure needs some kind of marker in the output, e.g.::

  // JX
  { foo: { bar: { quux: ... } } }
  { foo: { bar: { quux: {_limit:true} } } }

  // JC
  { foo: { bar: { quux: {"_limit":true} } } }

Serialization size limit
------------------------

Imposing a maximum byte size for serialization output would be useful when
dealing with untrusted data.

Serializing ancestors and/or non-enumerable keys
------------------------------------------------

JSON serialization currently only considers enumerable own properties.  This
is quite limiting for e.g. debugging.

Serializing array properties
----------------------------

JSON serializes only array elements, but the format could be easily extended
to also serialize enumerable properties, e.g. as::

  [ 'foo', 'bar', name: 'todo list' ]

Sorting keys for canonical encoding
-----------------------------------

If object keys could be sorted, the compact JSON output would be canonical.
This would often be useful.

Circular reference support
--------------------------

Something along the lines of:

* http://dojotoolkit.org/reference-guide/1.8/dojox/json/ref.html
* http://dojotoolkit.org/api/1.5/dojox/json/ref

Dojox/json/ref refers to objects using a marker object with a special
key, ``$ref``.

Better control over separators
------------------------------

E.g. Python JSON API allows caller to set separators in more detail
than in the Ecmascript JSON API which only allows setting the "space"
string.

RegExp JSON serialization
-------------------------

Currently RegExps serialize quite poorly::

  duk> JSON.stringify(/foo/)
  = {}

Automatic revival of special values when parsing JC
---------------------------------------------------

It would be nice to have an option for reviving special values parsed
from JC data.  With this, JC and JX formats would round trip equally well.

Expose encode/decode primitives in a more low level manner
----------------------------------------------------------

Allow more direct access to encoding/decoding flags and provide more
extensibility with an argument convention better than the one used
in Ecmascript JSON API.

For instance, arguments could be given in a table::

  Duktape.jsonDec(myValue, {
    allowHex: true
  });

However, passing flags and arguments in objects has a large footprint.

Alternative to "undefined"
--------------------------

Because "undefined" is not an actual keyword, it may be bound to an arbitrary
value and is thus unsafe to eval.  An alternative to "undefined" is "void 0"
which always evaluates to undefined, but is a bit cryptic.
