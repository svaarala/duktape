/*===
test 1
0 Symbol(foo)
1 Symbol(bar)
test 2
test 3
0 Symbol(foo)
test 4
0 Symbol(foo)
test 5
test 6
test 7
done
===*/

function basicTest() {
    var S1 = Symbol('foo');
    var S2 = Symbol.for('bar');
    var S3 = Symbol.for('quux');
    var O;

    Object.prototype[S3] = 'inherited';

    print('test 1');
    O = { foo: 'a', [S1]: 'b', [S2]: 'c', bar: 'd' };
    Object.getOwnPropertySymbols(O).forEach(function (v, i) {
        print(i, String(v));
    });

    print('test 2');
    O = [ 1, 2, 3 ];
    Object.getOwnPropertySymbols(O).forEach(function (v, i) {
        print(i, String(v));
    });

    print('test 3');
    O = [ 1, 2, 3 ];
    O[S1] = 'a';
    Object.getOwnPropertySymbols(O).forEach(function (v, i) {
        print(i, String(v));
    });

    print('test 4');
    O = { [S1]: 'a' };
    Object.getOwnPropertySymbols(O).forEach(function (v, i) {
        print(i, String(v));
    });

    print('test 5');
    O = 'foo';
    Object.getOwnPropertySymbols(O).forEach(function (v, i) {
        print(i, String(v));
    });

    print('test 6');
    O = Object('foo');
    Object.getOwnPropertySymbols(O).forEach(function (v, i) {
        print(i, String(v));
    });

    print('test 7');
    O = new Uint8Array(10);
    Object.getOwnPropertySymbols(O).forEach(function (v, i) {
        print(i, String(v));
    });
}

try {
    basicTest();
} catch (e) {
    print(e.stack || e);
}
print('done');
