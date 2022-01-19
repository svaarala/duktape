====
CBOR
====

CBOR is a standard format for JSON-like binary interchange.  It is
faster and smaller, and can encode more data types than JSON.  In particular,
binary data can be serialized without encoding e.g. in base-64.  These
properties make it useful for storing state files, IPC, etc.

Some CBOR shortcomings for preserving information:

* No property attribute or inheritance support.

* No DAGs or looped graphs.

* Array objects with properties lose their non-index properties.

* Array objects with gaps lose their gaps as they read back as undefined.

* Buffer objects and views lose much of their detail besides the raw data.

* ECMAScript strings cannot be fully represented; strings must be UTF-8.

* Functions and native objects lose most of their detail.

* CBOR tags are useful to provide soft decoding information, but the tags
  are just integers from an IANA controlled space with no space for custom
  tags.  So tags cannot be easily used for private, application specific tags.
  IANA allows reserving custom tags with little effort however, see
  https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml.

Future work
===========

General:

* Add flags to control encode/decode behavior.

* Allow decoding with a trailer so that stream parsing is easier.
  Similar change would be useful for JSON decoding.

* Reserve CBOR tag for missing value.

* Reserve other necessary CBOR tags.

* Explicit support for encoding with and without side effects (e.g.
  skipping Proxy traps and getters).

* JSON encoding supports .toJSON(), maybe something like .toCBOR()?

* Optimize encoding and decoding more.

Encoding:

* Tagging of typed arrays:
  https://datatracker.ietf.org/doc/draft-ietf-cbor-array-tags/.
  Mixed endian encode must convert to e.g. little endian because
  no mixed endian tag exists.

* Encoding typed arrays as integer arrays instead?

* Float16Array encoding support (once/if supported by main engine).

* Tagging of array gaps, once IANA reservation is complete:
  https://github.com/svaarala/duktape/blob/master/doc/cbor-missing-tag.rst.

* Support 64-bit integer when encoding, e.g. up to 2^53?

* Definite-length object encoding even when object has more than 23 keys.

* Map/Set encoding (once supported in the main engine), maybe tagged
  so they decode back into Map/Set.

* Bigint encoding (once supported in the main engine), as tagged byte
  strings like in Python CBOR.

* String encoding options: combining surrogate pairs, tagging non-UTF-8
  byte strings so they decode back to string, using U+FFFD replacement,
  etc.

* Detection of Symbols, encode them in a useful tagged form.

* Better encoding of functions.

* Hook for serialization, to allow caller to serialize values (especially
  objects) in a context specific manner (e.g. serialize functions with
  IPC metadata to allow them to be called remotely).  Such a hook should
  be able to emit tag(s) to mark custom values for decode processing.

Decoding:

* Typed array decoding support.  Should decoder convert to host
  endianness?

* Float16Array decoding support (once/if supported by main engine).

* Decoding objects with non-string keys, could be represented as a Map.

* Use bare objects and arrays when decoding?

* Use a Map rather than a plain object when decoding, which would allow
  non-string keys.

* Bigint decoding (once supported in the main engine).

* Decoding of non-BMP codepoints into surrogate pairs.

* Decoding of Symbols when call site indicates it is safe.

* Hooking for revival, to allow caller to revive objects in a context
  specific manner (e.g. revive serialized function objects into IPC
  proxy functions).  Such a hook should have access to encoding tags,
  so that revival can depend on tags present.

* Option to compact decoded objects and arrays.

* Improve fastint decoding support, e.g. decode non-optimally encoded
  integers as fastints, decode compatible floating point values as
  fastints.
