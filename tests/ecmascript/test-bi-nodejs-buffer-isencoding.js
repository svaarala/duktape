/*
 *  Buffer.isEncoding()
 */

/*@include util-nodejs-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
isEncoding test
empty: false
undefined: false
utf8: true
utf-8: false
UTF8: false
UTF-8: false
Utf8: false
Utf-8: false
uTf8: false
uTf-8: false
ascii: false
ASCII: false
AsCiI: false
dummy: false
undefined: false
null: false
true: false
false: false
[object Object]: false
: false
utf8: true
===*/

function isEncodingTest() {
    print('empty:', Buffer.isEncoding(''));
    print('undefined:', Buffer.isEncoding());

    [
        // Any capitalization (and dash / no dash) is accepted by Node.js.
        // Duktape accepts 'utf8' only for now.
        'utf8', 'utf-8', 'UTF8', 'UTF-8',
        'Utf8', 'Utf-8', 'uTf8', 'uTf-8',

        // Duktape doesn't support 'ascii' etc.
        'ascii',
        'ASCII',
        'AsCiI',
        'dummy',

        // Non-string values
        undefined, null, true, false, {}, [],

        // This coerces to 'utf8' and *is* accepted by Node.js, so apparently
        // ToString() coercion is used by Node.js.  Duktape mimics this behavior.
        { valueOf: function () { return 'utf8'; }, toString: function () { return 'utf8'; } }
    ].forEach(function (v) {
        print(v + ': ' + Buffer.isEncoding(v));
    });
}

try {
    print('isEncoding test');
    isEncodingTest();
} catch (e) {
    print(e.stack || e);
}
