/*
 *  Lexer UTF-8 support.
 */

/*===
äö
1 number
2 number
===*/

// These chars are in UTF-8 format in the source file
print('äö');

/* Note: smjs fails the tests below (Rhino does not) */

$äöå = 1;
print($äöå, typeof $äöå);

äöå = 2;
print(äöå, typeof äöå);

