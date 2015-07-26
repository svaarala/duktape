/*
 *  Large join() needs a valstack check.  Even larger joins run
 *  out of valstack unless the join is done in stages.
 *
 *  These were broken at some point.
 */

/*===
100
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
===*/

/* 100 elements */
tmp = [
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9
];

print(tmp.length);
print(tmp.join(''));

/*===
building
1000000
joining
1000000
checking
ok
===*/

/* This was broken at some point: joining happened naively through the
 * valstack whose size topped out for large joins.
 */

tmp = [];
var i;
var res;
var limit = 1e6;

print('building');
for (i = 0; i < limit; i++) {
	tmp[tmp.length] = String.fromCharCode(i % 65536);
}
print(tmp.length);

print('joining');
res = tmp.join('');
print(res.length);

print('checking');
for (i = 0; i < limit; i++) {
    if (res.charCodeAt(i) !== (i % 65536)) {
        throw new Error('invalid char at offset ' + i);
    }
}

print('ok');
