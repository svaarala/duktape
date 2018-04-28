==============
Duktape typing
==============

Typing overview
===============

TBD.

Internal and external type summary
==================================

The table below summarizes various types and how they appear in the external
API and internals.  What's missing from the table:

* ``duk_hobject`` has a flags field which allows some logical types to be
  distinguished from one another.  For example, bound functions have a
  specific internal flag.  Other structs also have flags that may be relevant
  to typing but are not presented in the table.

* The API type check function(s) are not comprehensive, but more example of
  what a typical "is the value of type X" could be.

* The C APIs for creating a value of each type are not listed.

+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| ECMAScript code            | API type            | API type check(s)      | Internal duk_tval tag | Internal struct(s)      | duk_hobject class number            | ECMAScript typeof | ECMAScript Object .toString() | Notes                               |
+============================+=====================+========================+=======================+=========================+=====================================+===================+===============================+=====================================+
| n/a                        | DUK_TYPE_NONE       | duk_is_valid_index()   | DUK_TAG_UNUSED        | n/a                     | n/a                                 | n/a               | n/a                           | Marker for "no value" when doing    |
|                            |                     |                        |                       |                         |                                     |                   |                               | a valus stack type lookup.          |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| void 0                     | DUK_TYPE_UNDEFINED  | duk_is_undefined()     | DUK_TAG_UNDEFINED     | n/a                     | n/a                                 | undefined         | [object Undefined]            |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| null                       | DUK_TYPE_NULL       | duk_is_null()          | DUK_TAG_NULL          | n/a                     | n/a                                 | object (!)        | [object Null]                 |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| true                       | DUK_TYPE_BOOLEAN    | duk_is_boolean()       | DUK_TAG_BOOLEAN       | n/a                     | n/a                                 | boolean           | [object Boolean]              |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| 123                        | DUK_TYPE_NUMBER     | duk_is_number()        | DUK_TAG_FASTINT       | n/a                     | n/a                                 | number            | [object Number]               | If 48-bit signed int, and fastint   |
|                            |                     |                        |                       |                         |                                     |                   |                               | support enabled.                    |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| 123.1                      | DUK_TYPE_NUMBER     | duk_is_number()        | DUK_TAG_NUMBER (*)    | n/a                     | n/a                                 | number            | [object Number]               | With packed duk_tval, no explicit   |
|                            |                     |                        |                       |                         |                                     |                   |                               | internal tag.                       |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| "foo"                      | DUK_TYPE_STRING     | duk_is_string()        | DUK_TAG_STRING        | duk_hstring,            | n/a                                 | string            | [object String]               |                                     |
|                            |                     |                        |                       | duk_hstring_external    |                                     |                   |                               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| Symbol('foo')              | DUK_TYPE_STRING     | duk_is_string()        | DUK_TAG_STRING        | duk_hstring,            | n/a                                 | symbol            | [object Symbol]               | Symbols                             |
|                            |                     |                        |                       | duk_hstring_external    |                                     |                   |                               | (NOT FINALIZED)                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| n/a                        | DUK_TYPE_LIGHTFUNC  | duk_is_lightfunc()     | DUK_TAG_LIGHTFUNC     | n/a                     | n/a                                 | function          | [object Function]             |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| ArrayBuffer.allocPlain(1)  | DUK_TYPE_BUFFER     | duk_is_buffer()        | DUK_TAG_BUFFER        | duk_hbuffer_fixed,      | n/a                                 | object            | [object Uint8Array]           |                                     |
|                            |                     |                        |                       | duk_hbuffer_dynamic,    |                                     |                   |                               |                                     |
|                            |                     |                        |                       | duk_hbuffer_external    |                                     |                   |                               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| Duktape.Pointer('dummy')   | DUK_TYPE_POINTER    | duk_is_pointer()       | DUK_TAG_POINTER       | n/a                     | n/a                                 | pointer           | [object Pointer]              |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| n/a                        | n/a                 | duk_is_valid_index()   | n/a                   | n/a                     | DUK_HOBJECT_CLASS_NONE              | n/a               | n/a                           | Marker for "no value" when doing    |
|                            |                     |                        |                       |                         |                                     |                   |                               | a class number lookup.              |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| { foo: 123 }               | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_OBJECT            | object            | [object Object]               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| [ 1, 2, 3 ]                | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_harray              | DUK_HOBJECT_CLASS_ARRAY             | object            | [object Array]                | duk_harray extends duk_hobject.     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| arguments                  | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_ARGUMENTS         | object            | [object Arguments]            | Not an array; array-like.           |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Boolean(true)          | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_BOOLEAN           | object            | [object Boolean]              |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Date()                 | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_DATE              | object            | [object Date]                 |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new TypeError('aiee')      | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_ERROR             | object            | [object Error]                |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| Math.cos                   | DUK_TYPE_OBJECT     | duk_is_object(),       | DUK_TAG_OBJECT        | duk_hnatfunc            | DUK_HOBJECT_CLASS_FUNCTION          | function          | [object Function]             | duk_hnatfunc extends duk_hobject.   |
|                            |                     | duk_is_function(),     |                       |                         |                                     |                   |                               |                                     |
|                            |                     | duk_is_callable()      |                       |                         |                                     |                   |                               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| function foo() {}          | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hcompfunc           | DUK_HOBJECT_CLASS_FUNCTION          | function          | [object Function]             | duk_hcompfunc extends duk_hobject.  |
|                            |                     | duk_is_function(),     |                       |                         |                                     |                   |                               |                                     |
|                            |                     | duk_is_callable()      |                       |                         |                                     |                   |                               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| func.bind(null, 123)       | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hboundfunc          | DUK_HOBJECT_CLASS_FUNCTION          | function          | [object Function]             | duk_hobject flag                    |
|                            |                     | duk_is_function(),     |                       |                         |                                     |                   |                               | DUK_HOBJECT_FLAG_BOUNDFUNC is set.  |
|                            |                     | duk_is_callable()      |                       |                         |                                     |                   |                               | duk_hboundfunc extends duk_hobject. |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| JSON                       | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_JSON              | object            | [object JSON]                 |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| Math                       | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_MATH              | object            | [object Math]                 |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Number(123)            | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_NUMBER            | object            | [object Number]               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| /foo/                      | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_REGEXP            | object            | [object RegExp]               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new String('foo')          | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_STRING            | object            | [object String]               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| Object(Symbol('foo'))      | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_SYMBOL            | object            | [object Symbol]               | (NOT FINALIZED)                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| Function('return this')()  | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_GLOBAL            | object            | [object global]               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| n/a                        | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_henv                | DUK_HOBJECT_CLASS_OBJENV            | object            | [object ObjEnv]               | Internal scope object for an        |
|                            |                     |                        |                       |                         |                                     |                   |                               | object environment.  duk_henv       |
|                            |                     |                        |                       |                         |                                     |                   |                               | extends duk_hobject.                |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| n/a                        | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_henv                | DUK_HOBJECT_CLASS_DECENV            | object            | [object DecEnv]               | Internal scope object for a         |
|                            |                     |                        |                       |                         |                                     |                   |                               | declarative environment.  duk_henv  |
|                            |                     |                        |                       |                         |                                     |                   |                               | extends duk_hobject.                |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Duktape.Pointer('foo') | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hobject             | DUK_HOBJECT_CLASS_POINTER           | object            | [object Pointer]              |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Duktape.Thread(func)   | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hthread             | DUK_HOBJECT_CLASS_THREAD            | object            | [object Thread]               |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new ArrayBuffer(8)         | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_ARRAYBUFFER       | object            | [object ArrayBuffer]          | duk_hbufobj extends duk_hobject.    |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new DataView(arrBuf)       | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_DATAVIEW          | object            | [object DataView]             |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Int8Array(1)           | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_INT8ARRAY         | object            | [object Int8Array]            |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Uint8Array(1)          | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_UINT8ARRAY        | object            | [object Uint8Array]           |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Uint8ClampedArray(1)   | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_UINT8CLAMPEDARRAY | object            | [object Uint8ClampedArray]    |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Int16Array(1)          | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_INT16ARRAY        | object            | [object Int16Array]           |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Uint16Array(1)         | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_UINT16ARRAY       | object            | [object Uint16Array]          |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Int32Array(1)          | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_INT32ARRAY        | object            | [object Int32Array]           |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Uint32Array(1)         | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_UINT32ARRAY       | object            | [object Uint32Array]          |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Float32Array(1)        | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_FLOAT32ARRAY      | object            | [object Float32Array]         |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Float64Array(1)        | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hbufobj             | DUK_HOBJECT_CLASS_FLOAT64ARRAY      | object            | [object Float64Array]         |                                     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+
| new Proxy(target, handler) | DUK_TYPE_OBJECT     | duk_is_object()        | DUK_TAG_OBJECT        | duk_hproxy              | DUK_HOBJECT_CLASS_OBJECT            | object            | [object Object]               | duk_hproxy extends duk_hobject.     |
+----------------------------+---------------------+------------------------+-----------------------+-------------------------+-------------------------------------+-------------------+-------------------------------+-------------------------------------+

Options for representing a value
================================

There are four basic alternatives to representing a value:

* **A tagged type with no heap allocation**.  This is the lowest footprint
  alternative, and memory usage is 8 bytes (for a packed ``duk_tval``) or
  (typically) 16 bytes (for a non-packed ``duk_tval``).  Example: undefined,
  null, boolean, number, pointer.

* **A heap allocated custom struct**.  A tagged value points to a heap
  allocated C struct which is customized for a certain purpose.  Flags in
  the object header allow a base C struct to be extended in certain cases.
  Example: fixed buffer, dynamic buffer, external buffer, string.

* **A heap allocated object**.  A tagged value points to a ``duk_hobject``.
  Because a ``duk_hobject`` has a property table, type specific values can
  be easily added to the property table, but properties have a relatively
  high cost.  Example: plain ECMAScript object.

* **A heap allocated extended object**.  A tagged value points to a struct
  extending ``duk_hobject``.  Flags in the shared ``duk_hobject`` header
  allow Duktape internals to detect the extended type and to access further
  fields in an extended C struct.  The extended values may only be available
  internally, but may also be accessible via property reads if the properties
  are virtualized.  Example: ECMAScript function, Duktape/C function, thread,
  buffer object.
