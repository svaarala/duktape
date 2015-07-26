/*===
SyntaxError
===*/

/* Don't allow empty object with just comma */

try {
    print(JSON.parse('[,]'));
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
===*/

/* Don't allow initial comma */

try {
    print(JSON.parse('[,1]'));
} catch (e) {
    print(e.name);
}


/*===
SyntaxError
===*/

/* Don't allow trailing comma.
 *
 * Rhino allows this.
 */

try {
    print(JSON.parse('[1,]'));
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
===*/

/* Don't allow successive commas (elisions) */

try {
    print(JSON.parse('[1,,2]'));
} catch (e) {
    print(e.name);
}
