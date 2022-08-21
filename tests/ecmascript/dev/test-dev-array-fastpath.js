/*
 *  Exercise array fast paths (DUK_USE_ARRAY_FASTPATH).
 */

/*---
duktape_config:
  DUK_USE_ARRAY_FASTPATH: true
---*/

/*===
499500
0
===*/

function pushPopTest() {
    var arr, i, arg;

    arr = []; arg = [];
    for (i = 0; i < 1000; i++) {
        arr.push.apply(arr, arg);
        arg.push('dummy');
    }
    print(arr.length);

    while (arr.length > 0) {
        arr.pop();
    }
    print(arr.length);

    // XXX: At the moment there's no shrink check in the fast path.
}

pushPopTest();
