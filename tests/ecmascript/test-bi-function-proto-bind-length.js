/*===
123
0
dummy
0
===*/

function test() {
    var f, g;

    // In ES2015 .bind() doesn't coerce the length so '123' is treated as 0.
    f = function f() {};
    Object.defineProperty(f, 'length', { value: '123' });
    print(f.length);
    g = f.bind(null, 123);
    print(g.length);

    // Same for a string which isn't number like.
    f = function f() {};
    Object.defineProperty(f, 'length', { value: 'dummy' });
    print(f.length);
    g = f.bind(null, 123);
    print(g.length);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
