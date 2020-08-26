// Cover empty search pattern and empty input.

/*===
foo
xfoo
fo
fxo
===*/

try {
    print('foo'.replace('', ''));
    print('foo'.replace('', 'x'));
    print('foo'.replace('o', ''));
    print('foo'.replace('o', 'x'));
} catch (e) {
    print(e.stack || e);
}
