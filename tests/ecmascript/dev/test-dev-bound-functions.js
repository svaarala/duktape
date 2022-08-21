/*===
args: 1 2 3 4
this: [object global]
args: f2 1 2 3
this: this-f2
args: f2 f3 1 2
this: this-f2
===*/

function orig(x,y,z,w) {
    print('args:', x, y, z, w);
    print('this:', this);
}

var f1, f2, f3, f4;

f1 = orig;
f2 = f1.bind('this-f2', 'f2');
f3 = f2.bind('this-f3', 'f3');

/* Plain function call, 'this' binding will be the global object. */
f1(1, 2, 3, 4);

/* Bound function: bind 'this' to 'this-f2' and prepend 'f1' to argument list:
 *
 *      f2(1, 2, 3)
 *  --> f1('f2', 1, 2, 3)  this='this-f2'
 */
f2(1, 2, 3);

/* Second order bound function: note the behavior of the this binding:
 *
 *      f3(1, 2)
 *  --> f2('f3', 1, 2)  this='this-f3'
 *  --> f1('f2', 'f3', 1, 2)  this='this-f2'
 *
 * In other words:
 *
 *   - The effective 'this' binding comes from the bound function right before
 *     the actual non-bound function (not the outermost bound function).
 *
 *   - The argument list to be prepended (for the final non-bound function call)
 *     can be formed by concatenating the [[BoundArgs]] lists of the bound
 *     functions, from innermost to outermost.  Or alternatively, by starting
 *     with an empty list, process each bound function from outermost to innermost,
 *     prepending the [[BoundArgs]].
 */
f3(1, 2);

/*===
args: 1 2 3 4
this: this-f2
args: 1 2 3 4
this: this-f2
===*/

/* Example of [[BoundArgs]] concatenation:
 *
 *    f1  <--  f2, [[BoundArgs]] = [1,2]  <--  f3, [[BoundArgs]] = [3,4]
 *
 * Process from outermost to innermost, prepending [[BoundArgs]]
 *
 *    []
 *    [3,4] + [] = [3,4]            (process f3)
 *    [1,2] + [3,4] = [1,2,3,4]     (process f2)
 *    [1,2,3,4]                     (f1 is not bound, finish)
 *
 * The ultimately effective [[BoundThis]] is the one right before the non-bound
 * function, i.e. f2's [[BoundThis]] here.
 *
 * For a "collapsed" bound function:
 *
 *    [[BoundArgs]] = [1,2,3,4]
 *    [[BoundThis]] = 'this-f2'
 *    [[TargetFunction]] = f1
 */

f1 = orig;
f2 = f1.bind('this-f2', 1, 2);
f3 = f2.bind('this-f3', 3, 4);

f4 = f1.bind('this-f2', 1, 2, 3, 4);  // equivalent to f3

f3();
f4();
