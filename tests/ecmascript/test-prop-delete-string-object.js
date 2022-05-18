/*===
false
false
false
false
true
true
false
false
===*/

try {
    var S = new String('foo\ucafe');
    print(delete S.length);
    print(delete S[0]);
    print(delete S[-0]);
    print(delete S['0']);
    print(delete S['-0']);
    print(delete S['+0']);
    print(delete S[2]);
    print(delete S[3]);
} catch (e) {
    print(e.stack || e);
}
