// Array initializers are not affected by inherited Array.prototype index
// properties, even when they are setters.

/*===
3
["foo","bar","quux"]
TypeError: rejected!
4
["foo","bar","quux","baz"]
5
["foo","bar","quux","baz","quuux"]
6
["foo","bar","quux","baz","quuux","quuuux"]
done
===*/

function test() {
    Object.defineProperty(Array.prototype, '4', { set: function () { throw new TypeError('rejected!'); } });
    var arr = eval('[ "foo", "bar", "quux" ]');
    print(arr.length);
    print(JSON.stringify(arr));
    try {
        arr[4] = 'aiee';
    } catch (e) {
        print(String(e));
    }

    var arr = eval('[ "foo", "bar", "quux", "baz" ]');
    print(arr.length);
    print(JSON.stringify(arr));

    var arr = eval('[ "foo", "bar", "quux", "baz", "quuux" ]');
    print(arr.length);
    print(JSON.stringify(arr));

    var arr = eval('[ "foo", "bar", "quux", "baz", "quuux", "quuuux" ]');
    print(arr.length);
    print(JSON.stringify(arr));
    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
