/*===
0
1
2
0
1
2
===*/

/* Latter variant was broken at some point */

var test = 'Foo';

for (i in test) {
    print(i)
}

for (var i in test) {
    print(i)
}
