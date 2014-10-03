/*===
finished
foo
===*/

/* Empty match with a global RegExp caused an infinite loop. */

try {
    var t = 'foo'.replace(/(?:)/g, '');
    print('finished');
    print(t);
} catch (e) {
    print(e);
}
