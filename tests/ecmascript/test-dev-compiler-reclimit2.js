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

function rep(n, c) {
    var a = []; a.length = n + 1;
    return a.join(c);
}

/*===
test
RangeError
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
    // expression recursion limit prevents this, with both shallow/deep stacks
    var inp = rep(2500, '(') + "'never here'" + rep(2500, ')');
    print(eval(inp));
} catch (e) {
    print(e.name);
}

/*===
test
RangeError
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
    // statement recursion limit prevents this, with both shallow/deep stacks
    var inp = rep(2500, '{') + "'test'" + rep(2500, '}');
    print(eval(inp));
} catch (e) {
    print(e.name);
}

/*===
function(){ var f=function(){}; }
function(){ var f=function(){ var f=function(){}; }; }
function(){ var f=function(){ var f=function(){ var f=function(){}; }; }; }
test
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

/* The test for function recursion limit is disabled now.  The code
 * below will trigger the limit with shallow stack config but not
 * with a deep stack config.  A value which triggers the deep stack
 * config takes an insane amount to execute now.
 */
/*
try {
    // function recursion limit prevents this
    print(eval("ignore = " + buildFunc(20) + "; 'test'"));
} catch (e) {
    print(e.name);
}
*/
