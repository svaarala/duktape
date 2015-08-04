/*===
basic
0 obj1 0 obj1 false
0 obj1 1 obj2 true
0 obj1 2 obj3 true
0 obj1 3 obj4 true
0 obj1 4 test1 false
1 obj2 0 obj1 false
1 obj2 1 obj2 false
1 obj2 2 obj3 true
1 obj2 3 obj4 true
1 obj2 4 test1 false
2 obj3 0 obj1 false
2 obj3 1 obj2 false
2 obj3 2 obj3 false
2 obj3 3 obj4 true
2 obj3 4 test1 false
3 obj4 0 obj1 false
3 obj4 1 obj2 false
3 obj4 2 obj3 false
3 obj4 3 obj4 false
3 obj4 4 test1 false
4 test1 0 obj1 false
4 test1 1 obj2 false
4 test1 2 obj3 false
4 test1 3 obj4 false
4 test1 4 test1 false
false
false
false
false
false
false
===*/

print('basic');

function basicTest() {
    var obj1, obj2, obj3, obj4;
    var test1;
    var objs = [];
    var i, j;

    // prototype chain: obj1 <- obj2 <- obj3 <- obj4
    obj1 = {};
    obj1.name = 'obj1';
    obj2 = Object.create(obj1);
    obj2.name = 'obj2';
    obj3 = Object.create(obj2);
    obj3.name = 'obj3';
    obj4 = Object.create(obj3);
    obj4.name = 'obj4';

    objs.push(obj1);
    objs.push(obj2);
    objs.push(obj3);
    objs.push(obj4);

    // unrelated object
    test1 = { name: 'test1' };

    objs.push(test1);

    for (i = 0; i < objs.length; i++) {
        for (j = 0; j < objs.length; j++) {
            print(i, objs[i].name, j, objs[j].name, objs[i].isPrototypeOf(objs[j]));
        }
    }

    // whenever 'V' is a non-object, result is false
    print(obj1.isPrototypeOf(undefined));
    print(obj1.isPrototypeOf(null));
    print(obj1.isPrototypeOf(true));
    print(obj1.isPrototypeOf(false));
    print(obj1.isPrototypeOf(123));
    print(obj1.isPrototypeOf('foo'));
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
[object Undefined] [object Undefined] ok
[object Undefined] [object Undefined] ok
[object Undefined] [object Null] ok
[object Undefined] [object Boolean] ok
[object Undefined] [object Boolean] ok
[object Undefined] [object Number] ok
[object Undefined] [object String] ok
[object Undefined] [object Array] TypeError
[object Undefined] [object Object] TypeError
[object Null] [object Undefined] ok
[object Null] [object Undefined] ok
[object Null] [object Null] ok
[object Null] [object Boolean] ok
[object Null] [object Boolean] ok
[object Null] [object Number] ok
[object Null] [object String] ok
[object Null] [object Array] TypeError
[object Null] [object Object] TypeError
[object Boolean] [object Undefined] ok
[object Boolean] [object Undefined] ok
[object Boolean] [object Null] ok
[object Boolean] [object Boolean] ok
[object Boolean] [object Boolean] ok
[object Boolean] [object Number] ok
[object Boolean] [object String] ok
[object Boolean] [object Array] ok
[object Boolean] [object Object] ok
[object Boolean] [object Undefined] ok
[object Boolean] [object Undefined] ok
[object Boolean] [object Null] ok
[object Boolean] [object Boolean] ok
[object Boolean] [object Boolean] ok
[object Boolean] [object Number] ok
[object Boolean] [object String] ok
[object Boolean] [object Array] ok
[object Boolean] [object Object] ok
[object Number] [object Undefined] ok
[object Number] [object Undefined] ok
[object Number] [object Null] ok
[object Number] [object Boolean] ok
[object Number] [object Boolean] ok
[object Number] [object Number] ok
[object Number] [object String] ok
[object Number] [object Array] ok
[object Number] [object Object] ok
[object String] [object Undefined] ok
[object String] [object Undefined] ok
[object String] [object Null] ok
[object String] [object Boolean] ok
[object String] [object Boolean] ok
[object String] [object Number] ok
[object String] [object String] ok
[object String] [object Array] ok
[object String] [object Object] ok
[object Array] [object Undefined] ok
[object Array] [object Undefined] ok
[object Array] [object Null] ok
[object Array] [object Boolean] ok
[object Array] [object Boolean] ok
[object Array] [object Number] ok
[object Array] [object String] ok
[object Array] [object Array] ok
[object Array] [object Object] ok
[object Object] [object Undefined] ok
[object Object] [object Undefined] ok
[object Object] [object Null] ok
[object Object] [object Boolean] ok
[object Object] [object Boolean] ok
[object Object] [object Number] ok
[object Object] [object String] ok
[object Object] [object Array] ok
[object Object] [object Object] ok
===*/

/* Coercion steps are interesting; if argument V is not an object,
 * false is returned regardless of whether 'this' is invalid.
 */

print('coercion');

function coercionTest() {
    var values = [ undefined, null, true, false, 123, 'foo', [1,2], { foo: 1, bar: 2 } ];
    var i, j;

    function test(this_val, arg, is_noarg) {
        var t;

        try {
            if (is_noarg) {
                t = Object.prototype.isPrototypeOf.call(this_val);
            } else {
                t = Object.prototype.isPrototypeOf.call(this_val, arg);
            }
            print(Object.prototype.toString.call(this_val), Object.prototype.toString.call(arg), 'ok');
        } catch (e) {
            print(Object.prototype.toString.call(this_val), Object.prototype.toString.call(arg), e.name);
        }
    }

    for (i = 0; i < values.length; i++) {
        test(values[i], undefined, true);

        for (j = 0; j < values.length; j++) {
            test(values[i], values[j]);
        }
    }
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
