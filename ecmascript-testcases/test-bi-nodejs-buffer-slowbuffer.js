/*
 *  SlowBuffer is only available through a 'require' statement, not
 *  implemented for now.
 */

/*---
{
    "skip": true
}
---*/

/*@include util-nodejs-buffer.js@*/

function testSlowBuffer() {
    var buffer = require('buffer');
    print(buffer.SlowBuffer);
}

try {
    testSlowBuffer();
} catch (e) {
    print(e.stack || e);
}
