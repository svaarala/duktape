/*
 *  Bug reported by: http://www.reddit.com/user/judofyr
 *
 *  The trigger for the bug is "return reduce(..." tailcall in the flatten
 *  function.  For some reason the 'shallow' variable access doesn't work
 *  correctly.  If the code is changed to "var res = reduce(...); return res;"
 *  it works in Duktape 0.8.0.
 *
 */

/* Incorrect output from Duktape 0.8.0:
function empty() {...}
function empty() {...}
[1,2,3,[[[4]]]]
*/

/*===
undefined
undefined
undefined
undefined
undefined
[1,2,3,4]
===*/

try {
    var reduce = function(obj, iterator, memo) {
        return obj.reduce(iterator, memo);
    };

    var flatten = function(array, shallow) {
        /* The problem here with a tailcall is that the flatten() invocation will
         * be replaced in the callstack without an explicit unwind.   The GETVAR
         * for 'shallow' will still match to the outer lexical environment but the
         * environment will not be closed.
         *
         * As a result, the 'shallow' lookup will look up an unrelated register and
         * return the incorrect 'function' value.  The fix is to ensure that the
         * current activation is closed on a TAILCALL.
         */
        return reduce(array, function(memo, value) {
            if (Array.isArray(value)) {
                print(shallow); // this should print undef
                return memo.concat(shallow ? value : flatten(value));
            } else {
                memo[memo.length] = value;
            }
            return memo;
        }, []);
    }

    print(JSON.stringify(flatten([1, [2], [3, [[[4]]]]]))); // [1,2,3,4]
} catch (e) {
    print(e)
}
