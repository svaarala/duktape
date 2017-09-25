/*===
function
propdesc isSafeInteger: value=function:isSafeInteger, writable=true, enumerable=false, configurable=true
propdesc length: value=1, writable=false, enumerable=false, configurable=true
propdesc name: value="isSafeInteger", writable=false, enumerable=false, configurable=true
-Infinity false
-1e+100 false
-1234567890.2 false
-305419896 true
-0 true
0 true
305419896 true
1234567890.2 false
1e+100 false
Infinity false
NaN false
-9007199254740992 false
-9007199254740991 true
-9007199254740990 true
9007199254740992 false
9007199254740991 true
9007199254740990 true
undefined false
null false
"" false
"123" false
true false
false false
[object Object] false
===*/

/*@include util-base.js@*/

print(typeof Number.isSafeInteger);
print(Test.getPropDescString(Number, 'isSafeInteger'));
print(Test.getPropDescString(Number.isSafeInteger, 'length'));
print(Test.getPropDescString(Number.isSafeInteger, 'name'));

[
    -1 / 0, -1e100, -1234567890.2, -0x12345678, -0,
    +0, 0x12345678, 1234567890.2, 1e100, 1 / 0,
    0 / 0,

    // Specific corner case values: note that fractions are not actually
    // significant so it suffices to test for integers.
    -9007199254740992.0,
    -9007199254740991.0,
    -9007199254740990.0,
    9007199254740992.0,
    9007199254740991.0,
    9007199254740990.0,

    void 0, null, '', '123', true, false,
    { valueOf: function () { return 123.0; } }
].forEach(function (v) {
    print(Test.valueToString(v), Test.valueToString(Number.isSafeInteger(v)));
});
