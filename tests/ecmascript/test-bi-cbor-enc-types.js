/*
 *  Encode test for all major ECMAScript types.
 *
 *  Encoding is not unique, so this demonstrates current output.
 */

/*===
done
===*/

function test(val, expect) {
    var res = Duktape.enc('hex', CBOR.encode(val));
    // print(res);
    if (res !== expect) {
        print('MISMATCH, got ' + res + ', expected ' + expect);
    }
}

// Primitive types.
test(void 0, 'f7');
test(null, 'f6');
test(true, 'f5');
test(false, 'f4');
test(123, '187b');
/*
>>> cbor.dumps(123.1).encode('hex')
'fb405ec66666666666'
*/
test(123.1, 'fb405ec66666666666');
/*
>>> cbor.dumps(u'foo\u07fe\ucafebar').encode('hex')
'6b666f6fdfbeecabbe626172'
*/
test('foo\u07fe\ucafebar', '6b666f6fdfbeecabbe626172');

// Object types.
test([ 'foo', 'bar', 1, 2, 3, [ 4, 5 ], 'quux' ], '8763666f6f636261720102038204056471757578');
test({ foo: 'bar', quux: { baz: true } }, 'a263666f6f636261726471757578a16362617af5');
test(new Uint8Array([ 1, 2, 3 ]), '43010203');
test(function foo() {}, 'a0');  // ECMAScript function
test(Math.cos, 'a0');  // native function
test(Math.cos.bind(123, 234), 'a0');  // bound function
test(new Date(2019, 1, 2, 3, 4, 5), 'a0');
test(/foobar/g, 'a0');
test(new Proxy({}, {}), 'a0');
test(new Proxy([ 1, 2, 3 ], {}), '83010203');
// Cover null pointer only, encodes to "(null)" now.
// XXX: non-null pointers
test(Duktape.Pointer(null), '66286e756c6c29');
// XXX: lightfunc

// Buffer type details.
// For > 1-byte types expect string is little endian.
var buf = Uint8Array.allocPlain(4);
buf[2] = 0xfe;
test(buf, '440000fe00');
test(new Uint8Array([ 1, 2, 3, 4 ]).buffer, '4401020304');
test(new Uint8Array([ 0xfe, 1, 2, 3 ]), '44fe010203');
test(new Uint8ClampedArray([ 0x10fe, -100, 1, 2 ]), '44ff000102');
test(new Int8Array([ 0xfe, 1, 2, 3 ]), '44fe010203');
test(new Uint16Array([ 0xfe, 1, 2, 3 ]), '48fe00010002000300');
test(new Int16Array([ 0xfe, 1, 2, 3 ]), '48fe00010002000300');
test(new Uint32Array([ 0xfe, 1, 2, 3 ]), '50fe000000010000000200000003000000');
test(new Int32Array([ 0xfe, 1, 2, 3 ]), '50fe000000010000000200000003000000');
test(new Float32Array([ 0xfe, 1, 2, 3 ]), '5000007e430000803f0000004000004040');
test(new Float64Array([ 0xfe, 1, 2, 3 ]), '58200000000000c06f40000000000000f03f00000000000000400000000000000840');
var dv = new DataView(new Uint8Array([ 1, 2, 3, 4, 5, 6 ]).buffer);
test(dv, '46010203040506');

// Gap in array, similar to undefined, now encodes the same.
var arr = new Array(1);
test(arr, '81f7');
var arr = [ void 0 ];
test(arr, '81f7');

// Integer tests.
/*
>>> cbor.loads('f98000'.decode('hex'))
-0.0
*/
test(-0, 'f98000');
test(+0, '00');
test(1, '01');
test(23, '17');
test(24, '1818');
test(255, '18ff');
test(256, '190100');
test(65534, '19fffe');
test(65535, '19ffff');
test(65536, '1a00010000');
test(0xfedcab98, '1afedcab98');
test(0xffffffff, '1affffffff');
// Currently 64-bit integers are never used, shortest float
// maintaining precision is used.  No integers encode to
// half-float because 16-bit integer encodings are better
// and same size.
/*
>>> cbor.loads('fa4f800000'.decode('hex'))
4294967296.0
>>> cbor.loads('fb41f0000000100000'.decode('hex'))
4294967297.0
>>> cbor.loads('fb41f0000000200000'.decode('hex'))
4294967298.0
>>> cbor.loads('fb41f0000000300000'.decode('hex'))
4294967299.0
>>> cbor.loads('fb41f0000000400000'.decode('hex'))
4294967300.0
>>> cbor.loads('fb41f0000000500000'.decode('hex'))
4294967301.0
>>> cbor.loads('fb41f0000000600000'.decode('hex'))
4294967302.0
*/
test(0x100000000, 'fa4f800000');  // encodes as float
test(0x100000001, 'fb41f0000000100000');  // encodes as double
test(0x100000002, 'fb41f0000000200000');
test(0x100000003, 'fb41f0000000300000');
test(0x100000004, 'fb41f0000000400000');
test(0x100000005, 'fb41f0000000500000');
test(0x100000006, 'fb41f0000000600000');
test(-1, '20');
test(-2, '21');
test(-23, '36');
test(-24, '37');
test(-25, '3818');
test(-255, '38fe');
test(-256, '38ff');
test(-257, '390100');
test(-65535, '39fffe');
test(-65536, '39ffff');
test(-65537, '3a00010000');
test(-0xfedcab98, '3afedcab97');
test(-0xffffffff, '3afffffffe');
test(-0x100000000, '3affffffff');
// Here 3b0000000100000000 would be fine, but Duktape CBOR uses floats.
test(-0x100000001, 'fbc1f0000000100000');
test(-0x100000002, 'fbc1f0000000200000');
test(-0x100000003, 'fbc1f0000000300000');
test(-0x100000004, 'fbc1f0000000400000');
test(-0x100000005, 'fbc1f0000000500000');
test(-0x100000006, 'fbc1f0000000600000');

// Other number tests.
/*
>>> cbor.loads('f93800'.decode('hex'))
0.5
>>> cbor.loads('f93e00'.decode('hex'))
1.5
>>> cbor.loads('fa47800040'.decode('hex'))
65536.5
>>> cbor.loads('fb400921fb54442d18'.decode('hex'))
3.141592653589793
>>> cbor.loads('fbc00921fb54442d18'.decode('hex'))
-3.141592653589793
>>> cbor.loads('f97c00'.decode('hex'))
inf
>>> cbor.loads('f97e00'.decode('hex'))
nan
>>> cbor.loads('f9fc00'.decode('hex'))
-inf
*/
test(0.5, 'f93800');  // half-float
test(1.5, 'f93e00');  // half-float
test(65536.5, 'fa47800040');  // single float
test(Math.PI, 'fb400921fb54442d18');
test(-Math.PI, 'fbc00921fb54442d18');
test(1 / 0, 'f97c00');  // half-float
test(-1 / 0, 'f9fc00');  // half-float
test(0 / 0, 'f97e00');  // half-float

// Number tests from CBOR spec Appendix A.
//test(1.0, 'f93c00');  // Encoded as 01.
test(1.0, '01');
test(1.1, 'fb3ff199999999999a');
test(1.5, 'f93e00');
//test(65504.0, 'f97bff');  // Encoded as 19ffe0.
test(65504.0, '19ffe0');
//test(100000.0, 'fa47c35000');  // Encoded as 1a000186a0.
test(100000.0, '1a000186a0');
test(3.4028234663852886e+38, 'fa7f7fffff');
test(1.0e+300, 'fb7e37e43c8800759c');
//test(5.960464477539063e-8, 'f90001');  // Encoded as fa33800000, no denormal encoding support yet.
test(5.960464477539063e-8, 'fa33800000');
test(0.00006103515625, 'f90400');
//test(-4.0, 'f9c400');  // Encoded as 23.
test(-4.0, '23');
test(-4.1, 'fbc010666666666666');

// String tests, surrogates etc.
test('', '60');
test('foo', '63666f6f');
/*
>>> u'\ucafe'.encode('utf-8')
'\xec\xab\xbe'
*/
test('foo\ucafebar', '69666f6fecabbe626172');
test('12345678901234567890123', '773132333435363738393031323334353637383930313233');
test('123456789012345678901234', '7818313233343536373839303132333435363738393031323334');
test('A'.repeat(255), '78ff' + '41'.repeat(255));
test('A'.repeat(256), '790100' + '41'.repeat(256));
test('B'.repeat(65535), '79ffff' + '42'.repeat(65535));
test('B'.repeat(65536), '7a00010000' + '42'.repeat(65536));
test('C'.repeat(655360), '7a000a0000' + '43'.repeat(655360));

// Non-UTF-8 strings covered in separate tests.

print('done');
