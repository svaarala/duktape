/*
 *  When deleteCount is not given, the ES5.1 specification treats it like
 *  deleteCount is zero.  However, at least V8 and Rhino treat this case
 *  such that the splice continues to the end of the array.
 *
 *  This specification "bug" was fixed in ES2015.
 */

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

    // ES2015+: same behavior if second argument omitted.
    // ES5.1 behavior seems to mandate same behavior as
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

test();
