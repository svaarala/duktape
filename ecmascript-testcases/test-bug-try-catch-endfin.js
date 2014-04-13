/*===
test1a
try
finally
123
test1b
try
catch
finally
undefined
test2a
try
finally
undefined
test2b
try
catch
finally
234
test3a
try
finally
345
test3b
try
finally
345
===*/

/* The ENDFIN opcode had an incorrect assertion that the TCF catcher's
 * CATCH flag had to be cleared when ENDFIN was executed.  This is not
 * the case if the try block does not throw an error.
 *
 * Fixed in Duktape 0.10.0.
 */

function test1a() {
    try {
        print('try');
        return 123;
    } catch (e) {
        print('catch');
    } finally {
        print('finally');
    }
}

function test1b() {
    try {
        print('try');
        throw new Error('error');
    } catch (e) {
        print('catch');
    } finally {
        print('finally');
    }
}

function test2a() {
    try {
        print('try');
    } catch (e) {
        print('catch');
        return 234;
    } finally {
        print('finally');
    }
}

function test2b() {
    try {
        print('try');
        throw new Error('error');
    } catch (e) {
        print('catch');
        return 234;
    } finally {
        print('finally');
    }
}

function test3a() {
    try {
        print('try');
    } catch (e) {
        print('catch');
    } finally {
        print('finally');
        return 345;
    }
}

function test3b() {
    try {
        print('try');
    } catch (e) {
        print('catch');
    } finally {
        print('finally');
        return 345;
    }
}

[ 'test1a', 'test1b', 'test2a', 'test2b', 'test3a', 'test3b' ].forEach(function (name) {
    print(name);
    try {
        print(this[name]());
    } catch (e) {
        print(e);
    }
});
