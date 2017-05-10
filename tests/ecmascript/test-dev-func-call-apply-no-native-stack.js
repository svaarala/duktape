/*
 *  Demonstrate that in Duktape 2.2 .call() and .apply() no longer
 *  consume native call stack when the resolved call is an Ecma-to-Ecma
 *  call.
 *
 *  Can't test against a specific limit but the native stack limit is
 *  ~500 calls while the default call stack limit is ~10000 calls.
 */

/*===
RangeError
true
RangeError
true
RangeError
true
RangeError
true
done
===*/

function test() {
    var count;
    function f1() {
        count++;
        f1();  // not a tail call
        void count;
    }
    function f2() {
        count++;
        f2.call();  // not a tail call
        void count;
    }
    function f3() {
        count++;
        f3.apply();  // not a tail call
        void count;
    }
    function f4() {
        count++;
        Reflect.apply(f4);  // not a tail call
        void count;
    }

    try {
        count = 0;
        f1();
    } catch (e) {
        print(e.name);
    }
    print(count > 2000);

    try {
        count = 0;
        f2();
    } catch (e) {
        print(e.name);
    }
    print(count > 2000);

    try {
        count = 0;
        f3();
    } catch (e) {
        print(e.name);
    }
    print(count > 2000);

    try {
        count = 0;
        f4();
    } catch (e) {
        print(e.name);
    }
    print(count > 2000);

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
