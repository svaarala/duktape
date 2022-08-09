/*===
- passthrough
true
false
- falsish
true
preventExtensions trap: true true
TypeError
true
- truish, but target still extensible
true
preventExtensions trap: true true
TypeError
true
- truish, target not extensible
true
preventExtensions trap: true true
false
- side effects, truish
true
preventExtensions trap: true true
isExtensible trap: true true
false
- side effects, falsish
true
preventExtensions trap: true true
TypeError
false
===*/

function testBasic() {
    // Pass-through case.
    print('- passthrough');
    var target = { foo: 123 };
    var P = new Proxy(target, {});
    print(Object.isExtensible(target));
    Object.preventExtensions(P);
    print(Object.isExtensible(target));

    // Proxy trap returns falsish, i.e. fail to prevent extensions.
    print('- falsish');
    var target = { foo: 123 };
    var handler = {};
    handler.preventExtensions = function (targ) {
        print('preventExtensions trap:', this === handler, targ === target);
        return 0;  // Failed to prevent.
    };
    var P = new Proxy(target, handler);
    print(Object.isExtensible(target));
    try {
        Object.preventExtensions(P);
    } catch (e) {
        print(e.name);
        //print(e.stack);
    }
    print(Object.isExtensible(target));

    // Proxy trap returns truish, i.e. succeeded to prevent extensions, but target is still
    // extensible.
    print('- truish, but target still extensible');
    var target = { foo: 123 };
    var handler = {};
    handler.preventExtensions = function (targ) {
        print('preventExtensions trap:', this === handler, targ === target);
        return 1;
    };
    var P = new Proxy(target, handler);
    print(Object.isExtensible(target));
    try {
        Object.preventExtensions(P);
    } catch (e) {
        print(e.name);
        //print(e.stack);
    }
    print(Object.isExtensible(target));

    // Proxy trap returns truish, i.e. succeeded to prevent extensions, target also
    // made non-extensible.
    print('- truish, target not extensible');
    var target = { foo: 123 };
    var handler = {};
    handler.preventExtensions = function (targ) {
        print('preventExtensions trap:', this === handler, targ === target);
        Object.preventExtensions(target);
        return 1;
    };
    var P = new Proxy(target, handler);
    print(Object.isExtensible(target));
    try {
        Object.preventExtensions(P);
    } catch (e) {
        print(e.name);
        //print(e.stack);
    }
    print(Object.isExtensible(target));
}

function testSideEffects() {
    // Observe IsExtensible(target) by making target a proxy.  Called only when
    // proxy trap returns truish.
    print('- side effects, truish');
    var finalTarget = { foo: 123 };
    var targetHandler = {};
    targetHandler.isExtensible = function (targ) {
        print('isExtensible trap:', this === targetHandler, targ === finalTarget);
        return Object.isExtensible(finalTarget);
    };
    var target = new Proxy(finalTarget, targetHandler);
    var handler = {};
    handler.preventExtensions = function (targ) {
        print('preventExtensions trap:', this === handler, targ === target);
        Object.preventExtensions(target);
        return 1;
    };
    var P = new Proxy(target, handler);
    print(Object.isExtensible(finalTarget));
    try {
        Object.preventExtensions(P);
    } catch (e) {
        print(e.name);
        //print(e.stack);
    }
    print(Object.isExtensible(finalTarget));

    // With falsish trap result isExtensible not called.
    print('- side effects, falsish');
    var finalTarget = { foo: 123 };
    var targetHandler = {};
    targetHandler.isExtensible = function (targ) {
        print('isExtensible trap:', this === targetHandler, targ === finalTarget);
        return Object.isExtensible(finalTarget);
    };
    var target = new Proxy(finalTarget, targetHandler);
    var handler = {};
    handler.preventExtensions = function (targ) {
        print('preventExtensions trap:', this === handler, targ === target);
        Object.preventExtensions(target);
        return 0;
    };
    var P = new Proxy(target, handler);
    print(Object.isExtensible(finalTarget));
    try {
        Object.preventExtensions(P);
    } catch (e) {
        print(e.name);
        //print(e.stack);
    }
    print(Object.isExtensible(finalTarget));
}

testBasic();
testSideEffects();
