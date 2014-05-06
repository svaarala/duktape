/*
 *  Tailcall bug when tailcalling Duktape.Thread.resume().  Reported by
 *  Andreas Öman.
 *
 *  What probably happens is:
 *
 *    - The compiler compiles 'return Duktape.Thread.resume(t)', and detects
 *      a tail call opportunity when compiling the return statement.
 *
 *    - The compiler adds a 'tailcall' flag to a previously emitted DUK_OP_JUMP
 *      instruction.  Because a tailcall was emitted, the compiler does not
 *      emit a return statement.
 *
 *    - When executing, the executor notices that the target function is native,
 *      so the requested tailcall is not possible, so a normal recursive call
 *      is made.
 *
 *    - Normally, once the recursive call returns, the executor simulates a
 *      return statement (as if a DUK_OP_RETURN was present).  This makes the
 *      behavior correct for normal functions.
 *
 *    - However, Duktape.Thread.yield() or Duktape.Thread.resume() are
 *      different: they manipulate the execution state (changing the current
 *      thread) and longjmp to resume execution.
 *
 *    - Because of the longjmp, the simulated DUK_OP_RETURN is not executed
 *      when the resumed thread returns.  Instead, execution continues after
 *      the opcode following the tailcall.  In the test below, this causes
 *      Duktape 0.10.0 to print "never here".
 */

/*===
retval
retval
123
234
TypeError
123
234
TypeError
===*/

function test1() {
    var t = new Duktape.Thread(function () {
        return "retval";
    });

    // base case: no tail call
    var res = Duktape.Thread.resume(t);
    return res;

    print('never here');
}

function test2() {
    var t = new Duktape.Thread(function () {
        return "retval";
    });

    // tail call, should have same result but fails
    return Duktape.Thread.resume(t);

    // the bug causes the 'return' to be skipped, so this statement
    // incorrectly executes too
    print('never here');
}

function test3() {
    var t = new Duktape.Thread(function () {
        // Base case: no tailcall
        var tmp = Duktape.Thread.yield(123);
        return tmp;
        print('never here');
        return 999;
    });

    // Thread should yield 123.
    var res = Duktape.Thread.resume(t);
    print(res);

    // Thread should be resumed with 234 which becomes the return value of
    // the Duktape.Thread.yield() and should be returned back here.
    res = Duktape.Thread.resume(t, 234);
    print(res);

    // Thread is finished, so attempt to resume is an error
    try {
        res = Duktape.Thread.resume(t, 345);
        print(res);
    } catch (e) {
        print(e.name);
    }
}

function test4() {
    var t = new Duktape.Thread(function () {
        // Tailcall breaks here again.
        return Duktape.Thread.yield(123);
        print('never here');
        return 999;
    });

    var res = Duktape.Thread.resume(t);
    print(res);

    // Because of the tailcall bug, this breaks: the 'return' of the yield
    // is skipped.  So print('never here') gets executed and 999 is returned.
    res = Duktape.Thread.resume(t, 234);
    print(res);

    // TypeError because thread is finished.
    try {
        res = Duktape.Thread.resume(t, 345);
        print(res);
    } catch (e) {
        print(e.name);
    }
}

try {
    print(test1());
} catch (e) {
    print(e);
}

try {
    print(test2());
} catch (e) {
    print(e);
}

try {
    test3();
} catch (e) {
    print(e);
}

try {
    test4();
} catch (e) {
    print(e);
}
