/*---
{
    "custom": true
}
---*/

/*===
0
1
100
3
[1,2,null,4,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,"foo"]
===*/

function sparseArrayLengthTest() {
    var arr, k;

    // sparse array test
    arr = [1, 2];
    arr[100] = 'foo';  // becomes sparse
    arr[3] = 4;        // will enumerate "incorrectly"

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
