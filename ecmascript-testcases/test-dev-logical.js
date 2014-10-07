/*
 *  Basic logical operation tests
 */

var t;

/*===
1
1
2
3
===*/

print(1 || 2);
print(1 || print('never'));  /* short circuit */
print(0 || 2);
print(0 || 0 || 3);

/*===
false
true
0
===*/

print(false || false);
print(false || true);
print(false || 0);

/*===
2
always
undefined
0
3
===*/

print(1 && 2);
print(1 && print('always'));
print(0 && print('never'));
print(1 && 1 && 3);

/*===
true
false
0
===*/

print(true && true);
print(true && false);
print(true && 0);

/*===
A
E
finished
===*/

/*
 *  The following:
 *
 *    A && B || C && D && E || F && G
 *
 *  parses as:
 *
 *    (A && B) || (C && D && E) || (F && G)
 *
 *  print(X) always evaluates as false as it returns undefined.
 */

var p = print;

print(p('A') && p('B') || true && 1 && print('E') || true && 'finished');
