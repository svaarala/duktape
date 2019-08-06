/*
 *  Decode tests for all major types, organized by each initial byte.
 *
 *  For inputs that should decode successfully, also tests that truncated and
 *  padded inputs are rejected.
 */

/*===
- Major type 0x00-0x1f: unsigned integer
- Major type 0x20-0x3f: negative integer (not -0)
- Major type 0x40-0x5f: byte string
typeof: object
instanceof ByteArray: false
instanceof Uint8Array: true
proto is Uint8Array.prototype: true
is bare: false
- Major type 0x60-0x7f: text string
- Major type 0x80-0x9f: array
typeof: object
instanceof Array: true
Array.isArray: true
proto is Array.prototype: true
is bare: false
- Major type 0xa0-0xbf: map
typeof: object
instanceof Object: true
proto is Object.prototype: true
is bare: false
- Major type 0xc0-0xdf: tag
- Major type 0xe0-0xff: simple types, floating point numbers, break, etc
done
===*/

var ERROR = {};

function testOne(hexInput, expect, opts) {
    opts = opts || {};
    var truncLimit = 255;
    var prInput = (hexInput.length > truncLimit ? hexInput.substring(0, truncLimit) + '...' : hexInput);
    var jxExpect = Duktape.enc('jx', expect);
    var prExpect = (jxExpect.length > truncLimit ? jxExpect.substring(0, truncLimit) + '...' : jxExpect);
    try {
        var result = CBOR.decode(Duktape.dec('hex', hexInput).buffer);
        var jxResult = Duktape.enc('jx', result);
        var prResult = (jxResult.length > truncLimit ? jxResult.substring(0, truncLimit) + '...' : jxResult);
        if (!opts.silent) {
            print(prInput, prResult);
        }
        if (expect === ERROR) {
            print('MISMATCH, got ' + prResult + ', expected ERROR');
        } else if (jxResult !== jxExpect) {
            print('MISMATCH, got ' + prResult + ', expected ' + prExpect);
        }
    } catch (e) {
        if (!opts.silent) {
            print(prInput, e.name);
        }
        if (expect !== ERROR) {
            print('MISMATCH, got ' + String(e) + ', expected ' + prExpect);
        }
    }
}

function getPad(bytelen) {
    var u8 = new Uint8Array(bytelen);
    for (var i = 0; i < bytelen; i++) {
        // Padding is not deterministic, but this should not affect
        // test result (= all paddings are invalid).
        u8[i] = Math.random() * 256;
    }
    return Duktape.enc('hex', u8);
}

function test(hexInput, expect) {
    var bytelen = hexInput.length / 2;
    var ntrunc = 10;
    var npad = 10;

    // Change silent to false to get verbose output.
    testOne(hexInput, expect, { silent: true });

    // Truncation tests; any truncated input of a valid input is
    // always invalid (converse is not true).
    if (expect !== ERROR) {
        for (var trunc = 1; trunc <= ntrunc; trunc++) {
            var clip = 2 * (bytelen - trunc);
            if (clip >= 0) {
                var truncHexInput = hexInput.substring(0, clip);
                testOne(truncHexInput, ERROR, { silent: true });
            }
        }
    }

    // Same for trailing garbage: any valid input with garbage added
    // is always invalid (converse is again not true).
    if (expect !== ERROR) {
        for (var pad = 1; pad <= npad; pad++) {
            var paddedHexInput = hexInput + getPad(pad);
            testOne(paddedHexInput, ERROR, { silent: true });
        }
    }

    // Any valid input remains valid when tagged.  Because tags are
    // ignored by the decoder, the expect value remains the same.
    if (expect !== ERROR) {
        var taggedHexInput = 'c4' + hexInput;
        testOne(taggedHexInput, expect, { silent: true });
    }
}

print('- Major type 0x00-0x1f: unsigned integer');

test('00', +0);
test('01', 1);
test('02', 2);
test('03', 3);
test('04', 4);
test('05', 5);
test('06', 6);
test('07', 7);
test('08', 8);
test('09', 9);
test('0a', 10);
test('0b', 11);
test('0c', 12);
test('0d', 13);
test('0e', 14);
test('0f', 15);
test('10', 16);
test('11', 17);
test('12', 18);
test('13', 19);
test('14', 20);
test('15', 21);
test('16', 22);
test('17', 23);
test('18', ERROR);
test('1800', +0);
test('1801', 1);
test('1816', 22);
test('1817', 23);
test('1818', 24);
test('18fe', 254);
test('18ff', 255);
test('19', ERROR);
test('1911', ERROR);
test('190000', +0);
test('1900ff', 255);
test('190100', 256);
test('191234', 4660);
test('19fedc', 65244);
test('19ffff', 65535);
test('1a', ERROR);
test('1a11', ERROR);
test('1a1122', ERROR);
test('1a112233', ERROR);
test('1a00000000', +0);
test('1a000000ff', 255);
test('1a0000ffff', 65535);
test('1a00010000', 65536);
test('1a12345678', 305419896);
test('1afedcba98', 4275878552);
test('1affffffff', 4294967295);
test('1b', ERROR);
test('1b11', ERROR);
test('1b1122', ERROR);
test('1b112233', ERROR);
test('1b11223344', ERROR);
test('1b1122334455', ERROR);
test('1b112233445566', ERROR);
test('1b11223344556677', ERROR);
test('1b0000000000000000', +0);
test('1b00000000000000ff', 255);
test('1b000000000000ffff', 65535);
test('1b00000000ffffffff', 4294967295);
test('1b001fffffffffffff', 9007199254740991);  // 2^53-1, exact
test('1b0020000000000000', 9007199254740992);  // 2^53, exact
test('1b0020000000000001', 9007199254740992);  // rounds down
test('1b0020000000000002', 9007199254740994);  // 2^53+2, exact
test('1b0020000000000003', 9007199254740996);  // rounds up
test('1b0020000000000004', 9007199254740996);  // 2^53+4, exact
test('1b0020000000000005', 9007199254740996);  // rounds down
test('1b0020000000000006', 9007199254740998);  // 2^53+6, exact
test('1b123456789abcdef0', 1311768467463790300);  // not exact: 0x123456789abcdf00
test('1bffeeddccbbaa9988', 18441921395520346000);
/*
>>> import cbor
>>> float(cbor.loads('1bfffffffffffff7ff'.decode('hex')))
1.844674407370955e+19
>>> float(cbor.loads('1bfffffffffffff800'.decode('hex')))
1.844674407370955e+19
>>> float(cbor.loads('1bfffffffffffffbfe'.decode('hex')))
1.844674407370955e+19
>>> float(cbor.loads('1bfffffffffffffbff'.decode('hex')))
1.844674407370955e+19
>>> float(cbor.loads('1bfffffffffffffc00'.decode('hex')))
1.8446744073709552e+19
*/
test('1bfffffffffffff7ff', 18446744073709550000);  // not exact: 0xfffffffffffff800
test('1bfffffffffffff800', 18446744073709550000);  // exact: 0xfffffffffffff800
test('1bfffffffffffffbff', 18446744073709550000);  // not exact: 0xfffffffffffff800
test('1bfffffffffffffc00', 18446744073709552000);  // not exact: 0x10000000000000000
test('1bfffffffffffffffe', 18446744073709552000);  // not exact: 0x10000000000000000
test('1bffffffffffffffff', 18446744073709552000);  // not exact: 0x10000000000000000
test('1c', ERROR);
test('1c00', ERROR);
test('1cff', ERROR);
test('1d', ERROR);
test('1d00', ERROR);
test('1dff', ERROR);
test('1e', ERROR);
test('1e00', ERROR);
test('1eff', ERROR);
test('1f', ERROR);
test('1f00', ERROR);
test('1fff', ERROR);

print('- Major type 0x20-0x3f: negative integer (not -0)');

test('20', -1);
test('21', -2);
test('22', -3);
test('23', -4);
test('24', -5);
test('25', -6);
test('26', -7);
test('27', -8);
test('28', -9);
test('29', -10);
test('2a', -11);
test('2b', -12);
test('2c', -13);
test('2d', -14);
test('2e', -15);
test('2f', -16);
test('30', -17);
test('31', -18);
test('32', -19);
test('33', -20);
test('34', -21);
test('35', -22);
test('36', -23);
test('37', -24);
test('38', ERROR);
test('3800', -1);
test('3801', -2);
test('3816', -23);
test('3817', -24);
test('3818', -25);
test('38fe', -255);
test('38ff', -256);
test('39', ERROR);
test('3911', ERROR);
test('390000', -1);
test('3900ff', -256);
test('390100', -257);
test('391234', -4661);
test('39fedc', -65245);
test('39ffff', -65536);
test('3a', ERROR);
test('3a11', ERROR);
test('3a1122', ERROR);
test('3a112233', ERROR);
test('3a00000000', -1);
test('3a000000ff', -256);
test('3a0000ffff', -65536);
test('3a00010000', -65537);
test('3a12345678', -305419897);
test('3afedcba98', -4275878553);
test('3affffffff', -4294967296);
test('3b', ERROR);
test('3b11', ERROR);
test('3b1122', ERROR);
test('3b112233', ERROR);
test('3b11223344', ERROR);
test('3b1122334455', ERROR);
test('3b112233445566', ERROR);
test('3b11223344556677', ERROR);
test('3b0000000000000000', -1);
test('3b00000000000000ff', -256);
test('3b000000000000ffff', -65536);
test('3b00000000ffffffff', -4294967296);
test('3b001ffffffffffffe', -9007199254740991);  // -2^53-1, exact
test('3b001fffffffffffff', -9007199254740992);  // -2^53, exact
test('3b0020000000000000', -9007199254740992);  // rounds towards zero
test('3b0020000000000001', -9007199254740994);  // 2^53+2, exact
test('3b0020000000000002', -9007199254740996);  // rounds towards -inf
test('3b0020000000000003', -9007199254740996);  // 2^53+4, exact
test('3b0020000000000004', -9007199254740996);  // rounds towards zero
test('3b0020000000000005', -9007199254740998);  // 2^53+6, exact
test('3b123456789abcdeef', -1311768467463790300);  // not exact: -0x123456789abcdf00
test('3bffeeddccbbaa9987', -18441921395520346000);
/*
>>> float(cbor.loads('3bfffffffffffff7fe'.decode('hex')))
-1.844674407370955e+19
>>> float(cbor.loads('3bfffffffffffff7ff'.decode('hex')))
-1.844674407370955e+19
>>> float(cbor.loads('3bfffffffffffff800'.decode('hex')))
-1.844674407370955e+19
>>> float(cbor.loads('3bfffffffffffffbfe'.decode('hex')))
-1.844674407370955e+19
>>> float(cbor.loads('3bfffffffffffffbff'.decode('hex')))
-1.8446744073709552e+19
>>> float(cbor.loads('3bfffffffffffffc00'.decode('hex')))
-1.8446744073709552e+19
*/
test('3bfffffffffffff7fe', -18446744073709550000);  // not exact
test('3bfffffffffffff7ff', -18446744073709550000);  // exact: -0xfffffffffffff800
test('3bfffffffffffff800', -18446744073709550000);  // not exact: -0xfffffffffffff800
test('3bfffffffffffffbfe', -18446744073709550000);  // not exact: -0xfffffffffffff800
test('3bfffffffffffffbff', -18446744073709552000);  // not exact: -0x10000000000000000
test('3bfffffffffffffc00', -18446744073709552000);  // not exact: -0x10000000000000000
test('3bfffffffffffffffe', -18446744073709552000);  // not exact: -0x10000000000000000
test('3bffffffffffffffff', -18446744073709552000);  // exact: -0x10000000000000000
test('3c', ERROR);
test('3c00', ERROR);
test('3cff', ERROR);
test('3d', ERROR);
test('3d00', ERROR);
test('3dff', ERROR);
test('3e', ERROR);
test('3e00', ERROR);
test('3eff', ERROR);
test('3f', ERROR);
test('3f00', ERROR);
test('3fff', ERROR);

print('- Major type 0x40-0x5f: byte string');

try {
    // Buffers decode as Uint8Arrays, matches cbor-js.
    (function () {
        var res = CBOR.decode(Duktape.dec('hex', '43112233').buffer);
        print('typeof:', typeof res);
        print('instanceof ByteArray:', res instanceof ArrayBuffer);
        print('instanceof Uint8Array:', res instanceof Uint8Array);
        print('proto is Uint8Array.prototype:', Object.getPrototypeOf(res) === Uint8Array.prototype);
        print('is bare:', Object.getPrototypeOf(res) === null);
    })();
} catch (e) {
    print(e.stack || e);
}

function mkbuf(hexData) {
    return Duktape.dec('hex', hexData).buffer;
}

test('40', new Uint8Array([]));
test('4111', new Uint8Array([ 0x11 ]));
test('421122', new Uint8Array([ 0x11, 0x22 ]));
test('43112233', new Uint8Array([ 0x11, 0x22, 0x33 ]));
test('4411223344', new Uint8Array([ 0x11, 0x22, 0x33, 0x44 ]));
test('451122334455', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55 ]));
test('46112233445566', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 ]));
test('4711223344556677', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 ]));
test('47666f6f00666f6f', new Uint8Array([ 0x66, 0x6f, 0x6f, 0x00, 0x66, 0x6f, 0x6f ]));
test('481122334455667788', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 ]));
test('49112233445566778899', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 ]));
test('4a112233445566778899aa', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa ]));
test('4b112233445566778899aabb', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb ]));
test('4c112233445566778899aabbcc', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc ]));
test('4d112233445566778899aabbccdd', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd ]));
test('4e112233445566778899aabbccddee', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee ]));
test('4f112233445566778899aabbccddeeff', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff ]));
test('50112233445566778899aabbccddeeff00', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00 ]));
test('51112233445566778899aabbccddeeff0011', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11 ]));
test('52112233445566778899aabbccddeeff001122', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22 ]));
test('53112233445566778899aabbccddeeff00112233', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33 ]));
test('54112233445566778899aabbccddeeff0011223344', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44 ]));
test('55112233445566778899aabbccddeeff001122334455', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 ]));
test('56112233445566778899aabbccddeeff00112233445566', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 ]));
test('57112233445566778899aabbccddeeff0011223344556677', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 ]));
test('58', ERROR);
test('5800', new Uint8Array([]));
test('5801ff', new Uint8Array([ 0xff ]));
test('5817112233445566778899aabbccddeeff0011223344556677', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 ]));
test('5818112233445566778899aabbccddeeff001122334455667788', new Uint8Array([ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 ]));
test('58ff' + 'fe'.repeat(255), mkbuf('fe'.repeat(255)));
test('59', ERROR);
test('5900', ERROR);
test('590000', new Uint8Array([]));
test('590001ff', new Uint8Array([ 0xff ]));
test('5900ff' + 'fe'.repeat(255), mkbuf('fe'.repeat(255)));
test('590100' + 'fe'.repeat(256), mkbuf('fe'.repeat(256)));
test('59fffe' + 'fe'.repeat(65534), mkbuf('fe'.repeat(65534)));
test('59ffff' + 'fe'.repeat(65535), mkbuf('fe'.repeat(65535)));
test('5a', ERROR);
test('5a00', ERROR);
test('5a0000', ERROR);
test('5a000000', ERROR);
test('5a00000000', new Uint8Array([]));
test('5a00000001ff', new Uint8Array([ 0xff ]));
test('5a000000ff' + 'fe'.repeat(255), mkbuf('fe'.repeat(255)));
test('5a0000fffe' + 'fe'.repeat(65534), mkbuf('fe'.repeat(65534)));
test('5a0000ffff' + 'fe'.repeat(65535), mkbuf('fe'.repeat(65535)));
// We'd like to test up to 4G but it's better to do as a separate test.
test('5a01000000' + 'fe'.repeat(0x1000000), mkbuf('fe'.repeat(0x1000000)));
test('5b', ERROR);
test('5b00', ERROR);
test('5b0000', ERROR);
test('5b000000', ERROR);
test('5b00000000', ERROR);
test('5b0000000000', ERROR);
test('5b000000000000', ERROR);
test('5b00000000000000', ERROR);
test('5b0000000000000000', new Uint8Array([]));
test('5b0000000000000001ff', new Uint8Array([ 0xff ]));
test('5b00000000000000ff' + 'fe'.repeat(255), mkbuf('fe'.repeat(255)));
test('5b000000000000fffe' + 'fe'.repeat(65534), mkbuf('fe'.repeat(65534)));
test('5b000000000000ffff' + 'fe'.repeat(65535), mkbuf('fe'.repeat(65535)));
test('5b0000000001000000' + 'fe'.repeat(0x1000000), mkbuf('fe'.repeat(0x1000000)));
test('5c', ERROR);
test('5c00', ERROR);
test('5cff', ERROR);
test('5d', ERROR);
test('5d00', ERROR);
test('5dff', ERROR);
test('5e', ERROR);
test('5e00', ERROR);
test('5eff', ERROR);
test('5f', ERROR);
test('5f00', ERROR);
test('5f00ff', ERROR);
test('5f01ff', ERROR);
test('5f20ff', ERROR);
test('5f21ff', ERROR);
test('5f40ff', new Uint8Array([]));
test('5f41abff', new Uint8Array([ 0xab ]));
test('5f' + '40' + '40' + '4111' + '5830' + '22'.repeat(48) + '40' + '590400' + '33'.repeat(1024) + '40' +
     '5a00012345' + '44'.repeat(0x12345) + '5b0000000000023456' + '55'.repeat(0x23456) + 'ff',
     mkbuf('11' + '22'.repeat(48) + '33'.repeat(1024) + '44'.repeat(0x12345) + '55'.repeat(0x23456)));
test('5f' + '40'.repeat(10000) + '43112233' + '40'.repeat(10000) + 'ff', mkbuf('112233'));
test('5f' + '43112233' + '6141' + 'ff', ERROR);
test('5f5fffff', ERROR);  // nested indefinite length, prohibited
test('5f60ff', ERROR);
test('5f61ff', ERROR);
test('5f80ff', ERROR);
test('5f81ff', ERROR);
test('5fa0ff', ERROR);
test('5fa1ff', ERROR);
test('5fc0ff', ERROR);
test('5fc1ff', ERROR);
test('5fe0ff', ERROR);
test('5fe1ff', ERROR);
test('5fff', new Uint8Array([]));
test('5fffff', ERROR);

print('- Major type 0x60-0x7f: text string');

test('60', '');
test('6141', 'A');
test('624142', 'AB');
test('62dfbe', '\u07fe');  // two-byte UTF-8
test('63dfbe41', '\u07feA');
test('6441dfbe42', 'A\u07feB');
test('6541efbb9c42', 'A\ufedcB');  // three-byte UTF-8
test('6641efbb9c4243', 'A\ufedcBC');
test('6741efbb9c424344', 'A\ufedcBCD');
test('6841efbb9c42434445', 'A\ufedcBCDE');
test('6941efbb9c4243444546', 'A\ufedcBCDEF');
test('6a41efbb9c424344454647', 'A\ufedcBCDEFG');
test('6b41efbb9c42434445464748', 'A\ufedcBCDEFGH');
test('6c41efbb9c4243444546474849', 'A\ufedcBCDEFGHI');
test('6d41efbb9c42434445464748494a', 'A\ufedcBCDEFGHIJ');
test('6e41efbb9c42434445464748494a4b', 'A\ufedcBCDEFGHIJK');
test('6f41efbb9c42434445464748494a4b4c', 'A\ufedcBCDEFGHIJKL');
test('7041efbb9c42434445464748494a4b4c4d', 'A\ufedcBCDEFGHIJKLM');
test('7141efbb9c42434445464748494a4b4c4d4e', 'A\ufedcBCDEFGHIJKLMN');
test('7241efbb9c42434445464748494a4b4c4d4e4f', 'A\ufedcBCDEFGHIJKLMNO');
test('7341efbb9c42434445464748494a4b4c4d4e4f50', 'A\ufedcBCDEFGHIJKLMNOP');
test('7441efbb9c42434445464748494a4b4c4d4e4f5051', 'A\ufedcBCDEFGHIJKLMNOPQ');
test('7541efbb9c42434445464748494a4b4c4d4e4f505152', 'A\ufedcBCDEFGHIJKLMNOPQR');
test('7641efbb9c42434445464748494a4b4c4d4e4f50515253', 'A\ufedcBCDEFGHIJKLMNOPQRS');
test('7741efbb9c42434445464748494a4b4c4d4e4f5051525354', 'A\ufedcBCDEFGHIJKLMNOPQRST');
test('78', ERROR);
test('7800', '');
test('78026162', 'ab');
test('781741efbb9c42434445464748494a4b4c4d4e4f5051525354', 'A\ufedcBCDEFGHIJKLMNOPQRST');
test('7818' + '41'.repeat(24), 'A'.repeat(24));
test('78ff' + '41'.repeat(255), 'A'.repeat(255));
test('79', ERROR);
test('7900', ERROR);
test('790000', '');
test('79000141', 'A');
test('7900ff' + '41'.repeat(255), 'A'.repeat(255));
test('790100' + '41'.repeat(256), 'A'.repeat(256));
test('79fffe' + '41'.repeat(65534), 'A'.repeat(65534));
test('79ffff' + '41'.repeat(65535), 'A'.repeat(65535));
test('7a', ERROR);
test('7a00', ERROR);
test('7a0000', ERROR);
test('7a000000', ERROR);
test('7a00000000', '');
test('7a0000000141', 'A');
test('7a000000ff' + '41'.repeat(255), 'A'.repeat(255));
test('7a0000fffe' + '41'.repeat(65534), 'A'.repeat(65534));
test('7a0000ffff' + '41'.repeat(65535), 'A'.repeat(65535));
test('7a01000000' + '41'.repeat(0x1000000), 'A'.repeat(0x1000000));
test('7b', ERROR);
test('7b00', ERROR);
test('7b0000', ERROR);
test('7b000000', ERROR);
test('7b00000000', ERROR);
test('7b0000000000', ERROR);
test('7b000000000000', ERROR);
test('7b00000000000000', ERROR);
test('7b0000000000000000', '');
test('7b000000000000000141', 'A');
test('7b00000000000000ff' + '41'.repeat(255), 'A'.repeat(255));
test('7b000000000000fffe' + '41'.repeat(65534), 'A'.repeat(65534));
test('7b000000000000ffff' + '41'.repeat(65535), 'A'.repeat(65535));
test('7b0000000001000000' + '41'.repeat(0x1000000), 'A'.repeat(0x1000000));
test('7c', ERROR);
test('7c00', ERROR);
test('7cff', ERROR);
test('7d', ERROR);
test('7d00', ERROR);
test('7dff', ERROR);
test('7e', ERROR);
test('7e00', ERROR);
test('7eff', ERROR);
test('7f', ERROR);
test('7f00', ERROR);
test('7f00ff', ERROR);
test('7f01ff', ERROR);
test('7f20ff', ERROR);
test('7f21ff', ERROR);
test('7f40ff', ERROR);
test('7f41ff', ERROR);
test('7f60ff', '');
test('7f6141ff', 'A');
test('7f' + '60' + '60' + '6142' + '7830' + '43'.repeat(48) + '60' + '790400' + '44'.repeat(1024) + '60' +
     '7a00012345' + '45'.repeat(0x12345) + '7b0000000000023456' + '46'.repeat(0x23456) + 'ff',
     'B' + 'C'.repeat(48) + 'D'.repeat(1024) + 'E'.repeat(0x12345) + 'F'.repeat(0x23456));
test('7f' + '60'.repeat(10000) + '63414243' + '60'.repeat(10000) + 'ff', 'ABC');
test('7f' + '63414243' + '4141' + 'ff', ERROR);
test('7f7fffff', ERROR);  // nested indefinite length, prohibited
test('7f80ff', ERROR);
test('7f81ff', ERROR);
test('7fa0ff', ERROR);
test('7fa1ff', ERROR);
test('7fc0ff', ERROR);
test('7fc1ff', ERROR);
test('7fe0ff', ERROR);
test('7fe1ff', ERROR);
test('7fff', '');
test('7fffff', ERROR);
// Non-BMP and non-UTF-8 tests separately.

print('- Major type 0x80-0x9f: array');

try {
    (function () {
        var res = CBOR.decode(Duktape.dec('hex', '83010203').buffer);
        print('typeof:', typeof res);
        print('instanceof Array:', res instanceof Array);
        print('Array.isArray:', Array.isArray(res));
        print('proto is Array.prototype:', Object.getPrototypeOf(res) === Array.prototype);
        print('is bare:', Object.getPrototypeOf(res) === null);
    })();
} catch (e) {
    print(e.stack || e);
}

function mkarrcbor(size) {
    var res = [];
    for (var i = 0; i < size; i++) {
        var t1 = 'foo' + String(i);
        var t1_cbor = (0x60 + t1.length).toString(16) + Duktape.enc('hex', t1);
        res.push(t1_cbor);
    }
    return res.join('');
}

function mkarr(size) {
    var res = [];
    for (var i = 0; i < size; i++) {
        var t1 = 'foo' + String(i);
        res.push(t1);
    }
    return res;
}

test('80', []);
test('8100', [0]);
test('8163414243', [ 'ABC' ]);
test('820001', [0,1]);
test('83000102', [0,1,2]);
test('8363666f6f607803626172', [ 'foo', '', 'bar' ]);
test('83820102810380', [ [1, 2], [3], [] ]);
test('8400010203', [0,1,2,3]);
test('850001020304', [0,1,2,3,4]);
test('86000102030405', [0,1,2,3,4,5]);
test('8700010203040506', [0,1,2,3,4,5,6]);
test('880001020304050607', [0,1,2,3,4,5,6,7]);
test('89000102030405060708', [0,1,2,3,4,5,6,7,8]);
test('8a00010203040506070809', [0,1,2,3,4,5,6,7,8,9]);
test('8b000102030405060708090a', [0,1,2,3,4,5,6,7,8,9,10]);
test('8c000102030405060708090a0b', [0,1,2,3,4,5,6,7,8,9,10,11]);
test('8d000102030405060708090a0b0c', [0,1,2,3,4,5,6,7,8,9,10,11,12]);
test('8e000102030405060708090a0b0c0d', [0,1,2,3,4,5,6,7,8,9,10,11,12,13]);
test('8f000102030405060708090a0b0c0d0e', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14]);
test('90000102030405060708090a0b0c0d0e0f', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]);
test('91000102030405060708090a0b0c0d0e0f10', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]);
test('92000102030405060708090a0b0c0d0e0f1011', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]);
test('93000102030405060708090a0b0c0d0e0f101112', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18]);
test('94000102030405060708090a0b0c0d0e0f10111213', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19]);
test('95000102030405060708090a0b0c0d0e0f1011121314', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]);
test('96000102030405060708090a0b0c0d0e0f101112131415', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21]);
test('97000102030405060708090a0b0c0d0e0f10111213141516', [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22]);
test('98', ERROR);
test('9800', []);
test('980163666f6f', [ 'foo' ]);
test('9803' + mkarrcbor(3), mkarr(3));
test('9880' + mkarrcbor(128), mkarr(128));
test('98ff' + mkarrcbor(255), mkarr(255));
test('99', ERROR);
test('9900', ERROR);
test('990000', []);
test('9900ff' + mkarrcbor(255), mkarr(255));
test('99fedc' + mkarrcbor(0xfedc), mkarr(0xfedc));
test('99ffff' + mkarrcbor(65535), mkarr(65535));
test('9a', ERROR);
test('9a00', ERROR);
test('9a0000', ERROR);
test('9a000000', ERROR);
test('9a00000000', []);
test('9a000000ff' + mkarrcbor(255), mkarr(255));
test('9a0000fedc' + mkarrcbor(0xfedc), mkarr(0xfedc));
test('9a0000ffff' + mkarrcbor(65535), mkarr(65535));
test('9a00023456' + mkarrcbor(0x23456), mkarr(0x23456));
test('9b', ERROR);
test('9b00', ERROR);
test('9b0000', ERROR);
test('9b000000', ERROR);
test('9b00000000', ERROR);
test('9b0000000000', ERROR);
test('9b000000000000', ERROR);
test('9b00000000000000', ERROR);
test('9b0000000000000000', []);
test('9b00000000000000ff' + mkarrcbor(255), mkarr(255));
test('9b000000000000fedc' + mkarrcbor(0xfedc), mkarr(0xfedc));
test('9b000000000000ffff' + mkarrcbor(65535), mkarr(65535));
test('9b0000000000023456' + mkarrcbor(0x23456), mkarr(0x23456));
test('9c', ERROR);
test('9c00', ERROR);
test('9cff', ERROR);
test('9d', ERROR);
test('9d00', ERROR);
test('9dff', ERROR);
test('9e', ERROR);
test('9e00', ERROR);
test('9eff', ERROR);
test('9f', ERROR);
test('9f00', ERROR);
test('9fff', []);
test('9fffff', ERROR);

print('- Major type 0xa0-0xbf: map');

try {
    (function () {
        var res = CBOR.decode(Duktape.dec('hex', 'a3010203040506').buffer);
        print('typeof:', typeof res);
        print('instanceof Object:', res instanceof Object);
        print('proto is Object.prototype:', Object.getPrototypeOf(res) === Object.prototype);
        print('is bare:', Object.getPrototypeOf(res) === null);
    })();
} catch (e) {
    print(e.stack || e);
}

function mkobjcbor(size) {
    var res = [];
    for (var i = 0; i < size; i++) {
        var t1 = 'foo' + String(i);
        var t1_cbor = (0x60 + t1.length).toString(16) + Duktape.enc('hex', t1);
        var t2 = 'bar' + String(i);
        var t2_cbor = (0x60 + t2.length).toString(16) + Duktape.enc('hex', t2);
        res.push(t1_cbor + t2_cbor);
    }
    return res.join('');
}

function mkobj(size) {
    var res = {};
    for (var i = 0; i < size; i++) {
        var t1 = 'foo' + String(i);
        var t2 = 'bar' + String(i);
        res[t1] = t2;
    }
    return res;
}

test('a0', {});
test('a10001', {0:1});
test('a16341424363434241', {ABC:'CBA'});
test('a200010203', {0:1,2:3});
test('a3000102030405', {0:1,2:3,4:5});
test('a36141a2010203046142a105066143a0', { A: { 1:2, 3:4 }, B: { 5:6 }, C: {} });
test('a40001020304050607', {0:1,2:3,4:5,6:7});
test('a500010203040506070809', {0:1,2:3,4:5,6:7,8:9});
test('a6000102030405060708090a0b', {0:1,2:3,4:5,6:7,8:9,10:11});
test('a7000102030405060708090a0b0c0d', {0:1,2:3,4:5,6:7,8:9,10:11,12:13});
test('a8000102030405060708090a0b0c0d0e0f', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15});
test('a9000102030405060708090a0b0c0d0e0f1011', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17});
test('aa000102030405060708090a0b0c0d0e0f10111213', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19});
test('ab000102030405060708090a0b0c0d0e0f101112131415', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21});
test('ac000102030405060708090a0b0c0d0e0f1011121314151617', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23});
test('ad000102030405060708090a0b0c0d0e0f101112131415161718181819', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25});
test('ae000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27});
test('af000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29});
test('b0000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31});
test('b1000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f18201821', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31,32:33});
test('b2000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f1820182118221823', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31,32:33,34:35});
test('b3000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f182018211822182318241825', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31,32:33,34:35,36:37});
test('b4000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f18201821182218231824182518261827', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31,32:33,34:35,36:37,38:39});
test('b5000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f1820182118221823182418251826182718281829', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31,32:33,34:35,36:37,38:39,40:41});
test('b6000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f1820182118221823182418251826182718281829182a182b', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31,32:33,34:35,36:37,38:39,40:41,42:43});
test('b7000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f1820182118221823182418251826182718281829182a182b182c182d', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31,32:33,34:35,36:37,38:39,40:41,42:43,44:45});
test('b8', ERROR);
test('b800', {});
test('b80161416142', {A:'B'});
test('b818000102030405060708090a0b0c0d0e0f101112131415161718181819181a181b181c181d181e181f1820182118221823182418251826182718281829182a182b182c182d182e182f', {0:1,2:3,4:5,6:7,8:9,10:11,12:13,14:15,16:17,18:19,20:21,22:23,24:25,26:27,28:29,30:31,32:33,34:35,36:37,38:39,40:41,42:43,44:45,46:47});
test('b803' + mkobjcbor(3), mkobj(3));
test('b880' + mkobjcbor(128), mkobj(128));
test('b8ff' + mkobjcbor(255), mkobj(255));
test('b9', ERROR);
test('b900', ERROR);
test('b90000', {});
test('b900011011', {16:17});
test('b900ff' + mkobjcbor(255), mkobj(255));
test('b9fedc' + mkobjcbor(0xfedc), mkobj(0xfedc));
test('b9ffff' + mkobjcbor(65535), mkobj(65535));
test('ba', ERROR);
test('ba00', ERROR);
test('ba0000', ERROR);
test('ba000000', ERROR);
test('ba00000000', {});
test('ba000000ff' + mkobjcbor(255), mkobj(255));
test('ba0000fedc' + mkobjcbor(0xfedc), mkobj(0xfedc));
test('ba0000ffff' + mkobjcbor(65535), mkobj(65535));
test('ba00023456' + mkobjcbor(0x23456), mkobj(0x23456));
test('bb', ERROR);
test('bb00', ERROR);
test('bb0000', ERROR);
test('bb000000', ERROR);
test('bb00000000', ERROR);
test('bb0000000000', ERROR);
test('bb000000000000', ERROR);
test('bb00000000000000', ERROR);
test('bb0000000000000000', {});
test('bb00000000000000ff' + mkobjcbor(255), mkobj(255));
test('bb000000000000fedc' + mkobjcbor(0xfedc), mkobj(0xfedc));
test('bb000000000000ffff' + mkobjcbor(65535), mkobj(65535));
test('bb0000000000023456' + mkobjcbor(0x23456), mkobj(0x23456));
test('bc', ERROR);
test('bc00', ERROR);
test('bcff', ERROR);
test('bd', ERROR);
test('bd00', ERROR);
test('bdff', ERROR);
test('be', ERROR);
test('be00', ERROR);
test('beff', ERROR);
test('bf', ERROR);
test('bf00', ERROR);
test('bfff', {});
test('bfffff', ERROR);

print('- Major type 0xc0-0xdf: tag');

test('c0', ERROR);  // tag alone is not valid
test('c012', 18);
test('c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c012', 18);  // multiple tags are fine
test('c0c1c2c3c4c5c6c7c8c9cacbcccdcecdcf' +
     'd0d1d2d3d4d5d6d7' + 'd812d91234da12345678db123456789abcdef0' + '12', 18);
test('c112', 18);
test('c212', 18);
test('c312', 18);
test('c412', 18);
test('c512', 18);
test('c612', 18);
test('c712', 18);
test('c812', 18);
test('c912', 18);
test('ca12', 18);
test('cb12', 18);
test('cc12', 18);
test('cd12', 18);
test('ce12', 18);
test('cf12', 18);
test('d012', 18);
test('d112', 18);
test('d212', 18);
test('d312', 18);
test('d412', 18);
test('d512', 18);
test('d612', 18);
test('d712', 18);
test('d8', ERROR);
test('d800', ERROR);
test('d80012', 18);
test('d8dc'.repeat(100000) + '18fe', 254);
test('d9', ERROR);
test('d900', ERROR);
test('d90000', ERROR);
test('d9000012', 18);
test('da', ERROR);
test('da00', ERROR);
test('da0000', ERROR);
test('da000000', ERROR);
test('da00000000', ERROR);
test('da0000000012', 18);
test('db', ERROR);
test('db00', ERROR);
test('db0000', ERROR);
test('db000000', ERROR);
test('db00000000', ERROR);
test('db0000000000', ERROR);
test('db000000000000', ERROR);
test('db00000000000000', ERROR);
test('db0000000000000000', ERROR);
test('db000000000000000012', 18);
test('dc', ERROR);
test('dd', ERROR);
test('de', ERROR);
test('df', ERROR);
test('df00', ERROR);
test('dfff', ERROR);
test('dfffff', ERROR);

print('- Major type 0xe0-0xff: simple types, floating point numbers, break, etc');

// 0xe0-0xf3: unassigned
test('e0', ERROR);
test('e1', ERROR);
test('e2', ERROR);
test('e3', ERROR);
test('e4', ERROR);
test('e5', ERROR);
test('e6', ERROR);
test('e7', ERROR);
test('e8', ERROR);
test('e9', ERROR);
test('ea', ERROR);
test('eb', ERROR);
test('ec', ERROR);
test('ed', ERROR);
test('ee', ERROR);
test('ef', ERROR);
test('f0', ERROR);
test('f1', ERROR);
test('f2', ERROR);
test('f3', ERROR);
test('f4', false);
test('f5', true);
test('f6', null);  // CBOR null
test('f7', void 0);  // CBOR undefined
// 0xf8: simple value, one byte follows, bytes 0-31 not allowed.
// See RFC 7049, Section 2.3.  However, some bindings will decode
// such simple types, e.g. f815 as true.
test('f8', ERROR);
test('f800', ERROR);
test('f801', ERROR);
test('f802', ERROR);
test('f803', ERROR);
test('f804', ERROR);
test('f805', ERROR);
test('f806', ERROR);
test('f807', ERROR);
test('f808', ERROR);
test('f809', ERROR);
test('f80a', ERROR);
test('f80b', ERROR);
test('f80c', ERROR);
test('f80d', ERROR);
test('f80e', ERROR);
test('f80f', ERROR);
test('f810', ERROR);
test('f811', ERROR);
test('f812', ERROR);
test('f813', ERROR);
test('f814', ERROR);
test('f815', ERROR);
test('f816', ERROR);
test('f817', ERROR);
test('f818', ERROR);
test('f819', ERROR);
test('f81a', ERROR);
test('f81b', ERROR);
test('f81c', ERROR);
test('f81d', ERROR);
test('f81e', ERROR);
test('f81f', ERROR);
// 0xf9: half-float
/*
>>> cbor.loads('f90000'.decode('hex'))
0.0
>>> cbor.loads('f90001'.decode('hex'))
5.960464477539063e-08
>>> cbor.loads('f9c400'.decode('hex'))
-4.0
>>> cbor.loads('f97c00'.decode('hex'))
inf
>>> cbor.loads('f97c01'.decode('hex'))
nan
>>> cbor.loads('f97fff'.decode('hex'))
nan
>>> cbor.loads('f98000'.decode('hex'))
-0.0
>>> cbor.loads('f98001'.decode('hex'))
-5.960464477539063e-08
>>> cbor.loads('f9fc00'.decode('hex'))
-inf
>>> cbor.loads('f9fc01'.decode('hex'))
nan
>>> cbor.loads('f9ffff'.decode('hex'))
nan
*/
test('f90000', 0);
test('f90001', 5.960464477539063e-08);
test('f9c400', -4);
test('f97c00', 1 / 0);
test('f97c01', 0 / 0);
test('f97fff', 0 / 0);
test('f98000', -0);
test('f98001', -5.960464477539063e-08);
test('f9fc00', -1 / 0);
test('f9fc01', 0 / 0);
test('f9ffff', 0 / 0);
// 0xfa: single
/*
>>> cbor.loads('fa00000000'.decode('hex'))
0.0
>>> cbor.loads('fa00000001'.decode('hex'))
1.401298464324817e-45
>>> cbor.loads('fa7f800000'.decode('hex'))
inf
>>> cbor.loads('fa7f800001'.decode('hex'))
nan
>>> cbor.loads('fa7fffffff'.decode('hex'))
nan
>>> cbor.loads('fa80000000'.decode('hex'))
-0.0
>>> cbor.loads('fa80000001'.decode('hex'))
-1.401298464324817e-45
>>> cbor.loads('faff800000'.decode('hex'))
-inf
>>> cbor.loads('faff800001'.decode('hex'))
nan
>>> cbor.loads('faffffffff'.decode('hex'))
nan
*/
test('fa00000000', 0);
test('fa00000001', 1.401298464324817e-45);
test('fa47c35000', 100000);
test('fa7f800000', 1 / 0);
test('fa7f800001', 0 / 0);
test('fa7fffffff', 0 / 0);
test('fa80000000', -0);
test('fa80000001', -1.401298464324817e-45);
test('faff800000', -1 / 0);
test('faff800001', 0 / 0);
test('fa7fffffff', 0 / 0);
// 0xfb: double
/*
>>> struct.pack('>d', math.pi).encode('hex')
'400921fb54442d18'
*/
/*
>>> cbor.loads('fb0000000000000000'.decode('hex'))
0.0
>>> cbor.loads('fb0000000000000001'.decode('hex'))
5e-324
>>> cbor.loads('fb7ff0000000000000'.decode('hex'))
inf
>>> cbor.loads('fb7ff0000000000001'.decode('hex'))
nan
>>> cbor.loads('fb7fffffffffffffff'.decode('hex'))
nan
>>> cbor.loads('fb8000000000000000'.decode('hex'))
-0.0
>>> cbor.loads('fb8000000000000001'.decode('hex'))
-5e-324
>>> cbor.loads('fbfff0000000000000'.decode('hex'))
-inf
>>> cbor.loads('fbfff0000000000001'.decode('hex'))
nan
>>> cbor.loads('fbffffffffffffffff'.decode('hex'))
nan
*/
test('fb0000000000000000', 0);
test('fb0000000000000001', 5e-324);
test('fb400921fb54442d18', Math.PI);
test('fb7ff0000000000000', 1 / 0);
test('fb7ff0000000000001', 0 / 0);
test('fb7fffffffffffffff', 0 / 0);
test('fb8000000000000000', -0);
test('fb8000000000000001', -5e-324);
test('fbc010666666666666', -4.1);
test('fbfff0000000000000', -1 / 0);
test('fbfff0000000000001', 0 / 0);
test('fbffffffffffffffff', 0 / 0);
test('fc', ERROR);  // unassigned
test('fd', ERROR);  // unassigned
test('fe', ERROR);  // unassigned
// 0xff: break
test('ff', ERROR);  // break, out of context
test('ff00', ERROR);
test('ffff', ERROR);
test('ffffff', ERROR);

print('done');
