/*
 *  Proxy values in instanceof operator
 */

/*===
true
true
true
===*/

/* 'instanceof' operator with a Proxy target that doesn't implement the
 * 'getPrototypeOf' trap.
 */
function instanceofWithoutTrapTest() {
    // Normal object
    var a = new Error('foo');

    // Proxy of an error
    var b = new Proxy(a, {});

    // Proxy object, target object's prototype is also a Proxy
    function MyError(msg) {
        this.message = msg;
    }
    MyError.prototype = new Proxy(Error.prototype, {});
    var c = new Proxy(new MyError('bar'), {});

    // XXX: test for:
    //     var d = new Proxy(b), {})
    // once Proxy of a Proxy is supported

    print(a instanceof Error);
    print(b instanceof Error);
    print(c instanceof Error);
}

try {
    instanceofWithoutTrapTest();
} catch (e) {
    print(e.stack || e);
}

// XXX: add test for instanceof with a getPrototypeOf trap

// XXX: Proxy of a Proxy
