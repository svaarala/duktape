/*
 *  Object.assign() (E7 19.1.2.1)
 *  http://www.ecma-international.org/ecma-262/7.0/#sec-object.assign
 */

/*===
basic tests
true
pig
cow
ape
0
1
2
badger
whale
maggie Kittycow Machel Fatty Whale 812 1208 9001
0,1,2,pig,cow,ape,badger,whale
maggie 1208 9001 Vegeta
cow,pig,ape,saiyan
TypeError
foo read-only unchanged undefined
foo,bar,baz
undefined
this belongs here
undefined
prop1
get
set 812
===*/

function basicTest() {
    var obj, src, dest;

    // If no coercion is needed, target object is returned (i.e. not a copy).
    obj = {};
    dest = Object.assign(obj, { test: "test" });
    print(dest === obj);

    // Properties are assigned in [[OwnPropertyKeys]] order, moving from left
    // to right in the argument list:
    //     1. Array indices (sequential, ascending)
    //     2. String keys (creation order)
    //     3. Symbol keys (creation order)
    // The only reliable way to verify this is by forcing Object.assign() to
    // call getters and watch the side effects.
    var obj1 = {}, obj2 = {}, arr = [];
    Object.defineProperties(obj1, {
        pig: { enumerable: true, get: function() { print("pig"); return "maggie"; } },
        cow: { enumerable: true, get: function() { print("cow"); return "Kittycow"; } },
        ape: { enumerable: true, get: function() { print("ape"); return "Machel"; } }
    });
    Object.defineProperties(arr, {  // note: This abandons the array part.
        badger: { enumerable: true, get: function() { print("badger"); return "pig food"; } },
        0: { enumerable: true, get: function() { print("0"); return 812; } },
        2: { enumerable: true, get: function() { print("2"); return 9001; } },
        1: { enumerable: true, get: function() { print("1"); return 1208; } }
    });
    Object.defineProperties(obj2, {
        whale: { enumerable: true, get: function() { print("whale"); return "Fatty Whale"; } }
    });
    obj = Object.assign({}, obj1, arr, obj2);
    print(obj.pig, obj.cow, obj.ape, obj.whale, obj[0], obj[1], obj[2]);
    print(Reflect.ownKeys(obj));

    // Existing properties in the target object can be overwritten.
    obj = Object.assign({ cow: 1208, pig: 812, ape: 9001 },
        { pig: "maggie", saiyan: "Vegeta" });
    print(obj.pig, obj.cow, obj.ape, obj.saiyan);
    print(Reflect.ownKeys(obj));

    // If a property write fails, an error is thrown (strict mode semantics)
    // and the object is left partially updated:
    obj = Object.create(Object.prototype, {
        foo: { writable: true, value: "unchanged" },
        bar: { writable: false, value: "read-only" },
        baz: { writable: true, value: "unchanged" },
    });
    try {
        Object.assign(obj, { foo: "foo", bar: "bar", baz: "baz", quux: "quux" });
        print("never here");
    } catch (e) {
        print(e.name);
        print(obj.foo, obj.bar, obj.baz, obj.quux);
        print(Reflect.ownKeys(obj));
    }

    // Only enumerable own properties of the source(s) are used.
    src = Object.create({ inherited: "something stupid happened" }, {
        prop1: { enumerable: true,  value: "this belongs here" },
        prop2: { enumerable: false, value: "something stupid happened" }
    });
    obj = Object.assign({}, src);
    print(obj.inherited);
    print(obj.prop1);
    print(obj.prop2);
    print(Reflect.ownKeys(obj));

    // Accessors in both target and source object(s) are called.
    obj = Object.create(Object.prototype, {
        prop: { set: function(value) { print("set", value); }}
    });
    src = Object.create(Object.prototype, {
        prop: { enumerable: true, get: function() { print("get"); return 812; }}
    });
    Object.assign(obj, src);
}

try {
    print("basic tests");
    basicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
argument policy
object 812
*MUNCH*
object cows eat kitties
moooo *munch*
TypeError
TypeError
foo,bar
baz quux
===*/

function argumentTest() {
    var obj;

    // The first argument to Object.assign() is ToObject() coerced.  The
    // coerced value is what's returned by the function.
    obj = Object.assign(812, { test: "*MUNCH*" });
    print(typeof obj, obj);
    print(obj.test);

    // XXX: ToObject() coercion is specified for all arguments, is there a way
    //      to test for this?

    // Coercion is performed even if no sources are provided:
    obj = Object.assign("cows eat kitties", { test: "moooo *munch*" });
    print(typeof obj, obj);
    print(obj.test);

    // null and undefined are not object coercible!
    try {
        Object.assign(null);
        print("never here");
    } catch (e) {
        print(e.name);
    }
    try {
        Object.assign(undefined);
        print("never here");
    } catch (e) {
        print(e.name);
    }

    // However, both null and undefined are accepted as sources.  They are
    // treated as though they were empty objects.
    obj = Object.assign({}, undefined, { foo: "baz" }, null, { bar: "quux" });
    print(Reflect.ownKeys(obj));
    print(obj.foo, obj.bar);
}

try {
    print("argument policy");
    argumentTest();
} catch (e) {
    print(e.stack || e);
}

/*===
virtual properties
0 255 44
0 undefined
0,1,2
0 255 44
===*/

function virtualPropTest() {
    var u8 = new Uint8Array(3);
    var arr = [ 9001 ], obj;

    Object.assign(u8, { 0: 0, 1: 255, 2: 812 });
    print(u8[0], u8[1], u8[2]);

    Object.assign(arr, { length: 0 });
    print(arr.length, arr[0]);

    obj = Object.assign({}, u8);
    print(Reflect.ownKeys(obj));
    print(obj[0], obj[1], obj[2]);
}

try {
    print("virtual properties");
    virtualPropTest();
} catch (e) {
    print(e.stack || e);
}
