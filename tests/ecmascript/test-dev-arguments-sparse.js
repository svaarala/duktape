/*===
3 foo bar quux undefined
["0","1","2","length","callee"]
3 foo bar quux aiee
["0","1","2","10000","length","callee"]
===*/

function func() {
    return arguments;
}

function test() {
    var args;

    args = func('foo', 'bar', 'quux');
    //print(Duktape.enc('jx', Duktape.info(args)));
    print(args.length, args[0], args[1], args[2], args[10000]);
    print(JSON.stringify(Object.getOwnPropertyNames(args)));

    args[10000] = 'aiee';
    //print(Duktape.enc('jx', Duktape.info(args)));
    print(args.length, args[0], args[1], args[2], args[10000]);
    print(JSON.stringify(Object.getOwnPropertyNames(args)));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
