/*
 *  'logical or assignment' and 'logical and assignment' operators (https://tc39.es/ecma262/multipage/ecmascript-language-expressions.html#sec-assignment-operators).
 */

function testSideEffect(exp, key) {
    print('tse ' + key);
    return exp;
}

/*===
5
===*/
var a = 1;
a &&= 5;
print (a);

/*===
tse 1
0
0
===*/
a &&= 1 && testSideEffect(0, '1');
print(a); // Should be 0
a &&= testSideEffect(false, '2'); // testSideEffect should not be evaluated
print(a); // Should be 0, not false

/*===
tse 3
tse 4
tse 5
false
false
===*/
var b = a ||= testSideEffect(0, '3') || testSideEffect(0, '4') || testSideEffect(false, '5');
print(a); // Should be false, not 0
print(b); // Should be false, not 0



/*===
[object Object]
false
===*/
var obj = {a: false};
obj.b ||= obj.a;
obj.a ||= {};
print(obj.a);
print(obj.b);

/*===
tse 6
true
===*/
testSideEffect(obj, '6').b ||= true;
print(obj.b);


/*===
tse 7
tse 8
tse 9
tse 10
99
===*/

testSideEffect(obj, '7').b &&= testSideEffect(obj, '8').b &&= testSideEffect(obj, '9').b = testSideEffect(99, '10');
print(obj.b);