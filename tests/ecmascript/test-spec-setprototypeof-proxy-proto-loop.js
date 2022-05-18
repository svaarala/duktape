/*
 *  [[SetPrototypeOf]] has a prototype loop check which terminates with
 *  success if a Proxy is found.  This may allow a prototype loop to be
 *  created because the target is not checked.  Such an indirect loop
 *  can affect a lot of operations using [[GetPrototypeOf]] iteration.
 *
 *  Specification behavior is to loop forever, but in practice a sanity
 *  RangeError should be thrown.  For example:
 *
 *  V8: RangeError: Maximum call stack size exceeded
 *  Duktape: RangeError: prototype chain limit
 */

/*===
prototype loop set up
RangeError
done
===*/

function test() {
    var target = {};

    var P = new Proxy(target, {});
    var O = Object.create(P);

    // This is allowed because the prototype loop check hits the Proxy
    // which terminates a loop check.  However, the Proxy handler table
    // has no traps, so in effect a prototype loop is created for many
    // operations:
    //
    //    O --[proto]--> P --[proto]--> target --[proto]--.
    //    ^                                               |
    //    |                                               |
    //    `-----------------------------------------------'
    Object.setPrototypeOf(target, O);

    print('prototype loop set up');
    try {
        print(O.foo);  // goodbye
    } catch (e) {
        print(e.name);
        //print(e.stack);
    }
    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
