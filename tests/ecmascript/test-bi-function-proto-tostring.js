/*
 *  Function.prototype.toString() requirements were quite general in ES5:
 *  output must parse as FunctionDeclaration but doesn't need to compile
 *  into useful code.
 *
 *  ES2015 requires that the source code eval()s to an equivalent object or,
 *  if that's not possible, evals to a SyntaxError.
 *
 *  Duktape 2.x follows the ES2015 requirements.
 */

/*===
SyntaxError
SyntaxError
SyntaxError
SyntaxError
===*/

function test() {
    var scriptFunc = function foo() {};
    var nativeFunc = Math.cos;
    var boundFunc1 = scriptFunc.bind(null, 1);
    var boundFunc2 = nativeFunc.bind(null, 1);

    [ scriptFunc, nativeFunc, boundFunc1, boundFunc2 ].forEach(function (v) {
        try {
            var res = eval(String(v));
            print('never here:', typeof res);
        } catch (e) {
            print(e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
