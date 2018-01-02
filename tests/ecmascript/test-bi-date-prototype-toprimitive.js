/*
 *  Date.prototype[@@toPrimitive] provides number and string primitive value
 *  coercions for Date instances.  The method is actually generic and works on
 *  any object argument, providing access to OrdinaryToPrimitive() with a
 *  number/string hint.
 */

/*===
symbol
true
function
1
false false true
===*/

function basicTest() {
    var pd;

    print(typeof Symbol.toPrimitive);
    print(Symbol.toPrimitive in Date.prototype);
    print(typeof Date.prototype[Symbol.toPrimitive]);
    print(Date.prototype[Symbol.toPrimitive].length);
    // print(Date.prototype[Symbol.toPrimitive].name);  // XXX: disabled for now, broken
    pd = Object.getOwnPropertyDescriptor(Date.prototype, Symbol.toPrimitive);
    print(pd.writable, pd.enumerable, pd.configurable);
}

try {
    basicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
0 0 TypeError
0 1 TypeError
0 2 TypeError
0 3 TypeError
0 4 TypeError
0 5 TypeError
0 6 TypeError
0 7 TypeError
0 8 TypeError
0 9 TypeError
0 10 TypeError
1 0 TypeError
1 1 TypeError
1 2 TypeError
1 3 TypeError
1 4 TypeError
1 5 TypeError
1 6 TypeError
1 7 TypeError
1 8 TypeError
1 9 TypeError
1 10 TypeError
2 0 TypeError
2 1 TypeError
2 2 TypeError
2 3 TypeError
2 4 TypeError
2 5 TypeError
2 6 TypeError
2 7 TypeError
2 8 TypeError
2 9 TypeError
2 10 TypeError
3 0 TypeError
3 1 TypeError
3 2 TypeError
3 3 TypeError
3 4 TypeError
3 5 TypeError
3 6 TypeError
3 7 TypeError
3 8 TypeError
3 9 TypeError
3 10 TypeError
4 0 TypeError
4 1 TypeError
4 2 TypeError
4 3 TypeError
4 4 DATESTRING
4 5 DATESTRING
4 6 12345
4 7 TypeError
4 8 TypeError
4 9 TypeError
4 10 TypeError
5 0 TypeError
5 1 TypeError
5 2 TypeError
5 3 TypeError
5 4 [object Object]
5 5 [object Object]
5 6 [object Object]
5 7 TypeError
5 8 TypeError
5 9 TypeError
5 10 TypeError
6 0 TypeError
6 1 TypeError
6 2 TypeError
6 3 TypeError
6 4 
6 5 
6 6 
6 7 TypeError
6 8 TypeError
6 9 TypeError
6 10 TypeError
7 0 TypeError
7 1 TypeError
7 2 TypeError
7 3 TypeError
valueOf called
7 4 bar
valueOf called
7 5 bar
valueOf called
7 6 123
7 7 TypeError
7 8 TypeError
7 9 TypeError
7 10 TypeError
===*/

function combinationTest() {
    // Date instance needs censoring because of timezone.
    function censorDate(x) {
        if (typeof x !== 'string') { return x; }
        var m = /^\d\d\d\d-\d\d-\d\d\s.*$/.exec(x);
        if (m) {
            return 'DATESTRING';
        } else {
            return x;
        }
    }

    [ void 0, null, 123, 'foo',
      new Date(12345),
      {},
      [],
      {
          valueOf: function () { print('valueOf called'); return 123; },
          toString: function () { print('valueOf called'); return 'bar'; }
      }
    ].forEach(function (thisArg, idx1) {
        [ '__NONE__', void 0, null, 123, 'default', 'string', 'number',
          'something', 'default\u0000foo', 'string\u0000foo',
          'number\u0000foo' ].forEach(function (firstArg, idx2) {
            try {
                var res;
                if (thisArg === '__NONE__') {
                    res = Date.prototype[Symbol.toPrimitive].call(thisArg);
                } else {
                    res = Date.prototype[Symbol.toPrimitive].call(thisArg, firstArg);
                }
                print(idx1, idx2, censorDate(res));
            } catch (e) {
                print(idx1, idx2, e.name);
            }
        });
    });
}

try {
    combinationTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Date.prototype[@@toPrimitive] called
NaN
Date.prototype[@@toPrimitive] called
aiee
===*/

function replaceTest() {
    // Because Date.prototype[@@toPrimitive] is configurable, it can be
    // replaced.

    Object.defineProperty(Date.prototype, Symbol.toPrimitive, {
        value: function () {
            print('Date.prototype[@@toPrimitive] called');
            return 'aiee';
        }
    });
    print(+(new Date()));
    print("" + (new Date()));
}

try {
    replaceTest();
} catch (e) {
    print(e.stack || e);
}
