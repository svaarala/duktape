/*
 *  Boolean objects (E5 Section 15.6).
 */

/*===
constructor as a function
undefined boolean false
undefined boolean false
null boolean false
true boolean true
false boolean false
-Infinity boolean true
-123 boolean true
0 boolean false
0 boolean false
123 boolean true
Infinity boolean true
NaN boolean false
 boolean false
foo boolean true
 boolean true
1,2 boolean true
[object Object] boolean true
[object Object] boolean true
===*/

/* Boolean constructor called as a function converts a value. */

print('constructor as a function');

function constructorAsFunctionTest(x) {
    var t = Boolean(x);
    print(x, typeof t, t);
}

try {
    constructorAsFunctionTest();
    constructorAsFunctionTest(undefined);
    constructorAsFunctionTest(null);
    constructorAsFunctionTest(true);
    constructorAsFunctionTest(false);
    constructorAsFunctionTest(Number.NEGATIVE_INFINITY);
    constructorAsFunctionTest(-123.0);
    constructorAsFunctionTest(-0.0);
    constructorAsFunctionTest(+0.0);
    constructorAsFunctionTest(123.0);
    constructorAsFunctionTest(Number.POSITIVE_INFINITY);
    constructorAsFunctionTest(Number.NaN);
    constructorAsFunctionTest('');
    constructorAsFunctionTest('foo');
    constructorAsFunctionTest([]);
    constructorAsFunctionTest([1,2]);
    constructorAsFunctionTest({});
    constructorAsFunctionTest({foo:1, bar:2});
} catch (e) {
    print(e.name, e);
}

/*===
constructor
undefined object false [object Boolean] false
undefined object false [object Boolean] false
null object false [object Boolean] false
true object true [object Boolean] true
false object false [object Boolean] false
-Infinity object true [object Boolean] true
-123 object true [object Boolean] true
0 object false [object Boolean] false
0 object false [object Boolean] false
123 object true [object Boolean] true
Infinity object true [object Boolean] true
NaN object false [object Boolean] false
 object false [object Boolean] false
foo object true [object Boolean] true
 object true [object Boolean] true
1,2 object true [object Boolean] true
[object Object] object true [object Boolean] true
[object Object] object true [object Boolean] true
===*/

/* Constructor called with 'new'. */

print('constructor');

function constructorTest(x) {
    var t = new Boolean(x);
    var v;

    if (Object.prototype.toString.call(t) === '[object Boolean]') {
        v = t.valueOf();
    }

    print(x, typeof t, t, Object.prototype.toString.call(t), v);
}

try {
    constructorTest();
    constructorTest(undefined);
    constructorTest(null);
    constructorTest(true);
    constructorTest(false);
    constructorTest(Number.NEGATIVE_INFINITY);
    constructorTest(-123.0);
    constructorTest(-0.0);
    constructorTest(+0.0);
    constructorTest(123.0);
    constructorTest(Number.POSITIVE_INFINITY);
    constructorTest(Number.NaN);
    constructorTest('');
    constructorTest('foo');
    constructorTest([]);
    constructorTest([1,2]);
    constructorTest({});
    constructorTest({foo:1, bar:2});
} catch (e) {
    print(e.name);
}

/*===
instance
true
[object Boolean]
===*/

print('instance');

function instanceTest() {
    var x = new Boolean(true);

    // prototype; cannot change because Boolean's "prototype" property
    // is non-configurable and non-writable
    print(Object.getPrototypeOf(x) === Boolean.prototype);

    // class
    print(Object.prototype.toString.call(x));
}

try {
    instanceTest();
} catch (e) {
    print(e.name);
}

/*===
toString
TypeError
boolean true string true
boolean false string false
object true string true
object false string false
===*/

/* toString() */

print('toString');

function toStringTest(x) {
    var t = Boolean.prototype.toString.call(x);
    print(typeof x, x, typeof t, t);
}

try {
    // 'this' binding must be a primitive boolean or a Boolean object
    toStringTest('true');
} catch (e) {
    print(e.name);
}

try {
    toStringTest(true);
} catch (e) {
    print(e.name);
}

try {
    toStringTest(false);
} catch (e) {
    print(e.name);
}

try {
    toStringTest(new Boolean(true));
} catch (e) {
    print(e.name);
}

try {
    toStringTest(new Boolean(false));
} catch (e) {
    print(e.name);
}

/*===
valueOf
TypeError
boolean true boolean true
boolean false boolean false
object true boolean true
object false boolean false
===*/

/* valueOf() */

print('valueOf');

function valueOfTest(x) {
    var t = Boolean.prototype.valueOf.call(x);
    print(typeof x, x, typeof t, t);
}

try {
    valueOfTest('true');
} catch (e) {
    print(e.name);
}

try {
    valueOfTest(true);
} catch (e) {
    print(e.name);
}

try {
    valueOfTest(false);
} catch (e) {
    print(e.name);
}

try {
    valueOfTest(new Boolean(true));
} catch (e) {
    print(e.name);
}

try {
    valueOfTest(new Boolean(false));
} catch (e) {
    print(e.name);
}
