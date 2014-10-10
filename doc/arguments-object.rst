================
Arguments object
================

Overview
========

The *arguments object* is a very special kind of object in Ecmascript E5
from an implementation perspective.

The arguments object is created during function entry, as part of declaration
binding instantiation (Section 10.5), and is bound to the ``arguments``
identifier unless a shadowing declaration exists.  It is then accessed by
program code through the bound identifier ``arguments``.  The arguments
object and its exotic properties and behavior is described in E5 Section
10.6.

An arguments object created for a strict callee (referred to as a
*strict arguments object* below) is essentially a normal
object from a property behavior standpoint (it does have some error-throwing
accessor properties, but these have standard behavior).

An arguments object created for a non-strict callee (referred to as a
*non-strict arguments object* below) is entirely different.
It has non-standard variants for many core algorithms:
``[[Get]]``, ``[[GetOwnProperty]]``, ``[[DefineOwnProperty]]``,
``[[Delete]]``.  The non-standard variants provide the following main
exotic behaviors:

* **Magic argument bindings**: numbered indices ("0", "1", ...) matching
  formal arguments are "magically" bound to the corresponding variables.
  Book-keeping of these magical bindings happens through the internal
  *parameter map*, see more discussion below.

* **The "caller" property**: if ``arguments.caller`` *value* is a strict
  mode function, it cannot be read with ``[[Get]]``.  This is interesting
  because the arguments object is non-strict, and in fact contains no
  ``caller`` property initially.

Because creating an ``arguments`` object on function entry is very expensive,
the Duktape compiler attempts to avoid creating an arguments object at all if
possible.
This can be done if we can be sure during compilation that no reference to
the ``arguments`` object can happen at run time: either the arguments object
is shadowed by another (non-deletable) declaration, or the function contains
no direct references to ``arguments`` and there can be no indirect run time
references (e.g. through a direct ``eval``).
See the compiler documentation for details.

Arguments object and its properties
===================================

Binding type
------------

The arguments object is bound to the ``arguments`` identifier of a callee's
variable environment provided that it is not shadowed (see E5 Section
10.5).  The ``arguments`` binding type depends on callee strictness:

* Strict callee: immutable binding (prevents deletion and write).
* Non-strict callee: non-deletable mutable binding (prevents
  deletion but not write).

Object type and internal flags
------------------------------

* Object class: ``Arguments``
* Internal prototype: standard built-in Object prototype
* The ``DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS`` flag needs to be set for
  non-strict arguments object instances.  This flag enables the exotic
  variable map and ``caller`` post-check behaviors.

Although the arguments object looks like an array, it is a normal object.
In particular, the ``length`` property has no array-like automatic behavior.

Properties
----------

The properties of an arguments object (including the internal properties
specific to our implementation) are as follows (``nargs`` refers to the
number of actual call arguments given and ``nformals`` to the number of
formal arguments declared):

+-------------+---------------------------+--------------------------------+
| Property    | Non-strict mode           | Strict mode                    |
+=============+===========================+================================+
| indexes     | Actual call arguments.    | Actual call arguments.         |
| [0,nargs[   | Exotic behavior for those | No exotic behavior.            |
|             | magically bound to formal |                                |
|             | arguments.                |                                |
+-------------+---------------------------+--------------------------------+
| ``length``  | Set to ``nargs``, i.e. the| Same as non-strict mode.       |
|             | number of actual call     |                                |
|             | arguments (may be less or |                                |
|             | more than the number of   |                                |
|             | formals).                 |                                |
+-------------+---------------------------+--------------------------------+
| ``callee``  | Function being called.    | Accessor property with setter  |
|             | (Function is non-strict.) | and getter set to the          |
|             |                           | ``[[ThrowTypeError]]``         |
|             |                           | shared function.               |
+-------------+---------------------------+--------------------------------+
| ``caller``  | Not set.                  | Accessor property with setter  |
|             | Still, exotic behavior    | and getter set to the          |
|             | for ``[[Get]]`` if later  | ``[[ThrowTypeError]]``         |
|             | assigned value is a strict| shared function.               |
|             | function instance.        |                                |
+-------------+---------------------------+--------------------------------+
| ``_Map``    | Points to the internal    | Not set.                       |
| (internal)  | parameter map (see below).|                                |
|             | Set if there are any      |                                |
|             | mapped formal names.      |                                |
+-------------+---------------------------+--------------------------------+
| ``_Varenv`` | Points to the variable    | Not set.                       |
| (internal)  | environment record of the |                                |
|             | callee (internal object   |                                |
|             | for a declarative         |                                |
|             | environment record).      |                                |
|             | Set if there are any      |                                |
|             | mapped formal names.      |                                |
+-------------+---------------------------+--------------------------------+

The exact property attributes are defined in E5 Section 10.6:

* No properties are enumerable except the index properties.

* All properties are configurable except the strict mode ``caller`` and
  ``callee`` properties.

* All (data) properties are writable.

Strict mode "callee" and "caller"
---------------------------------

The strict mode ``callee`` and ``caller`` properties must be set to the
specific ``[[ThrowTypeError]]`` function described in E5 Section 13.2.3.
In particular, all the arguments object "throwers" must point to the same
function (not just a similar one).  Example::

  function f(x,y) { 'use strict'; return arguments; };
  function g(x,y) { 'use strict'; return arguments; };

  a = f(1,2,3);
  b = g(3,2,1);

  pd1 = Object.getOwnPropertyDescriptor(a, "caller");
  pd2 = Object.getOwnPropertyDescriptor(a, "callee");
  pd3 = Object.getOwnPropertyDescriptor(b, "caller");
  pd4 = Object.getOwnPropertyDescriptor(b, "callee");

  // all of these should print true

  print(pd1.get === pd1.set);
  print(pd2.get === pd2.set);
  print(pd3.get === pd3.set);
  print(pd4.get === pd4.set);

  print(pd1.get === pd2.get);
  print(pd2.get === pd3.get);
  print(pd3.get === pd4.get);

Parameter map
=============

The parameter map is not directly visible to program code, so it does
not have to be implemented exactly as specified.  Indeed, the current
implementation differs from what is specified in E5 Section 10.6 to
avoid creating a bunch of setter/getter functions.

The parameter map contains accessor properties for the mapped indices:
e.g. "0" might be mapped to a setter/getter pair which reads and writes
the magically bound variable.  The accessors are created with the
``MakeArgSetter`` and ``MakeArgGetter`` helpers in E5 Section 10.6.
The setters and getters read/write a certain identifier name in the
callee's variable environment (the initial, top level declarative
lexical environment used for argument, variable, and function bindings).
The variants of the standard algorithms (for e.g. ``[[Get]]``) then
look up the parameter map, and if appropriate, call the setter or
getter to interact with the bound variable usually after the standard
behavior has finished without error.

To illustrate this more concretely, consider::

  function f(x,y,x) { ... }

  f(1,2,3,4);

The arguments object and its parameter map would be something like::

  arguments = {
    "0": 1,       // shadowed, no magic binding
    "1": 2,       // magic binding to 'y'
    "2": 3,       // magic binding to 'x'
    "3": 4,       // not a formal argument, no magic binding
    "length": 4,
    "callee": f,
  }

  // 'arguments' has an internal [[ParameterMap]] set to the following
  // object. The get/set functions have 'env' as their lexical environment,
  // where 'env' is the variable environment for the f() call.

  [[ParameterMap]] = {
    get 1() { return y; },
    set 1(v) { y = v; },
    get 2() { return x; },
    set 2(v) { x = v; }
  }

Note that the magic bindings *do not keep* variables and the corresponding
arguments object entries in perfect sync, although the exotic behavior
tries to hide this from the program.  For instance::

  function f(x) {
    // Initially, arguments[0] == x == 1.

    // After this, the underlying arguments[0] value is still 1, but
    // 'x' has the value 2.  The underlying value for arguments[0] is
    // no longer in sync with 'x'.

    x = 2;

    // ... however, this is not externally visible.  The following
    // prints '2'.  The initial property lookup returns 1, but the
    // exotic [[GetOwnProperty]] behavior overwrites the value with
    // the current value of 'x'.

    print(arguments[0]);

    // Similarly, the overridden value (current value of 'x') is
    // visible through the property descriptor, hiding the discrepancy.
    // The following prints:
    //
    //   { value: 2, writable: true, enumerable: true, configurable: true }

    print(Object.getOwnPropertyDescriptor(arguments, "0"));

    // After this, the underlying arguments[0] value and 'x' have the
    // same value, 3.  The values are again in sync.

    arguments[0] = 3;
  }

  f(1);

From an implementation point of view using explicit getter/setter
objects for the internal parameter map would be very wasteful:
there would be lots of stub getters/setter objects.

So, the current implementation keeps a parameter map which
simply maps an index to a formal argument name (e.g. "2" to "x").
An internal reference to the variable environment of the callee
is stored in the arguments object to allow the correct variables
to be read/written.

Consider, for instance::

  function f(x,y,x) { arguments[2] = 10; print(x); }
  f(1,2,3,4);  // prints 10

The implementation specific arguments object here would contain::

  arguments = {
    "0": 1,       // shadowed, no magic binding
    "1": 2,       // magic binding to 'y'
    "2": 3,       // magic binding to 'x'
    "3": 4,       // not a formal argument, no magic binding
    "length": 4,
    "callee": f,

    // internal, implementation specific properties
    "_Map": { "1": "y", "2": "x" },
    "_Varenv": <varenv of callee>
  }

Here, the assignment to ``arguments[2]`` would be processed as follows:

* The standard ``[[Put]]`` operation eventually calls
  ``[[DefineOwnProperty]]`` which has an arguments object specific
  variant (E5 Section 10.6).

* The variant algorithm consults the parameter map associated with
  the arguments object and sees that "2" is mapped to identifier "x".

* The variant algorithm performs a standard ``[[DefineOwnProperty]]``
  and if that succeeds, winds up calling ``[[Put]]`` on the variable
  map (key "2"):

  + Ordinarily this would invoke the setter created for "2"
    created with *MakeArgSetter*, writing to "x" in the callee's
    variable environment.

  + In our implementation we look up the callee's variable environment
    from an internal property stored in the arguments object during its
    creation.  We then perform an identifier write for the identifier name
    "x" in the variable environment.  The end result is the same but no
    getter/setter objects need to be explicitly created.

The initial entries in the parameter map are established during arguments
object creation, based on function formal arguments.  New entries cannot
be established after that, but existing ones can be deleted if the
corresponding arguments object property is deleted or sufficiently modified
(e.g. converted into an accessor).  Bindings deleted from the map lose their
"magic" binding and don't regain the magic binding even if they are later
re-added to the arguments object.
Example::

  function f(x,y) {
    print(x,y);           // -> "1 2"

    arguments[0] = 10;    // magically bound to 'x'
    print(x,y);           // -> "10 2"

    delete arguments[0];  // magic binding is lost (removed from
                          // parameter map)
    arguments[0] = 20;    // reintroduced but no magic binding
    print(x,y);           // -> "10 2"
  }

  f(1,2)

  1 2
  10 2
  10 2

In more detail, a property map binding is deleted (and never
reintroduced) if:

* The corresponding arguments object property is deleted.

* The corresponding arguments object property is write-protected
  with a ``[[DefineOwnProperty]]`` call with ``[[Writable]]=false``.

* The corresponding arguments object property is changed into
  an accessor property with a ``[[DefineOwnProperty]]`` call.

In principle, if the parameter map became empty at run time (through
deletions), it could be deleted from the arguments object along with
the variable environment reference.  This is not worth while: this
does not happen in relevant cases and would require additional checks.

Exotic [[Get]] behavior
=======================

A non-strict arguments object has an exotic ``[[Get]]`` implementation.
This is unusual, because most exotic behaviors are defined through a
custom ``[[GetOwnProperty]]`` or ``[[DefineOwnProperty]``.  Because
this exotic behavior operates at the ``[[Get]]`` level, it affects
the reading of property values, but is not visible through property
descriptors or e.g. ``[[GetOwnProperty]]``.

The exotic behavior is covered in E5 Section 10.6, description for
``[[Get]]``.  To summarize, if:

* the property being looked up is not currently mapped in the arguments
  "parameter map" (``caller`` never is, because only numeric indices
  like "0" are mapped);

* the name of the property is ``caller``; and

* the standard lookup from the arguments object succeeds

Then:

* Check the result value of the property lookup (i.e. the value for
  ``arguments.caller``).  If the result value is a strict mode
  function, throw a ``TypeError``.

Note that this behavior is only defined for a non-strict arguments
object (i.e. arguments object created for a non-strict callee), and
protects the ``caller`` property from being read, if the caller is
strict.  Quite oddly, if the function has no formal parameters, it
gets no "parameter map" and also doesn't get the exotic ``[[Get]]``
behavior for ``caller``!

However, the ``caller`` property *can* be read through e.g.
``Object.getOwnPropertyDescriptor()`` (which uses
``[[GetOwnProperty]]``).  The exotic behavior does not protect
against this because the check is at the ``[[Get]]`` level.
Example::

  function f(x,y) { return arguments; }
  function g() { 'use strict'; return f(1,2); }

  a = g();
  a.caller = g;  // this is not set by default, see below

  // this is OK
  print(Object.getOwnPropertyDescriptor(a, "caller"));

  // this fails due to exotic behavior
  // (though doesn't in Rhino, V8, or Smjs)
  print(a.caller);

Finally, this exotic behavior is puzzling because a non-strict
mode arguments object *does not even have* a ``caller`` property.
The strict mode arguments object does have a ``caller`` property,
but it is a "``TypeError`` thrower", and strict mode arguments
objects don't have any exotic behavior (like ``[[Get]]`` here).

Function objects and argument creation
======================================

The relevant ``duk_hobject`` flags for a function object are:

* ``DUK_HOBJECT_FLAG_CREATEARGS``: indicates that an arguments object needs
  to be created upon function call.  Must be set for functions where the
  arguments object might be accessed.

* ``DUK_HOBJECT_FLAG_NEWENV``: always set (for all functions).

Misc notes
==========

Shadowing
---------

In strict mode ``arguments`` shadowing is not possible:

* An attempt to declare a variable, a function, a formal parameter
  named ``arguments`` or to use ``catch (arguments) { ... }`` is a
  ``SyntaxError``, see  E5 Sections 12.2.1, 12.4.1, 13.1.

* The ``with`` statement in its entirety is a ``SyntaxError`` in
  strict mode, so no shadowing is possible, see E5 Section 12.10.1.

* Any ``eval`` calls cannot declare variables in the function
  variable environment, because a direct ``eval`` call gets a new variable
  environment in strict mode (E5 Section 10.4.2, step 3), and an indirect
  ``eval`` call is "bound" to the global object (E5 Section 10.4.2, step 1).
  This is not an issue as such anyway, because the ``eval`` call happens
  at run time and does not affect binding initialization on function entry.

In non-strict mode shadowing is possible and any argument, variable,
or function declaration with the name ``arguments`` shadows the
arguments object and results in the arguments object not being
created at all (E5 Section 10.5, step 7 is skipped entirely).

A temporary shadowing created by e.g. ``catch`` does not prevent
the creation of an arguments object, as it happens after the
declaration binding instantiation.

Example: shadowing formal argument::

  js> function f(a, arguments) {
    >   print(typeof arguments, arguments);
    > }
  js> f(1,2);
  number 2

Example: shadowing variable declaration::

  js> function g() {
    >   var arguments = 5;
    >   print(typeof arguments, arguments);
    > }
  js> g();
  number 5

Example: shadowing function declaration::

  js> function h() {
    >   function arguments() {}
    >   print(typeof arguments, arguments);
    > }
  js> h();
  function 
  function arguments() {
  }

Example: temporary shadowing by a ``catch`` clause
(arguments object *is* created)::

  js> function i() {
    >   try {
    >     throw new Error("test");
    >   } catch(arguments) {
    >     // arguments temporarily shadowed here
    >     print(typeof arguments, arguments);
    >   }
    >   print("...", typeof arguments, arguments);
    > }
  js> i();
  object Error: test
  ... object [object Object]

Multiple formal arguments of the same name
------------------------------------------

In strict mode multiple formal arguments of the same name are a
``SyntaxError``.

In non-strict mode the last occurrence of a certain name "wins"::

  function f(a,a) { print(a); }

  f(1,2);  // prints '2'

The magic arguments binding also binds to the last occurrence::

  function f(a,a) {
    // arguments[0] is not magically bound
    // arguments[1] is magically bound to 'a'

    arguments[0] = 10;
    print(a);  // prints '2'

    arguments[1] = 20;
    print(a);  // prints '20'
  }

  f(1,2);

This behavior is apparent from E5 Sections 10.5 and 10.6.  In particular:

* In E5 Section 10.5, the declaration of formal argument bindings and their
  values in step 4 runs through the formal names from left to right.  The
  argument binding is declared when the first occurrence of a certain name
  is encountered, but the last occurrence updates any previously assigned
  value, leaving the formal bound to the value of the last (rightmost)
  occurrence.

* In E5 Section 10.6, the parameter map initialization for a non-strict
  callee goes over the formal arguments from *right to left* and creates
  the magic binding from the first (rightmost) occurrence.  If the same
  name is encountered again, the mapping is not updated.  The result is
  that magic bindings go to the rightmost occurrence of a certain name.

Accessing arguments from an inner function
------------------------------------------

An inner function cannot access the arguments object of an outer function
using the ``arguments`` identifier, as the inner function will always have
an ``arguments`` binding of one type or another.  Either the function has a
non-deletable shadowing declaration with the name ``arguments``, or an
actual non-deletable arguments object binding for ``arguments`` is created.

From a compiler standpoint this means that if an outer function does not
directly or indirectly (e.g. through a direct ``eval``) access its
arguments object, the arguments object does not need to be created.
There is no need to analyze the inner functions to see whether they
could somehow access the arguments object: they will have a "blocking"
binding with the name ``arguments``.

Of course, an outer function can make the arguments object available to
the inner object indirectly, e.g. through a variable binding of a
different name.  Example::

  function f() {
    var foo = arguments;
    function g() {
      print(foo[2]);
    }
    return g;
  }

  t = f('foo', 'bar', 'quux');
  t();  // prints 'quux'

From a compiler standpoint, here the outer function does access its
``arguments`` binding directly, requiring an arguments object to be
created upon a call to ``f()``.

Argument count
--------------

The number of arguments given in a function call is theoretically
unlimited.  In particular, it is theoretically possible that there
are more than 2**32-1 arguments and thus some of the numeric keys
of an arguments object are beyond the range of "valid array indices"
(see ``hobject-design.rst`` for detailed discussion).

The current implementation assumes that this never happens in practice.
As a result, arguments exotic behavior can do a fast reject if the
key being accessed is not a valid array index.

