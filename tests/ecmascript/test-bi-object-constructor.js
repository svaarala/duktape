function dumpObject(o) {
    print(typeof o,
          Object.prototype.toString.call(o),
          Object.getPrototypeOf(o) === Object.prototype,
          Object.isExtensible(o));
}

/*===
object constructor as function
object [object Object] true true
object [object Object] true true
object [object Object] true true
object [object Boolean] false true
object [object Boolean] false true
object [object Number] false true
object [object String] false true
object [object Array] false true
true
object [object Object] true true
true
function [object Function] false true
true
===*/

/* Object constructor called as a function. */

print('object constructor as function');

function constructorAsFunctionTest() {
    var t1, t2;

    dumpObject(Object());
    dumpObject(Object(undefined));
    dumpObject(Object(null));
    dumpObject(Object(true));
    dumpObject(Object(false));
    dumpObject(Object(123.0));
    dumpObject(Object('foo'));

    // check that the same object comes back

    t1 = [];
    t2 = Object(t1);
    dumpObject(t1);
    print(t1 === t2);

    t1 = {};
    t2 = Object(t1);
    dumpObject(t1);
    print(t1 === t2);

    t1 = function() {};
    t2 = Object(t1);
    dumpObject(t1);
    print(t1 === t2);
}

try {
    constructorAsFunctionTest();
} catch (e) {
    print(e.name);
}

/*===
object constructor as constructor
object [object Object] true true
object [object Object] true true
object [object Object] true true
object [object Boolean] false true
object [object Boolean] false true
object [object Number] false true
object [object String] false true
object [object Array] false true
true
object [object Object] true true
true
function [object Function] false true
true
object [object Number] false true
===*/

/* Object constructor called as a constructor */

print('object constructor as constructor');

function constructorTest() {
    var t1, t2;

    dumpObject(new Object());
    dumpObject(new Object(undefined));
    dumpObject(new Object(null));
    dumpObject(new Object(true));
    dumpObject(new Object(false));
    dumpObject(new Object(123.0));
    dumpObject(new Object('foo'));

    // check that the same object comes back

    t1 = [];
    t2 = new Object(t1);
    dumpObject(t1);
    print(t1 === t2);

    t1 = {};
    t2 = new Object(t1);
    dumpObject(t1);
    print(t1 === t2);

    t1 = function() {};
    t2 = new Object(t1);
    dumpObject(t1);
    print(t1 === t2);

    // arguments beyond first optional arg are ignored
    t2 = new Object(123, 'foo');
    dumpObject(t2);
}

try {
    constructorTest();
} catch (e) {
    print(e.name);
}
