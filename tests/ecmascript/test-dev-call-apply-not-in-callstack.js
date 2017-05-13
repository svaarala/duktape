/*
 *  Since Duktape 2.2 .call() and .apply() no longer visible in call stack.
 */

/*===
f0 called
-1 act
-2 f0
-3 f1
-4 f2
-5 f3
-6 test
-7 global
===*/

function test() {
    var f0 = function f0() {
        print('f0 called');
        for (var i = -1; ; i--) {
            var act = Duktape.act(i);
            if (!act) { break; }
            print(i, act.function.name);
        }
    };
    var f1 = function f1() {
        Reflect.apply(f0, 'dummy');
    }
    var f2 = function f2() {
        f1.call('dummy');
    }
    var f3 = function f3() {
        f2.apply('dummy');
    }
    f3();
}
try {
    test();
} catch (e) {
    print(e.stack || e);
}
