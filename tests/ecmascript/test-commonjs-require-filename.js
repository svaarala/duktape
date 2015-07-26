/*
 *  Filename for functions inside a module loaded using require()
 *
 *  In Duktape 1.0.0 this would always be "duk_bi_global.c" which is confusing.
 *  For Duktape 1.1.0 this was fixed to be the fully resolved module ID.
 *  See GH-58 for discussion.
 */

/*---
{
    "custom": true
}
---*/

/*===
moduleFunc name: 
moduleFunc fileName: foo
testFunc name: testFunc
testFunc fileName: foo
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
    var mod = require('foo');

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
