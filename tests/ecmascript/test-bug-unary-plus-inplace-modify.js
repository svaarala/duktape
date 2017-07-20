/*===
NaN
foo
===*/

function test() {
    var foo = 'foo';

    print(+foo);
    print(foo);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
