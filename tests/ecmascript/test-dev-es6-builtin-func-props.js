/*
 *  Some ES2015 built-in .length and .name spot checks
 */

/*===
function
.name string cos false false true
.length number 1 false false true
function
.name string stringify false false true
.length number 3 false false true
function
.name string hasOwnProperty false false true
.length number 1 false false true
function
.name none
.length none
===*/

function test() {
    var pd;

    function dump(f) {
        print(typeof f);
        var pd = Object.getOwnPropertyDescriptor(f, 'name');
        if (pd) {
            print('.name', typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);
        } else {
            print('.name', 'none');
        }
        var pd = Object.getOwnPropertyDescriptor(f, 'length');
        if (pd) {
            print('.length', typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);
        } else {
            print('.length', 'none');
        }
    }

    // In ES2015 function .name and .length are non-writable, non-enumerable,
    // but configurable.  This also applies to built-ins.  Spot check a few.

    dump(Math.cos);
    dump(JSON.stringify);
    dump(Object.prototype.hasOwnProperty);

    // Currently missing .name and .length: https://github.com/svaarala/duktape/issues/1187
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'source');  // getter in ES2015
    dump(pd.get);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
