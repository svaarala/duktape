/*
 *  Ecmascript compiler recursion limits.  These are quite arbitrary, so here
 *  we test against the current limits, ensuring that reasonable expressions
 *  get parsed while "unreasonable" ones cause a recursion error.
 *
 *  The compiler limits recursion now in key points which are guaranteed to
 *  occur in any arbitrarily deep recursion: expression parsing, statement
 *  parsing, and function body parsing.  A single recursion count is used,
 *  so the actual limits for each expression (consisting of expressions,
 *  statements, and inner functions) varies.
 */

/*---
{
    "custom": true
}
---*/

/*===
test
Error
===*/

/* Expression recursion. */

try {
    // expression recursion limit does not prevent this
    print(eval("(((((((((((((((((((((((((((((((((((((((((((((((" + "'test'" +
               ")))))))))))))))))))))))))))))))))))))))))))))))"));
              //         111111111122222222223333333333444444444455555555555
              //123456789012345678901234567890123456789012345678901234567890
} catch (e) {
    print(e.name, e);
}

try {
    // expression recursion limit prevents this
    print(eval("((((((((((((((((((((((((((((((((((((((((((((((((((" + "'never here'" +
               "))))))))))))))))))))))))))))))))))))))))))))))))))"));
              //         111111111122222222223333333333444444444455555555555
              //123456789012345678901234567890123456789012345678901234567890
} catch (e) {
    print(e.name);
}

/*===
test
Error
===*/

/* Statement recursion. */

try {
    // statement recursion limit does not prevent this
    print(eval("{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{" + "'test'" +
               "}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}"));
              //         111111111122222222223333333333444444444455555555555
              //123456789012345678901234567890123456789012345678901234567890
} catch (e) {
    print(e.name);
}

try {
    // statement recursion limit prevents this
    print(eval("{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{" + "'test'" +
               "}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}"));
              //         111111111122222222223333333333444444444455555555555
              //123456789012345678901234567890123456789012345678901234567890
} catch (e) {
    print(e.name);
}

/*===
function(){ var f=function(){}; }
function(){ var f=function(){ var f=function(){}; }; }
function(){ var f=function(){ var f=function(){ var f=function(){}; }; }; }
test
Error
===*/

/* Function recursion. */

var ignore;

function buildFunc(n) {
    var res = 'function(){}';
    var i;

    for (i = 0; i < n; i++) {
        res = 'function(){ var f=' + res + '; }'
    }

    return res;
}

print(buildFunc(1));
print(buildFunc(2));
print(buildFunc(3));

try {
    // function recursion limit does not prevent this

    // NOTE: because inner functions are parsed and re-parsed an
    // exponentially number of times at the moment, this takes a
    // long time

    print(eval("ignore = " + buildFunc(15) + "; 'test'"));
} catch (e) {
    print(e.name);
}

try {
    // function recursion limit prevents this
    print(eval("ignore = " + buildFunc(20) + "; 'test'"));
} catch (e) {
    print(e.name);
}


