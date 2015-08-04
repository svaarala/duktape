/*
 *  ToBoolean() (E5 Section 9.2).
 */

function test(values) {
    for (var i = 0; i < values.length; i++) {
        /* Two logical NOTs is basically ToBoolean() */
        print(!!values[i]);
    }
}

/*===
false
false
true
false
===*/

test([ undefined, null, true, false ]);

/*===
false
false
false
true
true
true
true
===*/

test([ +0, -0, Number.NaN ]);
test([ 123.0, -123.0, Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY ]);

/*===
false
true
true
===*/

test([ "", "nonempty", "0" ]);

/*===
true
true
true
true
true
===*/

test([ {}, [], function() {}, Number.prototype.toString, Number.prototype.toString.bind('foo') ])
