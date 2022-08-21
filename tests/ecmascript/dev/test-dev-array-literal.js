/*===
0
undefined
1
1
undefined
2
1
2
undefined
===*/

print([].length);
print([][0]);

print([1].length);
print([1][0]);
print([1][1]);

print([1,2].length);
print([1,2][0]);
print([1,2][1]);
print([1,2][2]);

/*===
11
20
===*/

/* two sets of MPUTARR */

print([1,2,3,4,5,6,7,8,9,10,11].length);  /* 10 + 1 */
print([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20].length); /* 10 + 10 */

/*===
0
1
1
1
2
2
2
3
===*/

/* elisions */

print([].length);
print([,].length);     /* here, even one comma has an effect */
print([1].length);
print([1,].length);    /* here, one additional comma has no effect */
print([1,,].length);   /* but two do */
print([1,2].length);
print([1,2,].length);  /* quite confusingly, one additional comma has no effect */
print([1,2,,].length); /* but two do */

/*===
2
===*/

/* regexp values */

print([/foo/,/bar/].length);
