/*===
- Object.preventExtensions()
preventExtensions called
1
true
true
result: [object Object]
preventExtensions called
1
true
true
TypeError
preventExtensions called
1
true
true
TypeError
preventExtensions called
1
true
true
TypeError
- Reflect.preventExtensions()
preventExtensions called
1
true
true
result: true
preventExtensions called
1
true
true
TypeError
preventExtensions called
1
true
true
result: false
preventExtensions called
1
true
true
result: false
===*/

function objectPreventExtensions() {
    var target;
    var handler;
    var P;

    // Basic case: return trueish, and target has been made
    // non-extensible.
    target = {};
    var handler = {
        preventExtensions: function (targ) {
            print('preventExtensions called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            Object.preventExtensions(target);
            return true;
        }
    };
    try {
        P = new Proxy(target, handler);
        print('result: ' + Object.preventExtensions(P));
    } catch (e) {
        print(e.name);
    }

    // Basic case: return trueish, but target not extensible,
    // causes Proxy policy error, which is an unconditional throw.
    target = {};
    var handler = {
        preventExtensions: function (targ) {
            print('preventExtensions called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            return 1
        }
    };
    try {
        P = new Proxy(target, handler);
        print('result: ' + Object.preventExtensions(P));
    } catch (e) {
        print(e.name);
    }

    // Basic case: return falsy, but target is extensible.
    // This doesn't cause a policy error, but the false
    // [[PreventExtensions]] result causes a throw in
    // Object.preventExtensions().
    target = {};
    var handler = {
        preventExtensions: function (targ) {
            print('preventExtensions called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            return 0;
        }
    };
    try {
        P = new Proxy(target, handler);
        print('result: ' + Object.preventExtensions(P));
    } catch (e) {
        print(e.name);
    }

    // Basic case: return falsy, but target is not extensible.
    target = {};
    var handler = {
        preventExtensions: function (targ) {
            print('preventExtensions called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            Object.preventExtensions(target);
            return false;
        }
    };
    try {
        P = new Proxy(target, handler);
        print('result: ' + Object.preventExtensions(P));
    } catch (e) {
        print(e.name);
    }
}

// Same test but with Reflect.preventExtensions() which returns the
// [[PreventExtensions]] result as is.  Proxy policy still throws.
function reflectPreventExtensions() {
    var target;
    var handler;
    var P;

    target = {};
    var handler = {
        preventExtensions: function (targ) {
            print('preventExtensions called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            Object.preventExtensions(target);
            return true;
        }
    };
    try {
        P = new Proxy(target, handler);
        print('result: ' + Reflect.preventExtensions(P));
    } catch (e) {
        print(e.name);
    }

    target = {};
    var handler = {
        preventExtensions: function (targ) {
            print('preventExtensions called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            return 1
        }
    };
    try {
        P = new Proxy(target, handler);
        print('result: ' + Reflect.preventExtensions(P));
    } catch (e) {
        print(e.name);
    }

    target = {};
    var handler = {
        preventExtensions: function (targ) {
            print('preventExtensions called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            return 0;
        }
    };
    try {
        P = new Proxy(target, handler);
        print('result: ' + Reflect.preventExtensions(P));
    } catch (e) {
        print(e.name);
    }

    target = {};
    var handler = {
        preventExtensions: function (targ) {
            print('preventExtensions called');
            print(arguments.length);
            print(this === handler);
            print(targ === target);
            Object.preventExtensions(target);
            return false;
        }
    };
    try {
        P = new Proxy(target, handler);
        print('result: ' + Reflect.preventExtensions(P));
    } catch (e) {
        print(e.name);
    }
}

try {
    print('- Object.preventExtensions()');
    objectPreventExtensions();
} catch (e) {
    print(e.stack || e);
}

try {
    print('- Reflect.preventExtensions()');
    reflectPreventExtensions();
} catch (e) {
    print(e.stack || e);
}
