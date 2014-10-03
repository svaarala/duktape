/*
 *  E5 Section 10.6, main algorithm step 14 requires that a strict
 *  arguments object 'callee' and 'caller' throwers must be the specific
 *  thrower defined in E5 Section 13.2.3.
 */

/*===
test get and set
object
object
object
object
true
true
true
true
true
true
true
TypeError
TypeError
===*/

function f(x,y) { 'use strict'; return arguments; };
function g(x,y) { 'use strict'; return arguments; };

var a = f(1,2,3);
var b = g(3,2,1);

pd1 = Object.getOwnPropertyDescriptor(a, "caller");
pd2 = Object.getOwnPropertyDescriptor(a, "callee");
pd3 = Object.getOwnPropertyDescriptor(b, "caller");
pd4 = Object.getOwnPropertyDescriptor(b, "callee");

print('test get and set')
print(typeof pd1);
print(typeof pd2);
print(typeof pd3);
print(typeof pd4);

// all of these should print true

print(pd1.get === pd1.set);
print(pd2.get === pd2.set);
print(pd3.get === pd3.set);
print(pd4.get === pd4.set);

print(pd1.get === pd2.get);
print(pd2.get === pd3.get);
print(pd3.get === pd4.get);

try {
    pd1.get();
} catch (e) {
    print(e.name);
}

try {
    pd2.get();
} catch (e) {
    print(e.name);
}
