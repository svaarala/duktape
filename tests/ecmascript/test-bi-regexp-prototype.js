/*
 *  RegExp.prototype
 *
 *  Changed in ES6 in many ways, check for ES6 behavior.
 */

/*===
[object Function]
[object Object]
/(?:)/
(?:)

undefined
undefined
undefined
undefined
foo
gm
true
true
false
undefined
undefined
false true
false true
false true
false true
false true
false false
false false
function undefined
function undefined
function undefined
function undefined
function undefined
undefined
undefined
gm
===*/

function test() {
    print(Object.prototype.toString.call(RegExp));

    // In ES6 RegExp.prototype is no longer a regexp instance.
    print(Object.prototype.toString.call(RegExp.prototype));

    // In ES2015 and ES2016 the .source, .flags etc accessors will TypeError
    // if called with RegExp.prototype or any other non-RegExp-instance.
    // Draft ES2017 changes that so that RegExp.prototype gets special
    // treatment, to fix real world issues with the ES2015/ES2016 behavior.
    // Test for draft ES8 behavior (also implemented by V8 and Firefox).
    try {
        print(RegExp.prototype.toString());
    } catch (e) {
        print(e.name);
    }
    try {
        print(RegExp.prototype.source);
    } catch (e) {
        print(e.name);
    }
    try {
        print(RegExp.prototype.flags);
    } catch (e) {
        print(e.name);
    }
    try {
        print(RegExp.prototype.multiline);
    } catch (e) {
        print(e.name);
    }
    try {
        print(RegExp.prototype.ignoreCase);
    } catch (e) {
        print(e.name);
    }
    try {
        print(RegExp.prototype.sticky);
    } catch (e) {
        print(e.name);
    }
    try {
        print(RegExp.prototype.unicode);
    } catch (e) {
        print(e.name);
    }

    var x = /foo/gm;

    print(x.source);
    print(x.flags);
    print(x.global);
    print(x.multiline);
    print(x.ignoreCase);
    print(x.sticky);
    print(x.unicode);

    print(Object.hasOwnProperty(x, 'source'), 'source' in x);
    print(Object.hasOwnProperty(x, 'flags'), 'flags' in x);
    print(Object.hasOwnProperty(x, 'global'), 'global' in x);
    print(Object.hasOwnProperty(x, 'multiline'), 'multiline' in x);
    print(Object.hasOwnProperty(x, 'ignoreCase'), 'ignoreCase' in x);
    print(Object.hasOwnProperty(x, 'sticky'), 'sticky' in x);
    print(Object.hasOwnProperty(x, 'unicode'), 'unicode' in x);

    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'source');
    print(typeof pd.get, typeof pd.set);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'flags');
    print(typeof pd.get, typeof pd.set);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'global');
    print(typeof pd.get, typeof pd.set);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'multiline');
    print(typeof pd.get, typeof pd.set);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'ignoreCase');
    print(typeof pd.get, typeof pd.set);

    // .sticky and .unicode not implemented yet to avoid interfering with
    // feature detection code; document absence
    /*
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'sticky');
    print(typeof pd.get, typeof pd.set);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'unicode');
    print(typeof pd.get, typeof pd.set);
    */
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'sticky');
    print(typeof pd);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'unicode');
    print(typeof pd);

    // If a new RegExp is created from an existing one, ES6 reads .flags
    // and uses it as an argument.
    y = new RegExp(x);
    print(y.flags);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
