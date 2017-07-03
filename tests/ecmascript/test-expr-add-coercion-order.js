/*===
o1 valueOf
o2 valueOf
foobar
o2 valueOf
o1 valueOf
barfoo
o1 valueOf
o2 valueOf
o3 valueOf
foobarquux
===*/

function test() {
    var o1 = {
        toString: function () { print('o1 toString'); return 'FOO' },
        valueOf: function () { print('o1 valueOf'); return 'foo' },
    };
    var o2 = {
        toString: function () { print('o2 toString'); return 'BAR' },
        valueOf: function () { print('o2 valueOf'); return 'bar' },
    };
    var o3 = {
        toString: function () { print('o3 toString'); return 'QUUX' },
        valueOf: function () { print('o3 valueOf'); return 'quux' },
    };

    print(o1 + o2);
    print(o2 + o1);
    print(o1 + o2 + o3);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
