/*
 *  Duktape 1.3 added support for replacing module.exports both during
 *  modSearch() call and inside the module code.
 */

/*===
module.exports replaced by module
modSearch test1/foo
true
true
true
function
2+3=5
true
===*/

/* Common case: module replaces module.exports with e.g. a function value. */
function moduleExportsReplacedByModuleTest() {
    Duktape.modSearch = function (id, require, exports, module) {
        print('modSearch', id);
        print('id' in module);
        print('exports' in module);  // added in Duktape 1.3
        print(module.exports === exports);

        if (id === 'test1/foo') {
            return 'module.exports = function adder(x, y) { return x + y; };'
        }
        throw new Error('module not found: ' + id);
    }

    var adder = require('test1/foo');
    print(typeof adder);
    print('2+3=' + adder(2, 3));

    // ensure cached value is the same as returned value (easy to get wrong)
    print(adder === require('test1/foo'));
}

try {
    print('module.exports replaced by module');
    moduleExportsReplacedByModuleTest();
} catch (e) {
    print(e.stack || e);
}

/*===
module.exports replaced by modSearch
modSearch test2/foo
true
true
true
myExports1
true
myExports1
function
2+3=5
true
modSearch test2/bar
true
true
true
myexports2
true
function
myexports2 multiplier
function
2*3=6
true
modSearch test2/quux
true
true
true
myExports3
true
subber
function
2-3=-1
true
===*/

/* Duktape.modSearch() can also replace module.exports. */

function moduleExportsReplacedByModSearchTest() {
    var myExports1 = { name: 'myExports1' };
    var myExports2 = function myexports2(x, y) {
        return x * y;
    };
    myExports2.myName = 'myExports2';  // avoid .name, already provided for function
    var myExports3 = { name: 'myExports3' };

    Duktape.modSearch = function (id, require, exports, module) {
        print('modSearch', id);
        print('id' in module);
        print('exports' in module);  // added in Duktape 1.3
        print(module.exports === exports);

        if (id === 'test2/foo') {
            // modSearch replaces module.exports with another table which is
            // seen by the module both as 'exports' and 'module.exports'.
            module.exports = myExports1;
            return 'print(exports.name); /* myExports1, replacement seen here */\n' +
                   'print(exports === module.exports);\n' +
                   'exports.adder = function adder(x, y) { return x + y; };'
        }
        if (id === 'test2/bar') {
            // modSearch replaces module.exports with a Function value
            // which is then seen by the module both as 'exports' and
            // 'module.exports'.
            module.exports = myExports2;
            return 'print(exports.name);\n' +
                   'print(exports === module.exports);\n' +
                   'print(typeof exports);  /*function*/\n' +
                   'exports.myName = "multiplier";'  // replace previous .name
        }
        if (id === 'test2/quux') {
            // modSearch replaces module.exports with another table, and
            // module then replaces module.exports another time.  This is
            // more for completeness than being useful.
            module.exports = myExports3;
            return 'print(exports.name);\n' +
                   'print(exports === module.exports);\n' +
                   'module.exports = function subber(x, y) { return x - y; };'
        }
        throw new Error('module not found: ' + id);
    }

    var foo = require('test2/foo');
    print(foo.name);
    print(typeof foo.adder);
    print('2+3=' + foo.adder(2, 3));
    print(foo === require('test2/foo'));

    var bar = require('test2/bar');
    print(bar.name, bar.myName);
    print(typeof bar);
    print('2*3=' + bar(2, 3));
    print(bar === require('test2/bar'));

    var quux = require('test2/quux');
    print(quux.name);
    print(typeof quux);
    print('2-3=' + quux(2, 3));
    print(quux === require('test2/quux'));
}

try {
    print('module.exports replaced by modSearch');
    moduleExportsReplacedByModSearchTest();
} catch (e) {
    print(e.stack || e);
}
