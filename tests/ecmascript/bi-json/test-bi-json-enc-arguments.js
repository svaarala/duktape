/*===
3
{"0":"foo","1":"bar","2":"quux","3":"baz"}
===*/

function func() {
    return arguments;
}

function test() {
    var args = func('foo', 'bar', 'quux');
    args[3] = 'baz';
    print(args.length);
    print(JSON.stringify(args));
}

test();
