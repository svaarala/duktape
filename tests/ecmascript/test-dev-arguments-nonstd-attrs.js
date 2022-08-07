/*===
3 foo:wec bar:wec quux:wec gap gap
3 foo:wec bar:wec quux:wec gap QUUUX:ec
3 foo:wec bar:wec quux:wec BAZ:ec QUUUX:ec
3 foo:wec bar:wec QUUX:ec BAZ:ec QUUUX:ec
3 foo:wec BAR:wc QUUX:ec BAZ:ec QUUUX:ec
3 FOO:we BAR:wc QUUX:ec BAZ:ec QUUUX:ec
===*/

function func() {
    return arguments;
}

function test() {
    var args = func('foo', 'bar', 'quux');

    function dump() {
        var pd, i;
        var parts = [ args.length ];
        for (i = 0; i < 5; i++) {
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

    Object.defineProperty(args, '4', { value: 'QUUUX', writable: false, enumerable: true, configurable: true });
    dump();

    Object.defineProperty(args, '3', { value: 'BAZ', writable: false, enumerable: true, configurable: true });
    dump();

    Object.defineProperty(args, '2', { value: 'QUUX', writable: false, enumerable: true, configurable: true });
    dump();

    Object.defineProperty(args, '1', { value: 'BAR', writable: true, enumerable: false, configurable: true });
    dump();

    Object.defineProperty(args, '0', { value: 'FOO', writable: true, enumerable: true, configurable: false });
    dump();
}

test();
