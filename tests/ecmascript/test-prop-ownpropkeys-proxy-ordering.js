// While there are ordering guarantees for ordinary objects, Proxy
// ownKeys trap return value is used as is for [[OwnPropertyKeys]],
// and EnumerableOwnPropertyNames().

/*===
ownKeys
foo
3
1
bar
7
ownKeys
["foo","3","1","bar","7"]
===*/

try {
    var P = new Proxy({ foo: true, 1: true, 3: true, bar: true, 7: true }, {
        ownKeys: function () {
            print('ownKeys');
            return [ 'foo', '3', '1', 'bar', '7' ];
        }
    });

    for (var k in P) {
        print(k);
    }

    // About direct access to [[OwnPropertyKeys]].
    print(JSON.stringify(Reflect.ownKeys(P)));
} catch (e) {
    print(e.stack || e);
}
