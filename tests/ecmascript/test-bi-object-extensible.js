/*===
extensible: true
extensible: false
extensible: false
undefined
bar
===*/

function basicTest() {
    function printObj(o) {
        print('extensible: ' + Object.isExtensible(o));
    }

    var proto = {};
    var obj = Object.create(proto);

    printObj(obj);
    Object.preventExtensions(obj);
    printObj(obj);
    Object.preventExtensions(obj);
    printObj(obj);

    try {
        obj.foo = 'bar';
    } catch (e) {
        print(e.name);
    }

    print(obj.foo);

    // ancestor can still be extended

    try {
        proto.foo = 'bar';
    } catch (e) {
        print(e.name);
    }

    print(obj.foo);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
isExtensible 0
false
isExtensible 1
false
isExtensible 2
false
isExtensible 3
false
isExtensible 4
false
isExtensible 5
false
isExtensible 6
true
isExtensible 7
true
preventExtensions 0
undefined
preventExtensions 1
null
preventExtensions 2
true
preventExtensions 3
false
preventExtensions 4
123
preventExtensions 5
foo
preventExtensions 6
1,2,3
preventExtensions 7
[object Object]
===*/

function coercionTest() {
    // Note: ES5 behavior was to throw a TypeError for non-object values.  ES2015
    // changes this to treat them as already non-extensible objects instead.
    // This goes for undefined and null too, even though they are not normally
    // object coercible!

    var values = [ undefined, null, true, false, 123, 'foo', [1,2,3], { foo: 1, bar: 1 } ];

    for (i = 0; i < values.length; i++) {
        print('isExtensible', i);
        print(Object.isExtensible(values[i]));
    }

    for (i = 0; i < values.length; i++) {
        print('preventExtensions', i);
        print(Object.preventExtensions(values[i]));
    }
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
