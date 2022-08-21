/*---
custom: true
---*/

/*===
24 foo:wec bar:wec quux:wec baz:wec quuux:wec 1:wec 2:wec 3:wec 4:wec 5:wec 6:wec 7:wec 8:wec 9:wec 10:wec 11:wec 12:wec 13:wec 14:wec 15:wec 16:wec 17:wec 18:wec 19:wec gap gap gap gap gap gap
24 foo:wec gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap
24 foo:wec gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap
24 foo:wec gap gap gap gap gap gap gap gap gap aiee:wec gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap gap
===*/

function func() {
    return arguments;
}

function test() {
    var args = func('foo', 'bar', 'quux', 'baz', 'quuux', 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19);

    function dump() {
        var pd, i;
        var parts = [ args.length ];
        for (i = 0; i < 30; i++) {
            var pd = Object.getOwnPropertyDescriptor(args, i);
            if (pd) {
                parts.push(pd.value + ':' + (pd.writable ? 'w' : '') + (pd.enumerable ? 'e' : '') + (pd.configurable ? 'c' : ''));
            } else {
                parts.push('gap');
            }
        }
        print(parts.join(' '));
        //print(Duktape.enc('jx', Duktape.info(args)));
    }

    dump();

    // Leave 1 argument in place.
    for (var i = 1; i < args.length; i++) {
        delete args[i];
    }
    dump();

    Duktape.compact(args);
    dump();

    args[10] = 'aiee';
    dump();
}

test();
