/*===
0
0 true false false
3
3 true false false
3
3 false false false
TypeError
3
3 false false false
done
===*/

function test() {
    var R = /foo/g;

    function dump() {
        print(R.lastIndex);
        var pd = Object.getOwnPropertyDescriptor(R, 'lastIndex');
        print(pd.value, pd.writable, pd.enumerable, pd.configurable);

    }

    dump();
    R.exec('foofoofoo');
    dump();
    Object.defineProperty(R, 'lastIndex', { writable: false });
    dump();

    // The update of .lastIndex must use Set() semantics, i.e. fail
    // if not writable.
    try {
        R.exec('foofoofoo');
    } catch (e) {
        print(e.name);
    }

    dump();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}

print('done');
