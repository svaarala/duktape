/*
 *  SlowBuffer is only available through a 'require' statement, not
 *  implemented for now.
 */

/*---
skip: true
---*/

/*@include util-buffer.js@*/

function testSlowBuffer() {
    var buffer = require('buffer');
    print(buffer.SlowBuffer);
}

testSlowBuffer();
