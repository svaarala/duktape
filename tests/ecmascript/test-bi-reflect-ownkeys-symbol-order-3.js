/*===
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
obj['foo'] = 'foo';
obj[S2] = 's2';
obj[S3] = 's3';
obj['bar'] = 'bar';
obj[S1] = 's1';
obj['quux'] = 'quux';

Reflect.ownKeys(obj).forEach(function (v) {
    if (v === S1) { print('S1'); }
    else if (v === S2) { print('S2'); }
    else if (v === S3) { print('S3'); }
    else { print(v); }
});
