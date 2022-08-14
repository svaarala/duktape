/*===
isExtensible called
1
true
true
result: true
isExtensible called
1
true
true
result: false
isExtensible called
1
true
true
TypeError
isExtensible called
1
true
true
TypeError
===*/

function test() {
    var target;
    var handler;
    var P;

    // Basic case: return trueish and target is extensible.
    target = {};
    handler = {
        isExtensible: function (targ) {
            print('isExtensible called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            return 1;
        }
    };
    P = new Proxy(target, handler);
    try {
        print('result: ' + Object.isExtensible(P));
    } catch (e) {
        print(e.name);
    }

    // Basic case: return falsy and target is not extensible.
    target = {};
    Object.preventExtensions(target);
    handler = {
        isExtensible: function (targ) {
            print('isExtensible called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            return '';
        }
    };
    P = new Proxy(target, handler);
    try {
        print('result: ' + Object.isExtensible(P));
    } catch (e) {
        print(e.name);
    }

    // Invariant violation.
    target = {};
    handler = {
        isExtensible: function (targ) {
            print('isExtensible called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            return 0;
        }
    };
    P = new Proxy(target, handler);
    try {
        print('result: ' + Object.isExtensible(P));
    } catch (e) {
        print(e.name);
    }

    // Invariant violation.
    target = {};
    Object.preventExtensions(target);
    handler = {
        isExtensible: function (targ) {
            print('isExtensible called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            return 1;
        }
    };
    P = new Proxy(target, handler);
    try {
        print('result: ' + Object.isExtensible(P));
    } catch (e) {
        print(e.name);
    }
}

test();
