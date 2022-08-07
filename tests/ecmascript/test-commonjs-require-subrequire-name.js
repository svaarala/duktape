/*
 *  The fresh require() functions given to modules should have a .name so that
 *  they appear nicely in tracebacks.  This was not the case in Duktape 1.4.x,
 *  fixed in Duktape 1.5.x.
 */

/*---
custom: true
duktape_extras:
  module-duktape: true
---*/

/*===
function
string
require
false false false
===*/

function test() {
    Duktape.modSearch = function modSearch(id, require, exports, module) {
        var pd;

        print(typeof require);
        print(typeof require.name);
        print(require.name);

        pd = Object.getOwnPropertyDescriptor(require, 'name');
        print(pd.writable, pd.enumerable, pd.configurable);

        //print(new Error('aiee').stack);
        //require('../../../../foo')

        return undefined;
    };

    require('foo/bar');
}

test();
