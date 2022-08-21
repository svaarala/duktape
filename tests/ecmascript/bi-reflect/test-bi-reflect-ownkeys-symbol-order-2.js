/*===
0
1
2
3
7
foo
bar
quux
S2
S3
S1
===*/

var S1 = Symbol('s1');
var S2 = Symbol('s2');
var S3 = Symbol('s3');

var obj = {};
obj[1] = 'one';
obj['foo'] = 'foo';
obj[0] = 'zero';
obj[3] = 'three';
obj[S2] = 's2';
obj[2] = 'two';
obj[S3] = 's3';
obj['bar'] = 'bar';
obj[S1] = 's1';
obj['quux'] = 'quux';
obj[7] = 'seven';

Reflect.ownKeys(obj).forEach(function (v) {
    if (v === S1) { print('S1'); }
    else if (v === S2) { print('S2'); }
    else if (v === S3) { print('S3'); }
    else { print(v); }
});
