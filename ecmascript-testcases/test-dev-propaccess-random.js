/*
 *  Some stressing of object property insertion and deletion.
 */

/*===
0
1000000
2000000
3000000
4000000
5000000
6000000
7000000
8000000
9000000
===*/

function randomPropTest() {
    var obj = {};
    var i, j;
    var k, v;

    for (i = 0; i < 10000000; i++) {
        if (i % 1000000 === 0) {
            print(i);
            //print(Duktape.enc('jx', obj));
        }

        k = Math.floor(Math.random() * 10000);
        if (k < 1000) { k = k; }
        else { k = "key-" + String(k); }

        v = Math.floor(Math.random() * 100);
        if (v < 10) { v = undefined; }
        else if (v < 50) { v = i; }
        else if (v < 80) { v = { foo: i }; }
        else { v = "value-" + i; }

        if (v === undefined) {
            delete obj[k];
        } else {
            obj[k] = v;
        }
    }
}

try {
    randomPropTest();
} catch (e) {
    print(e);
}
