/*
 *  Prefix increment and decrement operators (E5 Sections 11.4.4, 11.4.5).
 */

/*===
123 234
124
125
124
123
ReferenceError
ReferenceError
123 234
===*/

var x = 123;
var y = 234;

print(x, y);

print(++x);
print(++(x));
print(--x);
print(--(x));

try {
    eval('print(++(x, y))');
} catch (e) {
    print(e.name);
}
try {
    eval('print(--(x, y))');
} catch (e) {
    print(e.name);
}

print(x, y);
