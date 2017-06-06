/*
 *  Return and replacement value handling in try-finally of a constructor call.
 */

/*===
finally
object undefined
finally
object true
finally
object true
finally
object true
===*/

function MyConstructor1() {
    try {
        return 123;  // return non-object value -> won't replace default instance
    } finally {
        print('finally');
    }
}

function MyConstructor2() {
    try {
        return 123;  // return non-object value -> won't replace default instance
    } finally {
        print('finally');
        return { replaced: true };  // replace with object value
    }
}

function MyConstructor3() {
    try {
        return { replaced: true };
    } finally {
        print('finally');
    }
}

function MyConstructor4() {
    try {
        return { foo: 123 };
    } finally {
        print('finally');
        return { replaced: true };
    }
}

function test() {
    var t;

    t = new MyConstructor1();
    print(typeof t, t.replaced);

    t = new MyConstructor2();
    print(typeof t, t.replaced);

    t = new MyConstructor3();
    print(typeof t, t.replaced);

    t = new MyConstructor4();
    print(typeof t, t.replaced);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
