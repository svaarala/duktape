/*
 *  Duktape 1.5 added module.fileName and module.name support.
 */

/*===
default behavior
test/foo2 test/foo2 test/foo2 foo2
test/foo2
2
foo2
TIME INF test/foo2: test
override .name and .fileName
test/bar2 test/bar2 test/bar2 bar2
my_source.js
2
my_module
TIME INF my_source.js: test
.name shadowing test
test/quux2 test/quux2 test/quux2 quux2
object
number
123
===*/

var testSource =
    '/* test */\n' +
    'var err = new Error("aiee");\n' +
    'print(err.fileName);\n' +
    'print(err.lineNumber);\n' +
    'print(Duktape.act(-2).function.name);\n' +
    '/*print(err.stack);*/\n' +
    'var logger = new Duktape.Logger();\n' +
    'logger.info("test");\n';

function test() {
    // Replace Duktape.Logger.prototype.raw to censor timestamps.

    Duktape.Logger.prototype.raw = function (buf) {
        print(String(buf).replace(/^\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\d.*?Z/, 'TIME'));
    };

    // Default behavior, module.fileName is resolved module ID.

    Duktape.modSearch = function modSearch(id, require, exports, module) {
        print(id, module.id, module.fileName, module.name);
        return testSource;
    };

    print('default behavior');
    var tmp = require('test/foo1/../foo2');

    // Override module.fileName and module.name in modSearch().

    Duktape.modSearch = function modSearch(id, require, exports, module) {
        print(id, module.id, module.fileName, module.name);
        module.fileName = 'my_source.js';  // match source filename for example
        module.name = 'my_module';
        return testSource;
    };

    print('override .name and .fileName');
    var tmp = require('test/bar1/../bar2');

    // Test that the forced .name property of the module wrapper function
    // does not introduce a shadowing binding -- this is important for
    // semantics and works because the wrapper function is initially
    // compiled as an anonymous function (which ensures the function doesn't
    // get a "has a name binding" flag) and .name is then forced manually.

    var global = new Function('return this')();
    global.myFileName = 123;  // this should be visible

    Duktape.modSearch = function modSearch(id, require, exports, module) {
        print(id, module.id, module.fileName, module.name);
        module.fileName = 'myFileName';
        return 'print(typeof Math); print(typeof myFileName); print(myFileName);';
    };

    print('.name shadowing test');
    var tmp = require('test/quux1/../quux2');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
