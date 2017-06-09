/*
 *  ES2017 Annex B: __defineGetter__, __defineSetter__, __lookupGetter__, __lookupSetter__
 */

/*===
setter called: 123
getter called
BAR
true
true
===*/

function testMisc() {
    var obj = {};

    function myGetter() {
        print('getter called');
        return 'BAR';
    }
    function mySetter(v) {
        print('setter called:', v);
    }

    // Set both getter and setter using the Annex B API.
    obj.__defineGetter__('foo', myGetter);
    obj.__defineSetter__('foo', mySetter);
    obj.foo = 123;
    print(obj.foo);

    // Read back and compare functions.
    print(obj.__lookupGetter__('foo') === myGetter);
    print(obj.__lookupSetter__('foo') === mySetter);
}

try {
    testMisc();
} catch (e) {
    print(e.stack || e);
}
