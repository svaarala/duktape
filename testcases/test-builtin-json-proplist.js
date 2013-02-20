print=console.log;

/*===
{"foo":1,"bar":2,"baz":4}
{"baz":4,"foo":1,"bar":2,"baz":4,"baz":4}
{"foo":1,"bar":2,"baz":4}
{"baz":4,"foo":1,"bar":2,"baz":4,"baz":4}
===*/

/* If the 2nd argument to stringify() is an array, it becomes the
 * PropertyList of the serialization algorithm and affects the JO()
 * algorithm concretely.
 *
 * The specification requires that serialization of properties will:
 *
 *   (1) allow serialization of non-enumerable properties
 *   (2) will follow PropertyList order, not the object's enumeration order
 *      (order of properties in Object.keys())
 *   (3) allows the same property to be serialized multiple times
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

    // different order, 'baz' appears multiple times
    var txt = JSON.stringify(obj, [ 'baz', 'foo', 'bar', 'baz', 'baz' ]);
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

    var txt = JSON.stringify(obj, [ 'baz', 'foo', 'bar', 'baz', 'baz' ]);
    print(txt);
}

try {
    stringifyPropertyListTest1();
    stringifyPropertyListTest2();
} catch (e) {
    print(e.name);
}

