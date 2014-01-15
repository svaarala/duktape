/*
 *  The "set" and "get" tokens are a bit of a corner case for the lexer/compiler
 *  combination.  They are not officially ReservedWords so they must be valid
 *  in many contexts which otherwise use the Identifier production which rejects
 *  ReservedWords:
 *
 *    - Variable declaration name
 *    - Catch variable name
 *    - Break and continue labels
 *    - Function argument names
 *    - Function name
 *
 *  On the other hand, "get" and "set" need to be recognized as tokens in the
 *  object literal notation.
 */

/*===
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
eval success
123
set foo 234
eval success
123
set foo 234
eval success
123
set set 234
eval success
123
set set 234
eval success
123
set get 234
eval success
123
set get 234
eval success
===*/

function tryEval(code) {
    try {
        eval(code);
        print('eval success');
    } catch (e) {
        print(code, '-->', e);
    }
}

tryEval('(function () { var set = 1; })');
tryEval('(function () { "use strict"; var set = 1; })');
tryEval('(function () { var get = 1; })');
tryEval('(function () { "use strict"; var get = 1; })');

tryEval('(function () { try {} catch(set) {} })');
tryEval('(function () { "use strict"; try {} catch(set) {} })');
tryEval('(function () { try {} catch(get) {} })');
tryEval('(function () { "use strict"; try {} catch(get) {} })');

tryEval('(function () { set: for(;;) { break set; } })');
tryEval('(function () { "use strict"; set: for(;;) { break set; } })');
tryEval('(function () { get: for(;;) { break get; } })');
tryEval('(function () { "use strict"; get: for(;;) { break get; } })');

tryEval('(function (set) {})');
tryEval('(function (set) { "use strict"; })');
tryEval('(function (get) {})');
tryEval('(function (get) { "use strict"; })');

tryEval('(function set(x) {})');
tryEval('(function set(x) { "use strict"; })');
tryEval('(function get(x) {})');
tryEval('(function get(x) { "use strict"; })');

var obj;

tryEval('obj = function (x) { return { get foo() { return 123; }, set foo(v) { print("set foo",v); } }; }(); print(obj.foo); obj.foo = 234;');
tryEval('obj = function (x) { "use strict"; return { get foo() { return 123; }, set foo(v) { print("set foo",v); } }; }(); print(obj.foo); obj.foo = 234;');

// add a bit more challenge and make the setter / getter names are 'set' and 'get'
tryEval('obj = function (x) { return { get set() { return 123; }, set set(v) { print("set set",v); } }; }(); print(obj.set); obj.set = 234;');
tryEval('obj = function (x) { "use strict"; return { get set() { return 123; }, set set(v) { print("set set",v); } }; }(); print(obj.set); obj.set = 234;');
tryEval('obj = function (x) { return { get get() { return 123; }, set get(v) { print("set get",v); } }; }(); print(obj.get); obj.get = 234;');
tryEval('obj = function (x) { "use strict"; return { get get() { return 123; }, set get(v) { print("set get",v); } }; }(); print(obj.get); obj.get = 234;');
