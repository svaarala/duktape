/*
 *  Errors thrown by Duktape internals during an errCreate() will work
 *  normally except they won't be augmented to avoid recursion.
 */

/*===
errCreate: RangeError
internal error: URIError
internal error: EvalError
errThrow: RangeError
RangeError
===*/

try {
    Duktape.errCreate = function (e) {
        print('errCreate:', e.name);
        try {
            decodeURIComponent('%ff%ff');
        } catch (e2) {
            print('internal error:', e2.name);
        }
        try {
            throw new EvalError('user error');  // created but not augmented
        } catch (e2) {
            print('internal error:', e2.name);
        }
        return e;
    };
    Duktape.errThrow = function (e) {
        print('errThrow:', e.name);
        return e;
    };

    throw new RangeError('aiee');
} catch (e) {
    print(e.name);
}
