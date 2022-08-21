/*
 *  In X <op>= Y the LHS value should be evaluated first, before evaluating
 *  the RHS.  Duktape 1.5.x and before evaluates RHS first, which matters in
 *  some chained <op>= statements.
 *
 *  See: https://github.com/svaarala/duktape/pull/987#issuecomment-251432738.
 */

/*===
Write x: 10
TEST
Read x: 10
Read x: 10
Write x: 40
Write x: 50
Read x: 50
Final x: 50
===*/

var obj = {};
var my_x;
Object.defineProperty(obj, 'x', {
    set: function (v) { print('Write x:', v); my_x = v; return true; },
    get: function () { print('Read x:', my_x); return my_x; }
});

with (obj) {
    x = 10;
    print('TEST');
    x += (x *= 4);
}
print('Final x:', obj.x);
