/*
 *  In Duktape 1.3 Duktape.modLoaded[] caches the "module" table instead of
 *  the "exports" table.  This has the advantage that if a module assigns to
 *  module.exports during loading, the change will be visible to recursively
 *  required sub-modules immediately.  This matches Node.js behavior.
 */

/*===
modSearch: test1/foo
modSearch: test1/bar
bar sees typeof foo: function
foo sees typeof bar: object
main sees typeof foo: function
main sees typeof bar: object
===*/

function moduleExportsCircularRequireTest() {
    Duktape.modSearch = function (id, require, exports, module) {
        print('modSearch:', id);

        if (id === 'test1/foo') {
            // The module.exports assignment is immediately visible to test1/bar.
            return 'module.exports = function adder(x,y) { return x+y; }\n' +  // immediately visible
                   'var bar = require("./bar");\n' +
                   'print("foo sees typeof bar: " + typeof bar);';
        }
        if (id === 'test1/bar') {
            return 'var foo = require("./foo");\n' +
                   'print("bar sees typeof foo: " + typeof foo);';
        }
        throw new Error('cannot find module: ' + id);
    }

    var foo = require('test1/foo')
    var bar = require('test1/bar')
    print('main sees typeof foo: ' + typeof foo);
    print('main sees typeof bar: ' + typeof bar);
}

try {
    moduleExportsCircularRequireTest();
} catch (e) {
    print(e.stack || e);
}
