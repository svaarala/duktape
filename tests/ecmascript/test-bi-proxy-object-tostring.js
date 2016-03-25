/*
 *  Object.prototype.toString() for a Proxy object should reflect the target,
 *  not the Proxy itself.
 */

/*===
[object Array]
===*/

function test() {
    var proxy = new Proxy([], {});

    // This should print [object Array] (verified against Firefox).
    print(Object.prototype.toString.call(proxy));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
