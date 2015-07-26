/*
 *  Test JSON error byte offset
 */

/*---
{
    "custom": true
}
---*/

/*===
SyntaxError: invalid json (at offset 11)
SyntaxError: invalid json (at offset 17)
===*/

function test1() {
    JSON.parse('[ "\ufedcfoo"; ]');  // error at char offset 8, byte offset, 11
}

function test2() {
    JSON.parse('{ "foo": "bar" } x');  // error at byte offset 17
}

try {
    test1();
} catch (e) {
    print(e);
}

try {
    test2();
} catch (e) {
    print(e);
}
