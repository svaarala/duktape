/*
 *  Deep merge test: just check that key ordering is preserved etc.
 */

/*===
{
    "foo": 123,
    "bar": {
        "quux": 234,
        "baz": {
            "prop5": 123,
            "foo": 333,
            "bar": 222
        }
    },
    "prop1": 987
}
{
    "foo": 234,
    "prop2": 876,
    "bar": {
        "prop3": 765,
        "baz": {
            "prop6": 555,
            "foo": 444
        }
    }
}
{
    "foo": 234,
    "bar": {
        "quux": 234,
        "baz": {
            "prop5": 123,
            "foo": 444,
            "bar": 222,
            "prop6": 555
        },
        "prop3": 765
    },
    "prop1": 987,
    "prop2": 876
}
===*/

function deepMerge(a,b) {
    // overriding value is not an object -> use as is
    if (typeof b !== 'object') { return b; }

    // overriding value is an object, but previous value is not -> use as is
    if (typeof a !== 'object') { return b; }

    // both are objects, merge keys recursively
    var c = {};
    var keyset = {};  // keys present in one or both
    Object.keys(a).forEach(function (k) { keyset[k] = true; });
    Object.keys(b).forEach(function (k) { keyset[k] = true; });

    // merge keys, recursively
    Object.keys(keyset).forEach(function (k) {
        if (k in a) {
            if (k in b) {
                c[k] = deepMerge(a[k], b[k]);  // k in a and b, merge
            } else {
                c[k] = a[k];  // k in a only
            }
        } else {
            c[k] = b[k];  // k in b only
        }
    });

    return c;
}

function deepMergeTest() {
    var A = {
        foo: 123,
        bar: {
            quux: 234,
            baz: {
                prop5: 123,
                foo: 333,
                bar: 222
            }
        },
        prop1: 987,
    };
    var B = {
        foo: 234,
        prop2: 876,
        bar: {
            prop3: 765,
            baz: {
                prop6: 555,
                foo: 444
            }
        }
    };

    print(JSON.stringify(A, null, 4));
    print(JSON.stringify(B, null, 4));
    print(JSON.stringify(deepMerge(A, B), null, 4));
}

try {
    deepMergeTest();
} catch (e) {
    print(e);
}
