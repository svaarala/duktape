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

try {
    print(null);
} catch (e) {
    print(e.name);
}

try {
    print(true);
} catch (e) {
    print(e.name);
}

try {
    print(false);
} catch (e) {
    print(e.name);
}

try {
    print(123);
} catch (e) {
    print(e.name);
}

try {
    print(0x41);
} catch (e) {
    print(e.name);
}

try {
    // = 4*8+1 = 33
    print(041);
} catch (e) {
    print(e.name);
}

// XXX: 077, 088, 099 -> V8 accepts 088 and 099 as decimal literals
try {
    print("foo");
} catch (e) {
    print(e.name);
}

try {
    print('foo');
} catch (e) {
    print(e.name);
}
