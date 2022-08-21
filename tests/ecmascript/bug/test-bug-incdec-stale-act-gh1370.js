/*
 *  Stale 'act' pointer in pre/post inc/dec handling.
 *  https://github.com/svaarala/duktape/issues/1370
 *

/*===
RangeError
===*/

try {
    (function foo() {
       foo--; foo();
    })();
} catch (e) {
    print(e.name);  // eventual RangeError
}
