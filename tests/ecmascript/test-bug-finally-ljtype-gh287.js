/*
 *  Bug in Duktape 1.2 and before where finalization just before
 *  'finally' caused an internal error (GH-287).
 */

/*===
The pig ate all the food, and then...
*munch*
Error: A pig ate everyone at 8:12
done
===*/

function test() {
    function Food() {
        this.getEaten = function() { print("The pig ate all the food, and then..."); };
    }
    Duktape.fin(Food.prototype, function() {});
    new Food().getEaten();

    // The above line is critical to triggering the bug: on entry to TRYCATCH
    // the first catch register must contain a value which (1) has only one
    // reference left, and (2) has a finalizer.  When the error value is written
    // to the catch register when setting up for the finally block, the value
    // becomes unreachable and the finalizer is executed.

    try { throw new Error("A pig ate everyone at 8:12"); }
    finally { print("*munch*"); }
}

try {
    test();
} catch (e) {
    print(e);
}
print('done');
