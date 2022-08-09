/*
 *  ES2016 added the concept of immutable prototype objects.
 *  There's only one such object so far, Object.prototype.
 */

/*===
null
TypeError
null
TypeError
null
still here
null
===*/

var op = Object.prototype;
print(Object.getPrototypeOf(op));
try {
    Object.setPrototypeOf(op, Object.create(null));
    print('never here');
} catch (e) {
    print(e.name);
}
print(Object.getPrototypeOf(op));
try {
    op.__proto__ = Object.create(null);
    print('never here');
} catch (e) {
    print(e.name);
}
print(Object.getPrototypeOf(op));
try {
    op.__proto__ = null;  // allowed, SameValue match
    print('still here');
} catch (e) {
    print(e.name);
}
print(Object.getPrototypeOf(op));
