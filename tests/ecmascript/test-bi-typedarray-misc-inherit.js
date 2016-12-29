/*
 *  If a typed array method like .subarray() is invoked on an object which
 *  is not itself a typed array but inherits from a typed array instance,
 *  ES2015 requires that the method calls fail:
 *
 *  - http://www.ecma-international.org/ecma-262/6.0/#sec-%typedarray%.prototype.subarray
 *
 *  - Target must have a [[TypedArrayName]] internal slot or it is rejected.
 *    Internal slots are not inherited.
 */

/*===
TypeError
===*/

function test() {
    var x = new Uint8Array(10);
    var y = {};
    Object.setPrototypeOf(y, x);

    try {
        print(y.subarray(1));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
