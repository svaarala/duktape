/* FIXME: missing a lot of coercion cases */

/*===
-124
123
-1410065409
1410065408
===*/

print(~123);
print(~~123);
print(~1e10);
print(~~1e10);

/*===
-124
123
-2
1
===*/

/* Ensure that target is not modified (this would happen if expression is
 * not allocated a real temporary register but operates directly on a register
 * bound value).
 */

var x = 123;
print(~x);
print(x);

var obj = {x:1};
print(~obj.x);
print(obj.x);

