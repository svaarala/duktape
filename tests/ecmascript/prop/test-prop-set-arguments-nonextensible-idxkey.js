/*===
2 foo bar
2 foo bar
2 foo quux
TypeError
2 foo quux undefined
===*/

function func() {
    return arguments;
}

function test() {
    'use strict';

    var args = func('foo', 'bar');
    print(args.length, args[0], args[1]);
    Object.preventExtensions(args);
    print(args.length, args[0], args[1]);
    args[1] = 'quux';
    print(args.length, args[0], args[1]);
    try {
        args[2] = 'baz';
    } catch (e) {
        print(e.name);
    }
    print(args.length, args[0], args[1], args[2]);
}

test();
