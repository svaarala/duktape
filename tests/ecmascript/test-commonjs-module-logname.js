/*
 *  Test that a logger created inside a module gets a reasonable default
 *  logger name.
 */

/*===
Duktape.modSearch foo
TIMESTAMP INF foo: hello from module
Duktape.modSearch foo/BAR/quux
TIMESTAMP INF foo/BAR/quux: hello from module
===*/

function moduleLogNameTest() {
    var mod;

    Duktape.modSearch = function (id) {
        print('Duktape.modSearch', id);
        return 'var logger = new Duktape.Logger(); logger.info("hello from module");';
    };
    Duktape.Logger.prototype.raw = function (buf) {
        var str = String(buf);
        str = str.replace(/^\S+/, 'TIMESTAMP');
        print(str);
    };

    mod = require('foo');
    mod = require('foo/./bar/../BAR/quux');  // foo/BAR/quux absolute
}

try {
    moduleLogNameTest();
} catch (e) {
    print(e);
}
