/*===
- Function.prototype.arguments get
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
- Function.prototype.arguments set
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
- Function.prototype.caller get
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
- Function.prototype.caller set
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
===*/

function strictFunc() {
    'use strict';
}

function nonStrictFunc() {
}

function test() {
    var protoArguments = Object.getOwnPropertyDescriptor(Function.prototype, 'arguments');
    var protoCaller = Object.getOwnPropertyDescriptor(Function.prototype, 'caller');

    print('- Function.prototype.arguments get');
    try {
        print(protoArguments.get.call(null));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoArguments.get.call(123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoArguments.get.call(Math.cos));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoArguments.get.call(strictFunc));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoArguments.get.call(nonStrictFunc));
    } catch (e) {
        print(e.name);
    }

    try {
        // Difference to V8, no own .arguments property with null value
        // to minimize function instance size.  Trigger thrower instead.
        // Both behaviors are compliant.
        print(nonStrictFunc.arguments);
    } catch (e) {
        print(e.name);
    }

    print('- Function.prototype.arguments set');
    try {
        print(protoArguments.set.call(null, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoArguments.set.call(123, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoArguments.set.call(Math.cos, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoArguments.set.call(strictFunc, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoArguments.set.call(nonStrictFunc, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        // Difference to V8 here, V8 provides a non-configurable and non-writable
        // own property (with null value).  To minimize function instance size we
        // avoid the own properties and trigger the thrower instead, just like for
        // strict functions.  Both behaviors are compliant.
        nonStrictFunc.arguments = 123;
        print(nonStrictFunc.arguments);
    } catch (e) {
        print(e.name);
    }

    print('- Function.prototype.caller get');
    try {
        print(protoCaller.get.call(null));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoCaller.get.call(123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoCaller.get.call(Math.cos));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoCaller.get.call(strictFunc));
    } catch (e) {
        print(e.name);
    }

    try {
        // Difference to V8 here, see comments for .arguments above.
        print(protoCaller.get.call(nonStrictFunc));
    } catch (e) {
        print(e.name);
    }

    try {
        print(nonStrictFunc.caller);
    } catch (e) {
        print(e.name);
    }

    print('- Function.prototype.caller set');
    try {
        print(protoCaller.set.call(null, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoCaller.set.call(123, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoCaller.set.call(Math.cos, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        print(protoCaller.set.call(strictFunc, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        // Difference to V8 here, see comments for .arguments above.
        print(protoCaller.set.call(nonStrictFunc, 123));
    } catch (e) {
        print(e.name);
    }

    try {
        nonStrictFunc.caller = 123;
        print(nonStrictFunc.caller);
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
