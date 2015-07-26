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
TypeError
isExtensible 1
TypeError
isExtensible 2
TypeError
isExtensible 3
TypeError
isExtensible 4
TypeError
isExtensible 5
TypeError
isExtensible 6
isExtensible 7
preventExtensions 0
TypeError
preventExtensions 1
TypeError
preventExtensions 2
TypeError
preventExtensions 3
TypeError
preventExtensions 4
TypeError
preventExtensions 5
TypeError
preventExtensions 6
preventExtensions 7
===*/

function coercionTest() {
    var values = [ undefined, null, true, false, 123, 'foo', [1,2,3], { foo: 1, bar: 1 } ];

    for (i = 0; i < values.length; i++) {
        print('isExtensible', i);
        try {
            Object.isExtensible(values[i]);
        } catch (e) {
            print(e.name);
        }
    }

    for (i = 0; i < values.length; i++) {
        print('preventExtensions', i);
        try {
            Object.preventExtensions(values[i]);
        } catch (e) {
            print(e.name);
        }
    }
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
