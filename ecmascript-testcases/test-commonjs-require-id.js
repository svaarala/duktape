/*
 *  When a module is being required Duktape provides the module with a
 *  fresh require() function to facilitate resolution of relative module
 *  identifiers.  The require() function passed will have an 'id' property
 *  identifying the module in question; any require() call will then
 *  resolve relative identifiers relative to require.id.
 *
 *  This behavior is Duktape specific, not required by CommonJS.
 */

/*---
{
    "custom": true
}
---*/

/*===
function
false undefined undefined
foo/bar
false
function string foo/bar
===*/

var global_require = require;

Duktape.modSearch = function (id) {
    if (id === 'foo/bar') {
        return 'print("foo/bar");' +
               'print(require === global_require);' +
               'print(typeof require, typeof require.id, require.id);';
    }
    throw new Error('cannot resolve module: ' + id);
};

function test() {
    // Global require(), 'id' is undefined
    print(typeof this.require);
    print('id' in this.require, typeof this.require.id, this.require.id);

    // Module require(), require.id gets the *resolved* id
    var foobar = require('foo/./bar');  // resolves to 'foo/bar'
}

try {
    test();
} catch (e) {
    print(e);
}
