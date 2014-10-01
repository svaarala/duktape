/*
 *  Module identifier resolution
 */

/*
 *  Global identifiers start with a term other than '.' or '..':
 *
 *    require('foo')       // absolute identifier
 *    require('foo/bar')   // absolute identifier
 *    require('foo/./bar') // absolute identifier, resolves to 'foo/bar'
 *
 *  Relative identifers are resolved relative to the current module, i.e.
 *  the module whose require() function is called.  For instance, when
 *  the current module is 'foo/bar':
 *
 *    require('baz')       // absolute identifier, 'baz'
 *    require('./baz')     // relative identifier, resolves to 'foo/baz'
 *    require('../baz')    // relative identifier, resolves to 'baz'
 *    require('../../baz') // relative identifier, resolution fails
 *
 *  For this to work, a separate require() function is given to every
 *  module, with require.id tracking the resolved identifier of the
 *  current module.
 *
 *  The internal implementation for relative paths is simply to concatenate
 *  the module's (resolved) absolute identifier with the requested relative
 *  path, separated by a slash.  After this, resolution proceeds like global
 *  require() resolution.
 */

/*
 *  NOTE: be careful with module caching; if module modSearch() is successful,
 *  Duktape won't modSearch() the module twice.  Different names need to be used
 *  in different tests to avoid this.  Alternatively, Duktape.modLoaded could
 *  be emptied.
 */

/*===
basic resolution
Duktape.modSearch foo/mod1
global require: foo/mod1 -> foo/mod1
global require: foo//mod1 -> foo/mod1
global require: foo/./mod1 -> foo/mod1
global require: foo//.//mod1 -> foo/mod1
global require: ./foo/./mod1 -> foo/mod1
global require: ./foo/././/.///./////////////mod1 -> foo/mod1
global require: ./foo/../foo/mod1 -> foo/mod1
Duktape.modSearch bar/a
global require: bar/a -> bar/a
global require: bar/a/ -> TypeError
Duktape.modSearch bar/a/b
global require: bar/a/b -> bar/a/b
global require: ../bar -> TypeError
global require: foo/../../bar -> TypeError
global require: .. -> TypeError
global require: /foo/mod1 -> TypeError
global require: foo/mod1/ -> TypeError
global require: foo/.bar -> TypeError
global require: foo/.../bar -> TypeError
global require: foo/mod1/. -> TypeError
global require: foo/mod1/.. -> TypeError
Duktape.modSearch baz
Duktape.modSearch xxx
Duktape.modSearch xxy
Duktape.modSearch xxx/yyy
Duktape.modSearch quux/foo
Duktape.modSearch xxz
Duktape.modSearch quux/xxw
Duktape.modSearch quux/xxw/yyy
Duktape.modSearch zzz
Duktape.modSearch www
===*/

function basicResolutionTest() {
    function globalTest(id) {
        var mod;

        try {
            mod = require(id);
            print('global require: ' + id + ' -> ' + mod.name);
        } catch (e) {
            print('global require: ' + id + ' -> ' + e.name);
        }
    }

    var moduleSources = {
        "foo/mod1": "exports.name='foo/mod1';",
        "foo/mod2": "exports.name='foo/mod2';",
        "bar/mod1": "exports.name='bar/mod1';",
        "bar/a": "exports.name='bar/a';",
        "bar/a/b": "exports.name='bar/a/b';",
        "quux": "exports.name='quux';",
    };
    Duktape.modSearch = function (id) {
        var ret;

        // The identifier given to modSearch() is a resolved absolute identifier
        print('Duktape.modSearch', id);
        ret = moduleSources[id];
        if (ret) { return ret; }
        throw new Error('cannot find module: ' + id);
    }

    /*
     *  Global require() tests - because of the internal implementation
     *  this also covers most of the relative module resolution cases.
     */

    // all of these resolve to 'foo/mod1'
    globalTest('foo/mod1');
    globalTest('foo//mod1');
    globalTest('foo/./mod1');
    globalTest('foo//.//mod1');
    globalTest('./foo/./mod1');
    globalTest('./foo/././/.///./////////////mod1');
    globalTest('./foo/../foo/mod1');

    // 'a' is both a module name and a "directory"
    globalTest('bar/a');
    globalTest('bar/a/');  // error: trailing slash
    globalTest('bar/a/b');

    // error when '..' cannot backtrack terms
    globalTest('../bar');
    globalTest('foo/../../bar');
    globalTest('..');

    // error when id begins with a slash (empty initial term)
    globalTest('/foo/mod1');

    // error when id ends with a slash (empty final term)
    globalTest('foo/mod1/');

    // error when a term begins with a period (this is Duktape specific
    // but CommonJS modules is even more strict)
    globalTest('foo/.bar');
    globalTest('foo/.../bar');

    // error when an ID ends with a '.' or '..' term
    globalTest('foo/mod1/.');
    globalTest('foo/mod1/..');

    /*
     *  Require from inside a module, both relative and absolute paths.
     */

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        if (id === 'baz') {
            return 'require("xxx");\n' +           // absolute
                   'require("./xxy");\n' +         // relative
                   'require("./xxx/yyy");\n'       // relative
                   ;
        }
        return '';   // return a fake empty module
    };

    void require('baz');

    /*
     *  Require from inside a module with a few more path components.
     */

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        if (id === 'quux/foo') {
            return 'require("xxz");\n' +           // absolute
                   'require("./xxw");\n' +         // relative
                   'require("./xxw/yyy");\n' +     // relative
                   'require("../zzz");\n' +        // relative
                   'require("././../www");\n'
                   ;
        }
        return '';   // return a fake empty module
    };

    void require('quux/foo');
}

print('basic resolution');

try {
    basicResolutionTest();
} catch (e) {
    print(e);
}

/*===
non-ascii
Duktape.modSearch: "foo"
Duktape.modSearch: "foo\u1234"
Duktape.modSearch: "foo\u1234\x01/\udead\ubeef"
Duktape.modSearch: "foo\u1234\x01/\u1234"
Duktape.modSearch: "foo\u1234\x01/\u1234"
===*/

/* Non-ASCII characters are allowed in terms; Duktape places no requirements
 * now except that terms must be non-empty, cannot begin with a period, and
 * cannot contain slashes.
 *
 * U+0000 is treated as an end-of-string in the current implementation.  This
 * is not desirable but also not worth fixing.
 */

function nonAsciiTest() {
    Duktape.modSearch = function (id) {
        print('Duktape.modSearch:', Duktape.enc('jx', id));
        throw Error('module not found');
    };

    function test(id) {
        try {
            void require(id);
            print('never here');
        } catch (e) {
            ;
        }
    }

    // a few basics
    test('foo');
    test('foo\u1234');
    test('foo\u1234\u0001/\udead\ubeef');

    // check that '..' works over non-ascii
    test('foo\u1234\u0001/\udead\ubeef/../\u1234');

    // document the fact that U+0000 terminates resolution
    test('foo\u1234\u0001/\udead\ubeef/../\u1234\u0000neverseen');
}

print('non-ascii');

try {
    nonAsciiTest();
} catch (e) {
    print(e);
}

/*===
length
Duktape.modSearch foo/bar
230: foo/bar
231: foo/bar
232: foo/bar
233: foo/bar
234: foo/bar
235: foo/bar
236: foo/bar
237: foo/bar
238: foo/bar
239: foo/bar
240: foo/bar
241: foo/bar
242: foo/bar
243: foo/bar
244: foo/bar
245: foo/bar
246: foo/bar
247: foo/bar
248: foo/bar
249: foo/bar
250: foo/bar
251: foo/bar
252: foo/bar
253: foo/bar
254: foo/bar
255: TypeError
256: TypeError
257: TypeError
258: TypeError
259: TypeError
260: TypeError
261: TypeError
262: TypeError
263: TypeError
264: TypeError
265: TypeError
266: TypeError
267: TypeError
268: TypeError
269: TypeError
270: TypeError
230: bar
231: bar
232: bar
233: bar
234: bar
235: TypeError
236: TypeError
237: TypeError
238: TypeError
239: TypeError
240: TypeError
241: TypeError
242: TypeError
243: TypeError
244: TypeError
245: TypeError
246: TypeError
247: TypeError
248: TypeError
249: TypeError
250: TypeError
251: TypeError
252: TypeError
253: TypeError
254: TypeError
255: TypeError
256: TypeError
257: TypeError
258: TypeError
259: TypeError
260: TypeError
261: TypeError
262: TypeError
263: TypeError
264: TypeError
265: TypeError
266: TypeError
267: TypeError
268: TypeError
269: TypeError
270: TypeError
===*/

/* Test the current implementation limit for ID lengths.  This also
 * does some boundary value testing for ID length.
 */

function lengthTest() {
    var i;
    var mod;

    function buildFooBarId(n) {
        var tmp = '';
        while (tmp.length < n - 6) {
            tmp += '/';
        }
        return 'foo' + tmp + 'bar';
    }

    function buildNumberedId(n, num) {
        var tmp = 'foo/num-' + num + '-';
        while (tmp.length < n) {
            tmp += 'x';
        }
        return tmp;  // foo/num-123-xxxxxx... to 'n' chars
    }

    /*
     *  Test the limit for the current global require() id
     */

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        return 'exports.name="' + id + '"';
    }

    for (i = 230; i <= 270; i++) {
        try {
            mod = require(buildFooBarId(i));
            print(i + ':', mod.name);
        } catch (e) {
            print(i + ':', e.name);
        }
    }

    /*
     *  Test the limit for a relative require() id from inside a
     *  module; length restriction is applied against an intermediate
     *  identifier constructed by joining the module's require.id
     *  with the requested relative ID with a slash.
     */

    Duktape.modSearch = function (id) {
        // Disable to avoid spam; each id is dynamic and long
        //print('Duktape.modSearch', id);
        if (id == 'bar') {
            return 'exports.name = "' + id + '";';
        } else {
            /* submodule will request '../bar' relative to its own path */
            return 'var mod = require(".././././././bar");\n' +
                   'exports.name = "' + id + '";\n' +
                   'exports.mod_name = mod.name;\n';
        }
    }

     for (i = 230; i <= 270; i++) {
        try {
            mod = require(buildNumberedId(i, i));
            print(i + ':', mod.mod_name);
        } catch (e) {
            print(i + ':', e.name);
        }
    }
}

print('length');

try {
    lengthTest();
} catch (e) {
    print(e);
}
