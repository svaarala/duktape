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

try {
    foo();
} catch (e) {
    print(e.name);
}

var funcexpr = 'not visible';
var temp = function funcexpr() {
    print(typeof funcexpr);
};

try {
    temp();
} catch (e) {
    print(e.name);
}
