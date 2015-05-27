/*
 *  A .slice() call creates a new duk_hbufferobject which inherits the class
 *  number and internal prototype of the "this" binding.
 *
 *  The class number inheritance is quite clear, but inheriting the internal
 *  prototype is less so.  Another alternative would be for the result to
 *  always have the original Buffer.prototype as its prototype.
 *
 *  This testcase illustrates the current behavior.
 */

/*===
original buffer
true
true
slice of original buffer
true
true
modified original buffer
true
false
true
my_proto
slice of modified original buffer
true
false
true
my_proto
===*/

function nodejsBufferSlicePrototypeTest() {
    var b = new Buffer('ABCDEFGH');
    var c;

    print('original buffer');
    print(Object.getPrototypeOf(b) === Buffer.prototype);
    print(b instanceof Buffer);

    print('slice of original buffer');
    c = b.slice(2, 6);
    print(Object.getPrototypeOf(c) === Buffer.prototype);
    print(c instanceof Buffer);

    var my_proto = {
        name: 'my_proto'
    };
    Object.setPrototypeOf(my_proto, Buffer.prototype);
    Object.setPrototypeOf(b, my_proto);

    print('modified original buffer');
    print(Object.getPrototypeOf(b) === my_proto);
    print(Object.getPrototypeOf(b) === Buffer.prototype);
    print(b instanceof Buffer);
    print(b.name);

    // Here behavior differs from Node.js Buffer: for Node.js Buffer
    // the internal prototype of 'c' will be Buffer.prototype.

    print('slice of modified original buffer');
    c = b.slice(2, 6);
    print(Object.getPrototypeOf(c) === my_proto);
    print(Object.getPrototypeOf(c) === Buffer.prototype);
    print(c instanceof Buffer);
    print(c.name);
}

try {
    nodejsBufferSlicePrototypeTest();
} catch (e) {
    print(e.stack || e);
}
