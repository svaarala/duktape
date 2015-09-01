/*
 *  PrimaryExpression -> ArrayLiteral
 */

/*===
array: 0 []
array: 1 [null]
0 undefined undefined
array: 2 [null,null]
0 undefined undefined
1 undefined undefined
array: 1 [1]
0 number 1
array: 1 [1]
0 number 1
array: 2 [1,null]
0 number 1
1 undefined undefined
array: 2 [null,1]
0 undefined undefined
1 number 1
array: 5 [null,null,null,null,1]
0 undefined undefined
1 undefined undefined
2 undefined undefined
3 undefined undefined
4 number 1
array: 9 [null,null,null,1,null,null,2,null,null]
0 undefined undefined
1 undefined undefined
2 undefined undefined
3 number 1
4 undefined undefined
5 undefined undefined
6 number 2
7 undefined undefined
8 undefined undefined
1st
2nd
3rd
array: 3 [1001,1002,1003]
0 number 1001
1 number 1002
2 number 1003
===*/

function dump(v) {
    var i;

    print('array:', v.length, JSON.stringify(v));

    for (i = 0; i < v.length; i++) {
        print(i, typeof v[i], String(v[i]));
    }
}

var t;

// empty
t = [];
dump(t);

// trailing commas after an empty list each increase length by one
t = [,];
dump(t);
t = [,,];
dump(t);

// one element
t = [1];
dump(t);

// first trailing comma after an actual element is effectively ignored,
// so [1,] is the same as [1].
t = [1,];
dump(t);

// this has length 2
t = [1,,];
dump(t);

// leading elisions
t = [,1];
dump(t);
t = [,,,,1];
dump(t);

// mixed elisions
t = [,,,1,,,2,,,];
dump(t);

// side effects of evaluation
t = [ (print('1st'), 1001),
      (print('2nd'), 1002),
      (print('3rd'), 1003) ];
dump(t);
