/*
 *  Node.js Buffer valueOf()
 *
 *  Inherited from Object.prototype.valueOf() and returns Buffer as is.
 */

/*@include util-buffer.js@*/

/*===
function
true
false
ABCDEFGH
object
true
===*/

function nodejsBufferValueOfTest() {
    var b;

    // Check inheritance
    print(typeof Buffer.prototype.valueOf);
    print(Buffer.prototype.valueOf === Object.prototype.valueOf);
    print(Buffer.prototype.hasOwnProperty('valueOf'));

    // Object.prototype.valueOf() returns the buffer as is
    b = new Buffer('ABCDEFGH');
    print('' + b.valueOf());
    print(typeof b.valueOf());
    print(b.valueOf() === b);
}

nodejsBufferValueOfTest();
