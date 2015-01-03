/*
 *  Return statement (E5 Section 12.9).
 *
 *  Syntax:
 *
 *    return;
 *    return [no LineTerminator here] Expression ;
 */

/*===
undefined
undefined
null
boolean [object Boolean]
boolean [object Boolean]
number [object Number]
string [object String]
object [object Object]
object [object Array]
function [object Function]
undefined
undefined
number [object Number]
234
SyntaxError
SyntaxError
===*/

function printValue(v) {
    print((v === null ? 'null' : typeof v) +
          (v != null ? (' ' + Object.prototype.toString.call(v)) : ''));
}

/*
 *  Must be able to return any type.  No argument means undefined.
 */

function retEmpty() { return; }
function retUndefined() { return undefined; }
function retNull() { return null; }
function retTrue() { return true; }
function retFalse() { return false; }
function retNumber() { return 123; }
function retString() { return 'foo'; }
function retObject() { return { foo: 1 }; }
function retArray() { return [ 1, 2 ]; }
function retFunction() { return function () {} }

function testReturnTypes() {
    [ retEmpty, retUndefined, retNull, retTrue, retFalse, retNumber,
      retString, retObject, retArray, retFunction ].forEach(function (v) {
        printValue(v());
    });
}

try {
    testReturnTypes();
} catch (e) {
    print(e.stack || e);
}

/*
 *  When parsing:
 *
 *      return
 *      123;
 *
 *  the [no LineTerminator here] means that an automatic semicolon is inserted
 *  after 'return' even if the return statement would otherwise be parseable
 *  as 'return 123'.  The result is:
 *
 *      return;
 *      123;
 *
 *  which is a valid expression and returns 'undefined'.
 */

function retLineTerm1() {
    return
    123;
}
function retLineTerm2() {
    // As long as the Expression has started a newline is allowed.
    // Here a unary plus is enough to start the expression.
    return +
    234;
}
function testReturnLineTerm() {
    var ret;

    ret = retLineTerm1();
    printValue(ret);
    print(ret);

    ret = retLineTerm2();
    printValue(ret);
    print(ret);
}

try {
    testReturnLineTerm();
} catch (e) {
    print(e.stack || e);
}

/*
 *  SyntaxError outside of a function body, E5 Section 12.9.
 */

function testReturnInEval() {
    eval('return 123');
}
try {
    testReturnInEval();
} catch (e) {
    print(e.name);
}
try {
    eval('return 123');
} catch (e) {
    print(e.name);
}
