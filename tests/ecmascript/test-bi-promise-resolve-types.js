/*---
{
    "skip": true
}
---*/

/*@include util-base.js@*/

/*===
done
fulfill: [null,null,true,false,null,-123,0,0,123,null,null,"","foo",{"foo":"bar"},["A","B","C"],null,{"0":1,"1":2,"2":3}]
undefined
null
true
false
-Infinity
-123
-0
0
123
Infinity
NaN
""
"foo"
[object Object]
[object Array]
function:foo
[object Uint8Array]
===*/

// Exercise resolve types other than thenables.

Promise.all([
    void 0, null, true, false,
    -1/0, -123, -0, 0, 123, 1/0, 0/0,
    '', 'foo', { foo: 'bar' }, [ 'A', 'B', 'C' ],
    function foo() {}, new Uint8Array([ 1, 2, 3 ])
].map(function (v) {
    return new Promise(function (resolve, reject) { resolve(v); });
})).then(function (v) {
    print('fulfill:', JSON.stringify(v));
    v.forEach(function (x) {
        print(Test.valueToString(x));
    });
}, function (e) {
    print('reject:', e);
});

print('done');
