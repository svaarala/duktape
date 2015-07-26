/*
 *  When deleteCount is not given, the specification treats it like
 *  deleteCount is zero.  However, at least V8 and Rhino treat this case
 *  such that the splice continues to the end of the array.
 *
 *  Mozilla documentation agrees with the V8/Rhino behavior:
 *
 *    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/splice
 *
 *    If no howMany parameter is specified (second syntax above, which is a
 *    SpiderMonkey extension), all elements after index are removed.
 */

/*---
{
    "nonstandard": true,
    "comment": "will fail with DUK_OPT_NO_NONSTD_ARRAY_SPLICE_DELCOUNT"
}
---*/

/*===
["foo","bar","quux"] ["baz","quuux"]
["foo","bar","quux"] ["baz","quuux"]
["foo","bar","quux","baz","quuux"] []
===*/

function test() {
    var arr;
    var res;

    // Standard: splice 'baz' and 'quuux'.
    arr = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    res = arr.splice(3, 2);
    print(JSON.stringify(arr), JSON.stringify(res));

    // Non-standard: same behavior if second argument omitted.
    // Standard behavior seems to mandate same behavior as
    // arr.splice(3, 0);
    arr = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    res = arr.splice(3);
    print(JSON.stringify(arr), JSON.stringify(res));

    // When given as 'undefined', Rhino and V8 both behave like
    // the standard mandates, e.g. same as arr.splice(3, 0);
    arr = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    res = arr.splice(3, undefined);
    print(JSON.stringify(arr), JSON.stringify(res));
}

try {
    test();
} catch (e) {
    print(e);
}
