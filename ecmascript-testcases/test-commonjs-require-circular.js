/*
 *  Circular require should work in CommonJS.
 */

/*---
{
    "custom": true
}
---*/

/*===
Duktape.modSearch foo
loading foo
Duktape.modSearch bar
loading bar
foo.val=123
foo.val + bar.val = 357
123
===*/

Duktape.modSearch = function (id) {
    print('Duktape.modSearch', id);
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
