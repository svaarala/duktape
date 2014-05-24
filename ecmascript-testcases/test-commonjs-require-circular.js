/*
 *  Circular require should work in CommonJS.
 */

/*---
{
    "custom": true
}
---*/

/*===
Duktape.find foo
loading foo
Duktape.find bar
loading bar
foo.val=123
foo.val + bar.val = 357
123
===*/

Duktape.find = function (id) {
    print('Duktape.find', id);
    if (id == 'foo') {
        return "print('loading foo');\n" +
               "exports.val = 123;\n" +
               "print('foo.val + bar.val = ' + (require('foo').val + require('bar').val));\n"
    } else if (id == 'bar') {
        return "print('loading bar');\n" +
               "print('foo.val=' + require('foo').val);\n" +
               "exports.val = 234;\n";
    } else {
        throw new Error('no such module: ' + id);
    }
}

try {
    print(require('foo').val);
} catch (e) {
    print(e);
}
