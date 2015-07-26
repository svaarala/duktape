/*
 *  JSON number parsing bug in Duktape 1.0 for explicitly positive exponents,
 *  eval() works.
 */

/*===
150
150
===*/

function testJson() {
    // The parse error here is caused by the exponent plus sign.
    print(JSON.parse('1.50e+2'));
}

function testEval() {
    print(eval('1.50e+2'));
}

try {
    testJson();
} catch (e) {
    print(e.stack || e);
}

try {
    testEval();
} catch (e) {
    print(e.stack || e);
}
