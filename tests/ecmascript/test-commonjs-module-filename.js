/*
 *  Duktape 1.5 added module.filename and module.name support.
 */

/*@include util-buffer.js@*/

/*===
default behavior
test/foo2 test/foo2 undefined undefined
test/foo2
2
foo2
TIME INF test/foo2: test
override .name and .filename
test/bar2 test/bar2 undefined undefined
my_source.js
2
my_module
TIME INF my_source.js: test
.name shadowing test
test/quux2 test/quux2 undefined undefined
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
        print(bufferToStringRaw(buf).replace(/^\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\d.*?Z/, 'TIME'));
    };

    // Default behavior, module wrapper .fileName is resolved module ID,
    // .name is last component of resolved ID (module.filename and module.name
    // are not set by default which triggers the default values).

    Duktape.modSearch = function modSearch(id, require, exports, module) {
        print(id, module.id, module.filename, module.name);
        return testSource;
    };

    print('default behavior');
    var tmp = require('test/foo1/../foo2');

    // Override module.filename and module.name in modSearch().

    Duktape.modSearch = function modSearch(id, require, exports, module) {
        print(id, module.id, module.filename, module.name);
        module.filename = 'my_source.js';  // match source filename for example
        module.name = 'my_module';
        return testSource;
    };

    print('override .name and .filename');
    var tmp = require('test/bar1/../bar2');

    // Test that the forced .name property of the module wrapper function
    // does not introduce a shadowing binding -- this is important for
    // semantics and works because the wrapper function is initially
    // compiled as an anonymous function (which ensures the function doesn't
    // get a "has a name binding" flag) and .name is then forced manually.

    var global = new Function('return this')();
    global.myFileName = 123;  // this should be visible

    Duktape.modSearch = function modSearch(id, require, exports, module) {
        print(id, module.id, module.filename, module.name);
        module.filename = 'myFileName';
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
