======================================
Function template and instance objects
======================================

Difference between a template and an instance
=============================================

A *function template* is a Duktape internal Ecmascript Function object.
Function templates are not exposed to user code, and represent a compiled
function without a concrete surrounding environment.  Function templates
are created by the compiler for every function and inner function.  Because
a template is missing a surrounding lexical environment, it cannot be called
as a function.

A function template is instantiated into a concrete *function instance*
(also called a *closure*) by creating a new Function object, copying most
(but not all) template fields into the Function object, and initializing
the instance specific fields like the outer lexical environment properly.
Function instances are created for function compilation results and when
inner functions are later instantiated (with the CLOSURE instruction).

This separation is necessary because a certain function template can be
instantied multiple times with a different outer environment each time.
Consider the following::

  function mkPrinter(str) {
    // inner function
    return function() { print(str); }
  }

  var p1 = mkPrinter("Hello world");
  var p2 = mkPrinter("still here");
  p1();
  p2();
  print(p1 === p2);  // => false

In this example:

* The ``mkPrinter`` function is first compiled into a function template and
  then immediately converted to a function instance.  The function instance
  has the global environment as its outer environment.  The instance is then
  associated with the ``mkPrinter`` property of the global object.

* The inner function inside ``mkPrinter`` is represented by a function
  template stored as part of the ``mkPrinter`` function inner function table.

* The ``p1`` and ``p2`` function objects are separate Function objects
  created by a CLOSURE instruction occurring in the bytecode of ``mkPrinter``.
  They have their own properties and separate outer lexical environments, but
  shared the same bytecode, pc-to-line conversion data, etc.  The outer lexical
  environment for ``p1`` and ``p2`` is the declarative environment created when
  ``mkPrinter`` was entered, and contains the ``str`` binding needed
  to print separate texts when ``p1`` and ``p2`` are called.

A function instance does not reference a function template from a garbage
collection point of view.  The function template can be collected while the
function instance remains reachable.

Properties of a function template
=================================

The E5 specification does not recognize a "function template", so there
are no standard properties for function templates.  The properties can also
change from release to release because they are not exposed to user code.
The following properties are used:

+---------------+---------------------------------------------------------+
| Property      | Description                                             |
+===============+=========================================================+
| ``_Varmap``   | Maps register-bound variable names to their register    |
|               | numbers.                                                |
|               | Example: ``{ arg1: 0, arg2: 1, myvar: 2 }``.            |
+---------------+---------------------------------------------------------+
| ``_Formals``  | An array of formal argument names.  ``formals.length``  |
|               | provides the number of formal arguments.  Note that the |
|               | number of formal arguments does not need to match       |
|               | function ``nargs``: the function might access all args  |
|               | through the arguments object and have ``nargs`` set to  |
|               | zero.  This property is used to initialize the          |
|               | arguments object (in non-strict code); the compiler     |
|               | should omit this whenever possible.                     |
|               | Example: ``[ "arg1", "arg2" ]``.                        |
+---------------+---------------------------------------------------------+
| ``name``      | Function name, set for function declarations and named  |
|               | function expressions.  If DUK_HOBJECT_FLAG_NAMEBINDING  |
|               | is set, the value of this property is bound in the      |
|               | function's environment (used for named function         |
|               | expressions).                                           |
|               | Example: ``"func"``.                                    |
+---------------+---------------------------------------------------------+
| ``fileName``  | Source filename (or equivalent).  Used to add source    |
|               | file information to error objects and tracebacks.       |
+---------------+---------------------------------------------------------+
| ``_Source``   | Function source code.  E5 specifies that the source     |
|               | code of a function must be valid syntax.                |
+---------------+---------------------------------------------------------+
| ``_Pc2line``  | Debug information: maps bytecode index to a source line |
|               | number.  Space-optimized binary format.                 |
+---------------+---------------------------------------------------------+

The compiler should omit whatever internal properties are not needed to
save space.  For instance:

* ``_Varmap`` is not needed if the function can never perform a slow path
  identifier reference.

* ``_Formals`` is not needed unless a non-strict arguments object is
  potentially constructed.  (However, ``_Formals`` is also used for deriving
  the "length" of the instance.  If _Formals is omitted, something else needs
  to be set in the template to allow instance "length" to be initialized.)

When debugging, it may be necessary to store more function properties than
needed by plain execution.  For instance, source code should be available
even for dynamically generated code.

Properties of a function instance
=================================

The creation of function instances is described in E5 Section 13.2.
Each function instance (each closure created from a function
expression or declaration) has the following standard properties:

* ``length``: set to number of formal parameters (length of ``_Formals``).

* ``prototype``: points to a fresh object which has a ``constructor``
  property pointing back to the function

* ``caller``: thrower (strict functions only)

* ``arguments``: thrower (strict functions only)

There is considerable variance in practical implementations:

* smjs::

    // the "name" property is non-standard; "arguments" and "caller" are
    // present for a non-strict function

    js> f = function foo() {}
    (function () {})
    js> Object.getOwnPropertyNames(f)
    ["prototype", "length", "name", "arguments", "caller"]

    // for strict mode, the same properties are present.

    js> f = function foo() { "use strict"; }
    (function foo() {"use strict";})
    js> Object.getOwnPropertyNames(f);
    ["prototype", "length", "name", "arguments", "caller"]

    // the "name" property contains the function expression name

    js> f.name
    "foo"

    // "name" is non-writable, non-configurable (and non-enumerable)
    // -> works as a reliable "internal" property too

    js> Object.getOwnPropertyDescriptor(f, 'name')
    ({configurable:false, enumerable:false, value:"foo", writable:false})

* nodejs (v8)::

    // "name" is non-standard; "arguments" and "caller" are present
    // for even a non-strict function

    > f = function foo() {}
    [Function: foo]
    > Object.getOwnPropertyNames(f)
    [ 'length',
      'caller',
      'arguments',
      'name',
      'prototype' ]
    > f.name
    'foo'

    // strict mode is the same

    > f = function foo() { "use strict"; }
    [Function: foo]
    > Object.getOwnPropertyNames(f)
    [ 'name',
      'length',
      'arguments',
      'prototype',
      'caller' ]

    // 'name' is writable but not configurable/enumerable

    > f.name
    'foo'
    > Object.getOwnPropertyDescriptor(f, 'name')
    { value: 'foo',
      writable: true,
      enumerable: false,
      configurable: false }

* rhino::

    // "name" is non-standard, "arity" is non-standard, "arguments"
    // is present (but "caller" is not)

    js> f = function foo() {}
    [...]
    js> Object.getOwnPropertyNames(f)
    arguments,prototype,name,arity,length

    // name is non-writable, non-enumerable, non-configurable

    js> pd = Object.getOwnPropertyDescriptor(f, 'name')
    [object Object]
    js> pd.writable
    false
    js> pd.enumerable
    false
    js> pd.configurable
    false

    // strict mode functions are similar

Notes:

* "caller" and "arguments" would be nice as virtual properties to minimize
  object property count.  They can't be inherited in the ordinary way without
  breaking compliance (the standard requires they be own properties).

* "prototype" would be nice as a virtual property: it's quite
  expensive to have for every function instance.

The properties for function instances are (these are also documented in
user documentation for the exposed parts):

+---------------+---------------------------------------------------------+
| Property      | Description                                             |
+===============+=========================================================+
| ``length``    | Set to the number of formal parameters.  For normal     |
|               | functions parsed from Ecmascript source code, this is   |
|               | set to ``_Formals.length``.  Built-in functions may be  |
|               | special.                                                |
+---------------+---------------------------------------------------------+
| ``prototype`` | Points to a fresh object which has a ``constructor``    |
|               | property pointing back to the function instance.        |
+---------------+---------------------------------------------------------+
| ``caller``    | For strict functions, set to the ``[[ThrowTypeError]]`` |
|               | function object defined in E5 Section 13.2.3.           |
+---------------+---------------------------------------------------------+
| ``arguments`` | Like ``caller``.                                        |
+---------------+---------------------------------------------------------+
| ``name``      | See function templates.                                 |
+---------------+---------------------------------------------------------+
| ``fileName``  | See function templates.                                 |
+---------------+---------------------------------------------------------+
| ``_Lexenv``   | Together with DUK_HOBJECT_FLAG_NEWENV, controls the     |
|               | initialization of variable/lexical environment when a   |
|               | function call occurs.                                   |
|               |                                                         |
|               | The DUK_HOBJECT_FLAG_NEWENV is set for ordinary         |
|               | functions, which always get a new environment record    |
|               | when they are called.  The flag is cleared for global   |
|               | code and eval code, which "share" an existing           |
|               | environment record.                                     |
|               |                                                         |
|               | If _Varenv is missing, it defaults to _Lexenv (which is |
|               | very often the case).                                   |
+---------------+---------------------------------------------------------+
| ``_Varenv``   | See ``_Lexenv``.                                        |
+---------------+---------------------------------------------------------+
| ``_Varmap``   | See function templates.                                 |
+---------------+---------------------------------------------------------+
| ``_Formals``  | See function templates.                                 |
+---------------+---------------------------------------------------------+
| ``_Source``   | See function templates.                                 |
+---------------+---------------------------------------------------------+
| ``_Pc2line``  | See function templates.                                 |
+---------------+---------------------------------------------------------+

Built-in functions
==================

The properties of built-in functions are a special case, because
they are not created with the algorithm in E5 Section 13.2;
instead, their properties are described explicitly in E5 Section 15.

There is considerable variance between implementations on what
properties built-in functions get.

Duktape/C functions
===================

Duktape/C functions are also represented by an Ecmascript Function object.
The properties of such functions are extremely minimal; for instance, they
are missing the ``length`` property.  This is done to keep the object size
as small as possible.  This means, however, that the Function objects are
non-standard.

Duktape/C functions also don't have any need for control variables such as
``_Lexenv``, ``_Pc2line``, etc.

pc2line format
==============

``_Pc2line`` property allows a program counter (bytecode index) to be
converted to an approximate line number of the expression which generated
the bytecode instruction in question.  Logically it can be considered an
array (in fact, Lua implements a similar structure as a simple array):

+----+------+
| PC | Line |
+====+======+
| 0  | 1    |
+----+------+
| 1  | 1    |
+----+------+
| 2  | 3    |
+----+------+
| 3  | 4    |
+----+------+
| 4  | 7    |
+----+------+

If the line number is represented as a 4-byte integer, the structure would
take as much memory as the related bytecode, doubling memory usage.  Clearly
a more space efficient format is desirable, as long as performance is not
impacted too much when throwing and catching errors.

Although the line number generally stays the same or increases when PC
increases, this is not always the case (e.g. in loop structures).  This
rules out search structures relying on monotonicity properties.  It's nice
if an arbitrary mapping can be expressed if necessary.

Error line number is needed when:

* Accessing the non-standard ``lineNumber`` property.  This property can be
  implemented as a getter in the Error prototype, which will get the PC
  from the traceback data (if any), and do the PC-to-line conversion only
  when actually needed.

* Creating a string-formatted traceback.  PC-to-line conversions are needed
  for most traceback lines.

The current format is based on the observation that when PC increases by one,
the typical delta for the line number is very small (and is usually zero or
positive).  Deltas can be expressed efficiently with variable bit length
encoding.  To provide a reasonably fast random access, explicit starting
point values are recorded for every nth bytecode instruction (currently,
every 64th; SKIP=64 below).  During a lookup one can first skip close to the
desired mapping entry and then scan the bit-packed format forwards.

The format consists of a header structure followed by bit packed diff
streams (each bit packed stream begins at a byte boundary):

+--------+------+---------------------------------------------------+
| Offset | Type | Description                                       |
+========+======+===================================================+
| 0      | u32  | PC limit (maximum PC, exclusive)                  |
+--------+------+---------------------------------------------------+
| 4      | u32  | Line number for PC 0*SKIP                         |
+--------+------+---------------------------------------------------+
| 8      | u32  | Byte offset of diff bitstream for PC 0*SKIP       |
+--------+------+---------------------------------------------------+
| 12     | u32  | Line number for PC 1*SKIP                         |
+--------+------+---------------------------------------------------+
| 16     | u32  | Byte offset of diff bitstream for PC 1*SKIP       |
+--------+------+---------------------------------------------------+
| ...    |      | A total of ceil(bytecode_length/SKIP)             |
|        |      | line/offset entries                               |
+--------+------+---------------------------------------------------+
| ...    |      | Diff bitstreams                                   |
+--------+------+---------------------------------------------------+

The diff bitstream consists of SKIP-1 diff entries for a certain
starting point.  Each diff entry simply encodes the line number
difference when PC increases by one; the difference may be
negative, zero, or positive.  The diff is encoded as one of the
following entry types:

+-----------------+--------------------------------------------------------+
| Bits            | Description                                            |
+=================+========================================================+
| 0               | Difference is +0                                       |
+-----------------+--------------------------------------------------------+
| 1 0 <2 bits>    | Difference is: +1, +2, +3, or +4 (encoded as 2 bits)   |
+-----------------+--------------------------------------------------------+
| 1 1 0 <8 bits>  | Difference is a signed 8-bit value, encoded with bias  |
|                 | +0x80 (as unsigned 0x00 ... 0xff)                      |
+-----------------+--------------------------------------------------------+
| 1 1 1 <32 bits> | Fallback, linenumber encoded as absolute 32-bit value  |
+-----------------+--------------------------------------------------------+

These cases are not optimized, but rather best guesses combined with some
experimentation:

* Usually multiple bytecode instructions are generated from a single line
  of source code, hence the case +0 is important to encode efficiently.

* When line changes, there are either no lines without code, or there are
  a few such lines (empty lines for readability, perhaps a few comment
  lines).  The cases +1...+4 are encoded compactly for these cases.

* The signed 8-bit offset covers large comment blocks, and the occasional
  negative steps (e.g. in loop structures).

* As a fallback, an absolute 32-bit line number can be encoded.  This covers
  any remaining cases and provides completeness.

As an example, the bitstream for the diffs [+0, +2, +9, -3, +0] would be::

     0 1001 11000001001 11011111101 0
  => 01001110 00001001 11011111 10100000  (padded with 0)
  => 0x4e 0x09 0xdf 0xa0

Typically the pc2line data is about 10-15% of the size of the corresponding
bytecode, a very modest addition to footprint compared to the 100% addition
of a straight table approach.

