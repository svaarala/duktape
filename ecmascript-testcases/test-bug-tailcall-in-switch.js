/*===
test1 1
100
test2 1
100
test3 1
100
===*/

/* In Duktape 0.9.0 a tailcall from inside a switch was allowed.
 * This means that the tailcall would be made even if the catch
 * stack contained catcher entries for the callstack entry being
 * reused by the tailcall.  With assertions enabled, the test below
 * would fail in duk_js_call.c:1788:
 *
 *     for (i = 0; i < thr->catchstack_top; i++) {
 *         DUK_ASSERT(thr->catchstack[i].callstack_index < our_callstack_index);
 *     }
 */

function tailcalled(x) {
    print('tailcalled', x);
    return x * 100;
}

function test1(x) {
    print('test1', x);
    return tailcalled(x);
}

function test2(x) {
    var t;

    print('test2', x);
    switch (x) {
    case 1:
        t = tailcalled(x); return t;
    default:
        t = tailcalled(x); return t;
    }
}

function test3(x) {
    print('test3', x);
    switch (x) {
    case 1:
        return tailcalled(x);
    default:
        return tailcalled(x);
    }
}


try {
    print(test1(1));
} catch (e) {
    print(e);
}

try {
    print(test2(1));
} catch (e) {
    print(e);
}

try {
    print(test3(1));
} catch (e) {
    print(e);
}
