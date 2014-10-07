/*
 *  Error objects (E5 Section 15.11).
 */

/*===
Error my message
gotName: false string "Error"
gotMessage: true string "my message"
Error 
gotName: false string "Error"
gotMessage: false string ""
URIError
gotName: false string "URIError"
gotMessage: true string 13
===*/

function checkProps(e, printMsg) {
    var gotName = false;
    var gotMessage = false;

    /*
     *  An error inherits its 'name' property.  Its only guaranteed own
     *  property is 'message', when the error is constructed with a message.
     *
     *  Most engines will provide several other properties (like 'stack'),
     *  but only standard properties are tested here.  Also engines differ
     *  in what properties are own and what are inherited.
     */

    Object.getOwnPropertyNames(e).forEach(function (k) {
        if (k === 'name') { gotName = true; }
        if (k === 'message') { gotMessage = true; }
    });

    print('gotName:', gotName, typeof e.name, JSON.stringify(e.name));
    print('gotMessage:', gotMessage, typeof e.message, printMsg ? JSON.stringify(e.message) : e.message.length);
}

function test() {
    var err;

    err = new Error('my message');
    print(err.name, err.message);
    checkProps(err, true);

    err = new Error();
    print(err.name, err.message);
    checkProps(err, true);

    try {
        decodeURIComponent('%ff%ff');
    } catch (e) {
        print(e.name);  // message varies between implementations
        checkProps(e, false);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
