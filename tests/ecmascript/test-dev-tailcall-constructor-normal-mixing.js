/*
 *  Tailcall requires that current and target function share the same return
 *  value handling; constructor calls have a special return value handling
 *  compared to normal function calls.
 *
 *  In more detail there are three different return value handling mechanisms
 *  in ES2015:
 *
 *    - Normal call, no special handling.
 *    - Constructor call, default instance vs. replacement instance handling
 *    - Proxy 'construct' trap call, return value invariant check
 *
 *  A tailcall is only allowed (by Duktape) when the return value handling of
 *  the calling code and target code is identical.
 */

/*---
{
    "custom": true
}
---*/

function getDepth() {
    var res = 0;
    //console.trace();
    for (var i = -1; Duktape.act(i); i--) {
        res++;
    }
    // 'res' is number of callstack entries
    return res - 2;  // -1 for Duktape.act, -1 for getDepth()
}

/*===
non-proxy test
normal -> normal
- tailcall
normal -> normal, using call
- tailcall
normal -> normal, using apply
- tailcall
normal -> normal, using Reflect.apply
- tailcall
constructor -> constructor, using new
- tailcall
constructor -> constructor, using Reflect.construct
- tailcall
normal -> constructor call, using new
- not a tailcall
normal -> constructor call, using Reflect.construct
- not a tailcall
constructor -> normal call
- not a tailcall
constructor -> normal call, using call
- not a tailcall
constructor -> normal call, using apply
- not a tailcall
constructor -> normal call, using Reflect.apply
- not a tailcall
===*/

function testNonProxy() {
    var baseline = getDepth() + 2;  // baseline: normal, non-tailcall call stack depth

    function check() {
        var d = getDepth() - 1;  // account for check() call
        if (d === baseline) {
            print('- not a tailcall');
        } else if (d > baseline) {
            print('- deeper than baseline');
        } else {
            print('- tailcall');
        }
    }

    // Normal -> normal tail call.
    print('normal -> normal');
    function nn_a() { check(); }
    function nn_b() { return nn_a(); }
    nn_b();
    print('normal -> normal, using call');
    function nncall_a() { check(); }
    function nncall_b() { return nncall_a.call(); }
    nncall_b();
    print('normal -> normal, using apply');
    function nnapply_a() { check(); }
    function nnapply_b() { return nnapply_a.apply(); }
    nnapply_b();
    print('normal -> normal, using Reflect.apply');
    function nnrapply_a() { check(); }
    function nnrapply_b() { return Reflect.apply(nnrapply_a, null, []); }
    nnrapply_b();

    // Constructor -> constructor tail call.  This works with both
    // Reflect.construct() and 'new Xxx()'.
    print('constructor -> constructor, using new');
    function cc_a() { check(); }
    function cc_b() { return new cc_a(); }
    new cc_b();
    print('constructor -> constructor, using Reflect.construct');
    function cccons_a() { check(); }
    function cccons_b() { return Reflect.construct(cccons_a, []); }
    new cccons_b();

    // Normal -> constructor call, not a tailcall.
    print('normal -> constructor call, using new');
    function nc_a() { check(); }
    function nc_b() { return new nc_a(); }
    nc_b();
    print('normal -> constructor call, using Reflect.construct');
    function nccons_a() { check(); }
    function nccons_b() { return Reflect.construct(nccons_a, []); }
    nccons_b();

    // Constructor -> normal call, not a tailcall.
    print('constructor -> normal call');
    function cn_a() { check(); }
    function cn_b() { return cn_a(); }
    new cn_b();
    print('constructor -> normal call, using call');
    function cncall_a() { check(); }
    function cncall_b() { return cncall_a.call(); }
    new cncall_b();
    print('constructor -> normal call, using apply');
    function cnapply_a() { check(); }
    function cnapply_b() { return cnapply_a.apply(); }
    new cnapply_b();
    print('constructor -> normal call, using Reflect.apply');
    function cnrapply_a() { check(); }
    function cnrapply_b() { return Reflect.apply(cnrapply_a, null, []); }
    new cnrapply_b();
}

try {
    print('non-proxy test');
    testNonProxy();
} catch (e) {
    print(e.stack || e);
}

/*===
proxy construct test 1
proxy2 construct called
proxy1 construct called
relative depth: 1
bar-1-replaced
proxy3 construct called
proxy1 construct called
relative depth: 2
bar-1-replaced
proxy construct test 2
proxy1 construct called
relative depth: 2
bar-1-replaced
proxy construct test 3
proxy1 construct called
MyConstructor1 called
relative depth: 2
bar
===*/

// Constructor and Proxy 'construct' trap calls have different return value
// handling which prevents mixing them in tailcalls.

function testProxyConstruct1() {
    var baseline = getDepth();  // baseline: function entry level
    var proxy1, proxy2, proxy3;

    // These are not actually called.
    function MyConstructor1() {
        print('MyConstructor1 called');
        return { foo: 'bar-1' };
    }
    function MyConstructor2() {
        print('MyConstructor2 called');
        return { foo: 'bar-2' };
    }

    proxy1 = new Proxy(MyConstructor1, {
        construct: function construct1(target, argArray, newTarget) {
            print('proxy1 construct called');
            print('relative depth:', getDepth() - baseline);
            return { foo: 'bar-1-replaced' };
        }
    });
    proxy2 = new Proxy(MyConstructor2, {
        construct: function construct2(target, argArray, newTarget) {
            print('proxy2 construct called');

            // This is a tailcall: calling code (this function) has Proxy
            // 'construct' return value handling, and so does the target.
            return new proxy1();  // MyConstructor2 not called
        }
    });
    proxy3 = new Proxy(MyConstructor2, {
        construct: function construct3(target, argArray, newTarget) {
            print('proxy3 construct called');

            // For comparison only, avoid tailcall to see depth difference.
            var ret = new proxy1();  // MyConstructor2 not called
            void 1+1;
            return ret;
        }
    });

    // Tailcall: Proxy trap to Proxy trap.
    print(new proxy2().foo);

    // Not a tailcall, for comparison.
    print(new proxy3().foo);
}

function testProxyConstruct2() {
    var baseline = getDepth();  // baseline: function entry level
    var proxy1;

    // Not actually called.
    function MyConstructor1() {
        print(getDepth() - baseline);
        print('MyConstructor1 called');
        return { foo: 'bar' };
    }

    proxy1 = new Proxy(MyConstructor1, {
        construct: function construct1(target, argArray, newTarget) {
            print('proxy1 construct called');
            print('relative depth:', getDepth() - baseline);
            return { foo: 'bar-1-replaced' };
        }
    });

    function MyConstructor2() {
        // This is NOT a tailcall: calling code (this function) has normal
        // constructor return value handling, target has Proxy 'construct'
        // return value handling.
        return new proxy1();
    }

    // Not a tailcall.
    print(new MyConstructor2().foo);
}

function testProxyConstruct3() {
    var baseline = getDepth();  // baseline: function entry level
    var proxy1;

    function MyConstructor1() {
        print('MyConstructor1 called');
        print('relative depth:', getDepth() - baseline);
        return { foo: 'bar' };
    }

    proxy1 = new Proxy(MyConstructor1, {
        construct: function construct1(target, argArray, newTarget) {
            print('proxy1 construct called');

            // Not a tailcall: calling context is a Proxy trap, target
            // is a normal constructor call.
            return new MyConstructor1();
        }
    });

    // Not a tailcall
    print(new proxy1().foo);
}

try {
    print('proxy construct test 1');
    testProxyConstruct1();
} catch (e) {
    print(e.stack || e);
}

try {
    print('proxy construct test 2');
    testProxyConstruct2();
} catch (e) {
    print(e.stack || e);
}

try {
    print('proxy construct test 3');
    testProxyConstruct3();
} catch (e) {
    print(e.stack || e);
}

/*===
proxy apply test 1
- normal -> normal
caller1 called
target called
relative depth: 1
123
- proxy apply -> normal
apply trap 1 called
target called
relative depth: 1
123
- normal -> proxy apply
caller2 called
apply trap 2 called
relative depth: 1
234
apply trap 4 called
apply trap 3 called
relative depth: 1
234
===*/

// Function call and Proxy 'apply' trap have same return value behavior
// so they can be mixed.

function proxyApplyTest1() {
    var baseline = getDepth();  // baseline: function entry level
    var proxy1;
    var proxy2;

    function target() {
        print('target called');
        print('relative depth:', getDepth() - baseline);
        return 123;
    }

    function caller1() {
        print('caller1 called');
        return target();
    }

    proxy1 = new Proxy(function dummy() {}, {
        apply: function (targ, thisArg, argArray) {
            print('apply trap 1 called');
            return target();
        }
    });

    proxy2 = new Proxy(function dummy() {}, {
        apply: function (targ, thisArg, argArray) {
            print('apply trap 2 called');
            print('relative depth:', getDepth() - baseline);
            return 234;
        }
    });

    function caller2() {
        print('caller2 called');
        return proxy2();
    }

    proxy3 = new Proxy(function dummy() {}, {
        apply: function (targ, thisArg, argArray) {
            print('apply trap 3 called');
            print('relative depth:', getDepth() - baseline);
            return 234;
        }
    });

    proxy4 = new Proxy(function dummy() {}, {
        apply: function (targ, thisArg, argArray) {
            print('apply trap 4 called');
            return proxy3();
        }
    });

    // Normal -> normal tailcall.
    print('- normal -> normal');
    print(caller1());

    // Proxy apply -> normal tailcall.
    print('- proxy apply -> normal');
    print(proxy1());

    // Normal -> Proxy apply tailcall.
    print('- normal -> proxy apply');
    print(caller2());

    // Proxy apply -> Proxy apply tailcall.
    print(proxy4());
}

try {
    print('proxy apply test 1');
    proxyApplyTest1();
} catch (e) {
    print(e.stack || e);
}
