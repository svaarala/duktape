/*===
basic
object 3 [object Array]
0 foo
1 bar
2 quux
object 7 [object Array]
0 0
1 1
2 2
3 3
4 4
5 5
6 length
===*/

print('basic');

function basicTest() {
    var proto = { proto: 'prototype' };
    var obj;

    function test(o) {
        var t = Object.getOwnPropertyNames(o);
        var i;

        print(typeof t, t.length, Object.prototype.toString.call(t));
        for (i = 0; i < t.length; i++) {
            print(i, t[i]);
        }
    }

    obj = Object.create(proto);
    obj.foo = 1;
    obj.bar = 2;
    obj.quux = 3;

    // return own properties only; order is not mandated but we expect
    // that properties are returned in definition order here too
    test(obj);

    // character index properties must be returned (however, Rhino does not)
    test(new String('foobar'));
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
TypeError
TypeError
TypeError
TypeError
TypeError
object
object
object
===*/

print('coercion');

function coercionTest() {
    function test(o, is_noarg) {
        var t;

        try {
            if (is_noarg) {
                t = Object.getOwnPropertyNames(o);
            } else {
                t = Object.getOwnPropertyNames(o);
            }
            print(typeof t);  // don't print length because function properties are custom
        } catch (e) {
            print(e.name);
        }
    }

    test(undefined, true);
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });
    test(function(){});
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
