/*===
test1 1
tailcalled 1
100
test2 1
tailcalled 1
100
test3 1
tailcalled 1
100
===*/

/* Tailcall from inside a siwthc is allowed: even though the catch stack
 * will contain entries referring to the callstack entry being reused,
 * this is not a problem because the entries are not error catching.
 * In Duktape 0.9.0 there was an incorrection assertion in duk_js_call.c
 * which would also trigger an assertion error when tailcall was made
 * from inside a switch:
 *
 *     for (i = 0; i < thr->catchstack_top; i++) {
 *         DUK_ASSERT(thr->catchstack[i].callstack_index < our_callstack_index);
 *     }
 *
 * This has been fixed in Duktape 0.10.0.
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
