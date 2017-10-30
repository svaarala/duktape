/*===
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
done
===*/

function test() {
    var i, j;
    var t1, t2, t3;
    for (i = 0; i < 25; i++) {
        print(i);
        t1 = new Uint8Array(1024 * 1024);
        for (j = 0; j < t1.length; j++) {
            t1[j] = Math.random() * 256;
        }
        t2 = Duktape.enc('base64', t1);
        t3 = Duktape.dec('base64', t2);
        if (t1.length !== t3.length) {
            throw new Error('roundtrip length mismatch: ' + t3.length);
        }
        for (j = 0; j < t1.length; j++) {
            if (t1[j] !== t3[j]) {
                throw new Error('roundtrip byte mismatch at offset ' + j);
            }
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
print('done');
