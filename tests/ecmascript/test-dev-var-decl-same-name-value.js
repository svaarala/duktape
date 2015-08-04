/*===
undefined undefined
===*/

/* What should 'var x = x' do?
 *
 * Because variable declarations are "hoisted", i.e. magically processed
 * upon function entry, the right-hand-side 'x' should NOT refer to e.g.
 * a global value of 'x', but should be undefined.
 */

var x = 10;
var y = 20;

function f1() {
    var x = x;
    print(typeof x, x);
}

f1();

/*===
undefined undefined
undefined undefined
===*/

/* Order trick should not help, both should be undefined. */

function f2() {
    var y = x;
    var x = y;
    print(typeof x, x);
    print(typeof y, y);
}

f2();
