/*===
function
number 0
number NaN [object Number]
number 0 [object Number]
number 1 [object Number]
number 0 [object Number]
number 123 [object Number]
number 0 [object Number]
number 123 [object Number]
number NaN [object Number]
number NaN [object Number]
number NaN [object Number]
number NaN [object Number]
arg valueOf
number 321 [object Number]
===*/

print('function');

function functionTest() {
    var t;

    function test(x, nargs) {
        try {
            if (nargs === 0) {
                t = Number();
            } else {
                t = Number(x);
            }
            print(typeof t, t, Object.prototype.toString.call(t));
        } catch (e) {
            print(e.name);
        }
    }

    // no arg given should be treated as +0
    t = Number(); print(typeof t, t);

    // other basic argument types
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('');
    test('123');
    test('  123foo');
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });

    // coercion side effect

    test({
        toString: function() { print('arg toString'); return '123'; },
        valueOf: function() { print('arg valueOf'); return '321'; }
    });
}

try {
    functionTest();
} catch (e) {
    print(e);
}

/*===
constructor
object 0 [object Number] true true
object NaN [object Number] true true
object 0 [object Number] true true
object 1 [object Number] true true
object 0 [object Number] true true
object 123 [object Number] true true
object 0 [object Number] true true
object 123 [object Number] true true
object NaN [object Number] true true
object NaN [object Number] true true
object NaN [object Number] true true
object NaN [object Number] true true
arg valueOf
object 321 [object Number] true true
===*/

print('constructor');

function constructorTest() {
    var t;

    function test(x, nargs) {
        try {
            if (nargs === 0) {
                t = new Number();
            } else {
                t = new Number(x);
            }
            print(typeof t, t, Object.prototype.toString.call(t),
                  Object.getPrototypeOf(t) === Number.prototype,
                  Object.isExtensible(t));
        } catch (e) {
            print(e.name);
        }
    }

    // no arg given should be treated as +0
    test(undefined, 0);

    // other basic argument types
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('');
    test('123');
    test('  123foo');
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });

    // coercion side effect

    test({
        toString: function() { print('arg toString'); return '123'; },
        valueOf: function() { print('arg valueOf'); return '321'; }
    });
}

try {
    constructorTest();
} catch (e) {
    print(e);
}

/*===
prototype
undefined
true
true
false
===*/

/* When called as a constructor, a Number instance will always have the
 * original Number.prototype regardless of what Number.prototype is now
 * set to.  E5 Section 15.7.2.1.
 *
 * However, Number.prototype is not writable or configurable, so this
 * behavior doesn't need to be implemented explicitly; just ensure that
 * Number.prototype has the right attributes.
 */

print('prototype');

function prototypeTest() {
    var orig_prototype = Number.prototype;
    var repl_prototype = { "foo": "bar" };
    Number.prototype = repl_prototype;  /* this write will fail silently */

    var num = new Number(123);
    print(num.foo);
    print(Object.getPrototypeOf(num) === Number.prototype);
    print(Object.getPrototypeOf(num) === orig_prototype);
    print(Object.getPrototypeOf(num) === repl_prototype);
}

try {
    prototypeTest();
} catch (e) {
    print(e);
}
