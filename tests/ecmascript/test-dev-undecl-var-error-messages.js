/*
 *  Error messages for reading/writing an undeclared variable in strict
 *  mode includes the variable name (safely sanitized).
 */

/*---
{
    "custom": true
}
---*/

/*===
ReferenceError: identifier 'noSuch' undefined
ReferenceError: identifier 'noSuch' undefined
===*/

function testRead() {
    'use strict';
    var dummy = noSuch;
}

function testWrite() {
    'use strict';
    noSuch = 123;
}

try {
    testRead();
} catch (e) {
    print(e);
}

try {
    testWrite();
} catch (e) {
    print(e);
}
