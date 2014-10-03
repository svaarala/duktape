/*===
getter
setter 10
getter
setter 10
getter
setter 10
getter
setter 10
===*/

/* Spot check: setter/getter named 'eval' or 'arguments' is valid in
 * both strict and non-strict mode.
 *
 * This test ensures that the function name check (done after pass 1)
 * does not compare getter/setter name by accident.
 */

try {
    var foo = eval("({ get eval() { return 'getter' }, set eval(x) { print('setter',x) } })");
    print(foo.eval);
    foo.eval = 10;
} catch (e) {
    print(e.name);
}

try {
    var foo = eval("({ get eval() { 'use strict'; return 'getter' }, set eval(x) { 'use strict'; print('setter',x) } })");
    print(foo.eval);
    foo.eval = 10;
} catch (e) {
    print(e.name, e.message);
}

try {
    var foo = eval("({ get arguments() { return 'getter' }, set arguments(x) { print('setter',x) } })");
    print(foo.arguments);
    foo.arguments = 10;
} catch (e) {
    print(e.name);
}

try {
    var foo = eval("({ get arguments() { 'use strict'; return 'getter' }, set arguments(x) { 'use strict'; print('setter',x) } })");
    print(foo.arguments);
    foo.arguments = 10;
} catch (e) {
    print(e.name, e.message);
}
