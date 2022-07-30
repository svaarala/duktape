function func() {
    var res = arguments[0];
    return res;
}

function test() {
    var i;

    for (i = 0; i < 1e6; i++) {
        func('foo', 'bar', 'quux', 'baz', 'quuux');
    }
}

test();
