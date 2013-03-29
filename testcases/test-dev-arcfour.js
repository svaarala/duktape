/* http://en.wikipedia.org/wiki/RC4 */

/*===
Key Plaintext
bbf316e8d940af0ad3
Wiki pedia
1021bf0420
Secret Attack at dawn
45a01f645fc35b383552544b9bf5
===*/

var S = [];
var gi, gj;

function swap(i, j) {
    var t;
    t = S[i]; S[i] = S[j]; S[j] = t;
}

function keySetup(key) {
    var i, j, t;

    S = [];
    for (i = 0; i < 256; i++) {
        S[i] = i;
    }

    for (j = 0, i = 0; i < 256; i++) {
        j = (j + S[i] + key.charCodeAt(i % key.length)) % 256;
        swap(i, j);
    }

    gi = 0; gj = 0;
}

function encrypt(plain) {
    var i;
    var t;
    var K;
    var res = [];

    for (i = 0; i < plain.length; i++) {
        gi = (gi + 1) % 256;
        gj = (gj + S[gi]) % 256;
        swap(gi, gj);
        K = S[(S[gi] + S[gj]) % 256];
        res.push(plain.charCodeAt(i) ^ K);
    }
    return res;
}

function testVectorTest() {
    var tests = [
        { key: 'Key', plain: 'Plaintext' },
        { key: 'Wiki', plain: 'pedia' },
        { key: 'Secret', plain: 'Attack at dawn' }
    ];

    tests.forEach(function(t) {
        var res;
        print(t.key, t.plain);
        keySetup(t.key);
        res = encrypt(t.plain);
        print(res.map(function(x) {
                  var nybbles = '0123456789abcdef';
                  return nybbles[x >> 4] + nybbles[x & 0x0f];
              }).join(''));
    });
}

try {
    testVectorTest();
} catch (e) {
    print(e);
}
