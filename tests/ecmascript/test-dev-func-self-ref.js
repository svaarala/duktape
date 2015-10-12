/*
 *  Function self-reference, compiles to specific bytecode for most self
 *  recursive functions.
 */

/*===
===*/

/* Basic counter test, with a self-referencing recursive call. */

function counterTest() {
    function counter(n) {
        print(n);
        print(counter);  // self ref as a value (LDCURRFN)
        print(typeof counter);  // FIXME: hard code result?
        if (n > 0) { counter(n - 1); }  // self ref in a call (LDCURRFN + CSREG)
    }

    counter(5);
}

try {
    print('counter test');
    counterTest();
} catch (e) {
    print(e.stack || e);
}

/*===
===*/

/* Self-reference captured by a 'with'. */

function capturingWithTest() {
    function counter(n) {
        with ({ counter: function (x) { print('captured:', x); } }) {
            if (n > 0) { counter(n - 1); }
        }
    }

    counter(5);
}

try {
    print('capturing with test');
    capturingWithTest();
} catch (e) {
    print(e.stack || e);
}

/*===
===*/

/* Self reference captured by 'catch'. */

function capturingCatchTest() {
    function counter(n) {
        try {
            throw function (x) { print('captured:', x); };
        } catch (counter) {
            if (n > 0) { counter(n - 1); }
        }
    }

    counter(5);
}

try {
    print('capturing catch test');
    capturingCatchTest();
} catch (e) {
    print(e.stack || e);
}

/*===
===*/

/* Write over a global binding during self-reference. */

function mutationHelper(n) {
    if (n == 3) {
        this.counterAssignTest = function (n) {
            print('captured by mutated counterAssignTest:', n);
        };
    }
}

function counterAssignTest(n) {
    print(n);
    mutationHelper(n);
    if (n > 0) {
        counterAssignTest(n - 1);
    }
}

try {
    // FIXME: check required semantics; V8 seems to allow first round
    // to use original value (= self reference always sees the same
    // original function), Rhino does not.
    print('counter assign test');
    counterAssignTest(5);  // on first round self refs allow counter to go through
    counterAssignTest(5);  // on second round uses mutated value
} catch (e) {
    print(e.stack || e);
}
