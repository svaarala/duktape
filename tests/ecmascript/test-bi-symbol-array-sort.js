/*
 *  Array sort with a symbol
 */

/*===
array sort
TypeError
sort finished
0 number 1
1 number 2
2 number 3
3 string A
4 symbol Symbol()
5 symbol Symbol(abc)
6 symbol Symbol(bar)
7 symbol Symbol(foo)
8 symbol Symbol(quux)
9 string Z
elements done
===*/

function test() {
    // The default array sort algorithm does a ToString() coercion which
    // fails for a Symbol.
    try {
        arr = [ 1, 2, 3, Symbol() ];
        arr.sort();
        print('never here');
    } catch (e) {
        print(e.name);
    }

    // Array .sort() is still useful if you give a custom comparison function.
    try {
        arr = [ 1, 2, 3, Symbol(), Symbol.for('foo'), Symbol.for('bar'), Symbol('quux'), Symbol('abc'), 'A', 'Z' ];
        arr.sort(function (a,b) {
            if (typeof a === 'symbol') { a = String(a); }  // replace symbol with its description
            if (typeof b === 'symbol') { b = String(b); }
            a = String(a);  // compare only strings in this example, stabilizes sort
            b = String(b);
            if (a < b) {
                //print('SORT', a, b, '->', -1);
                return -1;
            }
            if (a > b) {
                //print('SORT', a, b, '->', 1);
                return 1;
            }
            //print('SORT', a, b, '->', 0);
            return 0;
        });
        print('sort finished');
        arr.forEach(function (v, i) { print(i, typeof v, String(v)); });
        print('elements done');
    } catch (e) {
        print(e.name);
    }
}

try {
    print('array sort');
    test();
} catch (e) {
    print(e.stack || e);
}
