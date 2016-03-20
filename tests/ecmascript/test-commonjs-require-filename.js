/*
 *  Filename for functions inside a module loaded using require()
 *
 *  In Duktape 1.0.0 this would always be "duk_bi_global.c" which is confusing.
 *
 *  In Duktape 1.1.0 this was fixed to be the fully resolved module ID.
 *  See GH-58 for discussion.
 *
 *  In Duktape 1.5.0 the module wrapper function is also given a .name which
 *  defaults to the last component of the resolved module ID.  Both the .name
 *  and .fileName can be overridden by modSearch via module.name and
 *  module.fileName.
 */

/*---
{
    "custom": true
}
---*/

/*===
moduleFunc name: foo
moduleFunc fileName: my/foo
testFunc name: testFunc
testFunc fileName: my/foo
===*/

function modSearch() {
    return "function testFunc() { print('testFunc name:', Duktape.act(-2).function.name);\n" +
           "                      print('testFunc fileName:', Duktape.act(-2).function.fileName); }\n" +
           "\n" +
           "exports.testFunc = testFunc;\n" +
           "print('moduleFunc name:', Duktape.act(-2).function.name);\n" +
           "print('moduleFunc fileName:', Duktape.act(-2).function.fileName);\n";
}

function test() {
    /* For the module function itself (which is a Duktape internal artifact)
     * the fileName is already forced to be module ID.  This was OK in
     * Duktape 1.0.0.
     */

    Duktape.modSearch = modSearch;
    var mod = require('my/foo');

    /* However, functions defined within the module don't have a proper
     * fileName in Duktape 1.0.0.
     */
    mod.testFunc();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
