/*
 *  PrimaryExpression -> Literal
 */

/*===
null
true
false
123
65
33
foo
foo
===*/

/* Literal:
 *   -> NullLiteral    -> 'null'
 *   -> BooleanLiteral -> 'true' | 'false'
 *   -> NumericLiteral -> DecimalLiteral | HexIntegerLiteral | OctalIntegerLiteral (compatibility)
 *   -> StringLiteral  -> '"' chars '"' | "'" chars "'"
 *   -> RegularExpressionLiteral
 */

print(null);
print(true);
print(false);
print(123);
print(0x41);

// = 4*8+1 = 33
print(041);

// XXX: 077, 088, 099 -> V8 accepts 088 and 099 as decimal literals

print("foo");
print('foo');
