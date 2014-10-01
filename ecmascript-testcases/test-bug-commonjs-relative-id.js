/*
 *  Relative resolution bug in Duktape 0.12.0
 *
 *  https://github.com/svaarala/duktape/issues/48
 */

/*---
{
    "custom": true
}
---*/

/*===
Duktape.modSearch foo
Duktape.modSearch quux
Duktape.modSearch foo/bar
Duktape.modSearch foo/quux
===*/

function test() {
    /*
     *  Relative resolution from 'foo'
     */

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        if (id === 'foo') {
            return "var text = 'Hello world!';     // not visible outside the module\n" +
                   "var quux = require('./quux');  // loads quux\n" +
                   "exports.hello = function () { print(text); };";
        }
        return '';   // return a fake empty module
    };
    void require('foo');

    /*
     *  Relative resolution from 'foo/bar'
     */

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        if (id === 'foo/bar') {
            return "var text = 'Hello world!';     // not visible outside the module\n" +
                   "var quux = require('./quux');  // loads foo/quux\n" +
                   "exports.hello = function () { print(text); };";
        }
        return '';   // return a fake empty module
    };
    void require('foo/bar');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
