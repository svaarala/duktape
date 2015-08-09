/*
 *  Try-catch regression in master, GH-268.
 */

/*===
still here
===*/

/* For the bug to be triggered, the function being compiled must not contain
 * any constants and the try-finally must not have a catch clause.
 */
try {
    eval('try {} finally {}');
} catch (e) {
    print(e.stack || e);
}
print('still here');
