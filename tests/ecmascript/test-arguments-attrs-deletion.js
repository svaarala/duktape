/*===
length 3 true false true
0 foo true true true
1 bar true true true
2 quux true true true
insert 3
length 3 true false true
0 foo true true true
1 bar true true true
2 quux true true true
3 aiee true true true
delete length
0 foo true true true
1 bar true true true
2 quux true true true
3 aiee true true true
delete 0
1 bar true true true
2 quux true true true
3 aiee true true true
123
===*/

function test() {
    var args = arguments;

    function dump() {
        var pd;
        pd = Object.getOwnPropertyDescriptor(args, 'length');
        if (pd) {
            print('length', pd.value, pd.writable, pd.enumerable, pd.configurable);
        }

        for (var i = 0; i < 5; i++) {
            pd = Object.getOwnPropertyDescriptor(args, String(i));
            if (pd) {
                print(i, pd.value, pd.writable, pd.enumerable, pd.configurable);
            }
        }
    }

    dump();

    print('insert 3');
    arguments[3] = 'aiee';
    dump();

    print('delete length');
    delete arguments.length;
    dump();

    print('delete 0');
    delete arguments['0'];
    dump();

    // With .length deleted, it can be inherited from Object.prototype.
    Object.prototype.length = 123;
    print(args.length);
}

test('foo', 'bar', 'quux');
