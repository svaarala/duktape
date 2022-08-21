/*===
function
propdesc isInteger: value=function:isInteger, writable=true, enumerable=false, configurable=true
propdesc length: value=1, writable=false, enumerable=false, configurable=true
propdesc name: value="isInteger", writable=false, enumerable=false, configurable=true
-Infinity false
-1e+100 true
-1234567890.2 false
-305419896 true
-0 true
0 true
305419896 true
1234567890.2 false
1e+100 true
Infinity false
NaN false
undefined false
null false
"" false
"123" false
true false
false false
[object Object] false
===*/

/*@include util-base.js@*/

print(typeof Number.isInteger);
print(Test.getPropDescString(Number, 'isInteger'));
print(Test.getPropDescString(Number.isInteger, 'length'));
print(Test.getPropDescString(Number.isInteger, 'name'));

[
    -1 / 0, -1e100, -1234567890.2, -0x12345678, -0,
    +0, 0x12345678, 1234567890.2, 1e100, 1 / 0,
    0 / 0,
    void 0, null, '', '123', true, false,
    { valueOf: function () { return 123.0; } }
].forEach(function (v) {
    print(Test.valueToString(v), Test.valueToString(Number.isInteger(v)));
});
