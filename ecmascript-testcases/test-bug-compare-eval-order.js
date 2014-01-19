/*
 *  Eval order of relational operators should always be the same (eval left
 *  first), but it's easy to break due to internal trivia.
 */

/*===
<
valueOf obj1
valueOf obj2
true
>
valueOf obj1
valueOf obj2
true
<=
valueOf obj1
valueOf obj2
true
>=
valueOf obj1
valueOf obj2
true
===*/

function getObject(name, value) {
    return {
        valueOf: function() {
            print('valueOf', name);
            return value;
        }
    }
}

function test() {
    print('<');
    print(getObject('obj1', 1) < getObject('obj2', 2));
    print('>');
    print(getObject('obj1', 2) > getObject('obj2', 1));
    print('<=');
    print(getObject('obj1', 1) <= getObject('obj2', 2));
    print('>=');
    print(getObject('obj1', 2) >= getObject('obj2', 1));
}

try {
    test();
} catch (e) {
    print(e);
}
