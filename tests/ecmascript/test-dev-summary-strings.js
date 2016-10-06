/*
 *  Manual tests for value summarization.
 *
 *  Relies on a global 'summary()' binding so disabled by default.
 */

/*---
{
    "skip": true
}
---*/

function test() {
    if (typeof summary !== 'function') {
        throw new Error('this test requires a global summary() binding');
    }

    var u16a = new Uint16Array([ 1, 2, 3, 4, 0xcafe, 0xd00d ]);
    var u16b = new Uint16Array([ 1, 2, 3, 4, 0xcafe, 0xd00d, 0x1234, 0x4321 ]);
    var u16c = new Uint16Array([ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ]);

    var dva = new DataView(u16a.buffer);
    var dvb = new DataView(u16b.buffer);
    var dvc = new DataView(u16c.buffer);

    var erra = new URIError('failed to convert URI');
    var errb = new TypeError('invalid argument');
    errb.name = 'CustomErrorName';
    var errc = new TypeError('invalid argument');
    errb.name = 123;  // non-string
    var errd = new TypeError('invalid argument');
    Object.defineProperty(errd, 'name', {
        get: function () { print('error .name getter!'); return 'FakeName'; }
    });
    print('errd.name:', errd.name);

    function MyCustomError(message) {
        this.message = message;
    }
    MyCustomError.prototype = { name: 'MyCustomError' };
    Object.setPrototypeOf(MyCustomError.prototype, Error.prototype);
    var erre = new MyCustomError('aiee');

    [
        // Undefined, null, boolean, number
        void 0, null, true, false, 123, 1/0, -1/0, 0/0,

        // Objects and arrays
        { foo:123 }, [ 1, 2, 3],

        // String
        '', 'foo', 'foo\ucafe\u0000bar',
        'longstring1234567890abcdefghijklmnopqrstuvwxyz',
        'x\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000y',

        // Buffers and views
        ArrayBuffer.allocPlain('foo\ucafebar'),
        ArrayBuffer.allocPlain('1234567890123456'),
        ArrayBuffer.allocPlain('123456789012345678901234567890'),
        ArrayBuffer.allocPlain(1e2),
        new ArrayBuffer(5),
        new ArrayBuffer(16),
        new ArrayBuffer(50),
        u16a, u16b, u16c, dva, dvb, dvc,

        // Pointer; just test for NULL pointer
        Duktape.Pointer(),

        // Errors
        erra, errb, errc, errd, erre,

        // Functions
        Math.cos,
        function myFunction () {},
        function myFunction2 (a, b, c, d) {},
        function () {},  // no name
        Math.cos.bind('dummy'),

        // Boxed primitive objects
        new String('foo\u0000bar'),
        new Boolean(true),
        new Number(123),
        new Duktape.Pointer(),

        // Thread
        new Duktape.Thread(function () {}),

        // Date
        new Date(1234567890),

        // RegExp
        /[a-z]+[A-Z]{1,99} slash \//gim,

        // A few special types
        Math, JSON, new Function('return this')()
    ].forEach(function (v) {
        print(summary(v));
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
