/*
 *  Tests for CommonJS module environment bindings and scope behavior.
 *
 *  A module environment is isolated from the global environment.  Variable
 *  and function declarations do not pollute the global object (although
 *  plain assignments do).  In the current implementation a module is wrapped
 *  inside a temporary function which provides the scope for the module.
 */

/*===
basic bindings
modSearch: foo/bar
modSearch: foo/quux
require: function false
module: object foo/quux false false false
exports: object true
===*/

/* Check 'require', 'exports', and 'module'. */

var global_require = require;

Duktape.modSearch = function (id) {
    print('modSearch:', id);
    if (id === 'foo/bar') {
        return 'var mod = require("./quux");\n'
    }
    if (id === 'foo/quux') {
        return 'var pd;\n' +
               'print("require:", typeof require, require === global_require);\n' +
               'pd = Object.getOwnPropertyDescriptor(module, "id");\n' +
               'print("module:", typeof module, module.id, pd.writable, pd.enumerable, pd.configurable);\n' +
               'print("exports:", typeof exports, exports === this);\n'
               ;
    }
    throw new Error('cannot find module');
};

print('basic bindings');

function bindingTest() {
    var mod = require('foo/bar');

    // module.id must be a resolved absolute path so that it can be used
    // to require the correct module from any other module
}

try {
    bindingTest();
} catch (e) {
    print(e);
}

/*===
scoping
before global require
module: global-module
exports: global-exports
myVar1: global-myVar1
myVar2: global-myVar2
myVar3: global-myVar3
foo: global-foo
mod: object [object Object]
mod.name: test1
mod.foo: bar
after global require
module: global-module
exports: global-exports
myVar1: global-myVar1
myVar2: global-myVar2
myVar3: module-myVar3
foo: global-foo
before module require
module: global-module
exports: global-exports
myVar1: global-myVar1
myVar2: global-myVar2
myVar3: global-myVar3
foo: global-foo
mod: object [object Object]
mod.name: test2
mod.foo: bar
after module require
module: global-module
exports: global-exports
myVar1: global-myVar1
myVar2: global-myVar2
myVar3: module-myVar3
foo: global-foo
===*/

/* Assignment / scope tests. */

// These shouldn't get overwritten.
var module;
var exports;
var myVar1;
var myVar2;
var foo;

// This is ovwritten because module doesn't declare a variable
var myVar3;

// Temp
var mod;

function initVars() {
    module = 'global-module';
    exports = 'global-exports';
    myVar1 = 'global-myVar1';
    myVar2 = 'global-myVar2';
    myVar3 = 'global-myVar3';
    foo = 'global-foo';
}

function dumpVars() {
    print('module:', module);
    print('exports:', exports);
    print('myVar1:', myVar1);
    print('myVar2:', myVar2);
    print('myVar3:', myVar3);
    print('foo:', foo);
}

Duktape.modSearch = function (id) {
    var ret;

    if (id === 'test1' || id === 'test2') {
        ret = 'var myVar1;\n' +                      // declared inside the module
              'exports.name = "' + id + '";\n' +     // goes into exports
              'exports = "module-exports";\n' +      // overwrites module exports, not global, does not affect exports.name getting exported
              'this.foo = "bar";\n' +                // 'this' is bound to the original exports table, so 'foo' gets exported
              'require = "module-require";\n' +      // overwrite module require, not global
              'myVar1 = "module-myVar1";\n' +        // writes to local variable, not global
              'function myVar2() {}\n' +             // declared inside module
              'myVar3 = "module-myVar3";\n'          // not declared locally, *set to global variable*
              ;
        //print(ret);
        return ret;
    }
    throw new Error('cannot find module: ' + id);
};

function moduleEnvironmentTest() {
    var mod;
    mod = require('test2');
    print('mod:', typeof mod, mod);
    print('mod.name:', mod.name);
    print('mod.foo:', mod.foo);
    print('after module require');
    dumpVars();
}

print('scoping');

/* Using a require() from a global program. */
print('before global require');
initVars();
dumpVars();
try {
    mod = require('test1');
} catch (e) {
    print('global require failed:', e);
    print(e.stack || e);
}
print('mod:', typeof mod, mod);
print('mod.name:', mod.name);
print('mod.foo:', mod.foo);
print('after global require');
dumpVars();

initVars();
print('before module require');
dumpVars();

try {
    moduleEnvironmentTest();
} catch (e) {
    print(e);
}
