/* marked as custom because enum order differs */
/*---
{
    "custom": true
}
---*/

/*===
foo,bar
foo,bar,quux
0,1,2,length,foo
===*/

/* Some basic cases. */

var obj1 = { foo:1, bar:2 };

var obj2 = {};
Object.defineProperties(obj2, {
    'foo': { value: 123, writable: false, configurable: false, enumerable: false },
    'bar': { value: 123, writable: false, configurable: false, enumerable: true },
    'quux': { value: 123, writable: false, configurable: false, enumerable: false }
});

var obj3 = ['foo','bar','quux'];
Object.defineProperties(obj3, {
    'foo': { value: 123, writable: false, configurable: false, enumerable: false }
});

print(Object.getOwnPropertyNames(obj1));
print(Object.getOwnPropertyNames(obj2));
print(Object.getOwnPropertyNames(obj3));

/*===
foo,bar
===*/

/* Inherited property keys are not included. */

var ancestor = { inherited: 1 };
function f1() {
    this.foo = 1;
    this.bar = 2;
}
f1.prototype = ancestor;
var obj4 = new f1();

print(Object.getOwnPropertyNames(obj4));
