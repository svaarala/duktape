/*
 *  Use bind() indirectly with call() or apply().
 */

/*===
apply
foo this: boundthis args: argx argy argz undefined
call
foo this: boundthis args: argx argy argz undefined
===*/

function foo(x,y,z,w) {
    print('foo', 'this:', this, 'args:', x, y, z, w);
}

function test() {
    var f1, f2;

    print('apply');
    f1 = foo.bind.apply(foo, [ 'boundthis', 'argx', 'argy', 'argz' ]);
    f1();

    print('call');
    f2 = foo.bind.call(foo, 'boundthis', 'argx', 'argy', 'argz');
    f2();
}

try {
    test();
} catch (e) {
    print(e);
}
