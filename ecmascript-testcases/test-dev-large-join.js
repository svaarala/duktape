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
joining
1000000
===*/

/* This was broken at some point: joining happened naively through the
 * valstack whose size topped out for large joins.
 */

tmp = [];

/* The 'tmp' array build becomes sparse (array part abandoned) so this is
 * very memory and CPU intensive now.  It shouldn't affect the join test
 * though.
 */

print('building');
for (i = 1000000; i; i -= 1) {  // funny syntax; current version does not support comparisons yet
	tmp[tmp.length] = 'x';
}

print('joining');
print(tmp.join('').length);

