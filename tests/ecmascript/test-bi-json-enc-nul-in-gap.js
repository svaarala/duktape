/*
 *  Ensure gap/indent handling deals with NUL codepoints correctly.
 */

/*===
{
--<NUL>-->"foo": 1,
--<NUL>-->"bar": 2,
--<NUL>-->"quux": 3,
--<NUL>-->"baz": [
--<NUL>-->--<NUL>-->1,
--<NUL>-->--<NUL>-->2,
--<NUL>-->--<NUL>-->3
--<NUL>-->]
}
===*/

function nulInGapTest() {
    var obj = { foo: 1, bar: 2, quux: 3, baz: [ 1, 2, 3 ] };
    var res;

    // NUL in the middle of gap should not be an issue.
    res = JSON.stringify(obj, null, '--\u0000-->');

    print(res.replace(/\u0000/g, '<NUL>'));
}

try {
    nulInGapTest();
} catch (e) {
    print(e.stack || e);
}
