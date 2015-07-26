/*
 *  Test module search function.
 *
 *  A module search function can:
 *
 *    - Throw an error to indicate module cannot be found.
 *
 *    - Add symbols directly to the 'exports' table.
 *
 *    - Return a string which is interpreted as Ecmascript source
 *      of the module.
 *
 *    - Return a non-string which is interpreted to mean that the
 *      module has been found, but has no Ecmascript source.  In other
 *      words, the module is a "pure C" one, and all necessary symbols
 *      have been added to the exports table.
 */

/*===
Duktape.modSearch dummy1
require function false
require.id true string dummy1 false false true
exports object [object Object]
module object [object Object]
module.id true string dummy1 false false false
Error: module not found
Duktape.modSearch foo
Error: module not found
Duktape.modSearch foo
Error: module not found
false
Duktape.modSearch dummy2
bar
Duktape.modSearch dummy3
function
hello world
Duktape.modSearch dummy4
function
function
caught Error: funcRaw throws
Duktape.modSearch retval/mod0
0 object [object Object] true
Duktape.modSearch retval/mod1
1 object [object Object] true
Duktape.modSearch retval/mod2
2 object [object Object] true
Duktape.modSearch retval/mod3
3 object [object Object] true
Duktape.modSearch retval/mod4
4 object [object Object] true
Duktape.modSearch retval/mod5
5 object [object Object] true
Duktape.modSearch retval/mod6
6 object [object Object] true
Duktape.modSearch retval/mod7
7 object [object Object] true
===*/

var global_require = require;

function moduleSearchTest() {
    var mod;
    var i;
    var modId;

    function catchRequire(id) {
        try {
            return require(id);
        } catch (e) {
            print(e);
            return;
        }
    }

    /*
     *  First just check the arguments.
     */

    Duktape.modSearch = function (id, require, exports, module) {
        var pd;
        print('Duktape.modSearch', id);
        print('require', typeof require, require === global_require);
        pd = Object.getOwnPropertyDescriptor(require, 'id');
        print('require.id', 'id' in require, typeof require.id, require.id, pd.writable, pd.enumerable, pd.configurable);
        print('exports', typeof exports, exports);
        print('module', typeof module, module);
        pd = Object.getOwnPropertyDescriptor(module, 'id');
        print('module.id', 'id' in module, typeof module.id, module.id, pd.writable, pd.enumerable, pd.configurable);
        throw new Error('module not found');
    };

    catchRequire('./dummy1');

    /*
     *  Module search function can throw an error to indicate module is not
     *  found; module won't get registered to Duktape.modLoaded.
     */

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        throw new Error('module not found');
    };

    catchRequire('./foo');
    catchRequire('./foo');  // throws again because not registered
    print('foo' in Duktape.modLoaded);

    /*
     *  Module search function can return source code which is interpreted
     *  as the module source code executed in the CommonJS specified
     *  environment.  This is the base case, and there's a separate test
     *  for the environment specifics.
     */

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        return 'exports.foo = "bar";';
    };

    mod = require('dummy2');
    print(mod.foo);

    /*
     *  Module search function can export symbols through 'exports'
     *  and return a non-string: this behavior is needed for C modules.
     */

    Duktape.modSearch = function (id, require, exports, module) {
        print('Duktape.modSearch', id);
        exports.func1 = function() { print('hello world'); };
        return;  // undefined is treated as 'no source' but module is found
    };

    mod = require('dummy3');
    print(typeof mod.func1);
    mod.func1();

    /*
     *  Mixed modules are also possible.  This test simulates the case where
     *  a C module provides funcRaw() and Ecmascript module provides a safe
     *  wrapper around it.
     */

    Duktape.modSearch = function (id, require, exports, module) {
        print('Duktape.modSearch', id);
        exports.funcRaw = function () { throw new Error('funcRaw throws'); };
        return 'exports.func = function () { try { exports.funcRaw(); } catch (e) { print("caught", e); } };';
    };

    mod = require('dummy4');
    print(typeof mod.funcRaw);
    print(typeof mod.func);
    mod.func();

    /*
     *  Return values other than string are currently always treated like
     *  undefined/null, i.e. module found, no source code provided.  This
     *  is not necessarily desirable behavior, but test for it.
     */

    var retVals = [ undefined, null, true, false, 123, {foo:1}, [1,2], function (){} ];

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        return retVals.shift();
    };


    for (i = 0; i < 8; i++) {
        modId = 'retval/mod' + i;
        mod = require(modId);
        print(i, typeof mod, mod, modId in Duktape.modLoaded);
    }

    //print(Duktape.enc('jx', Duktape.modLoaded));
}

try {
    moduleSearchTest();
} catch (e) {
    print(e);
}
