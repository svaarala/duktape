/*
 *  Throw statement (E5 Section 12.13).
 *
 *  Syntax:
 *
 *    throw [no LineTerminator here] Expression ;
 */

/*===
undefined
null
boolean [object Boolean]
boolean [object Boolean]
number [object Number]
string [object String]
object [object Object]
object [object Array]
function [object Function]
SyntaxError
SyntaxError
number 357
===*/

function printValue(v) {
    print((v === null ? 'null' : typeof v) +
          (v != null ? ' ' + (Object.prototype.toString.call(v)) : ''));
}

function test() {
    /*
     *  Must be able to throw any type.
     */

    [ undefined, null, true, false, 123, 'foo', { foo: 1 }, [ 1, 2 ], function () {} ].forEach(function (v) {
        try {
            throw v;
        } catch (e) {
            // Avoid Object.prototype.toString() for undefined and null because
            // there's variance between implementations.
            printValue(e);
        }
    });

    /*
     *  Empty throw is not allowed (i.e. 'throw;')
     */

    try {
        eval('throw;');
    } catch (e) {
        print(e.name);
    }

    /*
     *  When parsing:
     *
     *      throw
     *      123;
     *
     *  the [no LineTerminator here] part means that an automatic semicolon
     *  is automatically inserted after 'throw' even if the statement could
     *  otherwise parsed.  The result is:
     *
     *      throw;
     *      123;
     *
     *  which is a SyntaxError.  In practical terms a newline before the
     *  expression is a SyntaxError.
     */

    try {
        eval('throw\n123;');
    } catch (e) {
        print(e.name);
    }

    /*
     *  Newlines are allowed right after the Expression part starts.
     */

    try {
        eval('throw 123\n + 234;');
    } catch (e) {
        print(typeof e + ' ' + e);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
