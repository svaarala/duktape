/*
 *  Node.js Buffer constructor properties
 */

function encValue(x) {
    if (typeof x === 'function') { return 'function' }

    // Avoid Node.js problem for: String(Buffer.prototype).
    // > process.version
    // 'v0.12.1'
    // > String(Buffer.prototype)
    // node: ../src/node_buffer.cc:262: void node::Buffer::StringSlice(const v8::FunctionCallbackInfo<v8::Value>&) [with node::encoding encoding = (node::encoding)1u]: Assertion `obj_data != __null' failed.
    // Aborted (core dumped)

    if (x === Buffer.prototype) { return '[object Object]'; }
    return String(x);
}

/*===
Node.js Buffer constructor properties test
name true string Buffer
length true number 2
prototype true object [object Object]
isEncoding true function function
isBuffer true function function
byteLength true function function
concat true function function
compare true function function
true
===*/

function nodejsBufferConstructorPropertiesTest() {
    var props = [
        'name',
        'length',
        'prototype',

        'isEncoding',
        'isBuffer',
        'byteLength',
        'concat',
        'compare'
    ];

    props.forEach(function (propname) {
        var obj = Buffer;
        var val = obj[propname];
        print(propname, propname in obj, typeof val, encValue(val));
    });

    print(Buffer.prototype.constructor === Buffer);
}

try {
    print('Node.js Buffer constructor properties test');
    nodejsBufferConstructorPropertiesTest();
} catch (e) {
    print(e.stack || e);
}
