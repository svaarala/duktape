/*===
SyntaxError
===*/

/* The current number parser is a lenient approximation and will be fixed
 * when compliant number parsing is implemented.  This test case illustrates
 * one leniency example.
 */

try {
    print(JSON.parse('1.2.3'));
} catch (e) {
    print(e.name);
}
