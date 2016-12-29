/*===
bar
bar
bar
SyntaxError
===*/

print({foo:'bar'}.foo);    // key without quotes
print({"foo":'bar'}.foo);  // key with quotes
print({foo:'bar',}.foo);   // one trailing comma

try {
    // two trailing commas is a SyntaxError
    eval("print({foo:'bar',,}.foo)");
} catch (e) {
    print(e.name);
}

/*===
1
2
3
4
5
6
7
===*/

/* Property name can be an identifier, a string, or a number.
 * The identifier can also be a reserved word.
 */

print({foo:1}.foo);
print({if:2}.if);
print({1:3}['1']);
print({1.23:4}['1.23']);   // coerced with ToString(1.23) -> '1.23'
print({1.00:5}['1']);      // coerced with ToString(1.00) -> '1'
print({Infinity:6}['Infinity']);
print({"foo":7}.foo);

/*===
1
2
3
4
5
6
7
===*/

/* Getter/setter name can be an identifier, a string, or a number.
 * The identifier can also be a reserved word.
 */

try {
    eval("print({get foo() { return 1 }}.foo);");
} catch (e) {
    print(e.name);
}

try {
    eval("print({get if() { return 2 }}.if);");
} catch (e) {
    print(e.name);
}

try {
    eval("print({get 1() { return 3 }}['1']);");
} catch (e) {
    print(e.name);
}

try {
    eval("print({get 1.23() { return 4 }}['1.23']);");  // coerced with ToString
} catch (e) {
    print(e.name);
}

try {
    eval("print({get 1.00() { return 5 }}['1']);");  // coerced with ToString
} catch (e) {
    print(e.name);
}

try {
    eval("print({get Infinity() { return 6 }}['Infinity']);");  // coerced with ToString
} catch (e) {
    print(e.name);
}

try {
    eval('print({get "foo"() { return 7 }}.foo);');
} catch (e) {
    print(e.name);
}

/*===
1
2
3
===*/

/* Some corner cases. */

try {
    eval("print({get: 1}.get);");                  // 'get' as a property name
} catch (e) {
    print(e.name);
}

try {
    eval("print({get foo() { return 2 }}.foo);");  // 'foo' as a getter
} catch (e) {
    print(e.name);
}

try {
    eval("print({get get() { return 3 }}.get);");  // 'get' as a getter name (pretty rare)
} catch (e) {
    print(e.name);
}

/*===
2
2
===*/

/* Duplicate keys were rejected in ES5, but ES2015 allows them. */

try {
    print(eval("function func1() { return ({foo:1,foo:2}).foo; }; func1();"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("function func2() { 'use strict'; return ({foo:1,foo:2}).foo; }; func2();"));
} catch (e) {
    print(e.name);
}

/*===
2
2
getter
undefined
1
1
1
===*/

/* Duplicate keys and mixed set/get/plain key handling */

try {
    print(eval("({foo:1,foo:2}).foo"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("'use strict'; ({foo:1,foo:2}).foo"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("({foo:1, get foo() { return 'getter'}}).foo"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("({foo:1, set foo(v) {}}).foo"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("({get foo() { return 'getter' }, foo:1}).foo"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("({set foo(v) {}, foo:1}).foo"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("({get foo() { return 'getter' }, set foo(v) {}, foo:1}).foo"));
} catch (e) {
    print(e.name);
}
