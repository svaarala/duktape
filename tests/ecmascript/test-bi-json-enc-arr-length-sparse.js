/*===
0
1
3
100
[1,2,null,4,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,"foo"]
===*/

/*---
{
    "custom": true
}
---*/

function sparseArrayLengthTest() {
    var arr, k;

    // sparse array test
    arr = [1, 2];
    arr[100] = 'foo';  // becomes sparse
    arr[3] = 4;        // in Duktape 1.x would enumerate "incorrectly",
                       // Duktape 2.x adheres to ES2015 [[OwnPropertyKeys]]
                       // enum order (even when ES2015 doesn't require it
                       // for the for-in statement)

    // custom behavior here
    for (k in arr) {
        print(k);
    }

    print(JSON.stringify(arr));
}

try {
    sparseArrayLengthTest();
} catch (e) {
    print(e.name);
}
