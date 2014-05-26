/*
 *  Normally global require() has no 'id' property and module 'id'
 *  properties are set by Duktape with fully resolved, canonical
 *  identifiers.
 *
 *  If user code tweaks these IDs, the global require() may then have
 *  an 'id' and global require() or module require() 'id' properties
 *  may have invalid or non-canonical values.  This testcase ensures
 *  that Duktape re-resolves even the manually tweaked 'id' values.
 */

/*===
Duktape.find quux
quux: Error
Duktape.find foo/bar/quux
./quux: Error
Duktape.find foo/quux
../quux: Error
Duktape.find testModule
Duktape.find test/innerRequire
testModule: Error
===*/

Duktape.find = function (id) {
    print('Duktape.find', id);
    if (id == 'testModule') {
        return 'var mod;\n' +
               'exports.name = "testModule";\n' +
               'require.id = "./././testModule/foo/../../test";\n' +   // same as 'test' but non-canonical
               'mod = require("./innerRequire");\n';
    }
    throw new Error('cannot find module');
}

function tweakedIdentifierTest() {
    var mod;

    function globalTest(id) {
        var mod;
        try {
            mod = require(id);
            print(id + ':', 'success, should not happen');
        } catch (e) {
            print(id + ':', e.name);
        }
    }

    // Global require.id

    require.id = './foo//./bar';  // same as 'foo/bar'
    globalTest('quux');
    globalTest('./quux');
    globalTest('../quux');
    delete require.id;

    // Module 'id' not relative
    try {
        mod = require('testModule');
        print('never here');
    } catch (e) {
        print('testModule:', e.name);
    }
}

try {
    tweakedIdentifierTest();
} catch (e) {
    print(e);
}
