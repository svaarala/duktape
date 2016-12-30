/*
 *  RegExp.prototype
 *
 *  Changed in ES2015 in many ways, check for ES2015 behavior with some draft ES2017
 *  behavior for accepting RegExp.prototype as a 'this' binding.
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
function undefined false true
function undefined false true
function undefined false true
function undefined false true
function undefined false true
undefined
undefined
gm
TypeError
gm
gi

g
i
gi
m
gm
im
gim
===*/

function test() {
    print(Object.prototype.toString.call(RegExp));

    // In ES2015 RegExp.prototype is no longer a regexp instance.
    print(Object.prototype.toString.call(RegExp.prototype));

    // In ES2015 and ES2016 the .source, .flags etc accessors will TypeError
    // if called with RegExp.prototype or any other non-RegExp-instance.
    // Draft ES2017 changes that so that RegExp.prototype gets special
    // treatment, to fix real world issues with the ES2015/ES2016 behavior.
    // Test for draft ES2017 behavior (also implemented by V8 and Firefox).
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
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'flags');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'global');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'multiline');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'ignoreCase');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);

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

    // If a new RegExp is created from an existing one, ES2015 reads .flags
    // and uses it as an argument.
    y = new RegExp(x);
    print(y.flags);

    // The .flags getter is generic.
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'flags');
    try {
        pd.get.call('foo');  // argument must still be an Object
    } catch (e) {
        print(e.name);
    }
    print(pd.get.call(/foo/gm));
    print(pd.get.call({ global: 'yes', ignoreCase: 1, multiline: 0, something: 'else' }));

    // .flags coverage for GIM flags.
    print(/foo/.flags);
    print(/foo/g.flags);
    print(/foo/i.flags);
    print(/foo/gi.flags);
    print(/foo/m.flags);
    print(/foo/gm.flags);
    print(/foo/im.flags);
    print(/foo/gim.flags);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
