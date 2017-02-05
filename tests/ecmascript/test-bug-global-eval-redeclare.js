/*
 *  Bug in handling of variable redeclaration from eval/global code.
 *  The redeclaration must happen in a separate compilation unit for
 *  the bug to manifest, e.g. eval code must redeclare a global variable.
 */

/*===
function
fn called
[object Math]
[object Math]
done
===*/

var global = new Function('return this')();

// Redeclare from eval without initializer, should be no-op.

global.fn = function () {
    print('fn called');
};

(0, eval)('var fn;');

print(typeof fn);
try {
    fn();
} catch (e) {
    print(e.stack || e);
}

// Related issue: redeclare Math from global code, should be no-op.

print(Math);
var Math;
print(Math);

print('done');
