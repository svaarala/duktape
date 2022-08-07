/*===
function
function
===*/

/* Simple test that function declaration name and function expression name
 * can be accessed from inside the function.
 */
function foo() {
    print(typeof foo);
}

foo();

var funcexpr = 'not visible';
var temp = function funcexpr() {
    print(typeof funcexpr);
};

temp();
