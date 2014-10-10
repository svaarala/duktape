=====================
Identifier algorithms
=====================

Introduction
============

This document discusses the internal representation of lexical environments
and the exposed algorithms for identifier handling, taking into account the
internal representation.

The goal of the compiler is to convert almost all identifier related
accesses to register accesses and to avoid the explicit creation of
environment record objects whenever possible.  For full compliance, a "slow
path" is needed, and is provided by the environment record objects and the
identifier algorithms described here.  The slow path is optimized for a
compact code footprint, not speed.

Identifier handling encompasses a large part of the E5 specification and
the implementation:

* Variable and function declarations in global code, eval code, and function
  code (compile time)

* Function argument list declarations (compile time)

* Establishing execution contexts (calls to global code, eval code, and
  function code; run time)

* Establishing other lexical environments (``catch`` blocks, ``with`` blocks;
  run time)

* Reading identifier values and ``typeof`` (run time)

* Assigning values to identifiers (run time)

* Deleting identifier bindings using ``delete`` (run time)

* Special argument name bindings for the ``arguments`` object (run time)

These have many complications from an implementation point of view:

* Many things are processed during compile time (and are, in fact, required
  to be processed compile time)

* Identifiers may be bound to compiled function registers (preferred) or to
  explicit environment record objects (fallback)

* Register-bound identifiers need to be moved to an explicit environment
  record object if they may be referred to later (closure)

* Establishing execution contexts is rather tricky and varies between call
  types

Specification notes
===================

Key parts
---------

Key parts of Ecmascript E5 discussion identifier handling include Section 10
(Executable Code and Execution Contexts):

* Sections 10.2.2.1 and 10.3.1 cover identifier lookup
  (``GetIdentifierReference``)

* Sections 10.4 and 10.5 cover execution context initialization for global
  code, eval code, and function calls

* Section 10.6 covers the ``arguments`` object initialization (for function
  calls)

Other relevant sections include:

* Section 11.4.1: the ``delete`` operator, which is used for variable
  deletion (and property deletion)

* Section 11.4.3: the ``typeof`` operator

* Section 12.10: the ``with`` statement

* Section 12.14: the ``try`` statement, special handling of ``catch``

* Section 13: function declarations and function expressions

* Section 15.1.2.1: ``eval (x)``, defines the concept of a *direct call
  to eval*

Key concepts
------------

Lexical environment
  Consists of an *environment record* and a reference to an *outer lexical
  environment*.

  The current implementation makes no distinction between a lexical
  environment and the environment record contained in it: they are
  represented by the same object.  Hence, the term "environment record"
  is used loosely below to refer to the lexical environment and/or its
  environment record.

  The *outer lexical environment* is also referred to as the environment
  record's *parent* below.

Environment record
  An environment record is either a *declarative environment record* or an
  *object environment record*.

  Declarative environment records hold variable bindings in an internal map
  not accessible from user code.

  Object environment records hold variable bindings in an Ecmascript object.
  They are used for handling global program execution for ``with`` statements.

Execution context
  Consists of (note the confusing names and terminology here):

  1. ``LexicalEnvironment``, a lexical environment (an environment
     record) for processing variable lookups,

  2. ``VariableEnvironment``, a lexical environment (an environment
     record) for processing variable and function declarations, and

  3. a ``this`` binding

  The two lexical environments are usually the same, except for:

  * ``with``, which establishes an object environment record for variable
    lookups, but keeps the previous one for declarations

  * ``catch``, which establishes a single-variable declarative environment
    record for the variable containing the error

  Note the confusing terminology, especially the use of "lexical environment"
  as a general term and ``LexicalEnvironment`` as a component of an execution
  context.  Below, we refer to these loosely as "lexical environment" (for
  ``LexicalEnvironment``) and "variable environment" (for
  ``VariableEnvironment``).

Immutable binding
  A binding whose value cannot be changed after initialization.
  An immutable binding cannot be deleted.

Mutable binding
  A binding whose value can be changed.  A mutable binding may be deletable
  or non-deletable.

Implicit ``this`` value
  A binding may be associated with an implicit ``this`` value.
  If the value is not provided, ``undefined`` is used in its place.

  Declarative environment records never provide an implicit
  ``this value``.  Object environment records for which the
  ``provideThis`` internal flag is set provide an implicit
  ``this`` value.

Unsorted notes
--------------

Almost all bindings in the E5 specification are non-deletable mutable
bindings, except: [#specialbindings]_

* *declaration in eval*: function and variable declarations in eval code are
  deletable mutable bindings

* *"arguments" binding*: the ``arguments`` binding for strict mode function is
  an immutable binding

* *function name in a function expression*: in a function expression
  ``function f(args) { body }``, the identifier ``f`` is an immutable binding
  for the function body; this is not true of function declarations

There are no practical cases where an immutable binding would be accessed in
an uninitialized state: [#uninitializedimmutable]_

* Immutable bindings are created with ``CreateImmutableBinding`` and
  initialized with ``InitializeImmutableBinding``

* For ``arguments``, creation and initialization are consecutive, there is
  little chance of failure (except "out of memory" like conditions); even
  in failure cases, the intermediate uninitialized binding is not accessible
  to program code

* For function name in a function expression, creation and initialization
  are not consecutive but the same applies as for ``arguments``: an
  uninitialized binding is never exposed to program code (the function
  expression is not "returned" until its initialization is complete)

* As a result, the implementation does not need to handle lookups for
  uninitialized immutable bindings, implied for declarative environment
  records in E5 Section 10.2.1.1.4, step 3

Implicit ``this`` value is relevant only for: [#implicitthis]_

* Object environment records initialized in a ``with`` statement.

* Function call using an identifier reference: a possible implicit
  ``this`` value is used for the ``this`` binding (similarly to
  how a function call using a property reference binds ``this``
  to the property base object).

New declarative environment records are created for: [#newdecrecord]_

* Entering strict eval code (with a direct eval call)

* Entering function code

* Entering ``catch`` clause

* Evaluating a named function expression

New object environment records are created for: [#newobjrecord]_

* Entering ``with`` statement body

* Implicitly for the global object environment record

.. [#specialbindings] See the following E5 sections:

   * Section 10.5 steps 2, 5.d, 8.c (eval code)

   * Section 10.5 step 7.b (``arguments`` binding)

   * Section 13 (function expression)

.. [#uninitializedimmutable] See the following E5 sections:

   * Section 10.5, steps 7.b.i and 7.b.ii (``arguments`` for a strict function)

   * Section 13, named function expression algorithm, steps 3-5 (named
     function expressions)

.. [#implicitthis] See the following E5 sections:

   * Sections 10.2.1.1.6, 10.2.1.2.6: when implicit ``this`` is provided

   * Section 11.2.3 step 6.b: function call handling

   * Sections 10.2.1.2, 12.10: object environment records which have
     ``provideThis`` set (``with`` statement only)

.. [#newdecrecord] See the following E5 sections:

   * Section 10.4.2, step 3: strict eval code

   * Section 10.4.3: function call

   * Section 12.14: ``catch`` clause, step 3

   * Section 13.1: function expression

.. [#newobjrecord] See the following E5 sections:

   * Section 10.2.3: global environment record

   * Section 12.10: ``with`` statement, step 4

Establishing an execution context (= calling)
---------------------------------------------

Different cases for calling:

+------------------------+----------------------+----------------------+-----------------------+-----------------------------+
| Call type              | Lexical environment  | Variable environment | This binding          | Notes                       |
+========================+======================+======================+=======================+=============================+
| Executing strict       | Global environment   | Global environment   | Global object         |                             |
| global code            | (object environment) | (object environment) |                       |                             |
+------------------------+----------------------+----------------------+-----------------------+-----------------------------+
| Executing non-strict   | Global environment   | Global environment   | Global object         |                             |
| global code            | (object environment) | (object environment) |                       |                             |
+------------------------+----------------------+----------------------+-----------------------+-----------------------------+
| Executing strict       | Global environment   | Global environment   | Global object         | Non-direct eval code, or    |
| non-direct eval code   | (object environment) | (object environment) |                       | eval code without a calling |
|                        |                      |                      |                       | context (when in practice?) |
+------------------------+----------------------+----------------------+-----------------------+-----------------------------+
| Executing non-strict   | Global environment   | Global environment   | Global object         | Non-direct eval code, or    |
| non-direct eval code   | (object environment) | (object environment) |                       | eval code without a calling |
|                        |                      |                      |                       | context (when in practice?) |
+------------------------+----------------------+----------------------+-----------------------+-----------------------------+
| Executing strict       | New declarative      | Same as lexical      | Use calling context's | The parent of the new       |
| direct eval code       | environment          | environment          | ``this`` binding      | declarative environment     |
|                        |                      |                      |                       | record is the calling       |
|                        |                      |                      |                       | context's lexical           |
|                        |                      |                      |                       | environment.                |
+------------------------+----------------------+----------------------+-----------------------+-----------------------------+
| Executing non-strict   | Use calling context's| Use calling context's| Use calling context's |                             |
| direct eval code       | lexical environment  | variable environment | ``this`` binding      |                             |
+------------------------+----------------------+----------------------+-----------------------+-----------------------------+
| Executing non-strict   | New declarative      | Same as lexical      | Coerced ``thisArg``:  | The parent of the new       |
| function code          | environment          | environment          |                       | declarative environment     |
|                        |                      |                      | If ``null`` or        | record is the called        |
|                        |                      |                      | ``undefined``, use    | function's ``[[Scope]]``    |
|                        |                      |                      | global object.        | internal property           |
|                        |                      |                      |                       | (see below).                |
|                        |                      |                      | If non-Object, use    |                             |
|                        |                      |                      | ``ToObject``          |                             |
|                        |                      |                      | ``(thisArg)``.        | ``thisArg`` is supplied by  |
|                        |                      |                      |                       | caller in the function call |
|                        |                      |                      | If Object, use        | algorithm.                  |
|                        |                      |                      | ``thisArg`` directly. |                             |
+------------------------+----------------------+----------------------+-----------------------+-----------------------------+
| Executing strict       | New declarative      | Same as lexical      | ``thisArg`` without   | The parent of the new       |
| function code          | environment          | environment          | any coercion          | declarative environment     |
|                        |                      |                      |                       | record is the called        |
|                        |                      |                      |                       | function's ``[[Scope]]``    |
|                        |                      |                      |                       | internal property           |
|                        |                      |                      |                       | (see below).                |
|                        |                      |                      |                       |                             |
|                        |                      |                      |                       | ``thisArg`` is supplied by  |
|                        |                      |                      |                       | caller in the function call |
|                        |                      |                      |                       | algorithm.                  |
+------------------------+----------------------+----------------------+-----------------------+-----------------------------+

Function ``[[Scope]]`` initialization is described in E5 Section 13:

* For function declarations,
  the ``[[Scope]]`` internal property is initialized to the variable environment
  of the execution context containing the declaration.  This is the same as
  initializing ``[[Scope]]`` to the lexical environment of the containing context,
  because function declarations are only allowed at the top level.
  If function declarations are allowed in other places (than the top level), it
  makes more sense to initialize ``[[Scope]]`` to the current lexical environment.

* For anonymous function expressions,
  the ``[[Scope]]`` internal property is initialized to the lexical environment
  of the execution context containing the expression.

* For named function expressions,
  a new declarative environment is created whose parent is the lexical environment
  of the execution context containing the expression.  The new environment will be
  initialized with an immutable binding for the name of the function, and will then
  be used as the ``[[Scope]]`` of the function object.

Environment record implementation
=================================

Specification
-------------

Environment records are described in E5 Section 10.2.1.  They define how
identifiers are bound to values in an Ecmascript execution context.
Bindings can be established in several ways:

#. variable and function declarations in the global scope

#. variable and function declarations in eval code

#. variable and function declarations in functions

#. ``catch`` clauses

#. ``with`` statements

An environment record maintains bindings to a certain set of identifiers,
and may have (at most one) parent record (called *outer environment
reference*) with further bindings.

There are two types of environment records:

#. *declarative environment record*: binds identifiers to values directly

#. *object environment record*: refers to an external Ecmascript object
   which contains the variable bindings

Declarative environment records are the ordinary way of binding identifiers.
The underlying map of identifiers to values is only visible to the program
through variable accesses.  For object environment records, the identifiers
are bound through an ordinary Ecmascript object (referred to by the object
environment record); object environment records are used e.g. in the global
propgram scope and in the ``with`` statement.

An Ecmascript E5 execution environment maintains two environment record
references (E5 Section 10.3):

* A *variable environment* refers to an environment record which is used
  for function and variable declarations

* A *lexical environment* refers to an environment record which is used
  for variable lookups (including deletes)

Very often these two refer to the same environment record.  A new, nested
lexical environment can be created e.g. through a ``with``, a ``catch``,
a nested function, etc.

Internal representation
-----------------------

The current implementation makes no distinction between a lexical
environment and the environment record it refers to.  Instead, a combination
of a lexical environment, its environment record, and a possible outer
lexical environment reference is implemented as a single internal object
which is not directly visible to program code.

An outer lexical environment reference (the "parent record") is
identified using the internal prototype field of the object.
This does *not* mean that ordinary property lookups (with automatic
prototype walking) are used for variable lookups: the internal prototype
is simply a convenient place to maintain the parent relationship, having
an easily accessible slot, being already visible to garbage collection etc.

Declarative environment record
------------------------------

A *declarative environment record* object maintains identifier mappings
in two ways:

#. directly as properties of the environment record

#. by identifying an active activation record ("open scope") and a set
   of identifiers bound to the registers of that activation record

In the latter case, the environment record identifies the thread, the
function, and the "register base" of the activation record in the thread's
value stack.  These allow an identifier to be mapped to a value stack
entry as follows:

#. The function's variable map (if it exists) is consulted to find the
   identifiers register number, relative to the bottom of the activation.
   (Of course, the identifier might not be mapped.)

#. The "register base" of the environment record identifies the absolute
   value stack index of the activation record's frame bottom, and is added
   to the relative register number to get an absolute value stack index.

Note that the actual activation record does not need to be identified,
we just need the register base to resolve absolute value stack index of
a variable.  However, this approach does not allow validation of the
resulting value stack index (e.g. to verify that it is indeed inside the
activation record's frame).

The internal object is initialized with:

* Object class set to ``DUK_HOBJECT_CLASS_DECENV``

* Object flag ``DUK_HOBJECT_FLAG_ENVRECCLOSED`` cleared (assuming the
  environment is open)

* Internal prototype referring to outer environment record

* Internal control properties: ``_Callee``, ``_Thread``, ``_Regbase``

When a declarative environment is "closed", identifiers bound to
activation registers are copied  to the internal environment record
object as plain properties (with the help of the callee's ``_Varmap``)
and the environment record's internal control properties are deleted.
The flag ``DUK_HOBJECT_FLAG_ENVRECCLOSED`` is set to allow open scope
lookups to be skipped in later lookups.

The variables mapped as properties have their attributes set as follows:

* ``[[Enumerable]]``: does not matter as the properties are not visible
  to the program (currently set to ``true``)

* ``[[Writable]]``: set to ``true`` for mutable bindings, ``false`` for
  immutable bindings

* ``[[Configurable]]``: set to ``true`` for mutable bindings which are
  also deletable, ``false`` for non-deletable mutable bindings and
  immutable bindings

Register-bound identifiers are assumed to be non-deletable mutable
bindings: register bindings cannot be easily deleted (the bytecode
may refer to them directly), and protecting register bound identifiers
from being modified would require some control information we don't
have.  As a result, other types of bindings cannot be mapped to registers
(i.e. declarations inside eval, ``arguments`` binding, and the name
binding of a named function expression) and require an explicit environment
record object.

Object environment record
-------------------------

An *object environment record* refers to an external Ecmascript object
(visible to the program) "backing" the identifier bindings.  The target
object is identified with an internal control property.

The internal object is initialized with:

* Object class set to ``DUK_HOBJECT_CLASS_OBJENV``

* Internal prototype referring to outer environment record

* Internal control property: ``_Target``

Identifier lookups proceed to the ``_Target`` object while the parent
environment record is identified by the prototype of the environment
record (not of ``_Target``).

Example internal environment record objects
===========================================

Let's consider an environment with:

#. the global object (outermost)

#. a declarative environment (of a function, scope is open)

#. an object environment (e.g. of a ``with`` clause)

This could happen in this example::

  // global scope: a variable lookup here would use 'record1'

  var with_object = { "xyz": "zy" };

  function func() {
    var foo = 'my foo';
    var bar = 'my bar';

    // in the example below, we assume this variable was not mapped
    // to a register for some reason (wouldn't ordinarily happen)
    var quux = "a non-register binding";

    // variable lookup here would use 'record2'

    with (with_object) {
      // variable lookup here would use 'record3'
    }
  }

The objects could be roughly as follows (leading underscore indicates
an internal value not visible to the program, ``__prototype`` denotes
internal prototype)::

  global_object = {
    "NaN": NaN,
    "Infinity": Infinity,
    "undefined": undefined,
    "eval": [function],
    "parseInt": [function],
    ...
  }

  func = {
    // "foo" maps to reg 0, "bar" to reg 1, "quux" not register mapped
    "_Varmap": { "foo": 0, "bar": 1 },
    ...
  }

  with_object = {
    "xyz": "zy"
  }

  record1 = {
    // Flag DUK_HOBJECT_CLASS_OBJENV set
    "__prototype": null,
    "_Target": global_object,    // identifies binding target
  }

  record2 = {
    // Flag DUK_HOBJECT_CLASS_DECENV set
    // Flag DUK_HOBJECT_CLASS_ENVRECCLOSED not set (still open)
    "__prototype": record1,
    "_Callee": func,     // provides access to _Varmap (name-to-reg)
    "_Thread": thread,   // identifies valstack
    "_Regbase": 100,     // identifies valstack base for regs
    "quux": "a non-register binding"

    // var "foo" resides in value stack absolute index 100 + 0 = 100,
    // var "bar" in absolute index 100 + 1 = 101
  }

  record3 = {
    // Flag DUK_HOBJECT_CLASS_OBJENV set
    "__prototype": record2,
    "_Target": with_object
  }

Once again, the compiler strives to avoid creating explicit environment
records whenever possible.  In the example above the compiler won't be
successful: the object environment required by ``with`` also causes the
declarative environment (record2) to be created.

Functions and environment records
---------------------------------

A function object (``duk_hcompiledfunction``) records the following
conceptual variable access related information:

* A variable map, which maps an identifier to a register number.  Ideally
  all function variables / functions are register mapped, but this is not
  always possible.

* Control data for initializing the lexical and variable environments
  of new invocations.

There are separate function template objects (which are essentially functions
without an outer environment) and concrete function instance objects.  The
detailed properties vary a bit between the two.

More concretely:

* The ``DUK_HOBJECT_FLAG_NEWENV`` object level flag, and the internal
  properties ``_Lexenv`` and ``_Varenv`` control activation record
  lexical and variable environment initialization as described below.

* The internal property ``_Varmap`` contains a mapping from an
  identifier name to a register number relative to the activation
  record frame bottom.

* The internal property ``_Formals`` contains a list of formal argument
  names.

* Template function objects, used for creating concrete function instances,
  use ``DUK_HOBJECT_FLAG_NAMEBINDING`` flag to indicate that the template
  represents a named function expression.  For such functions, the function
  name (stored in ``name``) needs to be bound in an environment record just
  outside a function activation's environment record.

To minimize book-keeping in common cases, the following short cuts
are supported:

* If both scope references are missing:

  + Assume that the function has an empty declarative environment record,
    whose parent is the global environment record.

  + For variable lookups this means that we proceed directly to the global
    environment record.

  + For variable declarations this means that a declarative environment
    record needs to be created on demand.

* If ``_Varenv`` is missing:

  + Assume that ``_Varenv`` has the same value as ``_Lexenv``.

  + This is very common, and saves one (unnecessary) reference.

  + Note: it would be more logical to allow ``_Lexenv`` to be missing
    and default it to ``_Varenv``; however, dynamic variable
    declarations are comparatively rare so the defaulting is more
    useful this way around

* If ``_Varmap`` is missing:

  + Assume that the function has no register-mapped variables.

* The compiler attempts to drop any fields not required from compiled
  objects.  In many common cases (even when dynamic variables accesses
  cannot be ruled out) no control fields are required.

The detailed handling of these is documented as part of the source for:

* Creating function templates in the compiler

* Instantiating function instances from templates (closures)

Notes:

* Environment record initialization is done only when (if) it is actually
  needed (e.g. for a function declaration).  It is not created
  unnecessarily when a function is called.

* The default behavior for ``_Lexenv`` and ``_Varenv`` allows them to
  be omitted in a large number of cases (for instance, many functions
  are declared in the global scope, and for many compiled eval
  functions the values are the same).

* The ``DUK_HOBJECT_FLAG_NEWENV`` is set for ordinary functions, which
  always get a new environment record for variable declaration and
  lookup.  The flag is cleared for global code and eval code; or rather
  functions compiled from global code and eval code.

* Unlike Duktape, Ecmascript does not separate the compilation and
  execution of global code and eval code.  Hence the handling is
  seemingly a bit different although the outcome should be the same.

Preliminary work
================

GETIDREF
--------

The ``GetIdentifierReference`` specification algorithm walks the chain of
environment records looking for a matching identifier binding.  It returns
a "reference" value which can act as both a left-hand-side and a
right-hand-side value.  The reference identifies (1) an environment record,
(2) an identifier name, and (3) a strictness flag.

When using the reference as a left-hand-side value, the environment record
is written to.  For declarative records, this is a write to internal object
(an internal ``duk_hobject`` in Duktape) or a value stack entry (a ``duk_tval``
for register bound identifiers).  For object environment records, this is a
property write (a ``[[Put]]`` call) to a user visible object, possibly invoking
a setter call.

When using the reference as a right-hand-side value, the environment record
is read from.  For declarative records, this is a read from an internal
object or a value stack entry (for register bound identifiers).  For object
environment records, this is a property read (a ``[[Get]]`` call), possibly
invoking a getter call.

Note that the storage location of any identifier value is conceptually an
external or an internal object.  However, the concrete implementation is a
bit more complicated: an identifier value may be either stored in an object
or in an activation's register file, i.e., a particular entry in the value
stack of some thread.  This needs to be taken into account when representing
a "reference" type internally.

A close analog of ``GetIdentifierReference`` is currently implemented as a
helper function (referred to as GETIDREF in this document).  GETIDREF returns
multiple values which the caller can use to implement the actual operation
(``GETVAR``, ``PUTVAR``, ``DELVAR``, ``HASVAR``).  The values returned include:

* ``result``: ``true`` if binding found, ``false`` otherwise.  If ``false``,
  other values are undefined.

* ``holder``: a pointer to the "holder" object, the internal or external
  object storing the binding value.  For register-bound identifiers, this
  is NULL.

* ``value``: an ``duk_tval`` pointer to the current value for register
  bindings, points to a value stored in a value stack.  For declarative
  environment records, ``value`` points to the ``duk_tval`` property
  entry of the internal object.  For object environment records, this
  is NULL.

* ``attrs``: property attributes of ``value`` (if ``value`` is NULL, this
  field is not needed).  Attributes are needed in PUTVAR: before updating
  a value in-place using a direct ``duk_tval`` write, we need to know that
  the value is writable.  Register bound variables are always writable
  (mutable), denoted "W" in the table below.

* ``this_binding``: an ``duk_tval`` pointer to the ``this`` binding related
  to the reference, points to a value stored in an object.

* ``env``: a pointer to the lexical environment record (an ``duk_hobject``)
  where the binding was found.  For register-bound identifiers, this is NULL.

The following table clarifies the return values in different cases:

+-------------------------+--------+--------+---------+-------+--------------+------+
| Case                    | result | holder | value   | attrs | this_binding | env  |
+=========================+========+========+=========+=======+==============+======+
| Delayed declarative     | true   | NULL   | points  | W     | NULL         | NULL |
| environment, bound in   |        |        | to      |       |              |      |
| register of current     |        |        | valstack|       |              |      |
| activation              |        |        |         |       |              |      |
+-------------------------+--------+--------+---------+-------+--------------+------+
| Declarative environment,| true   | NULL   | points  | W     | NULL         | env  |
| bound in register of    |        |        | to      |       |              |      |
| open environment record |        |        | valstack|       |              |      |
|                         |        |        |         |       |              |      |
+-------------------------+--------+--------+---------+-------+--------------+------+
| Declarative environment,| true   | env    | points  | from  | NULL         | env  |
| bound in (open or       |        |        | to prop | prop  |              |      |
| closed) environment     |        |        | storage |       |              |      |
| record object           |        |        |         |       |              |      |
+-------------------------+--------+--------+---------+-------+--------------+------+
| Object environment,     | true   | target | NULL    | n/a   | NULL         | env  |
| bound in target object, |        |        |         |       |              |      |
| no "this binding"       |        |        |         |       |              |      |
+-------------------------+--------+--------+---------+-------+--------------+------+
| Object environment,     | true   | target | NULL    | n/a   | target       | env  |
| bound in target object, |        |        |         |       |              |      |
| has "this binding"      |        |        |         |       |              |      |
+-------------------------+--------+--------+---------+-------+--------------+------+
| Not found               | false  | n/a    | n/a     | n/a   | n/a          | n/a  |
+-------------------------+--------+--------+---------+-------+--------------+------+

The object environment records created by the ``with`` statement provide
a "this binding" (``provideThis`` is true, see E5 Section 12.10); other
object environment records do not.  The "this binding" only affects
function calls made through bound identifiers, e.g. as in::

  var foo = {
    bar: function() { print("" + this); },
    toString: function() { print("i'm foo"); }
  }

  with (foo) {
    // prints "i'm foo", similar to being called
    // like: foo.bar()
    bar();
  }

Original algorithm
::::::::::::::::::

The original ``GetIdentifierReference`` is described in E5 Section 10.2.2.1.
The inputs are: lexical environment ``lex``, identifier string ``name``,
and a ``strict`` flag:

1. If ``lex`` is the value ``null``, then

   a. Return a value of type Reference whose base value is ``undefined``, whose
      referenced name is ``name``, and whose strict mode flag is ``strict``.

2. Let ``envRec`` be ``lex``\ ‘s environment record.

3. Let ``exists`` be the result of calling the ``HasBinding(N)`` concrete
   method of ``envRec`` passing ``name`` as the argument ``N``.

4. If ``exists`` is ``true``, then

   a. Return a value of type Reference whose base value is ``envRec``, whose
      referenced name is ``name``, and whose strict mode flag is ``strict``.

5. Else

   a. Let ``outer`` be the value of ``lex``\ ’s outer environment reference.

   b. Return the result of calling ``GetIdentifierReference`` passing
      ``outer``, ``name``, and ``strict`` as arguments.

Notes:

* The algorithm supports the case where the starting lexical environment is
  ``null``, although step 1 is more likely intended to just be the recursion
  terminator.

* The recursion walks the ``outer`` reference chain, which our implementation
  handles through internal prototypes of the environment records.

Eliminating recursion
:::::::::::::::::::::

1. **NEXT:**
   If ``lex`` is the value ``null``, then:

   a. Return a value of type Reference whose base value is ``undefined``, whose
      referenced name is ``name``, and whose strict mode flag is ``strict``.

2. Let ``envRec`` be ``lex``\ ‘s environment record.

3. Let ``exists`` be the result of calling the ``HasBinding(N)`` concrete
   method of ``envRec`` passing ``name`` as the argument ``N``.

4. If ``exists`` is ``true``, then

   a. Return a value of type Reference whose base value is ``envRec``, whose
      referenced name is ``name``, and whose strict mode flag is ``strict``.

5. Let ``lex`` be the value of ``lex``\ 's outer environment reference.

6. Goto NEXT.

Concrete implementation
:::::::::::::::::::::::

A few notes first:

* The implementation delays the creation of an explicit declarative
  environment record when possible.  In this case the initial ``lex``
  value is ``NULL`` and should be treated like an empty declarative
  environment record with a certain outer reference, and possibly a
  set of identifiers bound to registers.  To do this, we need a
  reference to the current activation (``act`` below).

* Some callers require a variant which does not follow the outer
  environment reference chain.  The algorithm incorporates a flag
  ``parents`` controlling this (if true, parent chain is followed).

First draft:

1. If ``lex`` is ``null`` and ``act`` is defined then
   (delayed declarative environment record):

   a. Check whether ``name`` is bound to a register of ``act``.
      To do this, the function object needs to be looked up based on
      ``act``, and the function metadata be consulted; in particular,
      the ``_Varmap`` internal property (which maps names to register
      numbers) is used.

   b. If ``name`` is mapped, return the following:

      * Result: ``true``

      * Value pointer: point to register storage

      * Attributes: writable

      * This pointer: NULL

      * Environment pointer: NULL

      * Holder pointer: NULL

   c. If ``parents`` is ``false``, goto NOTFOUND.

   d. Else, let ``lex`` be the outer environment record that a
      declarative environment record created for ``act`` would
      have.  This is concretetely looked up from the ``_Lexenv``
      internal property of the function related to ``act``.

2. **NEXT:**
   If ``lex`` is the value ``null``, then goto NOTFOUND.

3. If ``lex`` is a declarative environment record, then:

   a. If ``lex`` is *open* (activation registers are still in use):

      1. Check whether ``name`` is mapped to a register of the activation
         related to the environment record.  These are concretely looked
         up using internal properties of ``lex``.  (Note that the related
         activation may be any function, and even that of another thread.)

      2. If so, return the following values (value pointer can always be
         given, and the caller is always allowed to modify the value in-place,
         because all register bindings are mutable):

         * Result: ``true``

         * Value pointer: point to register storage

         * This pointer: NULL

         * Environment pointer: ``lex``

         * Holder pointer: NULL

   b. If ``lex`` has a property named ``name``, return the following values:

      * Result: ``true``

      * Value pointer: point to storage location of property in ``lex``

      * Attributes: from ``lex`` property (non-writable for immutable
        bindings, writable for others)

      * This pointer: NULL

      * Environment pointer: ``lex``

      * Holder pointer: ``lex``

4. Else ``lex`` is an object environment record:

   a. Let ``target`` be the binding object for ``lex``.
      (Note: this is always defined, and an object.)

   b. If the result of calling ``[[HasProperty]]`` for ``target`` with the
      property name ``name`` is ``true``:

      1. If ``lex`` has the internal property ``_This``, set ``thisBinding``
         to its value.  Else set ``thisBinding`` to ``NULL``.

      2. Return the following values:

         * Result: ``true``

         * Value pointer: NULL

         * Attributes: arbitrary (use zero)

         * This pointer: ``thisBinding``

         * Environment pointer: ``lex``

         * Holder pointer: ``target``

5. If ``parents`` is ``false``, goto NOTFOUND.

6. Let ``lex`` be the internal prototype of ``lex``.

7. Goto NEXT.

8. **NOTFOUND:**
   Return the following:

   * Result: ``false``

   * Value pointer: NULL

   * Attributes: arbitrary (use zero)

   * This pointer: NULL

   * Environment pointer: NULL

   * Holder pointer: NULL

HASVAR: check existence of identifier
=====================================

Unlike e.g. GETVAR, HASVAR does not traverse the environment record outer
reference chain.  HASVAR is also not really an exposed primitive; Ecmascript
code cannot access it directly.  It is used internally for function call
handling, and can also be used from the C API.

Using GETIDREF:

1. Let ``res`` be the result of calling ``GETIDREF`` with the arguments
   ``env``, ``name``, and ``parents`` set to ``false``.

2. Return the "result" component of ``res``.

GETVAR: read identifier value
=============================

Conceptual steps:

* Identifier resolution (E5 Section 10.3.1) is used to resolve identifier
  references.

* Identifier resolution calls ``GetIdentifierReference`` (E5 Section
  10.2.2.1), which returns a Reference type.

* A right-hand-side expression "coerces" the Reference to a value using
  ``GetValue`` (E5 Section 8.7.1).

In the optimal case, all of these can be resolved at compile time, converting
the identifier read into a register lookup.  No explicit run-time processing
happens in this case.

In other cases the compiler emits a ``GETVAR`` instruction which performs the
necessary (slow) steps at run time.  The identifier name (a string) is always
known at compile time as there is no indirect variable lookup; an ``eval``
call might look like one, but any identifier reference has a string name when
compiling the ``eval`` argument string, e.g. as in::

  function f() {
    return eval("return foo;);
  }

The run time part begins with, essentially, ``GetIdentifierReference`` which
is given a lexical environment ``env``, an identifier name ``name``, and a
``strict`` flag which depends on the function containing the expression.

The GETVAR primitive differs from a plain identifier lookup in that it also
returns the "this binding" related to the identifier, if defined.

Let's look at ``GetValue`` first.

GetValue
--------

GetValue simplifies to (here, ``V`` is the Reference):

1. Let ``base`` be the result of calling ``GetBase(V)`` (which must be an
   environment record).

2. If ``IsUnresolvableReference(V)``, throw a ``ReferenceError`` exception.

3. Return the result of calling the ``GetBindingValue`` (see 10.2.1)
   concrete method of ``base`` passing ``GetReferencedName(V)`` and
   ``IsStrictReference(V)`` as arguments.

Inlining the ``GetBindingValue`` calls (E5 Sections 10.2.1.1.4 and
10.2.1.2.4):

1. Let ``base`` be the result of calling ``GetBase(V)`` (which must be an
   environment record).

2. If ``IsUnresolvableReference(V)``, throw a ``ReferenceError`` exception.
   (Note: this is unconditional.)

3. If ``base`` is a declarative environment record, then:

   a. If the binding for ``name`` is an uninitialized immutable binding,
      then:

      1. If ``strict`` is ``true``, then throw a ``ReferenceError`` exception.

      2. Else, return ``undefined``.

   b. Return the value currently bound to ``name`` in ``base``.
      (Note: the value must exist, because ``IsUnresolvableReference()``
      checks that it does.)

4. Else ``base`` must be an object environment record and:

   a. Let ``bindings`` be the bindings object for ``base``.

   b. Let ``value`` be the result of calling the ``[[HasProperty]]``
      internal method of ``bindings``, passing ``name`` as the property
      name.

   c. If ``value`` is ``false``, then:

      1. If ``strict`` is ``true``, then throw a ``ReferenceError`` exception.

      2. Else, return ``undefined``.

   d. Return the result of calling the ``[[Get]]`` internal method of
      ``bindings``, passing ``name`` for the argument.
      (Note: this may invoke an accessor.)

Reworking a bit to eliminate duplication of ``ReferenceError`` throwing,
and cleaning up:

1. Let ``base`` be the result of calling ``GetBase(V)`` (which must be an
   environment record).

2. If ``IsUnresolvableReference(V)``, throw a ``ReferenceError`` exception.
   (Note: this is unconditional.)

3. If ``base`` is a declarative environment record, then:

   a. If the binding for ``name`` is an uninitialized immutable binding,
      then goto NOTFOUND.

   b. Return the value currently bound to ``name`` in ``base``.
      (Note: the value must exist, because ``IsUnresolvableReference()``
      checks that it does.)

4. Else ``base`` must be an object environment record and:

   a. Let ``bindings`` be the bindings object for ``base``.

   b. If the result of calling the ``[[HasProperty]]`` internal method of
      ``bindings``, passing ``name`` as the property name, is ``false``,
      then goto NOTFOUND.

   c. Return the result of calling the ``[[Get]]`` internal method of
      ``bindings``, passing ``name`` for the argument.
      (Note: this may invoke an accessor.)

5. **NOTFOUND:**

   a. If ``strict`` is ``true``, then throw a ``ReferenceError`` exception.

   b. Else, return ``undefined``.

Notes:

* Step 3.a: uninitialized immutable bindings don't occur when running
  user code, they only exist temporarily in the specification algorithms.

* Step 4.c: it is important to note that getting a value from an object
  environment record accesses a user visible property, and may lead to
  an accessor call.  The accessor can have arbitrary side effects, such
  as:

  + Modifying arbitrary objects, even the binding object itself.

  + Causing a garbage collection, and resizing and reallocation of any
    object's property allocation or any valstack.  This may invalidate
    *any* existing ``duk_tval`` pointers to such structures (but not
    any "plain" heap object pointers, such as pointers to strings and
    objects).

Using GETIDREF
--------------

Arguments are environment record ``env``, and identifier name ``name``.
The return value is a pair (value, this_binding).

1. Let ``res`` be the result of calling ``GETIDREF`` with the arguments
   ``env``, ``name``, and ``parents`` set to ``true``.

2. If ``res.result`` is ``false``, throw a ``ReferenceError``.
   (Note: this is unconditional.)

3. If ``res.value`` is not NULL (identifier bound to a register in a
   declarative environment record) then:

   a. Return ``res.value`` and ``undefined``.

4. Else ``res.holder`` must not be NULL (identifier bound to a declarative
   environment record or an object environment record target object):

   a. Let ``this`` be ``ref.this_binding``.

   b. Let ``val`` be the result of calling ``[[Get]]`` on ``res.holder`` with
      the property name ``name``.

   c. Return ``val`` and ``this``.

Notes:

* In step 4, note that the ``[[Get]]`` call may invoke a getter and may
  thus have an arbitrary number of side effects, including resizing of the
  property allocation of any object and any valstack.  Any existing
  ``duk_tval`` pointers may be invalidated.  This is why step 4.a should
  conceptually happen first.

Handling of ``typeof`` for an unresolvable identifier
-----------------------------------------------------

The ``typeof`` operator needs slightly different behavior to the above
algorithm for unresolvable references.  Instead of throwing a ``ReferenceError``
``typeof`` returns ``undefined`` for an unresolvable reference.

Another alternative would be to use HASVAR first and then (depending on
the result) use GETVAR.

PUTVAR: write identifier value
==============================

* ``GetIdentifierReference``

* ``PutValue``

Note: the E5 specification prohibits a binding or assignment to an
identifier named ``eval`` or ``arguments`` in strict mode.  This is
actually prevented during compilation, and causes a compile time
``SyntaxError``:

* E5 Section 11.13: single or compound assignment

* E5 Section 12.2.1: variable or function declaration in a function body

* E5 Section 13.1: function argument name

* E5 Section 12.14.1: ``catch`` clause variable name

As a result, there is no need to check for this at run time when
assigning values to variables (either in actual program code, or
in bytecode prologue initializing function bindings).  The implementation
does assert for this condition though.

Let's look at ``PutValue`` first.

PutValue
--------

PutValue simplifies to (here, ``V`` is the Reference and ``W`` is the value):

1. Let ``base`` be the result of calling ``GetBase(V)`` (which must be an
   environment record).

2. If ``IsUnresolvableReference(V)``, then:

   a. If ``IsStrictReference(V)`` is ``true`` then throw a
      ``ReferenceError`` exception.

   b. Call the ``[[Put]]`` internal method of the global object, passing
      ``GetReferencedName(V)`` for the property name, ``W`` for the value,
      and ``false`` for the ``Throw`` flag.

3. Call the ``SetMutableBinding`` (10.2.1) concrete method of ``base``,
   passing ``GetReferencedName(V)``, ``W``, and ``IsStrictReference(V)``
   as arguments.

4. Return.

Inlining the ``SetMutableBinding`` calls (E5 Sections 10.2.1.1.3 and
10.2.1.2.3):

1. Let ``base`` be the result of calling ``GetBase(V)`` (which must be an
   environment record).

2. If ``IsUnresolvableReference(V)``, then:

   a. If ``IsStrictReference(V)`` is ``true`` then throw a
      ``ReferenceError`` exception.

   b. Call the ``[[Put]]`` internal method of the global object, passing
      ``GetReferencedName(V)`` for the property name, ``W`` for the value,
      and ``false`` for the ``Throw`` flag.

3. If ``base`` is a declarative environment record, then:

   a. If the binding for ``GetReferencedName(V)`` in ``base`` is a mutable
      binding, change its bound value to ``W``.

   b. Else this must be an attempt to change the value of an immutable
      binding so throw a ``TypeError`` exception.

4. Else ``base`` must be an object environment record and:

   a. Let ``bindings`` be the binding object for ``base``.

   b. Call the ``[[Put]]`` internal method of ``bindings`` with
      arguments ``GetReferencedName(V)``, ``W``, and ``IsStrictReference(V)``.
      (Note: this may invoke an accessor.)

4. Return.

Notes:

* Step 4.c may have a wide variety of side effects including resizing any
  object property allocation or valstack.

Using GETIDREF
--------------

Arguments are environment record ``env``, identifier name ``name``, new
identifier value ``val``, and a ``strict`` flag indicating whether the
code executing a PUTVAR is strict.

1. Let ``res`` be the result of calling ``GETIDREF`` with the arguments
   ``env``, ``name``, and ``parents`` set to ``true``.

2. If ``res.result`` is ``false``:

   a. If ``strict`` is ``true``, throw a ``ReferenceError``.

   b. Call the ``[[Put]]`` internal method of the global object, passing
      ``name`` for the property name, ``val`` for the value,
      and ``false`` for the ``Throw`` flag.

3. If ``res.value`` is not NULL (identifier bound to a register, or to
   a property in a declarative environment record) and ``res.attrs``
   indicates value is writable, then:

   a. Write ``val`` to the target of the pointer ``res.value``.
      (Identifier bound to a register in a declarative environment record.)

   b. Return.

4. Else ``res.holder`` must not be NULL.  Identifier is bound to a declarative
   environment record (an immutable binding) or an object environment record
   target object:

   a. Call the ``[[Put]]`` internal method of ``res.holder`` with
      arguments ``name``, ``val``, and ``strict``.
      (Note: this may invoke an accessor.)

   b. Return.

Notes:

* In step 4, note that the ``[[Put]]`` call may invoke a setter and may
  thus have an arbitrary number of side effects, including resizing of the
  property allocation of any object and any valstack.  Any existing
  ``duk_tval`` pointers may be invalidated.

DELVAR: delete identifier
=========================

* ``GetIdentifierReference``

* ``delete`` operator applied to an identifier (not a property)

* ``DeleteBinding``

The deletion process locates the nearest declaration and deletes that (if possible).
There may be on outer declaration which is still in effect.  For instance (Rhino)::

  js> var a = 10;
  js> function f() {
  ...     eval("var a = 20; print(a); " +
  ...          "print(delete a); print(a); " +
  ...          "print(delete a)"); };
  js> f()
  20
  true
  10
  false

The innermost binding is an established by eval into an empty declarative environment
of the function.  The declaration succeeds and creates a deletable, mutable binding,
which is then printed and successfully deleted.  The global binding is still visible,
but it is a non-deletable, mutable binding, so the delete fails.  Multiple levels of
deletable bindings for the same identifier are thus possible, and ``delete`` will
always try to delete the one that is currently visible.

The delete operator
-------------------

The ``delete`` operator is defined in E5 Section 11.4.1.  A few notes:

* In non-strict mode, deletion of an unresolvable identifier succeeds
  silently (step 3.b), e.g.::

    function f() { return delete foo; }
    print(f());  // prints true

* In non-strict mode, deletion of a resolvable but undeletable binding
  succeeds with ``false``::

    // 'x' is a non-deletable mutable binding
    function f() {
        var x = 1;
        print(delete x);
    }

    // -> false
    f();

* In non-strict mode, deletion of a resolvable and deletable binding
  succeeds with ``true``::

    foo = 1;  // establishes 'foo' into global object

    // -> {"value":1,"writable":true,
    //     "enumerable":true,"configurable":true}
    print(JSON.stringify(this, 'foo'));

    // -> true
    print(delete foo);

    // -> empty
    print(JSON.stringify(this, 'foo'));

* In strict mode, any attempt to delete an identifier (resolvable or
  not) is always a *compile time* ``SyntaxError``, see steps 3.a and 5.a.

  + Example 1::

      // SyntaxError (compile time)
      function f() {
          'use strict';
          delete foo;  // unresolvable
      }

  + Example 2::

      // SyntaxError (compile time)
      foo = 1;
      function f() {
          'use strict';
          delete foo;  // resolves, still a SyntaxError
      }

  + Example 3 (applies even to object bindings)::

      foo = { bar: 1};

      with (foo) {
          var f = function() {
              'use strict';
              delete bar;  // resolves, still a SyntaxError
          }
      }

Note that *all* ``SyntaxError`` exceptions must be thrown at compile
time (E5 Section 16).  So, any run time attempts to delete identifiers
with a DELVAR operation *must* happen from non-strict code.

With this in mind, the *run time part* of ``delete`` operator (i.e.,
the DELVAR primitive) only executes in non-strict code, and for a
reference ``ref`` becomes:

1. If ``IsUnresolvableReference(ref)`` then return ``true``.

2. Else ``ref`` is a reference to an environment record binding; let
   ``bindings`` be ``GetBase(ref)``.

3. Return the result of calling the ``DeleteBinding`` concrete method
   of ``bindings``, providing ``GetReferencedName(ref)`` as the
   argument.

Inlining the concrete ``DeleteBinding`` algorithms (E5 Sections
10.2.1.1.5 and 10.2.1.2.5), and renaming ``ref`` to ``V`` and
``bindings`` to ``base`` to match the GETVAR and PUTVAR algorithms:

1. If ``IsUnresolvableReference(V)`` then return ``true``.

2. Else ``V`` is a reference to an environment record binding; let
   ``base`` be ``GetBase(V)``.

3. If ``base`` is a declarative environment record, then:

   a. If ``base`` does not have a binding for the name
      ``GetReferencedName(V)``, return ``true``.

   b. If the binding for ``GetReferencedName(V)`` in ``base`` cannot
      be deleted, return ``false``.

   c. Remove the binding for ``GetReferencedName(V)`` from ``base``.

   d. Return ``true``.

4. Else ``base`` must be an object environment record and:

   a. Let ``bindings`` be the binding (target) object for ``base``.

   b. Return the result of calling ``[[Delete]]`` internal method of
      ``bindings``, passing ``GetReferencedName(V)`` and ``false``
      arguments.

Notes:

* In step 4.b: ``[[Delete]]`` returns ``true`` if the own property either
  does not exist, or the property exists and is deletable.  ``false``
  is only returned if a non-configurable own property exists.  This matches
  the behavior for identifiers in declarative environment records.

Using GETIDREF
--------------

Arguments are environment record ``env``, identifier name ``name``, new
identifier value ``val``, and a ``strict`` flag indicating whether the
code executing a DELVAR is strict (the ``strict`` flag is always ``false``,
though).

1. Let ``res`` be the result of calling ``GETIDREF`` with the arguments
   ``env``, ``name``, and ``parents`` set to ``true``.

2. If ``res.result`` is ``false``:

   a. Return ``true``.

3. If ``res.value`` is not NULL (identifier bound to a register, or a
   property in a declarative environment record) and ``res.attrs``
   indicates value is non-configurable, then:

   a. Return ``false``.
      (Note: register-bound identifiers are not deletable.)

4. Else ``res.holder`` must not be NULL (identifier bound to a declarative
   environment record or an object environment record target object):

   a. Call the ``[[Delete]]`` internal method of ``res.holder`` with
      arguments ``name`` and ``false``.

   b. Return.

Notes:

* The compiler should never emit a DELVAR for strict code, and the bytecode
  executor should refuse to execute such an instruction for strict code.
  There is no explicit check in the algorithm.

* Step 4.a covers two cases:

  1. An identifier bound to a declarative environment record.  In this
     case ``res.holder`` is the internal environment record, and the
     property delete follows normal ``[[Delete]]`` behavior.  In particular,
     ``[[Delete]]`` only returns ``false`` if the property exists and is
     not configurable.  Otherwise ``[[Delete]]`` returns ``true``.  This
     matches the desired behavior for declarative environment records (see
     the abstract, inlined version of the algorithm).

  2. An identifier bound to an object environment record.  In this case
     ``res.holder`` is the target object, and the ``[[Delete]]`` call is
     the desired behavior.

DECLVAR: declare identifier
===========================

Background
----------

Bindings are created with ``CreateMutableBinding`` and
``CreateImmutableBinding`` in E5 Section 10.5 (Declaration Binding
Instantiation) and Section 12.14 (``catch`` clause).

``CreateMutableBinding`` and ``CreateImmutableBinding`` both assume that
they are never called if the binding has already been declared.  The
algorithms establishing new bindings carefully use ``HasBinding`` to avoid
duplicate declaration attempts (see "declaration binding instantiation"
for instance).

Declarations always go to a specified environment record; its outer
environment (parent) is not checked or affected.  Thus, a variable name
can be re-declared if it exists in an outer context, e.g.::

  var a = 10;
  function f() {
    // new declarative environment
    var a = 20;
  }

More specifically, new bindings are created in the following places (the
method of preventing a duplicate declaration is in parentheses):

* Section 10.5 step 4.d.iii - 4.d.iv: argument binding
  (checks for existing binding)

* Section 10.5 step 5.c - 5.d: function declaration
  (checks for existing binding; special handling for re-declarations of
  global functions added in E5.1)

* Section 10.5 step 6, 7.b.i, and 7.c.i: ``arguments`` binding
  (checks for existing binding)

* Section 10.5 step 8.b - 8.c: variable declaration
  (checks for existing binding)

* Section 12.14 ``catch`` step 4: ``catch`` variable binding
  (new environment, no need to check)

The DECLVAR algorithm can ignore attempts to re-declare a variable, except
that the re-declaration of global functions has special handling in E5.1.

Note that unlike GETVAR, PUTVAR, and DELVAR, DECLVAR has no "baseline"
algorithm in the E5 specification.  Rather, it is a primitive which is
useful internally, and needs to match the scattered variable declaration
needs identified above.

Also note that all non-register-bound identifiers are stored as object
properties (either of an internal or an external object).  Hence, DECLVAR
ultimately adds or updates a property of some holder object.

Algorithm
---------

Inputs:

* environment record ``env``

* variable name ``name``

* initial value ``val``

* property attributes ``attrs``, which allow the caller to control whether
  the binding is deletable (``[[Configurable]]`` attribute) and mutable
  (``[[Writable]]`` attribute)

* flag ``is_func_decl``, which indicates whether the binding being declared
  is a function declaration; this has no other effect than to control the
  special global function re-declaration behavior of E5.1

Outputs:

* none

Algorithm:

1. Let ``res`` be the result of calling ``GETIDREF`` with the arguments
   ``env``, ``name``, and ``parents`` set to ``false``.

2. If ``res.result`` is ``true`` (already declared):

   a. If ``is_func_decl`` is ``false`` or ``env`` is not the global object
      environment record, return (ignore re-declaration).
      Else ``is_func_decl`` is ``true`` and ``env`` is the global object
      environment record, and E5.1 special behavior is needed.

   b. Let ``holder`` be ``ref.holder``; this must be the global object,
      which must hold an own property called ``name``.  This is the case
      because the global object has a ``null`` internal prototype.

   c. Let ``X`` be the property descriptor for ``name`` in ``holder``.

   d. If ``X.[[Configurable]]`` is ``false``:

      1. If ``X`` is an accessor property, throw a ``TypeError``.

      2. If ``X.[[Writable]]`` is ``false`` or ``X.[[Enumerable]]`` is
         ``false``, throw a ``TypeError``.

      3. Set ``attrs`` to the current property attributes of ``X``.
         (Note: in effect, don't update ``X`` attributes; we know it is
         writable, enumerable, and non-configurable.)

   e. Update the property ``name`` of ``holder`` to be a data property with
      the value ``val``, and attributes set to ``attrs``.

   f. Return.

3. Let ``holder`` be the property holder object of ``env`` (this is ``env``
   itself for a declarative environment, and the target (bindings) object
   for an object environment).

4. Define a new property ``name`` to ``holder`` with property attributes
   ``attrs``.  Note that this may fail if ``holder`` is not extensible;
   this can only happen for object environment records, as declarative
   environment records are never non-extensible.

5. Return.

Notes:

* The concrete implementation has to deal with the fact that ``env`` creation
  for an activation may be delayed.  So, the environment needs to be created
  on-the-fly if it doesn't exist yet.

* Step 2 inlines yet another version of ``[[DefineOwnProperty]]``.

* If a function is redeclared, it must have its binding value updated.

Fast path
=========

As a general rule, the compiler uses static analysis in an attempt
to allocate variables to registers and convert variable accesses to
direct register read/write operations.  Only a minority of variable
accesses thus happen using the slow identifier lookup described above.

If the compiler can determine that *all* variable accesses can be
handled this way (and no dynamic accesses are possible e.g. through
a direct ``eval`` call), the control information does not need to be
recorded in the compiled function at all.  However, if the compiler
cannot rule out the possibility of arbitrary dynamic variable lookups,
the control information needs to be stored.

Future work
===========

* A declarative environment record now records ``_Callee`` to get
  access to its ``_Varmap`` property.  Instead, the record could
  store a ``_Varmap`` reference directly, which would drop one step
  from lookup of a register mapped variable.  Also, if the function
  itself is freed, only the varmap needs to survive in the heap.
  The downside would be that there would be no access to function
  metadata, should that be useful (e.g. for debugging).
