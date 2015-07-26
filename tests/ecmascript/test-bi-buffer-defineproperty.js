/*
 *  Duktape.Buffer virtual properties don't work in Object.defineProperty().
 */

/*---
{
    "custom": true
}
---*/

/*===
object 8 testdata
100
TypeError
100
object 8 testdata
===*/

function test() {
    var buf = new Duktape.Buffer('testdata');
    print(typeof buf, buf.length, buf);

    print(buf[4]);
    try {
        Object.defineProperty(buf, '4', {
            value: 68
        });
    } catch (e) {
        // TypeError: property is virtual
        print(e.name);
        //print(e.stack || e);
    }
    print(buf[4]);
    print(typeof buf, buf.length, buf);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
