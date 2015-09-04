/*
 *  PrimaryExpression -> Identifier
 */

/*===
foo
foo
234
function inner() {|* ecmascript *|}
123
234
function inner() {|* ecmascript *|}
123
===*/

/* Identifier access is covered quite extensively by other
 * tests.  Just test every identifier type quickly.
 */

var global1 = 'foo';
print(global1);  // slow path GETVAR from global code

eval('print(global1)');  // slow path GETVAR from eval code

function test(arg) {
    var x = 234;

    function inner() {
    }

    // fast path local variable, local function, local argument
    print(x);
    print(String(inner).replace(/\//g, '|'));
    print(arg);

    // same through slow path eval
    eval('print(x);');
    eval('print(String(inner).replace(/\\//g, "|"));');
    eval('print(arg);');
}

try {
    test(123);
} catch (e) {
    print(e.name);
}
