/*===
basic
string 3 foo
string 3 foo
string 12 fooundefined
string 16 fooundefinednull
string 20 fooundefinednulltrue
string 25 fooundefinednulltruefalse
string 28 fooundefinednulltruefalse123
string 31 fooundefinednulltruefalse123bar
string 34 fooundefinednulltruefalse123bar1,2
string 49 fooundefinednulltruefalse123bar1,2[object Object]
string 3 foo
===*/

print('basic');

function concatTest() {
    var str = String('foo');

    function pv(x) {
        print(typeof x, x.length, x);
    }

    pv(str);
    pv(str.concat());
    pv(str.concat(undefined));
    pv(str.concat(undefined, null));
    pv(str.concat(undefined, null, true));
    pv(str.concat(undefined, null, true, false));
    pv(str.concat(undefined, null, true, false, 123));
    pv(str.concat(undefined, null, true, false, 123, 'bar'));
    pv(str.concat(undefined, null, true, false, 123, 'bar', [1,2]));
    pv(str.concat(undefined, null, true, false, 123, 'bar', [1,2], { foo: 1, bar: 2 }));
    pv(str);
}

try {
    concatTest();
} catch (e) {
    print(e);
}

/*===
unicode
102
111
111
4660
2046
128
102
111
111
65535
===*/

print('unicode');

function unicodeTest() {
    var str = new String('foo\u1234');
    var t;
    var i;

    t = str.concat('\u07fe', '\u0080foo', '\uffff');
    for (i = 0; i < t.length; i++) {
        print(t.charCodeAt(i));
    }
}

try {
    unicodeTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
string 7 truebar
string 8 falsebar
string 6 123bar
string 6 foobar
string 6 1,2bar
string 18 [object Object]bar
toString() obj1
toString() obj2
toString() obj3
string 15 firstfoobarquux
===*/

print('coercion');

function coercionTest() {
    var t;

    function test(x) {
        var t;
        try {
            t = String.prototype.concat.call(x, 'bar');
            print(typeof t, t.length, t);
        } catch (e) {
            print(e.name);
        }
    }
    function mkObj(name, strval) {
        return {
            toString: function() { print('toString()', name); return strval; },
            valueOf: function() { print('valueOf()', name); return 'unexpected'; }
        };
    }

    // expect TypeError for these
    test(undefined);
    test(null);

    // and a ToString() coercion for these
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });

    // coercion side effect test
    t = String.prototype.concat.call('first', mkObj('obj1', 'foo'), mkObj('obj2', 'bar'), mkObj('obj3', 'quux'));
    print(typeof t, t.length, t);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
