/*===
123
123
===*/

function outer() {
    var x = 123;
    function inner() { print(x); }
    inner();
}
try {
    outer();
} catch (e) {
    print(e.stack || e);
}

try {
    eval('"use strict"; function outer() { var x = 123; function inner() { print(x); }; inner(); }; outer();');
} catch (e) {
    print(e.stack || e);
}
