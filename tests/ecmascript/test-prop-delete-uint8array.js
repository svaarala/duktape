/*===
false
false
false
true
true
false
false
true
true
true
===*/

var u8 = new Uint8Array(5);
print(delete u8[0]);
print(delete u8[-0]);
print(delete u8['0']);
print(delete u8['-0']);
print(delete u8['+0']);
print(delete u8[4]);
print(delete u8['4']);
print(delete u8['4.0']);
print(delete u8['4.1']);
print(delete u8[5]);
