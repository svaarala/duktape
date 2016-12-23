/*
 *  A few bugs in JSON (and JX/JC) for buffer objects:
 *
 *    - Buffer objects should be serialized as empty objects (not "undefined").
 *
 *    - Typed arrays have enumerable index properties which should be serialized
 *      in the absence of a .toJSON() method.
 */

/*@include util-buffer.js@*/

/*===
index: 0
json fast: {"0":0,"1":0,"2":0,"3":0}
json slow: {"0":0,"1":0,"2":0,"3":0}
jx fast: |00000000|
jx slow: |00000000|
jc fast: {"_buf":"00000000"}
jc slow: {"_buf":"00000000"}
json fast: {"buffer":{"0":0,"1":0,"2":0,"3":0}}
json slow: {"buffer":{"0":0,"1":0,"2":0,"3":0}}
jx fast: {buffer:|00000000|}
jx slow: {buffer:|00000000|}
jc fast: {"buffer":{"_buf":"00000000"}}
jc slow: {"buffer":{"_buf":"00000000"}}
index: 1
json fast: {}
json slow: {}
jx fast: |00000000|
jx slow: |00000000|
jc fast: {"_buf":"00000000"}
jc slow: {"_buf":"00000000"}
json fast: {"buffer":{}}
json slow: {"buffer":{}}
jx fast: {buffer:|00000000|}
jx slow: {buffer:|00000000|}
jc fast: {"buffer":{"_buf":"00000000"}}
jc slow: {"buffer":{"_buf":"00000000"}}
index: 2
json fast: {"0":0,"1":0,"2":0,"3":0}
json slow: {"0":0,"1":0,"2":0,"3":0}
jx fast: |00000000|
jx slow: |00000000|
jc fast: {"_buf":"00000000"}
jc slow: {"_buf":"00000000"}
json fast: {"buffer":{"0":0,"1":0,"2":0,"3":0}}
json slow: {"buffer":{"0":0,"1":0,"2":0,"3":0}}
jx fast: {buffer:|00000000|}
jx slow: {buffer:|00000000|}
jc fast: {"buffer":{"_buf":"00000000"}}
jc slow: {"buffer":{"_buf":"00000000"}}
index: 3
json fast: {"myProperty":123}
json slow: {"myProperty":123}
jx fast: |00000000|
jx slow: |00000000|
jc fast: {"_buf":"00000000"}
jc slow: {"_buf":"00000000"}
json fast: {"buffer":{"myProperty":123}}
json slow: {"buffer":{"myProperty":123}}
jx fast: {buffer:|00000000|}
jx slow: {buffer:|00000000|}
jc fast: {"buffer":{"_buf":"00000000"}}
jc slow: {"buffer":{"_buf":"00000000"}}
index: 4
json fast: {"0":0,"1":0,"2":0,"3":0,"myProperty":123}
json slow: {"0":0,"1":0,"2":0,"3":0,"myProperty":123}
jx fast: |00000000|
jx slow: |00000000|
jc fast: {"_buf":"00000000"}
jc slow: {"_buf":"00000000"}
json fast: {"buffer":{"0":0,"1":0,"2":0,"3":0,"myProperty":123}}
json slow: {"buffer":{"0":0,"1":0,"2":0,"3":0,"myProperty":123}}
jx fast: {buffer:|00000000|}
jx slow: {buffer:|00000000|}
jc fast: {"buffer":{"_buf":"00000000"}}
jc slow: {"buffer":{"_buf":"00000000"}}
===*/

function test() {
    var abWithProps = new ArrayBuffer(4);
    abWithProps.myProperty = 123;
    var u8WithProps = new Uint8Array(4);
    u8WithProps.myProperty = 123;

    function id(v) { return v; }

    [
        createPlainBuffer(4),  // plain
        new ArrayBuffer(4),
        new Uint8Array(4),  // indices are enumerable so should be serialized!
        abWithProps,  // enumerable properties are serialized
        u8WithProps
    ].forEach(function (v, i) {
        print('index:', i);

        function doTest() {
            delete ArrayBuffer.prototype.toJSON;  // fast path
            print('json fast:', JSON.stringify(v));
            ArrayBuffer.prototype.toJSON = 123;  // slow path; .toJSON() present but not callable
            print('json slow:', JSON.stringify(v));

            delete ArrayBuffer.prototype.toJSON;  // fast path
            print('jx fast:', Duktape.enc('jx', v));
            ArrayBuffer.prototype.toJSON = 123;  // slow path; .toJSON() present but not callable
            print('jx slow:', Duktape.enc('jx', v));

            delete ArrayBuffer.prototype.toJSON;  // fast path
            print('jc fast:', Duktape.enc('jc', v));
            ArrayBuffer.prototype.toJSON = 123;  // slow path; .toJSON() present but not callable
            print('jc slow:', Duktape.enc('jc', v));

            delete ArrayBuffer.prototype.toJSON;
        }

        doTest();

        // For some fast path handling it matters whether the value is at the
        // top level or not, so test for that explicitly too.

        v = { buffer: v };
        doTest();
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
