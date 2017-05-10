/*===
f0 called
applythis arg-0 arg-1 arg-2
500000
arg-499998
arg-499999
undefined
done
===*/

function test() {
    function f0(a, b, c) {
        print('f0 called');
        print(this, a, b, c);
        print(arguments.length);
        print(arguments[499998]);
        print(arguments[499999]);
        print(arguments[500000]);
    }

    var args = [];
    while (args.length < 500000) {
        args.push('arg-' + args.length);
    }

    f0.apply('applythis', args);
    print('done');
}
try {
    test();
} catch (e) {
    print(e.stack || e);
}
