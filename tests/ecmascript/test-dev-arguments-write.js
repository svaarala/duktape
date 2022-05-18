/*===
3 foo bar quux undefined undefined
3 FOO bar quux undefined undefined
3 FOO BAR quux undefined undefined
3 FOO BAR QUUX undefined undefined
3 FOO BAR QUUX BAZ undefined
3 FOO BAR QUUX BAZ QUUUX
===*/

function func() {
    return arguments;
}

function test() {
    var args = func('foo', 'bar', 'quux');

    function dump() {
        print(args.length, args[0], args[1], args[2], args[3], args[4]);
        //print(Duktape.enc('jx', Duktape.info(args)));
    }

    dump();

    args[0] = 'FOO';
    dump();

    args[1] = 'BAR';
    dump();

    args[2] = 'QUUX';
    dump();

    args[3] = 'BAZ';
    dump();

    args[4] = 'QUUUX';
    dump();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
