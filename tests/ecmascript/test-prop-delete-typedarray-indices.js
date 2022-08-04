/*===
- Uint8Array
false
false
false
true
false
true
undefined aiee
true
false
true
true
true
true
true
true
- Uint32Array
false
false
false
true
false
true
undefined aiee
true
false
true
true
true
true
true
true
===*/

print('- Uint8Array');
var u8 = new Uint8Array(10);
var u8proto = Object.getPrototypeOf(u8);
Object.defineProperty(u8proto, '-0', { value: 'aiee', configurable: true });
print(delete u8[0]);
print(delete u8[-0]);
print(delete u8[9]);
print(delete u8[10]);
print(delete u8['0']);
print(delete u8['-0']);
print(u8['-0'], u8proto['-0']);
print(delete u8['0.0']);
print(delete u8['9']);
print(delete u8['9.0']);
print(delete u8['10']);
print(delete u8['10.0']);
print(delete u8['NaN']);
print(delete u8['Infinity']);
print(delete u8['-Infinity']);

print('- Uint32Array');
var u32 = new Uint32Array(10);
var u32proto = Object.getPrototypeOf(u32);
Object.defineProperty(u32proto, '-0', { value: 'aiee', configurable: true });
print(delete u32[0]);
print(delete u32[-0]);
print(delete u32[9]);
print(delete u32[10]);
print(delete u32['0']);
print(delete u32['-0']);
print(u32['-0'], u32proto['-0']);
print(delete u32['0.0']);
print(delete u32['9']);
print(delete u32['9.0']);
print(delete u32['10']);
print(delete u32['10.0']);
print(delete u32['NaN']);
print(delete u32['Infinity']);
print(delete u32['-Infinity']);
