/*
 *  Duktape Node.js Buffer virtual properties are handled by
 *  Object.defineProperty() in Duktape 3.x (didn't work in Duktape 2.x).
 */

/*===
object 8 testdata
100
68
object 8 testData
===*/

function test() {
    var buf = new Buffer('testdata');
    print(typeof buf, buf.length, String(buf));

    print(buf[4]);
    try {
        Object.defineProperty(buf, '4', {
            value: 68
        });
    } catch (e) {
        // Duktape: TypeError: property is virtual
        // Node.js: TypeError: Cannot redefine a property of an object with external array elements
        print(e.name);
        //print(e.stack || e);
    }
    print(buf[4]);
    print(typeof buf, buf.length, String(buf));
}

test();
