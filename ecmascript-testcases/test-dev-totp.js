/*
 *  Standalone TOTP implementation in Ecmascript.
 *
 *  http://tools.ietf.org/html/rfc6238
 *  http://tools.ietf.org/html/rfc4226
 *  http://tools.ietf.org/html/rfc3174
 *  http://tools.ietf.org/html/rfc2104
 *  http://tools.ietf.org/html/rfc3548
 *
 *  ">>> 0" is used to coerce result to unsigned 32 bits.
 *
 *  Exercises bit arithmetic.
 */

var TOTP = {};
TOTP.nybbles = "0123456789abcdef";
TOTP.eightByteDivisors = [ 72057594037927936, 281474976710656, 1099511627776,
                           4294967296, 16777216, 65536, 256, 1 ];
TOTP.fiveByteDivisors = [ 4294967296, 16777216, 65536, 256, 1 ];
TOTP.stringToBytes = function (val) {
    var i, n, res;
    for (i = 0, n = val.length, res = []; i < n; i++) {
        res.push(val.charCodeAt(i) & 0xff);
    }
    return res;
};
TOTP.bytesToString = function (val) {
    return String.fromCharCode.apply(String, val);
};
TOTP.hexEncode = function (val) {
    var i, n, t, res, nybbles = TOTP.nybbles;
    for (i = 0, n = val.length, res = ''; i < n; i++) {
        t = (val.charCodeAt(i) & 0xff);
        res += nybbles[t >> 4] + nybbles[t & 0x0f];
    }
    return res;
};
TOTP.base32Decode = function (val) {
    // The authenticators given are not direct inputs to TOTP, but are base-32 encoded.
    // http://stackoverflow.com/questions/8529265/google-authenticator-implementation-in-python
    // https://docs.python.org/3/library/base64.html#base64.b32decode
    var i, j, n, x, t, npad, res, nstrip;
    while ((val.length % 8) !== 0) { val += '='; }  // tolerate missing pad
    for (i = 0, n = val.length, t = 0, npad = 0, res = ''; i < n; i++) {
        x = val.charCodeAt(i);
        if (x >= 0x41 && x <= 0x5a) { t = t * 32 + (x - 0x41); }
        else if (x >= 0x61 && x <= 0x7a) { t = t * 32 + (x - 0x61); }
        else if (x >= 0x32 && x <= 0x37) { t = t * 32 + (x - 0x32 + 26); }
        else if (x == 0x3d /* = */) { t = t * 32; npad++; }
        else { throw new Error('invalid base32'); }
        if ((i % 8) == 7) {
            TOTP.fiveByteDivisors.forEach(function (divisor) {
                res += String.fromCharCode((t / divisor) & 0xff);
            });
            t = 0;
        }
    }
    nstrip = [ 0, 1, undefined, 2, 3, undefined, 4][npad];
    if (nstrip === undefined) { throw new Error('invalid padding'); }
    return res.substring(0, res.length - nstrip);
};
TOTP.sha1Raw = function (words) {
    var w0_idx, w;
    var h0, h1, h2, h3, h4, a, b, c, d, e;
    var t, temp;
    var Kval = [ 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 ];
    var res;

    function S(x, n) { return ((x << n) | (x >>> (32 - n))) >>> 0; }
    function K(t) { return Kval[Math.floor(t / 20)]; }
    function f(t, b, c, d) {
        if (t < 20) { return ((b & c) | ((~b) & d)) >>> 0; }
        else if (t < 40) { return (b ^ c ^ d) >>> 0; }
        else if (t < 60) { return ((b & c) | (b & d) | (c & d)) >>> 0; }
        else { return (b ^ c ^ d) >>> 0; }
    }

    // 'words' is an array of 32-bit values, padded to a multiple of 16 entries
    if ((words.length % 16) !== 0) {
        throw new Error('words length must be a multiple of 16');
    }

    h0 = 0x67452301; h1 = 0xefcdab89; h2 = 0x98badcfe; h3 = 0x10325476; h4 = 0xc3d2e1f0;

    for (w0_idx = 0; w0_idx < words.length; w0_idx += 16) {
        for (w = [], t = 0; t < 16; t++) { w[t] = words[w0_idx + t]; }
        for (t = 16; t < 80; t++) {
            w[t] = S((w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]) >>> 0, 1);
        }
        a = h0; b = h1; c = h2; d = h3; e = h4;
        for (t = 0; t < 80; t++) {
            temp = (S(a, 5) + f(t, b, c, d) + e + w[t] + K(t)) >>> 0;
            e = d; d = c; c = S(b, 30); b = a; a = temp;
        }
        h0 = (h0 + a) >>> 0; h1 = (h1 + b) >>> 0; h2 = (h2 + c) >>> 0;
        h3 = (h3 + d) >>> 0; h4 = (h4 + e) >>> 0;
    }

    res = ''; [ h0, h1, h2, h3, h4 ].forEach(function (v) {
        for (var i = 24; i >= 0; i -= 8) { res += String.fromCharCode((v >> i) & 0xff); }
    });
    return res;
};
TOTP.sha1 = function (str) {
    var bytes, words, i, n;
    var bitlen = str.length * 8;  // assume representable with 53 bits
    bytes = TOTP.stringToBytes(str);
    bytes.push(0x80);
    while (bytes.length % 64 !== 56) { bytes.push(0x00); }
    TOTP.eightByteDivisors.forEach(function (divisor) {
        bytes.push((bitlen / divisor) & 0xff);
    });
    //print('sha1 bytes', TOTP.hexEncode(TOTP.bytesToString(bytes)));
    for (i = 0, n = bytes.length, words = []; i < n; i += 4) {
        words[i / 4] = ((bytes[i] << 24) | (bytes[i + 1] << 16) |
                        (bytes[i + 2] << 8) | bytes[i + 3]) >>> 0;
    }
    //words.forEach(function (v) { print((v >>> 0).toString(16)); });
    return TOTP.sha1Raw(words);
};
TOTP.hmacSha1 = function (key, text) {
    var L = 20, B = 64;
    var i, n, t1, t2;
    if (key.length > B) { throw new Error('key too long (key hashing not supported)'); }
    for (i = 0, n = key.length, t1 = '', t2 = ''; i < B; i++) {
        t1 += String.fromCharCode(((key.charCodeAt(i) || 0) ^ 0x36) & 0xff);
        t2 += String.fromCharCode(((key.charCodeAt(i) || 0) ^ 0x5c) & 0xff);
    }
    return TOTP.sha1(t2 + TOTP.sha1(t1 + text));
};
TOTP.HOTP = function (K, C, digits) {
    var Cstr = '';
    var HS, off, i, t, res;
    TOTP.eightByteDivisors.forEach(function (divisor) {
        Cstr += String.fromCharCode((C / divisor) & 0xff);
    });
    HS = TOTP.hmacSha1(K, Cstr);
    off = HS.charCodeAt(19) & 0x0f;
    for (i = off, t = 0; i < off + 4; i++) {
        t = (t << 8) + (HS.charCodeAt(i) & 0xff);
    }
    t &= 0x7fffffff;
    res = '';
    while (digits-- > 0) { res = Math.floor(t % 10) + res; t = Math.floor(t / 10); }
    return res;
};
TOTP.TOTP = function (K, time, digits) {
    // Assume T0 = 0 (Unix epoch), X = 30 seconds, time is in milliseconds
    var T = Math.floor(time / 30e3);
    return TOTP.HOTP(K, T, digits);
};

/*===
sha1
da39a3ee5e6b4b0d3255bfef95601890afd80709
8843d7f92416211de9ebb963ff4ce28125932878
hmac-sha1
effcdf6ae5eb2fa2d27416d5f184df9c259a7c79
hotp
755224
287082
359152
969429
338314
254676
287922
162583
399871
520489
totp
94287082
07081804
14050471
65353130
base32
f
f
fo
fo
foo
foo
foo 
foo 
foo b
foo bar
foo bar
===*/

function test() {
    var i;

    print('sha1');
    print(TOTP.hexEncode(TOTP.sha1('')));
    print(TOTP.hexEncode(TOTP.sha1('foobar')));

    print('hmac-sha1');
    print(TOTP.hexEncode(TOTP.hmacSha1('Jefe', 'what do ya want for nothing?')));

    print('hotp');
    for (i = 0; i < 10; i++) {
        print(TOTP.HOTP('12345678901234567890', i, 6));
    }

    print('totp');
    print(TOTP.TOTP('12345678901234567890', 59e3, 8));
    print(TOTP.TOTP('12345678901234567890', 1111111109e3, 8));
    print(TOTP.TOTP('12345678901234567890', 1111111111e3, 8));
    print(TOTP.TOTP('12345678901234567890', 20000000000e3, 8));

    print('base32');
    print(TOTP.base32Decode('MY======'));
    print(TOTP.base32Decode('MY'));
    print(TOTP.base32Decode('MZXQ===='));
    print(TOTP.base32Decode('MZXQ'));
    print(TOTP.base32Decode('MZXW6==='));
    print(TOTP.base32Decode('MZXW6'));
    print(TOTP.base32Decode('MZXW6IA='));
    print(TOTP.base32Decode('MZXW6IA'));
    print(TOTP.base32Decode('MZXW6IDC'));
    print(TOTP.base32Decode('MZXW6IDCMFZA===='));
    print(TOTP.base32Decode('MZXW6IDCMFZA'));
}

try {
    test();
} catch (e) {
    print(e);
}
