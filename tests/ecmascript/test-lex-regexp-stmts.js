/*
 *  RegExp token at beginning of statement in various contexts.
 */

/*===
true             - for (i = 0; i < 10; i++) /foo/.test("foo")
true             - for (var i = 0; i < 10; i++) /foo/.test("foo")
true             - for (k in { foo: 123 }) /foo/.test("foo")
true             - for (var k in { foo: 123 }) /foo/.test("foo")
SyntaxError      - for (i = 0; i < 10; i++) /foo/.test("bar") /foo/.test("foo")
true             - for (i = 0; i < 10; i++) /foo/.test("bar"); /foo/.test("foo")
SyntaxError      - for (i = 0; i < 10; i++) /foo/.test("bar")<LF>/foo/.test("foo")
true             - for (i = 0; i < 10; i++) {} /foo/.test("foo")
SyntaxError      - for (var i = 0; i < 10; i++) /foo/.test("bar") /foo/.test("foo")
true             - for (var i = 0; i < 10; i++) /foo/.test("bar"); /foo/.test("foo")
SyntaxError      - for (var i = 0; i < 10; i++) /foo/.test("bar")<LF>/foo/.test("foo")
true             - for (var i = 0; i < 10; i++) {} /foo/.test("foo")
SyntaxError      - for (k in { foo: 123 }) /foo/.test("bar") /foo/.test("foo")
true             - for (k in { foo: 123 }) /foo/.test("bar"); /foo/.test("foo")
SyntaxError      - for (k in { foo: 123 }) /foo/.test("bar")<LF>/foo/.test("foo")
true             - for (k in { foo: 123 }) {} /foo/.test("foo")
SyntaxError      - for (var k in { foo: 123 }) /foo/.test("bar") /foo/.test("foo")
true             - for (var k in { foo: 123 }) /foo/.test("bar"); /foo/.test("foo")
SyntaxError      - for (var k in { foo: 123 }) /foo/.test("bar")<LF>/foo/.test("foo")
true             - for (var k in { foo: 123 }) {} /foo/.test("foo")
true             - if (true) /foo/.test("foo")
SyntaxError      - if (true) /foo/.test("bar") /foo/.test("foo")
true             - if (true) /foo/.test("bar"); /foo/.test("foo")
SyntaxError      - if (true) /foo/.test("bar")<LF>/foo/.test("foo")
true             - if (true) {} /foo/.test("foo")
SyntaxError      - do /foo/.test("foo") while (0)
true             - do /foo/.test("foo"); while (0)
true             - do /foo/.test("foo")<LF>while (0)
true             - do {} while (0) /foo/.test("foo")
true             - do {} while (0)<LF>/foo/.test("foo")
true             - var i = 10; while (i--) /foo/.test("foo")
true             - var i = 10; while (i--)<LF>/foo/.test("foo")
SyntaxError      - var i = 10; while (i--) /foo/.test("bar") /foo/.test("foo")
true             - var i = 10; while (i--) /foo/.test("bar"); /foo/.test("foo")
SyntaxError      - var i = 10; while (i--) /foo/.test("bar")<LF>/foo/.test("foo")
true             - var i = 10; while (i--) {} /foo/.test("foo")
true             - with ({ foo: 123 })<LF>/foo/.test("foo")
true             - with ({ foo: 123 }) /foo/.test("foo")
SyntaxError      - with ({ foo: 123 }) /foo/.test("bar") /foo/.test("foo")
true             - with ({ foo: 123 }) /foo/.test("bar"); /foo/.test("foo")
SyntaxError      - with ({ foo: 123 }) /foo/.test("bar")<LF>/foo/.test("foo")
true             - with ({ foo: 123 }) {} /foo/.test("foo")
true             - function dummy() {} /foo/.test("foo")
true             - function dummy() {}<LF>/foo/.test("foo")
NaN              - ({ foo: 123 } / 2)
61.5             - ([ 123 ] / 2)
SyntaxError      - var x = [ 123, 234 ] /foo/.test("foo")
SyntaxError      - var x = [ 123, 234 ]<LF>/foo/.test("foo")
true             - var x = [ 123, 234 ]; /foo/.test("foo")
1                - (1 + 2) / 3
SyntaxError      - (1 + 2) /foo/.test("foo")
SyntaxError      - (1 + 2)<LF>/foo/.test("foo")
true             - (1 + 2); /foo/.test("foo")
true             - if (1) { /foo/.test("foo") }
true             - try { /foo/.test("foo") } catch (e) {}
true             - try { throw 123 } catch (e) { /foo/.test("foo") }
true             - try { throw 123 } catch (e) { /foo/.test("bar") } finally { /foo/.test("foo") }
true             - { /foo/.test("foo"); }
dummyvalue       - (function foo() { /foo/.test("foo"); }, "dummyvalue")
true             - [ /foo/.test("foo") ][0]
yes              - ({ [/foo/.test("foo")]: "yes" }).true
yes              - ({ true: "yes" })[/foo/.test("foo")]
true             - (/foo/.test("foo"))
true             - new String(/foo/.test("foo"))
true             - String(/foo/.test("foo"))
yes              - switch(/foo/.test("foo")) { case true: "yes"; break; default: "no" }
yes              - if (/foo/.test("foo")) { "yes" } else { "no" }
body             - do { "body" } while (/foo/.test("bar"))
never            - "never"; while (/foo/.test("bar")) { "body" }
body             - with (/foo/.exec("foo")) { "body" }
NaN              - (function () { return 1; } / {})
NaN              - (function () { return 1; } / 123)
SyntaxError      - (function () { return 1; } /foo/.test("foo"))
SyntaxError      - function decl1() { return 1; } / {}
SyntaxError      - function decl1() { return 1; } / 123
/foo/            - function decl1() { return 1; } /foo/
true             - function decl1() { return 1; } /foo/.test("foo")
NaN              - /foo/ / /bar/
/foo/            - /foo///bar/
NaN              - /foo/g/ /bar/
NaN              - /foo/g/ /bar/im
done
===*/

var sources = [
    // for statement body is RegExp
    'for (i = 0; i < 10; i++) /foo/.test("foo")',
    'for (var i = 0; i < 10; i++) /foo/.test("foo")',
    'for (k in { foo: 123 }) /foo/.test("foo")',
    'for (var k in { foo: 123 }) /foo/.test("foo")',

    // for statement followed by RegExp
    'for (i = 0; i < 10; i++) /foo/.test("bar") /foo/.test("foo")',
    'for (i = 0; i < 10; i++) /foo/.test("bar"); /foo/.test("foo")',
    'for (i = 0; i < 10; i++) /foo/.test("bar")\n/foo/.test("foo")',
    'for (i = 0; i < 10; i++) {} /foo/.test("foo")',
    'for (var i = 0; i < 10; i++) /foo/.test("bar") /foo/.test("foo")',
    'for (var i = 0; i < 10; i++) /foo/.test("bar"); /foo/.test("foo")',
    'for (var i = 0; i < 10; i++) /foo/.test("bar")\n/foo/.test("foo")',
    'for (var i = 0; i < 10; i++) {} /foo/.test("foo")',
    'for (k in { foo: 123 }) /foo/.test("bar") /foo/.test("foo")',
    'for (k in { foo: 123 }) /foo/.test("bar"); /foo/.test("foo")',
    'for (k in { foo: 123 }) /foo/.test("bar")\n/foo/.test("foo")',
    'for (k in { foo: 123 }) {} /foo/.test("foo")',
    'for (var k in { foo: 123 }) /foo/.test("bar") /foo/.test("foo")',
    'for (var k in { foo: 123 }) /foo/.test("bar"); /foo/.test("foo")',
    'for (var k in { foo: 123 }) /foo/.test("bar")\n/foo/.test("foo")',
    'for (var k in { foo: 123 }) {} /foo/.test("foo")',

    // if statement body is RegExp
    'if (true) /foo/.test("foo")',

    // if statement followed by RegExp
    'if (true) /foo/.test("bar") /foo/.test("foo")',
    'if (true) /foo/.test("bar"); /foo/.test("foo")',
    'if (true) /foo/.test("bar")\n/foo/.test("foo")',
    'if (true) {} /foo/.test("foo")',

    // do statement body is RegExp
    'do /foo/.test("foo") while (0)',
    'do /foo/.test("foo"); while (0)',
    'do /foo/.test("foo")\nwhile (0)',

    // do statement followed by RegExp
    'do {} while (0) /foo/.test("foo")',
    'do {} while (0)\n/foo/.test("foo")',

    // while statement body is RegExp
    'var i = 10; while (i--) /foo/.test("foo")',
    'var i = 10; while (i--)\n/foo/.test("foo")',

    // while statement followed by RegExp
    'var i = 10; while (i--) /foo/.test("bar") /foo/.test("foo")',
    'var i = 10; while (i--) /foo/.test("bar"); /foo/.test("foo")',
    'var i = 10; while (i--) /foo/.test("bar")\n/foo/.test("foo")',
    'var i = 10; while (i--) {} /foo/.test("foo")',

    // with statement body is RegExp
    'with ({ foo: 123 })\n/foo/.test("foo")',
    'with ({ foo: 123 }) /foo/.test("foo")',

    // with statement followed by RegExp
    'with ({ foo: 123 }) /foo/.test("bar") /foo/.test("foo")',
    'with ({ foo: 123 }) /foo/.test("bar"); /foo/.test("foo")',
    'with ({ foo: 123 }) /foo/.test("bar")\n/foo/.test("foo")',
    'with ({ foo: 123 }) {} /foo/.test("foo")',

    // function declaration followed by RegExp
    'function dummy() {} /foo/.test("foo")',
    'function dummy() {}\n/foo/.test("foo")',

    // right curly followed by division is also possible
    '({ foo: 123 } / 2)',

    // right bracket followed by division
    '([ 123 ] / 2)',

    // right bracket followed by RegExp
    'var x = [ 123, 234 ] /foo/.test("foo")',
    'var x = [ 123, 234 ]\n/foo/.test("foo")',
    'var x = [ 123, 234 ]; /foo/.test("foo")',

    // right paren followed by division
    '(1 + 2) / 3',

    // right paren followed by RegExp
    '(1 + 2) /foo/.test("foo")',
    '(1 + 2)\n/foo/.test("foo")',
    '(1 + 2); /foo/.test("foo")',

    // left curly followed by RegExp
    'if (1) { /foo/.test("foo") }',
    'try { /foo/.test("foo") } catch (e) {}',
    'try { throw 123 } catch (e) { /foo/.test("foo") }',
    'try { throw 123 } catch (e) { /foo/.test("bar") } finally { /foo/.test("foo") }',
    '{ /foo/.test("foo"); }',
    '(function foo() { /foo/.test("foo"); }, "dummyvalue")',

    // left bracket followed by RegExp
    '[ /foo/.test("foo") ][0]',
    '({ [/foo/.test("foo")]: "yes" }).true',
    '({ true: "yes" })[/foo/.test("foo")]',

    // left paren followed by RegExp
    '(/foo/.test("foo"))',
    'new String(/foo/.test("foo"))',
    'String(/foo/.test("foo"))',
    'switch(/foo/.test("foo")) { case true: "yes"; break; default: "no" }',
    'if (/foo/.test("foo")) { "yes" } else { "no" }',
    'do { "body" } while (/foo/.test("bar"))',
    '"never"; while (/foo/.test("bar")) { "body" }',
    'with (/foo/.exec("foo")) { "body" }',

    // RegExp is not allowed after a function expresion
    '(function () { return 1; } / {})',
    '(function () { return 1; } / 123)',
    '(function () { return 1; } /foo/.test("foo"))',

    // RegExp *is* allowed after a function declaration which is a statement
    'function decl1() { return 1; } / {}',
    'function decl1() { return 1; } / 123',
    'function decl1() { return 1; } /foo/',
    'function decl1() { return 1; } /foo/.test("foo")',

    // RegExp and division mixed.
    '/foo/ / /bar/',  // NaN
    '/foo///bar/',    // interpreted as '/foo/' because ///bar/ treated as comment
    '/foo/g/ /bar/',
    '/foo/g/ /bar/im',
];

sources.forEach(function (src) {
    function f(v) {
        return (v + '                 ').substring(0, 16);
    }
    var fmtSrc = src.replace('\n', '<LF>');
    try {
        print(f(eval(src)) + ' - ' + fmtSrc);
    } catch (e) {
        print(f(e.name) + ' - ' + fmtSrc);
    }
});

print('done');
