/*===
string string 0 0   true
string string 9 9 undefined undefined true
string string 4 4 null null true
string string 4 4 true true true
string string 5 5 false false true
string string 3 3 123 123 true
string string 3 3 foo foo true
string string 3 3 1,2 1,2 true
string string 15 15 [object Object] [object Object] true
string string -1 -1 noval noval true
TypeError
TypeError
===*/

function toStringValueOfTest() {
    function t(x, noval) {
        var t1 = x.toString();
        var t2 = x.valueOf();
        print(typeof t1, typeof t2,
              (noval ? -1 : t1.length), (noval ? -1 : t2.length),
              (noval ? 'noval' : t1), (noval ? 'noval' : t2),
              t1 === t2)
    }

    t(new String());
    t(new String(undefined));
    t(new String(null));
    t(new String(true));
    t(new String(false));
    t(new String(123.0));
    t(new String('foo'));
    t(new String([1,2]));
    t(new String({ foo: 1, bar: 2 }));

    // avoid printing the exact value, as it is implementation dependent
    t(new String(function (){}), true);
}

try {
    toStringValueOfTest();
} catch (e) {
    print(e);
}

try {
    // not generic, require TypeError
    String.prototype.toString.call({});
} catch (e) {
    print(e.name);
}

try {
    // not generic, require TypeError
    String.prototype.valueOf.call({});
} catch (e) {
    print(e.name);
}
