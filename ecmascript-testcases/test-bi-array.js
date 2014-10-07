/*
 *  Array objects (E5 Section 15.4).
 */

/*===
string 0 number 1
string 1 number 2
string 2 number 3
string length number 3
string 0 number 1
string 1 number 2
string 2 number 3
string 10 number 999
string length number 11
string 0 number 1
string 1 number 2
string length number 2
string 0 string foo
string 1 string bar
string 2 string quux
string length number 3
string length number 6
===*/

function dump(x) {
    Object.getOwnPropertyNames(x).forEach(function (k) {
        print(typeof k, k, typeof x[k], x[k]);
    });
}

function test() {
    var a;

    a = [ 1, 2, 3 ];
    dump(a);
    a[10] = 999;
    dump(a);
    a.length = 2;
    dump(a);

    a = new Array('foo', 'bar', 'quux');
    dump(a);

    a = new Array(6);
    dump(a);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
