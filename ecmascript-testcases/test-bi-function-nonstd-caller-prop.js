/*
 *  Many engines provide a "caller" property which is automatically
 *  updated on function calls, see e.g.:
 *
 *    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function/caller
 *
 *  This non-standard behavior is available in Duktape with a special
 *  feature option (not by default).  The behavior is modelled after
 *  V8 behavior.  Note: when testing with NodeJS, the program does not
 *  execute in a genuine Ecmascript global/program context, so some
 *  V8/NodeJS behavior is not as expected.  This is an artifact of the
 *  NodeJS wrapper.
 */

/*---
{
    "nonstandard": true
}
---*/

function summarizeCaller(f) {
    if (f.caller === undefined) {
        return 'undefined';
    } else if (f.caller === null) {
        return 'null';
    } else if (typeof f.caller === 'function') {
        return f.caller.myName || f.caller.name || 'anon';
    } else if (typeof f.caller === 'number') {
        return f.caller;  // not needed, used during development
    } else {
        return typeof f.caller;
    }
}

/*===
basic tests
null false false false undefined undefined
null false false false undefined undefined
undefined undefined false false function function
TypeError
function basicTest
function false false false undefined undefined
TypeError
===*/

/* Some basic tests. */

function f1_nonstrict() {
    var caller = arguments.callee.caller;
    print(typeof caller, caller.name);

    // 'caller' gets updated but is still non-writable
    pd = Object.getOwnPropertyDescriptor(arguments.callee, 'caller');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable, typeof pd.get, typeof pd.set);
}

function f1_strict() {
    'use strict';
    var pd;
    var caller = arguments.callee.caller;

    // we never get this far because the '.caller' access above is a
    // TypeError

    print(typeof caller, caller.name);  // basicTest

    // 'caller' gets updated but is still non-writable
    pd = Object.getOwnPropertyDescriptor(arguments.callee, 'caller');
    print(pd.value, pd.writable, pd.enumerable, pd.configurable, typeof pd.get, typeof pd.set);
}

function basicTest() {
    var pd;

    // When called from top level, the 'caller' property is 'null'.
    // This is not trivial because in Duktape the program/eval code
    // is also a function.

    pd = Object.getOwnPropertyDescriptor(arguments.callee, 'caller');
    print(pd.value, pd.writable, pd.enumerable, pd.configurable, typeof pd.get, typeof pd.set);

    // Non-strict function: 'caller' is a normal property with 'null'
    // initial value.  Although the property changes value, it is
    // non-writable and non-configurable.

    pd = Object.getOwnPropertyDescriptor(f1_nonstrict, 'caller');
    print(pd.value, pd.writable, pd.enumerable, pd.configurable, typeof pd.get, typeof pd.set);

    // Strict function: obeys Ecmascript specification, and is an accessor
    // with a thrower.
    pd = Object.getOwnPropertyDescriptor(f1_strict, 'caller');
    print(pd.value, pd.writable, pd.enumerable, pd.configurable, typeof pd.get, typeof pd.set);
    try {
        print(f1_strict.caller);
    } catch (e) {
        print(e.name);
    }

    try {
        f1_nonstrict();
    } catch (e) {
        print(e.name);
    }

    try {
        f1_strict();
    } catch (e) {
        print(e.name);
    }
}

print('basic tests');

try {
    basicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
call types
entry null
in ecmaNoTail callTypeTest
after ecmaNoTail null
in ecmaTail2, ecmaTail1 null
in ecmaTail2, ecmaTail2 callTypeTest
3 in ecmaTail1, ecmaTail1 ecmaTail2
3 in ecmaTail1, ecmaTail2 callTypeTest
2 in ecmaTail1, ecmaTail1 ecmaTail1
2 in ecmaTail1, ecmaTail2 callTypeTest
1 in ecmaTail1, ecmaTail1 ecmaTail1
1 in ecmaTail1, ecmaTail2 callTypeTest
0 in ecmaTail1, ecmaTail1 ecmaTail1
0 in ecmaTail1, ecmaTail2 callTypeTest
after ecmaNoTail, ecmaTail1 null
after ecmaNoTail, ecmaTail2 null
in nativeCall forEach
after nativeCall null
in nonStrictEvalCall null
after nonStrictEvalCall null
in strictEvalCall eval
after strictEvalCall null
in boundNonStrictCall callTypeTest
after boundNonStrictCall null
TypeError
TypeError
===*/

/* Different call type cases.  These have different internal handling for
 * setting 'caller'.
 */

function callTypeTest() {
    // Global-to-Ecmascript call (same as native-to-Ecmascript with slight
    // handling differences because 'caller' will be null).  NOTE: NodeJS
    // prints 'anon' here because the test case executes from a function, not
    // an actual global context.
    print('entry', summarizeCaller(callTypeTest));

    // Ecmascript-to-Ecmascript call, no tail recursion
    function ecmaNoTail() {
        print('in ecmaNoTail', summarizeCaller(ecmaNoTail));
    }
    ecmaNoTail();
    print('after ecmaNoTail', summarizeCaller(ecmaNoTail));

    // Ecmascript-to-Ecmascript call, with tail recursion
    function ecmaTail1(idx) {
        print(idx, 'in ecmaTail1, ecmaTail1', summarizeCaller(ecmaTail1));
        print(idx, 'in ecmaTail1, ecmaTail2', summarizeCaller(ecmaTail2));
        if (idx > 0) {
            return ecmaTail1(idx - 1);  // tail call
        }
    }
    function ecmaTail2() {
        print('in ecmaTail2, ecmaTail1', summarizeCaller(ecmaTail1));
        print('in ecmaTail2, ecmaTail2', summarizeCaller(ecmaTail2));
        return ecmaTail1(3);  // tail call
    }
    ecmaTail2();
    print('after ecmaNoTail, ecmaTail1', summarizeCaller(ecmaTail1));
    print('after ecmaNoTail, ecmaTail2', summarizeCaller(ecmaTail2));

    // Native-to-Ecmascript call
    function nativeCall() {
        print('in nativeCall', summarizeCaller(nativeCall));
    }
    [1].forEach(nativeCall);
    print('after nativeCall', summarizeCaller(nativeCall));

    // Non-strict eval: V8 provides 'callTypeTest' in the target function
    // 'caller' property.  Duktape handles eval() as an intermediate function
    // in the call stack, and provides 'null' instead.
    function nonStrictEvalCall() {
        print('in nonStrictEvalCall', summarizeCaller(nonStrictEvalCall));
    }
    eval('nonStrictEvalCall()');
    print('after nonStrictEvalCall', summarizeCaller(nonStrictEvalCall));

    // Strict eval: V8 provides 'callTypeTest' in the target function
    // 'caller' property.  Duktape handles eval() as an intermediate function
    // in the call stack, but because strict eval functions look like normal
    // functions internally, it provides 'eval' here at the moment.
    function strictEvalCall() {
        print('in strictEvalCall', summarizeCaller(strictEvalCall));
    }
    eval('"use strict"; strictEvalCall()');
    print('after strictEvalCall', summarizeCaller(strictEvalCall));

    // Call to bound function, final function is non-strict.
    function boundNonStrictCall() {
        print('in boundNonStrictCall', summarizeCaller(boundNonStrictCall));
    }
    boundNonStrictCall.bind(null)();
    print('after boundNonStrictCall', summarizeCaller(boundNonStrictCall));

    // call to bound function, final function is strict
    function boundStrictCall() {
        "use strict";
        print('in boundStrictCall', summarizeCaller(boundStrictCall));
    }
    try {
        boundStrictCall.bind(null)();
    } catch (e) {
        print(e.name);
    }
    try {
        print('after boundStrictCall', summarizeCaller(boundStrictCall));
    } catch (e) {
        print(e.name);
    }
}

print('call types');

try {
    callTypeTest();
} catch (e) {
    print(e);
}

/*===
multiple occurrences in callstack
f called multipleOccurrenceTest null null
g called multipleOccurrenceTest f null
h called multipleOccurrenceTest f g
g called multipleOccurrenceTest h g
f called g h g
h called g h f
after null null null
f called multipleOccurrenceTest null null
g called multipleOccurrenceTest f null
h called multipleOccurrenceTest f g
g called multipleOccurrenceTest h g
f called g h g
h called g h f
Error
after null null null
===*/

/* Multiple callstack occurrences: 'caller' always reflects the most
 * recent invocation, and is updated properly as the calls wind down.
 *
 *   Here:  f -> g -> h -> g -> f -> h
 *
 * Avoid tail recursion.  Also tests what happens when exception occurs:
 * callstack unwind should reset 'caller' property for all entries.
 */

function multipleOccurrenceTest(throwAtEnd) {
    var targets = [ f, g, h, g, f, h ];
    var idx = 0;
    function getNext() {
        if (idx >= targets.length) { return undefined; }
        return targets[idx++];
    }

    function f() {
        var fn, ign;
        print('f called', summarizeCaller(f), summarizeCaller(g), summarizeCaller(h));

        fn = getNext();
        if (fn) {
            ign = fn();
            return;
        } else {
            if (throwAtEnd) { throw new Error('finished'); }
            return;
        }
    }
    function g() {
        var fn, ign;
        print('g called', summarizeCaller(f), summarizeCaller(g), summarizeCaller(h));

        fn = getNext();
        if (fn) {
            ign = fn();
            return;
        } else {
            if (throwAtEnd) { throw new Error('finished'); }
            return;
        }
    }
    function h() {
        var fn, ign;
        print('h called', summarizeCaller(f), summarizeCaller(g), summarizeCaller(h));

        fn = getNext();
        if (fn) {
            ign = fn();
            return;
        } else {
            if (throwAtEnd) { throw new Error('finished'); }
            return;
        }
    }

    try {
        getNext()();
    } catch (e) {
        print(e.name);
    }

    print('after', summarizeCaller(f), summarizeCaller(g), summarizeCaller(h));
}

print('multiple occurrences in callstack');

try {
    multipleOccurrenceTest(false);
    multipleOccurrenceTest(true);
} catch (e) {
    print(e);
}

/*===
recursion
rec 5 recursionTest
rec 4 f_plain_rec
rec 3 f_plain_rec
rec 2 f_plain_rec
rec 1 f_plain_rec
rec 0 f_plain_rec
15
null
===*/

/* Recursion (no tail recursion).  Tail recursion is already covered by
 * 'call types' test so no detailed test for it now.
 */

function f_plain_rec(idx) {
    var c = arguments.callee.caller;
    var res;

    print('rec', idx, typeof c === 'function' ? c.name : c);
    if (idx > 0) {
        res = idx + f_plain_rec(idx - 1);
    } else {
        res = 0;
    }
    return res;
}

print('recursion');

function recursionTest() {
    print(f_plain_rec(5));
    print(f_plain_rec.caller);
}

try {
    recursionTest();
} catch (e) {
    print(e);
}

/*===
coroutine overwrite test
co1 entry1 (before f) null null
co2 entry2 (before f) null null
co1 f (before g) entry1 null
co1 g entry1 f
co2 f (before g) entry2 f
co1 f (after g) entry2 null
co1 entry1 (after f) null null
co2 g null f
co2 f (after g) null null
co2 entry2 (after f) entry1 null
===*/

/* When a function is entered/exited, the 'caller' property is overwritten
 * by whichever thread has last executed.  This is confusing when the same
 * function instances are updated by multiple coroutines.
 *
 * Test for the expected (but confusing) behavior here.  Note that the final
 * value for f.caller here is 'entry1' which is very undesirable, but not
 * easy to fix with the save/restore approach to 'caller' handling.
 */

function coroutineOverwriteTest() {
    function shared() {
        print('shared called');
    }

    function g() {
        print(Duktape.Thread.current().name, 'g', summarizeCaller(f), summarizeCaller(g));
        Duktape.Thread.yield();  // C
    }

    function f() {
        print(Duktape.Thread.current().name, 'f (before g)', summarizeCaller(f), summarizeCaller(g));
        Duktape.Thread.yield();  // B
        g();
        print(Duktape.Thread.current().name, 'f (after g)', summarizeCaller(f), summarizeCaller(g));
        Duktape.Thread.yield();  // D
    }

    function entry1() {
        print(Duktape.Thread.current().name, 'entry1 (before f)', summarizeCaller(f), summarizeCaller(g));
        Duktape.Thread.yield();  // A1
        f();
        print(Duktape.Thread.current().name, 'entry1 (after f)', summarizeCaller(f), summarizeCaller(g));
        Duktape.Thread.yield();  // E1
    }

    function entry2() {
        print(Duktape.Thread.current().name, 'entry2 (before f)', summarizeCaller(f), summarizeCaller(g));
        Duktape.Thread.yield();  // A2
        f();
        print(Duktape.Thread.current().name, 'entry2 (after f)', summarizeCaller(f), summarizeCaller(g));
        Duktape.Thread.yield();  // E2
    }

    var co1 = new Duktape.Thread(entry1);
    co1.name = 'co1';
    var co2 = new Duktape.Thread(entry2);
    co2.name = 'co2';

    /*
    Duktape.Thread.resume(co1);  // until A1
    Duktape.Thread.resume(co1);  // until B
    Duktape.Thread.resume(co1);  // until C
    Duktape.Thread.resume(co1);  // until D
    Duktape.Thread.resume(co1);  // until E1
    Duktape.Thread.resume(co1);  // finish

    Duktape.Thread.resume(co2);  // until A2
    Duktape.Thread.resume(co2);  // until B
    Duktape.Thread.resume(co2);  // until C
    Duktape.Thread.resume(co2);  // until D
    Duktape.Thread.resume(co2);  // until E2
    Duktape.Thread.resume(co2);  // finish
    */

    // This is quite intricate; the sequence is not that important but it
    // should exercise entry/exit from functions while switching between
    // the two coroutines.  The 'caller' configurations are quite weird.

    Duktape.Thread.resume(co1);  // until A1
    Duktape.Thread.resume(co2);  // until A2
    Duktape.Thread.resume(co1);  // until B
    Duktape.Thread.resume(co1);  // until C
    Duktape.Thread.resume(co2);  // until B
    Duktape.Thread.resume(co1);  // until D
    Duktape.Thread.resume(co1);  // until E1
    Duktape.Thread.resume(co2);  // until C
    Duktape.Thread.resume(co2);  // until D
    Duktape.Thread.resume(co2);  // until E2
    Duktape.Thread.resume(co1);  // finish
    Duktape.Thread.resume(co2);  // finish
}

print('coroutine overwrite test');

try {
    coroutineOverwriteTest();
} catch (e) {
    print(e);
}

/*===
coroutine gc
f started null null
g called null f
canary finalized
after null f
===*/

/* Coroutine garbage collection test.  A garbage collected coroutine gets
 * freed.  Conceptually its call stack is NOT unwound but rather just freed.
 * Even so, 'caller' should get reset to its original value.
 *
 * At the moment Duktape doesn't handle this case correctly and 'caller'
 * values are not reset.  In general, coroutines and 'caller' don't mix
 * well (see test above for a normal case where 'caller' is non-null after
 * coroutine stacks have been unwound).
 */

function coroutineGcTest() {
    var canary = {};
    Duktape.fin(canary, function() { print('canary finalized'); });

    function g() {
        print('g called', summarizeCaller(f), summarizeCaller(g));
        Duktape.Thread.yield();
        // coroutine stops here and is GC'd; g.caller is f.
    }

    function f() {
        var ref = canary;
        print('f started', summarizeCaller(f), summarizeCaller(g));
        g();
    }

    var co = new Duktape.Thread(f);
    Duktape.Thread.resume(co);

    co = null;
    canary = null;
    Duktape.gc();  // coroutine and canary freed at latest here

    print('after', summarizeCaller(f), summarizeCaller(g));
}

print('coroutine gc');

try {
    coroutineGcTest();
} catch (e) {
    print(e);
}

/*===
unwind gc test
1
f caller: unwindGcTestInner
g caller: f
h caller: g
0
f caller: h
g caller: f
h caller: g
f_fin true
g_fin true
h_fin true
force gc
f_fin true
g_fin true
h_fin true
1
f caller: unwindGcTestInner
g caller: f
h caller: g
0
f caller: h
g caller: f
h caller: g
Error
f_fin true
g_fin true
h_fin true
force gc
f_fin true
g_fin true
h_fin true
===*/

/* Test refcount updates for unwind.  Create a call chain, delete all function
 * references from the innermost function.  The call chain must have a function
 * which occurs more than once to exercise the refcount handling better.  Then
 * return or throw an error which causes the call chain to unwind and check the
 * finalizers are executed.
 *
 * The inner-outer function split and inner functions are somewhat paranoid,
 * the idea is to minimize risk of having references in active function temp
 * registers which are reachability roots but not immediately apparent.
 */

var f_fin = false, g_fin = false, h_fin = false;

function unwindGcTestInner(throwAtEnd) {
    var obj = {};

    obj.h = function (num) {
        print(num);
        print('f caller:', summarizeCaller(obj.f));
        print('g caller:', summarizeCaller(obj.g));
        print('h caller:', summarizeCaller(obj.h));
        if (num > 0) {
            obj.f(num - 1);
            return;
        }
        obj.f = null;
        obj.g = null;
        obj.h = null;
        if (throwAtEnd) {
            throw Error('finished');
        } else {
            return;
        }
    };
    obj.g = function (num) {
        obj.h(num);
    };
    obj.f = function (num) {
        obj.g(num);
    };

    // Named function expressions could be used but they cause an internal
    // reference loop which cannot be broken from use code (see
    // test-dev-named-funcexpr-refcount.js for details).
    obj.f.myName = 'f';
    obj.g.myName = 'g';
    obj.h.myName = 'h';

    function breakCycles() {
        // The circular references that the functions participate in are
        // broken so that we can check that refcount will collect them
        // (not just mark-and-sweep).
        obj.f.prototype.constructor = null;
        obj.g.prototype.constructor = null;
        obj.h.prototype.constructor = null;
    }
    breakCycles();

    function setFinalizers() {
        Duktape.fin(obj.f, function() { f_fin = true; });
        Duktape.fin(obj.g, function() { g_fin = true; });
        Duktape.fin(obj.h, function() { h_fin = true; });
    }
    setFinalizers();

    try {
        Duktape.gc();  // minimize gc variance
        obj.f(1);
    } catch (e) {
        print(e.name);
    }
}

function unwindGcTest(throwAtEnd) {
    f_fin = false;
    g_fin = false;
    h_fin = false;

    unwindGcTestInner(throwAtEnd);

    print('f_fin', f_fin);
    print('g_fin', g_fin);
    print('h_fin', h_fin);

    print('force gc');
    Duktape.gc();  // ensure consistency even when refcount disabled

    print('f_fin', f_fin);
    print('g_fin', g_fin);
    print('h_fin', h_fin);
}

print('unwind gc test');

try {
    unwindGcTest(false);
    unwindGcTest(true);
} catch (e) {
    print(e);
}
