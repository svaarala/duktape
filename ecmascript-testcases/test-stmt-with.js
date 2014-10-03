/*
 *  With statement (E5 Section 12.10).
 */

var obj;

/*===
undefined
10
===*/

/* Since 'with' does not alter the variable environment (which is
 * used for variable declarations), any declarations should go to
 * the function level instead.
 */

obj = { x: 100 };

function f_decl1() {
    with (obj) {
        eval("var foo = 10;");  /* created in function */
    }
    print(obj.foo);  // -> undefined
    print(foo);      // 10
}

try {
    f_decl1();
} catch (e) {
    print(e.name);
}

/*===
100
undefined
===*/

/* Since 'with' delete operations walk through the lexical environment
 * (same as variable lookup) and not the variable environment (for
 * declarations), deleting properties of the bound object should be
 * possible.
 */

obj = { x: 100 };

function f_del1() {
    var x = 200;  /* should not affect anything */
    print(obj.x);
    with (obj) {
        delete x;
    }
    print(obj.x);
}

try {
    f_del1();
} catch (e) {
    print(e.name);
}
