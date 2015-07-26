/*
 *  Some tests for unary logical not inline optimizations.
 */

/*===
comparison
-Infinity false
-1 false
-0.5 false
0 true
0 true
0.5 false
1 false
Infinity false
NaN true
true false
false true
inlined lnot
false
false
false
true
true
false
false
false
true
false
true
inlined double lnot
true
true
true
false
false
true
true
true
false
true
false
===*/

function test() {
    // This will not be inlined, of course; for comparison
    print('comparison');
    [ -1/0, -1, -0.5, -0, +0, 0.5, 1, 1/0, 0/0, true, false ].forEach(function (x) {
        print(x, !x);
    });

    print('inlined lnot');
    print(!(-1/0));
    print(!-1);
    print(!-0.5);
    print(!-0);
    print(!+0);
    print(!0.5);
    print(!1);
    print(!(1/0));
    print(!(0/0));
    print(!true);
    print(!false);

    print('inlined double lnot');
    print(!!(-1/0));
    print(!!-1);
    print(!!-0.5);
    print(!!-0);
    print(!!+0);
    print(!!0.5);
    print(!!1);
    print(!!(1/0));
    print(!!(0/0));
    print(!!true);
    print(!!false);
}

function getBytecodeSize(f) {
    var info = Duktape.info(f);
    //print(info[9]);
    return info[9];  // function data size: bytecode, consts, inner funcs
}

try {
    test();
} catch (e) {
    print(e);
}

/*===
here
===*/

try {
    // Because of inlining, these should compile to the same bytesize
    var f0 = function f0() { print(-0); };  // -0 will not use LDINT and will differ from others
    var f1 = function f1() { print(0); };   // +0 will use LDINT
    var f2 = function f2() { print(!0); };
    var f3 = function f3() { print(!!0); };
    var f4 = function f4() { print(1); };
    var f5 = function f5() { print(!1); };
    var f6 = function f6() { print(!!1); };
    var f7 = function f7() { print(true); };
    var f8 = function f8() { print(!true); };
    var f9 = function f9() { print(!!true); };
    var f10 = function f10() { print(false); };
    var f11 = function f11() { print(!false); };
    var f12 = function f12() { print(!!false); };

    var f1size = getBytecodeSize(f1);
    [ f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12 ].forEach(function (f) {
        //print(f, getBytecodeSize(f));
        if (getBytecodeSize(f) !== f1size) {
            print('bytecode size differs: ' + f);
        }
    });

    print('here');
} catch (e) {
    print(e);
}
