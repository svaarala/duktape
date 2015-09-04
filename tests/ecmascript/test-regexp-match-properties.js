/*
 *  Test properties and property attributes of match results.
 */

/*===
0 0 string foobarquux true true true
1 1 string bar true true true
2 index number 3 true true true
3 input string xyzfoobarquux true true true
4 length number 2 true false false
===*/

function test() {
    var re = /foo(...)quux$/;
    var m = re.exec('xyzfoobarquux');
    var props = Object.getOwnPropertyNames(m);

    props.sort();
    props.forEach(function (k, i) {
        var pd = Object.getOwnPropertyDescriptor(m, k);
        print(i, k, typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
