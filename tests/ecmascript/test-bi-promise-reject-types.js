/*---
{
    "skip": true
}
---*/

/*@include util-base.js@*/

/*===
done
reject: [null,null,true,false,null,-123,0,0,123,null,null,"","foo",{"foo":"bar"},["A","B","C"],null,{"0":1,"1":2,"2":3}]
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

// Exercise reject types other than thenables (which are not recognized by
// reject in any case).
//
// Cannot use Promise.all() because the values are rejections, so implement
// something similar manually.

var results = [];
var done = 0;
var values = [
    void 0, null, true, false,
    -1/0, -123, -0, 0, 123, 1/0, 0/0,
    '', 'foo', { foo: 'bar' }, [ 'A', 'B', 'C' ],
    function foo() {}, new Uint8Array([ 1, 2, 3 ])
];

values.forEach(function (v, idx) {
    new Promise(function (resolve, reject) { reject(v); }).catch(function (e) {
        results[idx] = e;
        if (++done === values.length) {
            print('reject:', JSON.stringify(results));
            results.forEach(function (x) {
                print(Test.valueToString(x));
            });
        }
    });
});

print('done');
