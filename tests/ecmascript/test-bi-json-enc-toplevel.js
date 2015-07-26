/*===
undefined undefined
string null
string true
string false
string 123
string null
string null
string null
string "text"
string {"foo":"bar"}
string ["foo","bar"]
===*/

/* JSON top level value can be any type, not just an object or an array. */

function testStringify(x) {
    var t = JSON.stringify(x);
    print(typeof t, t);
}

try {
    testStringify(undefined);  // this returns 'undefined', not a string
    testStringify(null);
    testStringify(true);
    testStringify(false);
    testStringify(123.0);
    testStringify(Number.NaN);
    testStringify(Number.POSITIVE_INFINITY);
    testStringify(Number.NEGATIVE_INFINITY);
    testStringify('text');
    testStringify({foo:'bar'});
    testStringify(['foo','bar']);
} catch (e) {
    print(e.name);
}

/*===
object
boolean
boolean
number
string
object
object
===*/

/* Test parsing of arbitrary top-level value. */

function testParse(x) {
    var t = JSON.parse(x);
    print(typeof t);
}

try {
    testParse('null');  // note: typeof null -> 'object'
    testParse('true');
    testParse('false');
    testParse('123.0');
    testParse('"text"');
    testParse('{"foo":"bar"}');
    testParse('["foo","bar"]');
} catch (e) {
    print(e.name);
}
