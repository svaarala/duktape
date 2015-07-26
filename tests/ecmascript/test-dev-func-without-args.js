/*
 *  Development time testcase for a function without arguments
 */

/*===
hello world
===*/

try {
    eval("(function() { print('hello world'); })();");
} catch (e) {
    print(e.name);
}
