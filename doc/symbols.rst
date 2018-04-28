=============================
ES2015 Symbols in Duktape 2.x
=============================

Overview
========

Duktape 2.x adds ES2015 Symbol support.  Duktape 1.x internal keys are unified
with the Symbol concept, and are considered a custom "hidden symbol" type
which is not normally visible to ECMAScript code.  C code can access hidden
symbols, however.

The internal implementation is similar to existing internal keys.  Symbols
are represented as ``duk_hstring`` heap objects, with the string data
containing a byte prefix which is invalid (extended) UTF-8 so that it can
never occur for normal ECMAScript strings, or even strings with non-BMP
codepoints.  Object coerced strings have a special object class and the
underlying symbol is stored in ``_Value`` similarly to e.g. Number object.

Representation basics:

* Symbols have an external type ``DUK_TYPE_STRING``.

* Symbols have internal type tag ``DUK_TAG_STRING``.

* Symbols can be distinguished internally from ordinary strings by looking
  up the ``DUK_HSTRING_FLAG_SYMBOL`` flag.  Hidden symbols also have the
  ``DUK_HSTRING_FLAG_HIDDEN`` set.

Behavior basics:

* Symbols are visible to ECMAScript code as required by ES2015 and later.
  Hidden symbols are not visible through e.g.
  ``Object.getOwnPropertySymbols()``.  They can only be accessed if a
  reference to the hidden symbol string is somehow available, e.g. via a
  C binding.

* Symbols are visible to the public C API as strings: ``duk_is_string()``
  is true, ``duk_get_string()`` returns a pointer to the symbol internal
  string representation, etc.  C code can create symbols simply by pushing
  C strings with a specific format, see below.

* While symbols are strings in the C API, coercion semantics are based on
  the ECMAScript behavior.  For example, ``duk_to_string()`` applied to a
  symbol throws a ``TypeError``.

See:

* https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/Symbol

* http://www.2ality.com/2014/12/es6-symbols.html

Internal key formats
====================

Initial bytes in the ranges [0x00,0x7F] and [0xC0,0xFE] are valid for Duktape's
extended UTF-8 flavor.  The byte 0xFF and the range [0x80,0xBF] are free to be
used as symbol markers.

+-----------------------------------------------+-----------------------------------------------------------------+
| Internal string format                        | Description                                                     |
+-----------------------------------------------+-----------------------------------------------------------------+
| <ff> anyValue                                 | Hidden symbol (Duktape specific) used by application code.      |
|                                               | Prior to Duktape 2.2 Duktape internal hidden symbols also used  |
|                                               | the 0xFF prefix followed by a capital letter (A-Z).  Starting   |
|                                               | from Duktape 2.2 all 0xFF prefixed strings are reserved for     |
|                                               | application code.                                               |
+-----------------------------------------------+-----------------------------------------------------------------+
| <80> symbolDescription                        | Global symbol with description 'symbolDescription' created      |
|                                               | using Symbol.for().                                             |
+-----------------------------------------------+-----------------------------------------------------------------+
| <81> symbolDescription <ff> uniqueSuffix      | Local symbol with description 'symbolDescription'.  Trailing    |
|                                               | unique string makes the symbol unique.  The unique suffix is    |
|                                               | opaque and chosen arbitrarily by Duktape.  It's unique within a |
|                                               | Duktape heap (across all global environments).                  |
+-----------------------------------------------+-----------------------------------------------------------------+
| <81> <ff> uniqueSuffix                        | Local symbol with an empty description.  Unique suffix makes    |
|                                               | each such symbol unique.  The unique suffix is arbitrary but    |
|                                               | must not contain the 0xFF byte.                                 |
+-----------------------------------------------+-----------------------------------------------------------------+
| <81> <ff> uniqueSuffix <ff>                   | Local symbol with undefined description.  ES2015 differentiates |
|                                               | internally between symbols with an empty string description vs. |
|                                               | symbols with an undefined description.                          |
+-----------------------------------------------+-----------------------------------------------------------------+
| <81> symbolDescription <ff>                   | Well known symbol with description 'symbolDescription'.  Well   |
|                                               | known symbols (like Symbol.iterator) are local symbols which    |
|                                               | are still shared across "code realms".  Any fixed suffix never  |
|                                               | colliding with runtime generated unique local symbols works,    |
|                                               | currently an empty suffix is used.                              |
+-----------------------------------------------+-----------------------------------------------------------------+
| <82> anyValue                                 | Hidden symbol (Duktape specific) used by Duktape internals.     |
|                                               | User code should never use this byte prefix or rely on any      |
|                                               | Duktape internal hidden Symbols.                                |
+-----------------------------------------------+-----------------------------------------------------------------+
| <83 to be>                                    | Reserved for future use, behavior is undefined (Duktape 2.1     |
|                                               | interprets as Symbols, Duktape 2.2 does not, don't rely on      |
|                                               | either behavior.                                                |
+-----------------------------------------------+-----------------------------------------------------------------+
| <bf>                                          | Initial byte marker for bytecode dump format since Duktape 2.2. |
+-----------------------------------------------+-----------------------------------------------------------------+
| <00 to 7f>                                    | Valid ASCII initial byte.                                       |
+-----------------------------------------------+-----------------------------------------------------------------+
| <c0 to f7>                                    | Valid standard UTF-8 (or CESU-8) initial byte.                  |
+-----------------------------------------------+-----------------------------------------------------------------+
| <f8 to fe>                                    | Valid extended UTF-8 initial byte.                              |
+-----------------------------------------------+-----------------------------------------------------------------+

There are public API macros (DUK_HIDDEN_SYMBOL() etc) to create symbol literals
from C code.

Global symbols
==============

Global symbols are the same across separate global environments and even across
Duktape heaps.  ES2015 Section 19.4.2.1:

    The GlobalSymbolRegistry is a List that is globally available.
    It is shared by all Code Realms.

and ES2015 Section 8.2:

    Before it is evaluated, all ECMAScript code must be associated with a Realm.
    Conceptually, a realm consists of a set of intrinsic objects, an ECMAScript
    global environment, all of the ECMAScript code that is loaded within the
    scope of that global environment, and other associated state and resources.

The current approach satisfies these simply by making a globally registered
Symbol have a fixed format so that if a Symbol with the same description is
created in another Duktape thread (or even Duktape heap), its internal
representation will be identical.  No explicit registry is maintained.

Well-known symbols
==================

Well-known symbols (such as ``Symbol.iterator``) are distinct from any local or
global symbols.  ES2015 Section 6.1.5.1:

    Well-known symbols are built-in Symbol values that are explicitly referenced
    by algorithms of this specification. They are typically used as the keys of
    properties whose values serve as extension points of a specification algorithm.
    Unless otherwise specified, well-known symbols values are shared by all Code
    Realms (8.2).

The fixed representation described above ensures that well-known symbols are
the same across all code realms (and even across Duktape heaps).  The internal
representation is essentially the same as for a unique local symbol, but the
suffix that makes local symbols unique is missing.  Thus, they behave like
local symbols other than having a fixed representation.

Unifying with Duktape internal keys
===================================

Necessary changes to add symbol behavior:

* Strings with initial byte 0x80, 0x81, 0x82 or 0xFF are flagged as symbols
  (``DUK_HSTRING_FLAG_SYMBOL``).  If the initial byte is 0xFF or 0x82, also
  the hidden symbol flag (``DUK_HSTRING_FLAG_HIDDEN``) is set.

* ``typeof(sym)`` should return "symbol" rather than string.  This is done
  for Duktape hidden symbols too.

* ``ToString(sym)`` must be rejected for a symbol, while ``String(sym)``
  must specifically check for symbols.  Coercion needs to strip possible
  "unique suffix" when coming up with the Symbol description.

* Symbols should be safe from accidental enumeration, JSON serialization, etc.
  This is actually already the case because internal keys are already excluded
  in Duktape 1.x.

* ``Object.getOwnPropertySymbols(``) should return a list of symbol properties
  for an object, but filter out Duktape hidden symbols.

* ``Object(sym)`` should create an object with internal class "Symbol",
  with the plain symbol value stored behind ``_Value`` (hidden symbol
  property) as for Number objects, etc.

* Non-strict comparison needs to handle symbols.  ToPrimitive() coercion
  is maybe enough to ensure ``sym == Object(sym)`` is accepted.

* Property code needs to accept plain Symbols as is (treated like any other
  strings), and Symbol objects should look up their internal string value
  (instead of being coerced to e.g. ``Symbol(symbolDescription)``.  Current
  code just uses ``ToString()`` which causes a TypeError.

* Dozens of similar semantics checks throughout the code base.

Some design questions
=====================

How should C code see Symbols?
------------------------------

Easiest approach:

* Symbols are not enumerated by duk_enum() unless requested.  Either fold in with
  internal keys, add a separate flags.  Maybe rename existing internal keys
  flag.

* Property operations work with symbols and internal keys without distinction.

* API call to create a symbol from C code.  Hides the construction of the internal
  string.

Best naming for Duktape internal keys
-------------------------------------

With https://github.com/svaarala/duktape/pull/979 Duktape internal properties
would become unreachable from ECMAScript code, even if you construct the
internal string using a buffer and then try to use it as an object key.
This offers more protection for sandboxing than ES2015 Symbols which can be
enumerated.

Current naming for Duktape 1.x internal keys is "hidden symbols".  Some
alternatives considered:

* Internal symbol: easy to confuse with specification symbols for example.
  One benefit would be that as a term close to "internal property".

* Hidden symbol: conveys semantics (assuming GH-797) pretty well.

* Private symbol

* Native symbol

* Invisible symbol
