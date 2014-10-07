/*
 *  Function call as a left-hand side expression (E5 Sections 11.2.3, 11.13,
 *  11.3.1, 11.3.2, 11.4.4, 11.4.5).
 */

/*===
ReferenceError
f2 error
f3 error
===*/

function f1() { return 1; }
function f2() { throw new Error("f2 error"); }
function f3() { throw new Error("f3 error"); }

try {
    f1() = f1();
} catch (e) {
    print(e.name);
}

try {
    f2() = f3();
} catch (e) {
    print(e.message);
}

try {
    f1() = f3();
} catch (e) {
    print(e.message);
}
