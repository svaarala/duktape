==========================
Lightweight function value
==========================

Overview
========

A lightweight function (or a "lightfunc") is a plain duk_tval value type
which refers to a Duktape/C function without needing a representative
Ecmascript Function object.  The duk_tval tagged type encapsulates a
reference to the native function, as well as a small set of control bits,
without needing any heap allocations.  This is useful in especially low
memory environments, where the memory footprint of typical Duktape/C
bindings can be reduced.

A lightfunc has a separate API type (``DUK_TYPE_LIGHTFUNC``) so it is
a clearly distinguished type for C code.  However, for Ecmascript code
a lightfunc behaves as closely as possible like an ordinary Function
instance.  Various techniques (such as virtual properties) are used to
achieve this goal as well as possible.

Memory representation
=====================

The 8-byte representation for a lightfunc is::

     16 bits  16 bits      32 bits
    +--------+--------+----------------+
    | 0xfff5 | flags  | function ptr   |
    +--------+--------+----------------+

The flags field is further split into::

          8 bits       4 bits   4 bits
    +----------------+--------+--------+
    | magic          | length | nargs  |
    +----------------+--------+--------+

    magic: signed 8-bit value
    length: 0 to 15
    nargs: 0 to 14, 15 indicates DUK_VARARGS

Using lightfuncs
================

duk_push_c_lightfunc()
----------------------

You can make your own function bindings lightfuncs by simply pushing
lightfunction values with ``duk_push_c_lightfunc()``.  Lightfunc limits:

* Number of stack arguments must be 0 to 14 or varargs.

* Virtual "length" property can be given separately and must be between
  0 and 15.

* Magic must be between -128 to 127 (-0x80 to 0x7f).

DUK_OPT_LIGHTFUNC_BUILTINS
--------------------------

The feature option ``DUK_OPT_LIGHTFUNC_BUILTINS`` converts most built-in
functions forcibly into lightweight functions, reducing memory usage on
low memory platforms by around 14 kB.

Behavior notes
==============

Testcases
---------

A lot of detailed behavior is described in testcases:

* ``test-dev-lightfunc*.js``

* ``test-dev-lightfunc*.c``

Virtual properties
------------------

A lightfunc has the following virtual properties:

* ``name``: fixed format of ``lightfunc_<ptr>_<flags>`` where ``<ptr>`` is a
  platform dependent rendering of a function pointer, and ``<flags>`` is a
  16-bit internal flags field encoded as-is.

* ``length``: number between 0 and 15, encoded as 4 bits into the internal
  flags field.

Lightfunc cannot have a "prototype" property
--------------------------------------------

A lightfunc can be used as a constructor function, and is in fact always
constructable.  However, lightfuncs cannot have a "prototype" property,
so when called as a constructor, the automatically created default instance
object inherits from ``Object.prototype``.

You can still construct objects that inherit from a custom prototype, but
you need to create and return that value explicitly in the constructor, so
that it replaces the automatic default instance.

Lightfunc cannot be a setter/getter
-----------------------------------

A property value slot can either hold a ``duk_tval`` or two ``duk_hobject *``
pointers for the setter/getter of an accessor (in 32-bit environments
both take 8 bytes).  The setter/getter slot cannot hold two lightfunc
references (which would take 16 bytes).

As a result, lightfuncs cannot be used as setter/getter values.  If you
give a lightfunc as a setter/getter reference, it will be silently coerced
into a normal function object: the setter/getter will work, but a normal
function object will be created (consuming memory).

Lightfunc cannot have a finalizer
---------------------------------

Lightfuncs cannot have a finalizer because they are primitive values.
As such they don't have a reference count field nor do they participate
in garbage collection like actual objects.

Hypothetically, even if lightfuncs were garbage collected somehow, they
don't have space for a virtual ``_Finalizer`` property.  It would be
possible to set a finalizer on ``Function.prototype`` though and let that
finalize the lightfuncs.

Implementation notes
====================

Some changes needed
-------------------

This list is not exhaustive:

* Add a new tagged type, DUK_TAG_LIGHTFUNC, which points to a Duktape/C
  function.

  - A lightweight function can be a constructor, but will never have an
    automatic prototype object (a property slot would be needed to store it).
    A lightweight constructor function can create a replacement object from
    scratch and discard the automatically created instance object.

* The representation depends on the ``duk_tval`` layout:

  - For 8-byte packed type: 32-bit function pointer (pointing to the
    Duktape/C function) and 16 bits for function metadata.

  - For unpacked type: function pointer and 16 bits options field.

* The 16-bit metadata field is divided into the following sub-fields:

  - 8-bit magic value: important to be able to represent built-ins as
    lightfuncs (they use magic value extensively)

  - 4-bit ``nargs`` (with 15 indicating varargs)

  - 4-bit ``length`` property value

* Regarding Ecmascript semantics, the lightweight function type should
  behave like a Function object as much as possible.

  - This means, for example, that operators and built-in functions which
    strictly require an object argument must handle lightweight function
    values specially.

  - Some property algorithms can be implemented by first checking for
    lightfunc virtual properties, and if no virtual property matches,
    replacing the original argument with ``Function.prototype``.  This
    doesn't always work, however.  For instance, if getters/setters can
    be invoked, the ``this`` binding must bind to the original lightfunc,
    not ``Function prototype``.

* All call sites in code which expect an object need to be considered.

  - For example, if a call site uses ``duk_require_hobject()`` it needs to
    be changed to allow an object or a lightfunc.  There's a specific helper
    to implement minimal lightfunc support to such call sites by coercing
    lightfuncs to full Function objects: ``duk_require_hobject_or_lfunc_coerce()``.

* Add support in call handling for calling a lightfunc:

  - Bound function handling

  - Magic and ``nargs``

* Add support in traceback handling:

  - Function name

* Add virtual object properties so that lightweight functions will appear
  like ordinary Function objects to some extent

* Add reasonable behavior for all coercion operations, e.g. ToObject()
  should probably coerce a lightfunc into a normal Function with the same
  internal parameters (such as nargs and magic).

* Add an option to change built-in functions into lightweight functions
  instead of Function objects.  This should not be active by default,
  because this change makes the built-ins strictly non-compliant.  However,
  this is quite useful in RAM constrained environments.

* Extend the public API to allow the user to push lightweight function
  pointers in addition to ordinary ones.

  - For now there is no module registration helper which supports lightweight
    functions.

* Fix operators requiring a function value:

  - ``in``
  - ``instanceof``

* JSON/JX/JC support for lightfuncs

Automatic conversion of built-ins to lightfuncs
-----------------------------------------------

Most built-ins can be converted into lightweight functions because they
don't have a ``.prototype`` property which would prevent such a conversion.
The built-ins do have a ``.length`` property which doesn't always match the
actual argument count, but both ``nargs`` and ``length`` are stored in the
lightfunc value to allow these functions to be represented as lightfuncs.

The top level constructors (like ``Number``) cannot be converted to lightfuncs
because they have property values (e.g. ``Number.POSITIVE_INFINITY``) which
require a property table.

Built-in methods use "magic" values extensively, and the 8-bit magic is
sufficient for everything except the Date built-in.  The Date built-in magic
value was changed to be an index to a table of actual magic values to work
around this limit.

As a result, almost all built-in methods (except eval, yield, resume, and
require) are now converted to lightfuncs.

Future work
===========

More call sites with direct support of lightfuncs
-------------------------------------------------

Add support for direct lightfunc support in places where object coercion
(e.g. ``duk_require_hobject_or_lfunc_coerce()``) is used.  Such coercion
has a memory churn impact so it's preferable to avoid it when it matters.
The best places to improve on are those seen in practical code.

For example, currently enumerating a lightfunc goes through coercion which
is not ideal.

Improved JX/JC support
----------------------

Should lightfuncs be visible in a special way in JX/JC encoding?  For
instance::

    {_func:true}    ecma function
    {_cfunc:true}   C function
    {_lfunc:true}   lightweight C function

On the other hand C/Ecmascript functions are not distinguished in JX/JC now.

ToLightFunc()
-------------

There's currently no way to coerce an ordinary native Function into a
lightfunc.  Lightfuncs can only be created through the Duktape API.  If
such a coercion was added, it would need to check compatibility for the
coercion, at least magic and nargs must match for even the basic calling
convention guarantees to work.

Better virtual names for forced built-in lightfuncs
---------------------------------------------------

By matching Duktape/C function pointer and magic value, proper virtual
names could be given to built-in lightfuncs.  The function name table
goes into code memory (e.g. flash) which is often less restricted than
RAM.

A similar approach would be to allow user code to provide a hook which
could try to provide a name for a lightfunc when given its function
pointer and the 16-bit flags field.  User code could then consult symbol
tables or similar to provide better names.

Improve the Duktape C API
-------------------------

Right now there is just one call to push a lightfunc on the stack.  The
magic value of the lightfunc can be read.  However, the magic value, nargs
or length of a lightfunc cannot be modified.  User can construct a new
lightfunc from scratch, but won't be able to read e.g. the "nargs" value
of a lightfunc on the stack.

API questions:

* Add a push variant which has no 'length' or 'magic', so that it matches
  duk_push_c_function()?

* Add necessary API calls to read and write 'length', 'magic', and 'nargs'
  of a lightfunc.

API additions are not necessarily preferable if there is not concrete need
for them.

Symbol file for lightweight functions
-------------------------------------

* Address/offset + 16-bit flags (or just magic), allows reconstruction of
  lightfunc name

* Build could provide some symbol information that could be read into a
  debugger environment to improve traceback verbosity

Improve defineProperty() behavior
---------------------------------

Object.defineProperty() could throw a TypeError ("not extensible") when a
new property is created into a lightfunc.  Currently this succeeds but of
course new properties cannot actually be created into a lightfunc.

Improve ToObject() coercion
---------------------------

Current ToObject() coercion has two logical but confusing issues:

* The result is extensible while the input lightfunc is not.  This is
  useful because it's quite likely the user wants to extend the resulting
  function if the lightfunc is explicitly object coerced.  It also matches
  the standard Ecmascript behavior for strings: ``new String('foo')``
  returns an extensible String object.

  Another alternative would be to make the result non-extensible.

* The 'name' property of the coercion result is the lightfunc name, which
  is a bit confusing because the object is no longer a lightfunc.

  Another alternative would be to make the 'name' differ from the lightfunc
  name.  However, this would be confusing in a different way.
