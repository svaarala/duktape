/*
 *  JSON.stringify() for a Buffer
 */

/*@include util-nodejs-buffer.js@*/

/*===
JSON.stringify test
{"type":"Buffer","data":[18,18,18,18,18,18,18,18,255,18]}
===*/

/* JSON.stringify test */

function jsonStringifyTest() {
    var b = new Buffer(10);
    b.fill(0x12);
    b[8] = 0xff;  // test potentially signed value too
    print(JSON.stringify(b));
}

try {
    print('JSON.stringify test');
    jsonStringifyTest();
} catch (e) {
    print(e.stack || e);
}
