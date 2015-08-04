/*
 *  Attempt to delete buffer indices within the valid buffer length range
 *  results in a TypeError in a strict function and a delete expression
 *  evaluating to false in a non-strict function.
 *
 *  Duktape 0.11.0 had this behavior for buffer objects but not for plain
 *  buffer values (deletions had no effect but were not rejected).  Duktape
 *  0.12.0 extends the rejection behavior for plain buffer values too.
 */

/*---
{
    "custom": true
}
---*/

/*===
buffer
-1 ok
0 TypeError
1 TypeError
2 TypeError
3 TypeError
4 ok
buffer
-1 ok
0 TypeError
1 TypeError
2 TypeError
3 TypeError
4 ok
===*/

/* Use a strict function to get a TypeError to be thrown.  A non-strict
 * function would simply evaluate an offending delete expression to false.
 */

function test() {
    'use strict';

    var buf_obj, buf_plain;

    buf_plain = Duktape.dec('hex', 'deadbeef');
    print(typeof buf_plain);
    [ -1, 0, 1, 2, 3, 4 ].forEach(function (i) {
        try {
            delete buf_plain[i];
            print(i, 'ok');
        } catch (e) {
            print(i, e.name)
        }
    });

    buf_obj = new Duktape.Buffer(Duktape.dec('hex', 'deadbeef'));
    print(typeof buf_plain);
    [ -1, 0, 1, 2, 3, 4 ].forEach(function (i) {
        try {
            delete buf_obj[i];
            print(i, 'ok');
        } catch (e) {
            print(i, e.name)
        }
    });
}

try {
    test();
} catch (e) {
    print(e);
}
