/*
 *  When calling a constructor via a Proxy object which doesn't have a
 *  'construct' trap, the target function's new.target should evaluate
 *  to the Proxy (original target of constructor call) and not the Proxy's
 *  target object.
 *
 *  In Duktape 2.2 new.target will evaluate to the target object instead.
 */

/*===
function
true
false
function
false
true
===*/

var myProxy;

function MyConstructor() {
    print(typeof new.target);
    print(new.target === MyConstructor);
    print(new.target === myProxy);
}

function test() {
    // No 'construct' trap, just call through.
    myProxy = new Proxy(MyConstructor, {});

    // Direct constructor call, new.target should be MyConstructor.
    print('- direct call');
    new MyConstructor();

    // Call via Proxy without capturing trap.
    new myProxy();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
