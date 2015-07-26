/*
 *  If module loading fails due to a thrown error, CommonJS doesn't really
 *  specify what to do.  The module must be registered in modLoaded[] when
 *  loading starts because the module may require another module which then
 *  requires the module being loaded (i.e. a circular require, which is a
 *  supported scenario).  But what to do with that registration if module
 *  loading fails due to an error -- keep or remove?
 *
 *  Node.js seems to remove the module cache entry so that require()'ing
 *  again throws another error like the original.  Duktape does the same
 *  since Duktape 1.3 (before that the partial module would get cached).
 */

/*===
loading module...
Error: aiee
false
loading module...
Error: aiee
false
===*/

function moduleLoadErrorTest() {
    Duktape.modSearch = function (id, require, exports, module) {
        if (id === 'test1/foo') {
            return 'print("loading module...");\n' +
                   'exports.magic = 42;\n' +
                   'throw new Error("aiee");'
        }
        throw new Error('module not found: ' + id);
    };

    try {
        require('test1/foo');
        print('success');
    } catch (e) {
        print(e);
    }
    print('test1/foo' in Duktape.modLoaded);

    try {
        require('test1/foo');
        print('success');
    } catch (e) {
        print(e);
    }
    print('test1/foo' in Duktape.modLoaded);
}

try {
    moduleLoadErrorTest();
} catch (e) {
    print(e.stack || e);
}
