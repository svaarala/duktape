/*
 *  CallExpression tests
 */

/*===
basic
1 2 3
1 2 3 4
===*/

function f(x) {
    return function (y) {
        return function (z) {
            print(x, y, z);
        }
    }
}

function g(x) {
    return {
        foo: function (y) {
            return function (z) {
                return {
                    bar: function (w) {
                        print(x, y, z, w);
                    }
                }
            }
        }
    }
}

function basicTest() {
    f(1)(2)(3);
    g(1).foo(2)(3).bar(4);
}

try {
    print('basic');
    basicTest();
} catch (e) {
    print(e.stack || e);
}
