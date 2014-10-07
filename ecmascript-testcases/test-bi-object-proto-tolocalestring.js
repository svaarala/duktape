/*===
basic
string proto-toString
TypeError
===*/

print('basic');

function basicTest() {
    var proto, obj;

    function test(o) {
        var t;

        try {
            t = Object.prototype.toLocaleString.call(o);
            print(typeof t, t);
        } catch (e) {
            print(e.name);
        }
    }

    // toLocaleString() inherited from Object.prototype, causes
    // proto.toString() to be called eventually

    proto = Object.create(Object.constructor);
    proto.toString = function() { return 'proto-toString'; };
    obj = Object.create(proto);
    test(obj);

    // non-calleble toString()

    proto.toString = 123;
    test(obj);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
string true
string false
string 123
string foo
string 1,2
string [object Object]
===*/

print('coercion');

function coercionTest() {
    function test(o) {
        var t;

        try {
            t = Object.prototype.toLocaleString.call(o);
            print(typeof t, t);
        } catch (e) {
            print(e.name);
        }
    }

    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 1 });
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
