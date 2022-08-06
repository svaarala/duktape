/*===
finished
foo
===*/

/* Empty match with a global RegExp caused an infinite loop. */

var t = 'foo'.replace(/(?:)/g, '');
print('finished');
print(t);
