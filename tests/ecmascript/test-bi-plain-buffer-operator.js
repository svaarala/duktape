/*@include util-buffer.js@*/

/*===
operator test
[object Uint8Array][object Uint8Array]
false
false
true
true
false
false
false
false
false
false
string "length" true
string "byteLength" true
string "byteOffset" true
string "BYTES_PER_ELEMENT" true
number -1 false
number 0 true
number 15 true
number 16 false
string "15" true
string "16" false
string "15.0" false
true
true
===*/

function operatorTest() {
    var pb = createPlainBuffer('abcdefghijklmnop');

    // '+' operator
    print(pb + pb);  // 'abcdefghijklmnopabcdefghijklmnop' in Duktape 1.x, '[object Uint8Array][object Uint8Array]' in Duktape 2.x

    // equality comparison: no content comparison in Duktape 2.x when
    // comparing plain buffers using '==' (or '==='), all comparisons
    // are now reference based
    print(createPlainBuffer('abcd') == createPlainBuffer('abcd'));
    print(createPlainBuffer('abcd') === createPlainBuffer('abcd'));
    print(pb == pb);
    print(pb === pb);

    // number and string comparisons are always false
    pb = createPlainBuffer(4);
    pb[0] = '1'.charCodeAt(0);
    pb[1] = '2'.charCodeAt(0);
    pb[2] = '3'.charCodeAt(0);
    pb[3] = '4'.charCodeAt(0);
    print(pb == 1234);
    print(pb === 1234);
    print(pb == '1234');
    print(pb === '1234');
    pb = createPlainBuffer('abcdefghijklmnop');  // reset after change

    // object comparison is always false
    print(pb == {});
    print(pb === {});

    [ 'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT', -1, 0, 15, 16, '15', '16', '15.0' ].forEach(function (v) {
        print(typeof v, Duktape.enc('jx', v), v in pb);
    });

    // plain buffers ToBoolean() coerce to true, even if a zero length
    // buffer (this differs from Duktape 1.x)
    print(!!pb);
    print(!!createPlainBuffer(0));
}

try {
    print('operator test');
    operatorTest();
} catch (e) {
    print(e.stack || e);
}
