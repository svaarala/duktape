var t;

/*===
1 2 3
3
===*/

print(1,2,3)
print((1,2,3))

/*===
foo
bar
undefined
foo
bar
3
===*/

/* This parses as: (t = print('foo')), (print('bar')), (1+2);
 * So 't' gets return value of print('foo') = undefined.
 */
t = print('foo'), print('bar'), 1 + 2;
print(t);

t = (print('foo'), print('bar'), 1 + 2);
print(t);

/*===
3
===*/

t = eval("1, 2, 3");
print(t);
