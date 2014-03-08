/*
 *  Virtual properties for plain buffers and buffer objects.
 */

/*---
{
    "custom": true
}
---*/

/*===
buffer
length 4
222
173
190
239
object
length 4
222
173
190
239
===*/

function test() {
    var buf_plain, buf_object;
    var i;

    buf_plain = Duktape.dec('hex', 'deadbeef');
    print(typeof buf_plain);
    print('length', buf_plain.length);
    for (i = 0; i < buf_plain.length; i++) {
        print(buf_plain[i]);
    }

    buf_object = new Duktape.Buffer(Duktape.dec('hex', 'deadbeef'));
    print(typeof buf_object);
    print('length', buf_object.length);
    for (i = 0; i < buf_object.length; i++) {
        print(buf_object[i]);
    }
}

try {
    test();
} catch (e) {
    print(e);
}
