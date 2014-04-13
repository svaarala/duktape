/*===
test1
do top
with top
do bottom
test2
do top
with top
===*/

/* In Duktape 0.9.0 test1() would pass but test2() would trigger an assertion
 * failure in duk_js_executor:2716, ENDLABEL handling:
 *
 *     DUK_ASSERT(DUK_CAT_GET_TYPE(cat) == DUK_CAT_TYPE_LABEL);
 *
 * The reason for this was that the compiler would convert the break statement
 * into a fast break (= jump) so that the trycatch catcher used by with statement
 * was not properly unwound when ENDLABEL was executed.  This was fixed in
 * Duktape 0.10.0 by ensuring that with statement parsing increases catch depth
 * tracking correctly, which causes the break to emitted as an explicit slow
 * break which unwinds the trycatch state properly.
 */

function test1() {
    var obj = {};
    do {
        print('do top');
        with (obj) {
            print('with top');
        }
        print('do bottom');
    } while (0);
}

function test2() {
    var obj = {};
    do {
        print('do top');
        with (obj) {
            print('with top');
            break;
        }
        /* not reached */
        print('do bottom');
    } while (0);
}

print('test1');

try {
    test1();
} catch (e) {
    print(e);
}

print('test2');

try {
    test2();
} catch (e) {
    print(e);
}
