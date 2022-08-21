/*
 *  A throw statement does not allow a newline before an argument,
 *  but Duktape 0.11.0 allows that.
 *
 *  Found with test262 ch07/7.9/S7.9_A4.
 */

/*===
e=123
SyntaxError
===*/

try {
   eval('try {\n' +
        '    throw 123\n' +
        '} catch (e) {\n' +
        '    print("e=" + e);\n' +
        '}\n');
} catch (e) {
    print(e);
}

try {
    eval('try {\n' +
         '    throw\n' +
         '    123;\n' +
         '} catch (e) {\n' +
         '    print("e=" + e);\n' +
         '}\n');
} catch (e) {
    print(e.name);
}
