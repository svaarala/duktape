/*===
123
foo
123
===*/

function foo(e) {
    print(e);

    try {
        throw 'foo'
    } catch (e) {
        print(e);
    }

    print(e);
}

foo(123);
