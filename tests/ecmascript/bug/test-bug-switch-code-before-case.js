/*
 *  Any code before a case block must be rejected.
 */

/*===
SyntaxError
===*/

try {
    eval('switch (2) { print("hello"); case 2: print("match 2"); }');
} catch (e) {
    print(e.name);
}
