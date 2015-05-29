/*
 *  Module return value / error thrown does not affect caching of the
 *  module in Duktape.modLoaded.  The module is registered before its
 *  source code is executed.
 */

/*===
Duktape.modSearch foo
foo.name: foo
Duktape.modSearch bar
bar.name: bar
Duktape.modSearch quux
Error: aiee
Duktape.modSearch quux
Error: aiee
===*/

function moduleReturnValueTest() {
    var mod;

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        var ret = {
            foo: 'exports.name = "foo";',
            bar: 'exports.name = "bar"; return 123;',
            quux: 'exports.name = "quux"; exports.exported = "exportedValue"; throw new Error("aiee");'
        }[id];
        if (ret) {
            return ret;
        }
        throw new Error('module not found: ' + id);
    };

    /* Undefined return value */

    mod = require('foo');
    print('foo.name:', mod.name);

    /* Number return value (just an example; this is ignored entirely) */

    mod = require('bar');
    print('bar.name:', mod.name);

    /* Error thrown */

    try {
        mod = require('quux');
        print('quux.name:', mod.name);  // never here
    } catch (e) {
        print(e);
    }

    /* CommonJS doesn't really specify what to do with a cached module if
     * module loading fails.  In Duktape 1.2 the modLoaded[] cache entry
     * would be kept; Duktape 1.3 removes the cache entry so that a module
     * can be reloaded (this matches Node.js behavior).
     */

    try {
        mod = require('quux');
        print('quux.name:', mod.name);  // never here
    } catch (e) {
        print(e);
    }
}

try {
    moduleReturnValueTest();
} catch (e) {
    print(e);
}
