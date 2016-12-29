/*===
basic tests
true
true
true
true
true
true
false
true
false
true
true
false
true
false
false
false
false
false
false
false
true
===*/

function basicTest() {
    var arr;

    // includes() uses SameValue algorithm, so it sees zero sign as well
    // as being able to find NaNs, unlike indexOf().
    arr = [ 0, 1, 2, 3, 4, 5, 0/0, "foo" ];
    print(arr.includes(0));
    print(arr.includes(1));
    print(arr.includes(2));
    print(arr.includes(3));
    print(arr.includes(4));
    print(arr.includes(5));
    print(arr.includes(6));
    print(arr.includes("foo"));
    print(arr.includes(-0));   // SameValue(0, -0) is false
    print(arr.includes(0/0));  // SameValue(NaN, NaN) is true

    // Gaps in sparse arrays are searched; nonpresent = undefined.
    arr = [];
    arr[0] = "foo";
    arr[2] = "baz";
    print(arr.includes(void 0));  // true
    arr[1] = "bar";
    print(arr.includes(void 0));  // now false

    // Like indexOf(), fromIndex is effectively clamped to (-len,len).  If
    // fromIndex === len, nothing is searched.
    arr = [ "pig", "cow", "ape" ];
    print(arr.includes("pig", 0));
    print(arr.includes("pig", 1));
    print(arr.includes("pig", 2));
    print(arr.includes("pig", 3));
    print(arr.includes("cow", 3));
    print(arr.includes("ape", 3));
    print(arr.includes("pig", -1));
    print(arr.includes("cow", -1));
    print(arr.includes("ape", -1));
}

try {
    print('basic tests');
    basicTest();
} catch (e) {
    print(e);
}
