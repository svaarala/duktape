/*
 *  Test the property list (2nd argument) of JSON.stringify().
 */

/*===
{"foo":1,"bar":2,"baz":4}
{"baz":4,"foo":1,"bar":2}
{"foo":1,"bar":2,"baz":4}
{"baz":4,"foo":1,"bar":2}
===*/

/* If the 2nd argument to stringify() is an array, it becomes the
 * PropertyList of the serialization algorithm and affects the JO()
 * algorithm concretely.
 *
 * The specification requires that serialization of properties will:
 *
 *   (1) allow serialization of non-enumerable properties
 *   (2) will follow PropertyList order, not the object's enumeration
 *       order (order of properties in Object.keys())
 *   (3) block serializing the same property name multiple times
 *       (E5.1 Section 15.12.3, main algorithm, step 4.b.ii.5)
 *
 * E5.1 Section 15.12.3, main algorithm, step 4.b.ii.ii is tricky:
 * it requires that array indexed properties are enumerated in
 * ascending order.  This is stricter than normal enumeration
 * requirements and causes some compliance issues in the current
 * implementation when a sparse array is used.  There are separate
 * tests for this case.
 *
 * Note that at least V8 does not block serialization of the same
 * property name.  There is a separate test for this, too.
 */

function stringifyPropertyListTest1() {
    var obj = {
        "foo": 1,
        "bar": 2,
        "quux": 3
    };

    Object.defineProperties(obj, {
        baz: { value: 4, enumerable: false, configurable: true, writable: true }
    });

    // baz is non-enumerable
    var txt = JSON.stringify(obj, [ 'foo', 'bar', 'baz' ]);
    print(txt);

    // different order
    var txt = JSON.stringify(obj, [ 'baz', 'foo', 'bar' ]);
    print(txt);
}

function stringifyPropertyListTest2() {
    // test that inherited properties are also correctly enumerated
    // when using a PropertyList

    var proto = {};

    function F() {
        // quux and baz are inherited
        this.foo = 1;
        this.bar = 2;
    }
    F.prototype = proto;

    var obj;

    Object.defineProperties(proto, {
        quux: { value: 3, enumerable: true, writable: true, configurable: true },
        baz: { value: 4, enumerable: false, writable: true, configurable: true },
    });

    obj = new F();

    var txt = JSON.stringify(obj, [ 'foo', 'bar', 'baz' ]);
    print(txt);

    var txt = JSON.stringify(obj, [ 'baz', 'foo', 'bar' ]);
    print(txt);
}

try {
    stringifyPropertyListTest1();
    stringifyPropertyListTest2();
} catch (e) {
    print(e.name);
}

/*===
{"foo":1,"baz":4}
{"foo":1,"baz":4,"bar":2}
{"foo":1,"baz":4}
{"foo":1,"1.2":"val:1.2","2.2":"val:2.2","NaN":"val:NaN"}
===*/

/* Test invalid values in the property list: anything other than a number,
 * a string, a Number object, or a String object is ignored.
 */

function stringifyPropertyListTest3() {
    var obj = { foo: 1, bar: 2, quux: 3, baz: 4 };

    // add some properties to ensure invalid PropertyList keys are not
    // coerced incorrectly and look up one of these
    obj['' + true] = 'val:true';
    obj['' + false] = 'val:false';
    obj['null'] = 'val:null';
    obj['0'] = 'val:0';
    obj['1'] = 'val:1';

    // these will be legitimately accessed, numbers are coerced
    obj['1.2'] = 'val:1.2';
    obj['2.2'] = 'val:2.2';
    obj['NaN'] = 'val:NaN';

    // undefined will be skipped
    print(JSON.stringify(obj, [ 'foo', undefined, 'baz' ]));

    // null, true, false will be skipped
    print(JSON.stringify(obj, [ 'foo', null, 'baz', true, false, 'bar' ]));

    // function will be skipped, Date will be skipped, array and object
    // will be skipped
    print(JSON.stringify(obj, [ 'foo', function () {}, new Date(0), {}, [], 'baz' ]));

    // null will be skipped
    // ToString(1.2) = '1.2'; ToString(new Number(2.2)) = '2.2', ToString(0/0) = 'NaN'
    print(JSON.stringify(obj, [ 'foo', null, 1.2, new Number(2.2), 0/0 ]));
}

try {
    stringifyPropertyListTest3();
} catch (e) {
    print(e.name);
}
