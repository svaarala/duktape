/*
 *  Native functions can act as setters and getters.
 */

/*===
val is a number
===*/

function test() {
    var obj = {};
    Object.defineProperty(obj, 'prop', {
        get: Math.random,
        set: function() { throw Error('setter'); }
    });

    var val = obj.prop;
    if (typeof val === 'number') {
        print('val is a number');
    } else {
        print('val is not a number');
    }
}

try {
    test();
} catch (e) {
    print(e);
}
