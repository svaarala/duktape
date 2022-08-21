/*
 *  While [[OwnPropertyKeys]] has a well-defined ordering for most objects,
 *  Proxy 'ownKeys' trap is returned from [[OwnPropertyKeys]] as is, and
 *  is not re-sorted for enumeration.
 */

/*===
reflect ownKeys
ownKeys called
["bar","0","Symbol(S2)","2","Symbol(S1)","quux","foo","1"]
for-in enum of obj
ownKeys called
quux
bar
0
2
foo
1
for-in enum of target
0
1
2
foo
bar
quux
===*/

var S1 = Symbol.for('S1');
var S2 = Symbol.for('S2');
var target = { foo: 123, bar: 234, quux: 345, '0': 'zero', '1': 'one', '2': 'two', [S1]: true, [S2]: true };
var P = new Proxy(target, {
    ownKeys: function (targ) {
        // Return keys not in ES2015+ order.
        print('ownKeys called');
        return [ 'bar', '0', S2, '2', S1, 'quux', 'foo', '1' ];
    }
});
print('reflect ownKeys');
print(JSON.stringify(Reflect.ownKeys(P).map(String)));
var obj = { quux: 'mine' };
Object.setPrototypeOf(obj, P);
print('for-in enum of obj');
for (var k in obj) {
    print(String(k));
}
print('for-in enum of target');
for (var k in target) {
    print(String(k));
}
