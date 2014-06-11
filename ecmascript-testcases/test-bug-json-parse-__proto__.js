/*
 *  Bug in test262 test case: ch15/15.12/15.12.2/S15.12.2_A1
 *
 *  Duktape does not parse __proto__ as a concrete property when ES6
 *  __proto__ property is enabled, but rather sets the object prototype.
 */

/*===
basic case
[object Object]
object
true
false
true
[object Object]
===*/

/*===
reviver case
reviver: __proto__
reviver: 
[object Object]
object
true
false
true
[object Object]
===*/

/*===
complex reviver case
reviver: foo [object Object] bar
before: this.__proto__: [object Object]
after: this.__proto__: undefined
reviver: __proto__ undefined [object Object]
reviver:  [object Object] [object Object]
[object Object]
object
true
false
true
[object Object]
===*/

function test() {
    var x;

    function dump(x) {
        print(x);
        print(typeof x);
        print(x instanceof Object);
        print(x instanceof Array);
        print('__proto__' in x);
        print(Object.getOwnPropertyDescriptor(x, '__proto__'));
    }

    // Basic parse case

    print('basic case');
    dump(JSON.parse('{"__proto__":[]}'));

    // Similar issue triggered when reviving '__proto__'

    print('reviver case');
    dump(JSON.parse('{"__proto__":[]}', function (name, val) {
        // this=holder
        print('reviver:', name);
        return val;
    }));

    // Here the reviver is quite devious, and modifies the holder
    // so that an own property '__proto__' is removed before that
    // property is revived.  Because Duktape writes the replacement
    // value with duk_put_prop() now (in duk_bi_json.c), this runs
    // the risk of doing a normal property put on __proto__ with no
    // such own property in the object -- which then changes the
    // prototype of the result (which is incorrect).

    print('complex reviver case');
    dump(JSON.parse('{"foo":"bar", "__proto__":[1,2,3]}', function (name, val) {
        // this=holder
        print('reviver:', name, Object.getOwnPropertyDescriptor(this, name), val);
        if (name === 'foo') {
            // Delete '__proto__' while reviving 'foo'.
            print('before: this.__proto__:', Object.getOwnPropertyDescriptor(this, '__proto__'));
            delete this.__proto__;
            print('after: this.__proto__:', Object.getOwnPropertyDescriptor(this, '__proto__'));
        }
        if (name === '__proto__') {
            // Since __proto__ is deleted, duk_bi_json.c will read its value as
            // Object.prototype (= val) before calling us.  That's why the test
            // case prints out Object.getOwnPropertyDescriptor(this, name) returning
            // undefined, but 'val' being '[object Object'].  Force an array return
            // so that it gets (incorrectly) written to __proto__ using duk_put_prop().
            // The correct behavior here would be to set it as an own property.
            return [];
        }
        return val;
    }));
}

try {
    test();
} catch (e) {
    print(e);
}
