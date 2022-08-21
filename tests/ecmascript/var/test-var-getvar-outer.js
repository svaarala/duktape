/*===
123
123
===*/

function outer() {
    var x = 123;
    function inner() { print(x); }
    inner();
}
outer();

eval('"use strict"; function outer() { var x = 123; function inner() { print(x); }; inner(); }; outer();');
