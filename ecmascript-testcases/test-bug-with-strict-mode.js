/*
 *  'with' statement is a SyntaxError in strict mode.
 */

/*===
non-strict with bar
SyntaxError
===*/

function test() {
    try {
        eval("(function() { var obj={foo:'bar'}; with(obj) { print('non-strict with', obj.foo); } })();");
    } catch (e) {
        print(e.name);
    }
    try {
        eval("(function() { 'use strict'; var obj={foo:'bar'}; with(obj) { print('strict with', obj.foo); } })();");
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e);
}
