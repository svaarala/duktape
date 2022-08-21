/*
 *  Postfix increment and decrement operators (E5 Sections 11.3.1, 11.3.2).
 */

/*===
123 234
123
124
125
124
ReferenceError
ReferenceError
123 234
===*/

var x = 123;
var y = 234;

print(x, y);

print(x++);
print((x)++);
print(x--);
print((x)--);

try {
    eval('print((x, y)++)');
} catch (e) {
    print(e.name);
}
try {
    eval('print((x, y)--)');
} catch (e) {
    print(e.name);
}

print(x, y);
