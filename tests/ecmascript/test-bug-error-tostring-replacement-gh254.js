/*
 *  In Duktape 1.2.2 and before, replacing Error.prototype.toString() with
 *  your own function works, but if you access .fileName or .lineNumber from
 *  the replacement there's an infinite loop.
 *
 *  The bug is caused by A shared helper used for handling .stack, .fileName,
 *  and .lineNumber; the ToString() coercion is needed for .stack but was also
 *  accidentally done for .fileName and .lineNumber.  See GH-254.
 */

/*---
{
    "custom": true
}
---*/

/*===
Error.prototype.toString() called: object
Linenumber type is: number
Caught: Error: aiee
===*/

function testErrorToStringReplacement() {
    var origToString = Error.prototype.toString;

    Error.prototype.toString = function () {
        var line;
        print('Error.prototype.toString() called:', typeof this);

        line = (this || {}).lineNumber;
        print('Linenumber type is:', typeof line);

        // A useful replacement could be something like this
        //return this.name + ': ' + this.message + ' (at line ' + this.lineNumber + ')';

        return this.name + ': ' + this.message;
    };

    try {
        throw new Error('aiee');
    } catch (e) {
        print('Caught: ' + e);
    }
}

try {
    testErrorToStringReplacement();
} catch (e) {
    print(e.stack || e);
}
