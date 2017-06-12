/*
 *  Tailcall requires that current and target function share the same return
 *  value handling; constructor calls have a special return value handling
 *  compared to normal function calls.
 */

/*---
{
    "custom": true
}
---*/

/*===
normal -> normal
- tailcall
normal -> normal, using call
- tailcall
normal -> normal, using apply
- tailcall
normal -> normal, using Reflect.apply
- tailcall
constructor -> constructor, using new
- not a tailcall
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

function getDepth() {
    //console.trace();
    for (var i = -1; Duktape.act(i); i--) {
    }
    return -i;
}

function test() {
    var baseline;

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

    // Baseline: normal, non-tailcall call stack depth.
    function baseline_a() { baseline = getDepth(); }
    function baseline_b() { baseline_a(); }
    baseline_b();

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

    // Constructor -> constructor tail call.  This works now with
    // Reflect.construct() but with with 'new Xxx()' but this is
    // easy to fix.
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
    test();
} catch (e) {
    print(e.stack || e);
}
