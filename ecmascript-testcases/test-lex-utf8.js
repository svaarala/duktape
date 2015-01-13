/*
 *  Lexer UTF-8 support.
 */

/*===
äö
变量
1 number
2 number
3 number
hello world!
===*/

// These chars are in UTF-8 format in the source file
print('äö');
print('变量');

/* Note: smjs fails the tests below (Rhino does not) */

$äöå = 1;
print($äöå, typeof $äöå);

äöå = 2;
print(äöå, typeof äöå);

my_变量 = 3;
print(my_变量, typeof my_变量);

obj = {};
obj.变量1 = 'hello';
print(obj.变量1, 'world!');
