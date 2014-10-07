/*
 *  Bug testcase for a stringtable memory leak in Duktape 0.7.0, reported
 *  by Greg Burns.
 *
 *  In Duktape 0.7.0 the mark-and-sweep handling of strings, specifically
 *  the sweeping of strings, would mark a string reachable and never clear
 *  the reachable flag.  As a result, strings were never garbage collected
 *  if they had once been marked reachable.  This doesn't easily show any
 *  symptoms when reference counting is in use (circular references are
 *  needed).  When only mark-and-sweep is used, the problem is easy to
 *  reproduce.
 *
 *  The test1() variant below works when reference counting is enabled,
 *  even when the bug is present.  test2() uses circular references to
 *  cause the bug to manifest itself even with reference counting.
 *
 *  This testcase can also be run manually with valgrind massif to see that
 *  the memory behavior is correct.
 */

/*---
{
    "custom": true,
    "slow": true
}
---*/

/*===
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
===*/

var count = 0;
var kilo = Array.prototype.join.call({ length: 1024 + 1 }, 'x');
var meg = Array.prototype.join.call({ length: 1024 + 1 }, kilo);  // 1 meg prefix

function createUniqueString() {
    var res = meg + (count++);
    return res;
}

function test1() {
    // Goes through 16G of strings
    for (var i = 0; i < 16; i++) {
        print(i);
        for (var j = 0; j < 1024; j++) {
            var ignore = createUniqueString();
            Duktape.gc();
        }
    }
}

function test2() {
    for (var i = 0; i < 16; i++) {
        print(i);
        for (var j = 0; j < 1024; j++) {
            var obj1 = {};
            var obj2 = { str: createUniqueString(), ref: obj1 };
            obj1.ref = obj2;   // circular reference, object contains string to collect
            Duktape.gc();      // force collection once: string gets marked reachable
            obj1 = undefined;
            obj2 = undefined;  // unreachable but not collected when refcounts enabled
            Duktape.gc();      // force collection: should be collected but isn't
        }
    }
}

try {
    test1();
} catch (e) {
    print(e);
}

try {
    test2();
} catch (e) {
    print(e);
}
