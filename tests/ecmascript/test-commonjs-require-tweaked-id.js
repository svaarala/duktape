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
Duktape.modSearch quux
quux: Error
Duktape.modSearch foo/quux
./quux: Error
Duktape.modSearch quux
../quux: Error
Duktape.modSearch testModule/subModule
Duktape.modSearch test/innerRequire
testModule: Error
===*/

Duktape.modSearch = function (id) {
    print('Duktape.modSearch', id);
    if (id == 'testModule/subModule') {
        // require.id is non-writable but is configurable, so its value must
        // be changed with Object.defineProperty().
        return 'var mod;\n' +
               'exports.name = "testModule/subModule";\n' +
               'Object.defineProperty(require, "id", { value: "./././testModule/subModule/foo/../../../test/foo" });\n' +   // same as 'test/foo' but non-canonical
               'mod = require("./innerRequire");\n';  // test/foo + ./innerRequire -> test/innerRequire
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
    globalTest('./quux');         // foo/bar + ./quux -> foo/quux
    globalTest('../quux');        // foo/bar + ../quux -> quux
    delete require.id;

    // Module 'id' not relative
    try {
        mod = require('testModule/subModule');
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
