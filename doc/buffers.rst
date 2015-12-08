=======
Buffers
=======

Overview
========

Ecmascript did not originally have a binary array (or binary string) data
type so various approaches are used:

* Khronos typed array:

  - https://www.khronos.org/registry/typedarray/specs/latest/
  - ``svn co -r 30720 https://cvs.khronos.org/svn/repos/registry/trunk/public/typedarray``
  - https://developer.mozilla.org/en/docs/Web/JavaScript/Typed_arrays
  - http://www.html5rocks.com/en/tutorials/webgl/typed_arrays/
  - http://clokep.blogspot.fi/2012/11/javascript-typed-arrays-pain.html
  - http://blogs.msdn.com/b/ie/archive/2011/12/01/working-with-binary-data-using-typed-arrays.aspx
  - https://www.inkling.com/read/javascript-definitive-guide-david-flanagan-6th/chapter-22/typed-arrays-and-arraybuffers

* ES6 has adopted the Khronos specification, with more specific semantics:

  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-arraybuffer-constructor
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-typedarray-constructors
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-arraybuffer-objects
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-dataview-objects

  These section links point to specific built-ins but link to the above:

  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-constructor-properties-of-the-global-object-arraybuffer
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-constructor-properties-of-the-global-object-dataview
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-uint8array
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-uint8clampedarray
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-uint16array
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-uint32array
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-int8array
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-int16array
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-int32array
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-float32array
  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-float64array

  ES6 spec:

  - http:/www.ecma-international.org/ecma-262/6.0/index.html/

* Node.js Buffer:

  - http://nodejs.org/api/buffer.html
  - https://nodejs.org/docs/v0.12.1/api/buffer.html
  - https://github.com/joyent/node/blob/master/lib/buffer.js

* Duktape has its own custom types:

  - Plain buffer data type which can point to a fixed size buffer,
    a dynamic (resizable) buffer, or an external (user allocated)
    buffer

  - Duktape.Buffer() object wrapper for plain buffer values (similar
    to how a String object wraps a plain string value)

* Blob (not very relevant):

  - https://developer.mozilla.org/en-US/docs/Web/API/Blob

This document describes how various buffer types have been implemented in
Duktape.  The goal is to minimize footprint, so the internal buffer type
implementation shares a lot of code even though multiple APIs are provided.

Duktape buffer support
======================

Overview
--------

Duktape currently supports the following buffer and buffer-related values:

* Plain Duktape buffer

* Duktape.Buffer object

* Node.js Buffer object

* ArrayBuffer, DataView, and TypedArray (Uint8Array etc) objects

The plain buffer value forms the basic underlying type for all the other
buffer types; the relationship is similar to a plain string and a String
object.  Plain buffers can be fixed, dynamic, or external:

* Fixed buffers cannot be resized but have a stable data pointer.

* Dynamic buffers can be resized at the cost of an unstable data pointer.
  They also have an internal spare area to minimize realloc operations
  (this spare is currently not exposed to user code).  You can also "steal"
  the current buffer allocation through the duk_steal_buffer() API call.

* External buffers point to user-allocated external data area whose pointer
  and length can be changed but Duktape won't resize or automatically free
  the buffer.

Plain buffers have virtual properties for buffer byte indices and 'length'.
Assignment has modulo semantics (e.g. 0x101 is written as 0x01), values
read back as unsigned 8-bit values.  The plain buffer type is designed to
be as friendly as possible for low level embedded programming, and has a
minimal footprint because there's no Ecmascript object associated with it.
It is mostly intended to be accessed from C code.  Duktape also uses buffer
values internally.

The various buffer and view objects ultimately point to an underlying buffer
and provide access to the full buffer or a slice/view of the buffer using
either accessor methods (like getUint32()) or virtual index properties.

All buffer objects are internally implemented using the ``duk_hbufferobject``
type which makes it easy to mix values between different APIs.  As a result
Duktape e.g. accepts a Node.js Buffer as an input for a Khronos DataView.

duk_hbufferobject
-----------------

The internal ``duk_hbufferobject`` type is a heap-allocated structure
extended from ``duk_hobject``.  In addition having the usual object
property tables and such, it has C struct fields for quick access:

* A reference to an underlying plain buffer value (``duk_hbuffer``),
  which may be of any type: fixed, dynamic, or external.

* A byte offset/length pair providing a window into the underlying
  buffer.  These values directly map to the virtual ``byteOffset``
  and ``byteLength`` properties.

* An element type and a shift count.  These provide enough information
  to support Khronos TypedArray views so that index values can be mapped
  to byte offsets and encoded/decoded appropriately.  The virtual ``length``
  property indicates the number of *elements* (not bytes) available, and
  is provided by dividing the byte length field with the element size
  (rounding downwards).  The virtual ``BYTES_PER_ELEMENT`` is provided based
  on the element shift count (as "1 << shift").

The following figure illustrates these for a fictional Int16-view::

    :  0  1: 2  3  4  5  6  7  8  9 10 11:12 13 14 15 :
    +------+-----------------------------+------------+
    | xx xx:xx xx xx xx xx xx xx xx xx xx:xx xx xx xx |   underlying buffer
    +------+-----------------------------+------------+   (16 bytes)
           :     :     :     :     :     :
           :     :     :     :     :     :    shift is 1, element size is
           :     :     :     :     :     :    (1 << 1) => 2 bytes
           |-----|-----|-----|-----|-----|    (= .BYTES_PER_ELEMENT)
           : [0] : [1] : [2] : [3] : [4] :
           :     :                            elem. type is Int16 (signed)
           :     :
           :<--->:  (2-byte elements)         byte offset: 2 (= .byteOffset)
                                              byte length: 10 (= .byteLength)
                                              => view maps byte range [2,12[

                                              length in elements: 5 (= .length)
                                              virtual indices: 0, 1, 2, 3, 4

Each ``duk_hbufferobject`` has virtual index behavior with indices mapping
logically to elements in the range [0,length[.  Elements may be signed or
unsigned integers of multiple sizes, IEEE floats, or IEEE doubles.  All
accesses to the underlying buffer are byte-based, and no alignment is required
by Duktape; however, Khronos TypedArray specification restricts creation of
non-element-aligned views.  All multi-byte elements are accessed in the host
endianness (this is required by the Khronos/ES6 TypedArray specification).

A ``duk_hbufferobject`` acts as a both a buffer representation (providing
Node.js Buffer and ArrayBuffer) and a view representation (prodiving e.g.
DataView, Uint8Array, and other TypedArray views).  It supports both a direct
1:1 mapping to an underlying buffer and a slice/view mapping to a subset of
the buffer.

The byteLength/byteOffset pair provides a logical window for the buffer object.
The underlying buffer may be smaller, e.g. as a result of a dynamic buffer
being resized after a ``duk_hbufferobject`` was created.  For example::

    +------+---------------------+
    | xx xx:xx xx xx xx xx xx xx | / / / /    underlying buffer resized to 9 bytes
    +------+---------------------+
           :     :     :     :     :     :
           :     :     :     :  ?  :  ?  :    index 3 is only partially mapped
           :     :     :     :     :     :    inde4 5 is not mapped
           |-----|-----|-----|-----|-----:
           : [0] : [1] : [2] : [3] : [4] :

This is not intended to be a normal usage scenario, so the main goal for
Duktape is only to provide memory safe behavior:

* The virtual properties (byteLength, byteOffset, length) are unchanged.

* Attempt to read outside the view (fully or partially) returns zero values.

* Attempt to write outside the view (fully or partially) is silently ignored.

* Other operations requiring access to the underlying buffer vary in behavior,
  some operations are silently skipped, etc.

Summary of buffer-related values
--------------------------------

+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Type              | Specification | .length        | .byteLength | .byteOffset | .BYTES_PER_ELEMENT | .buffer | [index] | Element type | Read coercion | Write coercion      | Endianness  | Accessor methods | Notes                             |
+===================+===============+================+=============+=============+====================+=========+=========+==============+===============+=====================+=============+==================+===================================+
| plain buffer      | Duktape       | yes (bytes)    | no          | no          | no                 | no      | yes     | uint8        | uint8         | ToUint32() & 0xff   | n/a         | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Duktape.Buffer    | Duktape       | yes (bytes)    | yes         | yes         | 1                  | no      | yes     | uint8        | uint8         | ToUint32() & 0xff   | n/a         | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Buffer            | Node.js       | yes (bytes)    | yes         | yes         | 1                  | no      | yes     | uint8        | uint8         | ToUint32() & 0xff   | n/a         | yes              | Based on Node.js v0.12.1.         |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| ArrayBuffer       | TypedArray    | yes (bytes)    | yes         | yes         | 1                  | no      | yes     | uint8        | uint8         | ToUint32() & 0xff   | n/a         | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| DataView          | TypedArray    | yes (bytes)    | yes         | yes         | 1                  | yes     | yes     | uint8        | uint8         | ToUint32() & 0xff   | n/a         | yes              |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Int8Array         | TypedArray    | yes (bytes)    | yes         | yes         | 1                  | yes     | yes     | int8         | int8          | ToUint32() & 0xff   | n/a         | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Uint8Array        | TypedArray    | yes (bytes)    | yes         | yes         | 1                  | yes     | yes     | uint8        | uint8         | ToUint32() & 0xff   | n/a         | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Uint8ClampedArray | TypedArray    | yes (bytes)    | yes         | yes         | 1                  | yes     | yes     | uint8        | uint8         | special             | n/a         | no               | Write: special clamp/round.       |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Int16Array        | TypedArray    | yes (elements) | yes         | yes         | 2                  | yes     | yes     | int16        | int16         | ToUint32() & 0xffff | host        | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Uint16Array       | TypedArray    | yes (elements) | yes         | yes         | 2                  | yes     | yes     | uint16       | uint16        | ToUint32() & 0xffff | host        | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Int32Array        | TypedArray    | yes (elements) | yes         | yes         | 4                  | yes     | yes     | int32        | int32         | ToUint32()          | host        | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Uint32Array       | TypedArray    | yes (elements) | yes         | yes         | 4                  | yes     | yes     | uint32       | uint32        | ToUint32()          | host        | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Float32Array      | TypedArray    | yes (elements) | yes         | yes         | 4                  | yes     | yes     | float        | float         | cast to float       | host        | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+
| Float64Array      | TypedArray    | yes (elements) | yes         | yes         | 8                  | yes     | yes     | double       | double        | cast to double      | host        | no               |                                   |
+-------------------+---------------+----------------+-------------+-------------+--------------------+---------+---------+--------------+---------------+---------------------+-------------+------------------+-----------------------------------+

Notes:

* A Duktape.Buffer object is a wrapper around a plain buffer value.
  It provides a means to create Buffer values and convert a value to a
  buffer.  Duktape.Buffer.prototype provides buffer handling methods
  which are also usable for plain buffer values due to automatic object
  promotion.

* DataView and Node.js Buffer inherit a set of accessor methods from their
  prototype.  These accessors allow fields of different width and type to
  be manipulated directly.  Endianness can be specified, but is limited to
  little/big (there's no support for ARM mixed endian IEEE doubles).

* TypedArray views are host endian.  Their byte offset relative to the
  ArrayBuffer they are used on must also be a multiple of the element
  size (i.e. views must be naturally aligned).  These requirements are not
  very useful from Duktape point of view but they are required by the
  Khronos/ES6 specifications.

  (It would be trivial to use a specific endianness or allow unaligned
  views because Duktape works with the values byte-by-byte anyway.)

* ``Uint8ClampedArray`` has a very specific clamping and rounding behavior
  which differs from all other view types.

* An unsigned ``ToUint32()`` coercion is used in writing signed values too.
  For the bytes written to memory the signedness of this coercion doesn't
  really matter.

* Every buffer object type in Duktape provides virtual index access (either
  as bytes or as elements), and the virtual "length", "byteLength",
  "byteOffset", and "BYTES_PER_ELEMENT" properties.  These are a union of
  various virtual properties used (e.g. byteLength, byteOffset, and
  BYTES_PER_ELEMENT come from TypedArray specification).  They're uniformly
  provided for all objects implemented internally as a ``duk_hbufferobject``.

Built-in objects related to buffers
-----------------------------------

Duktape plain buffer value:

* None

Duktape.Buffer:

* Duktape.Buffer

* Duktape.Buffer.prototype

Node.js Buffer:

* Buffer

* Buffer.prototype

* SlowBuffer, only available if one does: require("buffer") and omitted
  from Duktape implementation

TypedArray:

* ArrayBuffer

* ArrayBuffer.prototype

* DataView

* DataView.prototype

* Int8Array

* Int8Array.prototype

* Uint8Array

* Uint8Array.prototype

* Uint8ClampedArray

* Uint8ClampedArray.prototype

* Int16Array

* Int16Array.prototype

* Uint16Array

* Uint16Array.prototype

* Int32Array

* Int32Array.prototype

* Uint32Array

* Uint32Array.prototype

* Float32Array

* Float32Array.prototype

* Float64Array

* Float64Array.prototype

None of the prototype objects are mandated by the Khronos specification but
are present in ES6.

Conversions between buffer values
---------------------------------

Because Duktape supports three Buffer object APIs, it's important that buffer
values can be comfortably exchanged between the APIs (none of the API
specifications require such behavior, of course).

As a general rule:

* Any Buffer object/view (implemented internally as a ``duk_hbufferobject``)
  is accepted by any API expecting a specific object/view.  For example,
  Khronos DataView() constructor accepts a Node.js Buffer, and Node.js
  Buffer() accepts a Duktape.Buffer as an input.

* A plain Duktape buffer is accepted as if it was coerced to a Duktape.Buffer.
  (This is not always the case now, e.g. for typed array constructors.)

This general rules is complicated by a few practical issues:

* Some APIs create slices/views that share an underlying buffer value,
  while others create copies.  Both behaviors are necessary in some
  situations.

* A slice/view which doesn't map 1:1 to an underlying buffer cannot be
  coerced to a plain buffer value without copying, as the extra offset
  and length information is not supported for plain buffer values.

The current mixing behavior is described in Duktape Wiki:

* http://wiki.duktape.org/HowtoBuffers.html

Buffer values in the Duktape C API
----------------------------------

The C API for plain buffer and buffer object handling is described in
Duktape Wiki:

* http://wiki.duktape.org/HowtoBuffers.html

Node.js Buffer notes
====================

The Node.js ``Buffer`` type is widely used in server-side programming
but is not standardized as such.

Specification notes
-------------------

Specification notes:

* A Buffer may point to a slice of an underlying buffer.

* String-to-buffer coercion has a set of encoding values (other than UTF-8).

* Buffer prototype's ``slice()`` does not copy contents of the slice, but
  creates a new Buffer which points to the same underlying buffer.  This is
  similar to the TypedArray ``subarray()`` operation, but different from the
  ArrayBuffer ``slice()`` operation which creates a new buffer for the slice.
  With typed arrays a non-copying slice would just be a new view on top of a
  previous one instead of a new ArrayBuffer.

* The ``slice()`` operation provides offsetted access to the underlying
  buffer (same as with e.g. Uint8Array).  However, a slice is a fully
  fledged buffer and can be used to create another slice() etc.

* Buffers have virtual index properties and a virtual 'length' property.

* Reads and writes have an optional offset and value range check which
  causes an error for out-of-bounds indices (RangeError) and values
  (TypeError); the behavior is not always consistent, and chosen Duktape
  behavior is documented in testcases.  When the checks are disabled
  (noAssert == true), the behavior is memory unsafe and variable; some
  memory unsafe behavior results.  Duktape semantics are always memory safe
  even at the cost of some performance.

* Buffer accessor method read and write offsets are byte offsets regardless
  of data type being accessed.  This is similar to Khronos DataView, but
  different from Khronos TypedArray views whose indices are element-based.

* There are no alignment requirements for field access.  This also matches
  Khronos DataView behavior, but differs from Khronos TypedArrays which must
  be aligned.

* write(U)Int(LE|BE) and read(U)Int(LE|BE) operate on variable-size integers
  (up to 48-bit) and caller selects number of bytes (and endianness) to read
  or write.

* Newly created buffers don't seem to be zeroed automatically.  Duktape zeroes
  buffer data as a side effect of underlying ``duk_hbuffer`` values being
  automatically zeroed.  However, if DUK_USE_ZERO_BUFFER_DATA is not set,
  Node.js Buffers are not zeroed.

* Buffer inspect() provides a limited hex dump of buffer contents.  Duktape
  doesn't currently provide a similar function by default.

* SlowBuffer: probably not needed.

* User code can ``require('buffer')``; this is not supported by Duktape.

Implementation notes
--------------------

* Share Duktape.Buffer exotic behavior for indices and "length".

* Representation must point to a plain buffer and also needs internal slice
  offset/length properties to implement slice semantics.  Slices must be
  valid inputs for other slices; such slice-of-slice objects can point to
  the same plain buffer with offset/length pairs resolved at each step.

* For fast operations, guaranteed property slots could be used.  Alternatively
  a dedicated ``duk_hobject`` subtype can be used.  (The latter was chosen.)

* Should be optional and disabled by default because of footprint concerns.

* Should have a toLogString() which prints inspect() output or some other
  useful oneliner?

Buffers are not automatically zeroed
------------------------------------

::

  > b = new Buffer(16)
  <Buffer 00 99 f2 00 00 00 00 00 00 00 00 00 00 00 00 00>
  > b.fill(0)
  undefined
  > b
  <Buffer 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00>

Range checks and partial writes
-------------------------------

By default offset and value ranges are checked::

  > b.writeUInt8(0x101, 0)
  TypeError: value is out of bounds
      at TypeError (<anonymous>)
      at checkInt (buffer.js:784:11)
      [...]

With an explicit option asserts can be turned off.  With assertions
disabled invalid offsets are ignored and values are treated with
modulo semantics::

  > b.writeUInt8(0x101, 0, true)
  undefined
  > b
  <Buffer 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00>

When writing values larger than a byte, partial writes are allowed::

  > b.fill(0)
  undefined
  > b.writeUInt32BE(0xdeadbeef, 13)
  RangeError: Trying to write outside buffer length
      at RangeError (<anonymous>)
      at checkInt (buffer.js:788:11)
      [...]
  > b.writeUInt32BE(0xdeadbeef, 13, true)
  undefined
  > b
  <Buffer 00 00 00 00 00 00 00 00 00 00 00 00 00 de ad be>
  > b.fill(0)
  undefined
  > b.writeUInt32BE(0xdeadbeef, -1, true)
  undefined
  > b
  <Buffer ad be ef 00 00 00 00 00 00 00 00 00 00 00 00 00>

However, such values are not actually "dropped" but can actually be read
back with an unchecked out-of-bounds read::

  > b = new Buffer(16); b.fill(0); b.writeUInt32BE(0xdeadbeef, -1, true); b
  <Buffer ad be ef 00 00 00 00 00 00 00 00 00 00 00 00 00>
  > b.readUInt32BE(-1, true).toString(16)
  'deadbeef'
  > b.fill(1); b
  <Buffer 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01>
  > b.readUInt32BE(-1, true).toString(16)
  'de010101'

This is not just a "safe zone" to avoid implementing partial writes: the
out-of-bounds offsets can be large::

  > b = new Buffer(16); b.fill(0); b.writeUInt32BE(0xdeadbeef, -10000, true); b
  <Buffer 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00>
  > b.readUInt32BE(-10003, true).toString(16)
  'de'
  > b.readUInt32BE(-10000, true).toString(16)
  'deadbeef'

Running under valgrind this causes no valgrind gripes, so apparently this is
supported behavior.  It might be caused by "buffer sharing" where Node.js
actually uses a large Buffer to provide multiple smaller Buffers (as slices),
and these out-of-bounds accesses hit the shared large Buffer.  Sometimes
memory unsafe behavior occurs, though.

This behavior is difficult to implement in Duktape, so probably the best
approach is to either ignore partial reads/writes, or implement them in
an actual "clipping" manner.

Khronos typed array notes
=========================

The Khronos typed array specification is related to HTML canvas and WebGL
programming.  Some of the design choices are affected by this, e.g. the
endianness handling and clamped byte write support.  The Khronos specification
has been refined and merged into ES6 so this specification has an official
status now.

Specification notes
-------------------

* ArrayBuffer wraps an underlying buffer object, ArrayBufferView and DataView
  classes provide "windowed" access to some underlying ArrayBuffer.  A buffer
  object can be "neutered".  Apparently neutering happens when "transferring"
  an ArrayBuffer which is HTML specific.  Unsure if neutering needs to be
  supported.

* ArrayBuffer does not have virtual indices or 'length' behavior, but TypedArray
  views do.  DataView does not have virtual indices but e.g. V8 provides them in
  practice.  (For internal reasons, Duktape ArrayBuffers do provide 'length' and
  virtual indices.)

* ArrayBuffer has 'byteLength' and 'byteOffset' but no 'length'.  Views have
  a 'byteLength' and a 'length', where 'length' refers to number of elements,
  not bytes.  For example a Uint32Array view with length 4 would have
  byteLength 16.  (For internal reasons, all Duktape ArrayBuffer and view
  objects provide 'length', 'byteLength', and 'byteOffset'.)

* ArrayBufferView classes are host endian.  DataView is endian independent
  because caller specifies endianness for each call.

* TypedArray instances must be created with a byte offset that is a multiple
  of the element size (i.e. aligned).  DataView doesn't have this restriction.
  (This requirement is unnecessary for Duktape because the implementation
  never assumes alignment.  But, this requirement is implemented for
  compatibility.)

* NaN handling is rather fortunate, as it is compatible with packed duk_tval:
  in other words, NaNs can be substituted with one another.  When coerced to
  integer, NaN is coerced to zero.

* Modulo semantics for number writes, except Uint8ClampedArray which provides
  clamped semantics with special rounding when writin values.  Both modulo and
  clamping coerces NaN to zero.  With modulo semantics flooring is used (1.999
  writes as 1) while clamped semantics uses a specific form of rounding.

* For the clamping behavior, see:

  - http://heycam.github.io/webidl/#Clamp

  - http://heycam.github.io/webidl/#es-type-mapping

  - http://heycam.github.io/webidl/#es-byte

  Steps for unsigned byte (octet) clamped coercion:

  - Set x to min(max(x, 0), 2^8 − 1).

  - Round x to the nearest integer, choosing the even integer if it lies
    halfway between two, and choosing +0 rather than −0.

  - Return the IDL octet value that represents the same numeric value as x.

* Error is thrown for out-of-bounds accesses.

* When using ``set()`` the arrays may refer to the same underlying array and
  the write source and destination may overlap.  Must handle as if a temporary
  copy was made, i.e. like ``memmove()``.

* DataView and Node.js buffer have similar (but not identical) methods, which
  can share the same underlying implementation.  Endianness is specified with
  an argument in DataView but is implicit in Node.js buffer::

    // DataView
    setUint16(unsigned long byteOffset, unsigned short value, optional boolean littleEndian)

    // Node.js buffer
    buf.writeUInt16LE(value, offset, [noAssert])
    buf.writeUInt16BE(value, offset, [noAssert])

  Unfortunately also the argument order (value/offset) are swapped.

* There are explicit zeroing guarantees for ArrayBuffer constructor and
  typedarray constructors, so buffer data must be zeroed even when
  DUK_USE_ZERO_BUFFER_DATA is not set.

Implementation notes
--------------------

* ArrayBuffer wraps an underlying buffer object.  A buffer object can be
  "neutered".  ArrayBuffer is similar to Duktape.Buffer; eliminate
  Duktape.Buffer?

* ArrayBufferView classes and DataView refer to an underlying ArrayBuffer,
  and may have an offset.  These could be implemented similar to Node.js
  Buffer: refer to a plain underlying buffer, byte offset, and byte length
  in internal properties.  Reference to the original ArrayBuffer (boxed
  buffer) is unfortunately also needed, via the '.buffer' property.

* There are a lot of classes in the typed array specification.  Each class
  is an object, so this is rather heavyweight.

* Should be optional and disabled by default because of footprint concerns.

* Should have a toLogString() which prints inspect() output or some other
  useful oneliner.

Merged read/write algorithm for element access
==============================================

This section describes a merged algorithm for reading and writing fields
(uint8, int8, uint16, int16, etc) with the explicit read/write calls provided
by DataView and Node.js Buffer.  The same native code can be used with "magic"
value providing flags for differences in behavior.

Virtual index properties also need handling; they can either be implemented
separately or call into this algorithm.

Summary of read methods
-----------------------

Related methods are summarized in the table below, notes:

* "buf.XXX" refers to Node.JS Buffer instance methods (inherited)

* "dv.XXX" refers to Khronos DataView instance methods (inherited)

* "XyzArray index" refers to Khronos typed array view number index reads

* Endianness "user" means that caller gives a littleEndian flag so that
  effective endianness is either big or little (there's no support for ARM
  mixed endian)

* Endianness "host" means that host endianness is used

* When reading values, there's no clamping behavior because integers are
  converted to IEEE doubles upon read in the natural way (zeroes read out
  as positive zeroes).

* Bounds "arg" means argument indicates yes/no, "yes" means bounds are
  checked, "n/a" means not applicable.  Virtual indices don't really have
  bounds checking, as any reads outside the range [0,length[ just become
  concrete string-keyed property lookups.

+-------------------------+--------+-------+--------+---------------------------------------------------+
| Method                  | Endian | Bytes | Bounds | Notes                                             |
+=========================+========+=======+========+===================================================+
| buf.readIntLE           | little | 1-6   | arg    | Can read up to 48-bit integers, caller specifies  |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readIntBE           | big    | 1-6   | arg    | Can read up to 48-bit integers, caller specifies  |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readUIntLE          | little | 1-6   | arg    | Can read up to 48-bit integers, caller specifies  |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readUIntBE          | big    | 1-6   | arg    | Can read up to 48-bit integers, caller specifies  |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readInt8            | n/a    | 1     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readUInt8           | n/a    | 1     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readInt16LE         | little | 2     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readInt16BE         | big    | 2     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readUInt16LE        | little | 2     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readUInt16BE        | big    | 2     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readInt32LE         | little | 4     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readInt32BE         | big    | 4     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readUInt32LE        | little | 4     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readUInt32BE        | big    | 4     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readFloatLE         | little | 4     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readFloatBE         | big    | 4     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readDoubleLE        | little | 8     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| buf.readDoubleBE        | big    | 8     | arg    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| DataView.getInt8        | n/a    | 1     | yes    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| DataView.getUint8       | n/a    | 1     | yes    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| DataView.getInt16       | user   | 2     | yes    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| DataView.getUint16      | user   | 2     | yes    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| DataView.getInt32       | user   | 4     | yes    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| DataView.getUint32      | user   | 4     | yes    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| DataView.getFloat32     | user   | 4     | yes    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| DataView.getFloat64     | user   | 8     | yes    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Int8Array index         | n/a    | 1     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Uint8Array index        | n/a    | 1     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Uint8ClampedArray index | n/a    | 1     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Int16Array index        | host   | 2     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Uint16Array index       | host   | 2     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Int32Array index        | host   | 4     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Uint32Array index       | host   | 4     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Float32Array index      | host   | 4     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+
| Float64Array index      | host   | 8     | n/a    |                                                   |
+-------------------------+--------+-------+--------+---------------------------------------------------+

Summary of write methods
------------------------

Related methods are summarized in the table below, notes:

* "buf.XXX" refers to Node.JS Buffer instance methods (inherited)

* "dv.XXX" refers to Khronos DataView instance methods (inherited)

* "XyzArray index" refers to Khronos typed array view number index writes

* Endianness "user" means that caller gives a littleEndian flag so that
  effective endianness is either big or little (there's no support for ARM
  mixed endian)

* Endianness "host" means that host endianness is used

* Coercion behavior describes how an input value is coerced into an integer
  value; usually truncation but there are special cases.  "truncate*" means
  that truncation happens in Node.js Buffer API calls when "noAssert==true";
  a TypeError occurs for out-of-range writes (though fractional values are
  still silently accepted).

* Bounds "arg" means argument indicates yes/no, "yes" means bounds are
  checked, "n/a" means not applicable.  Virtual indices don't really have
  bounds checking, as any writes outside the range [0,length[ just become
  concrete string-keyed properties of the object (provided the object is
  extensible).

* Return value of Node.js Buffer write calls is the number of bytes written.
  TypedArray write return value is ``undefined``.

* Node.js Buffer write() method is left out because it's not an element write

+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Method                  | Endian | Bytes | Bounds | Coercion  | Notes                                             |
+=========================+========+=======+========+===========+===================================================+
| buf.writeIntLE          | little | 1-6   | arg    | truncate* | Can write up to 48-bit integers, caller specifies |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeIntBE          | big    | 1-6   | arg    | truncate* | Can write up to 48-bit integers, caller specifies |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeUIntLE         | little | 1-6   | arg    | truncate* | Can write up to 48-bit integers, caller specifies |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeUIntBE         | big    | 1-6   | arg    | truncate* | Can write up to 48-bit integers, caller specifies |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeInt8           | n/a    | 1     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeUInt8          | n/a    | 1     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeInt16LE        | little | 2     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeInt16BE        | big    | 2     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeUInt16LE       | little | 2     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeUInt16BE       | big    | 2     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeInt32LE        | little | 4     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeInt32BE        | big    | 4     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeUInt32LE       | little | 4     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeUInt32BE       | big    | 4     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeFloatLE        | little | 4     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeFloatBE        | big    | 4     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeDoubleLE       | little | 8     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| buf.writeDoubleBE       | big    | 8     | arg    | truncate* |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| DataView.setInt8        | n/a    | 1     | yes    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| DataView.setUint8       | n/a    | 1     | yes    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| DataView.setInt16       | user   | 2     | yes    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| DataView.setUint16      | user   | 2     | yes    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| DataView.setInt32       | user   | 4     | yes    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| DataView.setUint32      | user   | 4     | yes    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| DataView.setFloat32     | user   | 4     | yes    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| DataView.setFloat64     | user   | 8     | yes    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Int8Array index         | n/a    | 1     | n/a    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Uint8Array index        | n/a    | 1     | n/a    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Uint8ClampedArray index | n/a    | 1     | n/a    | special   | Coercion is rounding with specific rules          |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Int16Array index        | host   | 2     | n/a    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Uint16Array index       | host   | 2     | n/a    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Int32Array index        | host   | 4     | n/a    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Uint32Array index       | host   | 4     | n/a    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Float32Array index      | host   | 4     | n/a    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+
| Float64Array index      | host   | 8     | n/a    | truncate  |                                                   |
+-------------------------+--------+-------+--------+-----------+---------------------------------------------------+

Implementation notes
====================

TypedArray inheritance
----------------------

The prototype chain for a TypedArray instance in V8 is::

    view object -> Uint8Array.prototype -> Object.prototype

This means that view properties like ``set()`` and ``subarray()`` are
provided by the prototype, and each view type has its own prototype with
these properties.  This duplicates the properties several times.

Duktape now inherits from an intermediate object::

    view object -> Uint8Array.prototype -> TypedArray prototype -> Object.prototype

The ``set()`` and ``subarray()`` methods are inherited from the intermediate
prototype object.  This reduces property count by about 16 at the cost of one
additional object.

View/slice notes
----------------

* Affects all code that accesses the underlying buffer through an Object
  reference (Buffer, ArrayBuffer, DataView, Uint8Array, etc):

  - Must look up internal plain buffer but also check for offset/length
    information.

  - Lookups should be fast, so:

    + Use an extended structure like for compiled functions

    + Use slotted internal properties (must be non-configurable so that
      their location won't change by accident)

* Need reference to underlying buffer:

  - Could use a raw pointer to the buffer data as long as there's also a
    buffer reference to avoid freeing the underlying data.

  - But a raw pointer would only work with a fixed buffer which has a
    stable buffer pointer.

  - So, must reference the original buffer and figure out its data area
    dynamically.

* Need byte offset and length for the view:

  - These should be validated on creation so that sanity checks are not
    necessary for every access.

  - If internal properties, should be non-writable and non-configurable
    to ensure that only C code can create a situation where assertions
    fail.

* Need element size for the view:

  - For Node.js Buffer the element size is the byte size.  For TypedArrays
    it may be 1, 2, 4, or 8 bytes.

  - Virtual "length" property must provide length in elements.  Maintain
    two length fields (byte and element) or only the other and shift as
    necessary.

  - Virtual element "length": easier index/bound checks, virtual "length"
    read needs no change.  Must be taken into account when byte length is
    needed.

Buffer validity checks
----------------------

To ensure memory safety, all memory accesses need to be checked against the
size of the underlying buffer even if the access is within the configured
view/slice.  This is needed because an underlying buffer may be a dynamic one
and can be resized at any point.

In particular, the underlying buffer may be resized as a side effect of any
operation that triggers code to run: the code may call into user code which
manipulates the buffer.

As a result, the following checks must be made just before an operation and
there must be no side effects between the check and the operation:

* Checking that byte range is covered by underlying buffer

* Checking that bufferobject is neutered (buf == NULL vs. buf != NULL)

Future work
===========

Improve consistency of argument coercion
----------------------------------------

For Node.js Buffer bindings there's considerable variation of how arguments
are coerced (in both Node.js and Duktape; and these are not always the same
now).  Improve consistency either by matching Node.js more closely, or by
making Duktape specific behavior more consistent with itself.

Add support for neutering (detached buffer)
-------------------------------------------

Currently not supported.  Neutering an ArrayBuffer must also affect all views
referencing that ArrayBuffer.  Because duk_hbufferobject has a direct
duk_hbuffer pointer (not a pointer to ArrayBuffer which is stored as .buffer)
the neutering cannot be implemented by replacing the duk_hbuffer pointer with
zero, as that wouldn't affect all the shared views.

Instead, neutering probably needs to be implemented at the plain buffer level;
for example, by adding a "neutered" flag to duk_hbuffer.  A dynamic buffer can
also be resized to zero bytes at neutering time.

Another option is to support neutering only when the underlying buffer is
dynamic, and simply resize the buffer to zero bytes.  This produces much of
the required behavior (e.g. zero .byteLength) but not all (e.g. zero
.byteOffset).  So an explicit neutered check, or a change in data structures,
may be necessary.

In ES6 neutering seems to be covered under the name "detached buffer" and
many operations on detached buffers (like reads and writes) throw a TypeError
which is close to what current code is doing:

- See e.g. Step 9 of http://www.ecma-international.org/ecma-262/6.0/index.html#sec-setviewvalue

Configurable endianness for TypedArray views
--------------------------------------------

Change duk_hbufferobject so that it records requested endianness explicitly:
host, little, or big endian.  Then use the specified endianness in readfield
and writefield internal primitives.

This should be relatively straightforward to do, and perhaps useful.

Allow non-aligned views
-----------------------

The Khronos/ES6 alignment limitation is not necessary with Duktape because
all element accesses are ultimately done using byte-by-byte reads without
making any alignment assumptions.

Additional arguments to TypedArray .set()
-----------------------------------------

It would be nice to be able to specify an offset/length (or offset/end) for
a .set() call, so that one could::

    v1.set(v2, 5, 10);

Currently one needs to do something like::

    v1.set(v2.subarray(5, 15));

Additional arguments to TypedArray constructor
----------------------------------------------

It would be nice to have offset/length when constructing a TypedArray from
another TypedArray.

Accept plain buffer values where duk_hbufferobject is accepted
--------------------------------------------------------------

This would be convenient and easy to add by automatically coercing the
"this" argument (which needs to be type checked anyway).

Make the .buffer property virtual
---------------------------------

The ``.buffer`` property required by TypedArray specification is the only
concrete property on TypedArray instances.  The property points to the
backing ArrayBuffer object (different from the ``duk_hbuffer *buf`` which
is used now).

Perhaps change the data structure to support the ``.buffer`` reference
directly (perhaps instead of ``buf`` or in addition to ``buf``) and make
it a virtual property.

Node.js .parent property
------------------------

Not currently included in Node.js Buffer instances.

Testcase coverage improvements
------------------------------

* Fine-grained tests for argument/this coercion

* Property attributes

* Object.defineProperty() and Object.getOwnPropertyDescriptor() for
  virtual properties

* Constructing DataView and TypedArray from another view (allowed now
  but semantics may need improvement)

* Node.js Buffer slice() coverage, argument coercion, etc.

Low memory support
------------------

Implement low-memory support (16-bit fields, pointer compression, etc) for
Buffer objects.  Currently buffer objects will have "long" fields.

Improve fastint support
-----------------------

Improve fastint handling for buffer indices, lengths, values, etc.

Unsorted future work
--------------------

* Clean up ``duk_hbufferobject`` ``buf == NULL`` handling.  Perhaps don't
  allow ``NULL`` at all; this depends on the neutering / detached buffer
  solution.

* Implement and test for integer arithmetic wrap checks e.g. when coercing
  an index into a byte offset by shifting.

* Accept a plain buffer everywhere where a Duktape.Buffer, ArrayBuffer, or
  Node.js Buffer would be accepted, coercing the plain buffer automatically
  to a full object (either conceptually or concretely)?

* duk_to_buffer(): coerce a Buffer object into a plain buffer value
  (similarly to how duk_to_string() coerces a String to a plain string)?
  Slice information will be lost unless a copy is made.

* duk_is_buffer(): return true for a Buffer object? For comparison,
  duk_is_string() returns false for a String object, so returning false
  might be most consistent.

* Other Duktape C API changes to interact with Buffer objects.

* Duktape.Buffer.prototype.toString() and Duktape.Buffer.prototype.valueOf():
  what should their behavior be for slices?  Currently slice information is
  lost, same as if Duktape.Buffer(obj) was called.

* Node.js Buffer.isBuffer(): what is the best behavior for plain buffer and
  other buffer object values?

* ToObject() coercion for a plain buffer now results in Duktape.Buffer because
  Duktape.Buffer is its "object counterpart" (similar to how a plain string
  has a String counterpart).  This is consistent as a plain buffer also now
  inherits properties from Duktape.Buffer.prototype.  It might make sense to
  make ArrayBuffer the object counterpart for plain buffers and deprecate
  Duktape.Buffer?

* What to do with Node.js SlowBuffer, INSPECT_MAX_BYTES, and code that does
  ``require('buffer')``?

* Mixing buffer types between APIs: go through the various cases, document,
  add testcases, etc.

* Implement fast path for Node.js Buffer constructor when argument is another
  duk_hbufferobject (now reads indexed properties explicitly).

* Duktape C API tests for buffer handling.

* Duktape C API test exercising "underlying buffer doesn't cover logical
  buffer slice" cases which cannot be exercised with plain Ecmascript code.

* Add a toLogString() into the prototype to make buffers log better?
  Currently logging a buffer may produce non-printable characters (e.g.
  NUL).

* Document Buffer object relationship to JSON, JX, and JC.

* Explicit maximum element and byte size checks for all operations that
  create new bufferobjects.

* Change the TypedArray subarray() implementation to avoid copying the
  argument internal prototype and use a "default" prototype instead
  (e.g. Uint8Array.prototype instead of copying the argument internal
  prototype which may be different).
