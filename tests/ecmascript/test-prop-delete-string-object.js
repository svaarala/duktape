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

var S = new String('foo\ucafe');
print(delete S.length);
print(delete S[0]);
print(delete S[-0]);
print(delete S['0']);
print(delete S['-0']);
print(delete S['+0']);
print(delete S[2]);
print(delete S[3]);
