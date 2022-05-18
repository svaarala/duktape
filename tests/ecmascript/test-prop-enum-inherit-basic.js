/*===
1
2
7
bar
foo
0
4
5
abc
quux
===*/

try {
    var o1 = { abc: 987, foo: 123, '0': true, '5': true, '4': true, '7': true, '1': true, bar: 234, quux: 345 };
    var o2 = { bar: 321, foo: 432, '1': true, '2': true, '7': true };
    Object.setPrototypeOf(o2, o1);
    for (var k in o2) {
        print(k);
    }
} catch (e) {
    print(e.stack || e);
}
