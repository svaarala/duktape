/*
 *  Demonstrate current argument limits for normal and constructor calls.
 */

/*---
{
    "custom": true
}
---*/

/*===
function
255 normal called-with-255-arguments
255 constr called-with-255-arguments
256 normal RangeError
256 constr RangeError
511 normal RangeError
511 constr RangeError
512 normal RangeError
512 constr RangeError
===*/

function createCall(count, isConstructor) {
    var res = [];
    res.push(isConstructor ? 'new func(' : 'func(');
    for (var i = 0; i < count; i++) {
        if (i > 0) {
            res.push(',');
        }
        res.push(123);
    }
    res.push(');');
    return res.join('');
}

function test() {
    function func() {
        // Return a String object so that even with a constructor call
        // a useful message gets printed.
        //print('func called with ' + arguments.length + ' arguments');
        return new String('called-with-' + arguments.length + '-arguments');
    }

    print(eval('typeof func'));

    function f(count, isConstructor) {
        var label = isConstructor ? 'constr' : 'normal';
        try {
            print(count, label, eval(createCall(count, isConstructor)));
        } catch (e) {
            //print(e.stack || e);
            print(count, label, e.name);
        }
    }

    // 255 is the limit in Duktape 2.x
    f(255, false);
    f(255, true);

    f(256, false);
    f(256, true);

    // 511 was the limit in Duktape 1.x
    f(511, false);
    f(511, true);

    f(512, false);
    f(512, true);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
