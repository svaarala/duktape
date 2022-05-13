/*===
2
1 2 3 4
1 123 3 4
1 123 3 4
234
1 123 3 4
===*/

function func(a, b, c, d) {
    return {
        a: arguments,
        e: function (v) { return eval(v); }
    };
}

try {
    var obj = func(1, 2, 3, 4);
    print(obj.e('b'));
    print(obj.a[0], obj.a[1], obj.a[2], obj.a[3]);
    void obj.e('b = 123;');
    print(obj.a[0], obj.a[1], obj.a[2], obj.a[3]);
    Object.defineProperty(obj.a, '1', { writable: false });
    print(obj.a[0], obj.a[1], obj.a[2], obj.a[3]);
    // Here 'b' is no longer mapped to arguments[1] so obj.a[1]
    // will reflect 123.
    void obj.e('b = 234;');
    print(obj.e('b'));
    print(obj.a[0], obj.a[1], obj.a[2], obj.a[3]);
} catch (e) {
    print(e.stack || e);
}
