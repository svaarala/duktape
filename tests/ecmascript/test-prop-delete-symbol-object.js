/*===
true
true
true
true
true
true
true
true
===*/

try {
    var S = Object(Symbol.for('foo'));
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
