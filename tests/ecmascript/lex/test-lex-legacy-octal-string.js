/*
 *  Legacy octal escape in string literal.
 */

function safeString(v) {
    return v.replace(/[^\u0020-u007e]/g, function (c) { return '<' + c.charCodeAt(0).toString(16) + '>'; });
}

function safePrint(v) {
    print(safeString(v));
}

/*===
fooAbar
===*/

// Basic case.
try {
    safePrint('foo\101bar');
} catch (e) {
    print(e.name);
}

/*===
0 "foo\0bar" foo<0>bar
0 "foo\0bar" foo<0>bar
1 "foo\1bar" foo<1>bar
1 "foo\1bar" SyntaxError
2 "foo\2bar" foo<2>bar
2 "foo\2bar" SyntaxError
3 "foo\3bar" foo<3>bar
3 "foo\3bar" SyntaxError
4 "foo\4bar" foo<4>bar
4 "foo\4bar" SyntaxError
5 "foo\5bar" foo<5>bar
5 "foo\5bar" SyntaxError
6 "foo\6bar" foo<6>bar
6 "foo\6bar" SyntaxError
7 "foo\7bar" foo<7>bar
7 "foo\7bar" SyntaxError
8 "foo\8bar" foo8bar
8 "foo\8bar" foo8bar
9 "foo\80bar" foo80bar
9 "foo\80bar" foo80bar
10 "foo\9bar" foo9bar
10 "foo\9bar" foo9bar
11 "foo\90bar" foo90bar
11 "foo\90bar" foo90bar
12 "foo\00bar" foo<0>bar
12 "foo\00bar" SyntaxError
13 "foo\01bar" foo<1>bar
13 "foo\01bar" SyntaxError
14 "foo\02bar" foo<2>bar
14 "foo\02bar" SyntaxError
15 "foo\03bar" foo<3>bar
15 "foo\03bar" SyntaxError
16 "foo\04bar" foo<4>bar
16 "foo\04bar" SyntaxError
17 "foo\05bar" foo<5>bar
17 "foo\05bar" SyntaxError
18 "foo\06bar" foo<6>bar
18 "foo\06bar" SyntaxError
19 "foo\07bar" foo<7>bar
19 "foo\07bar" SyntaxError
20 "foo\08bar" foo<0>8bar
20 "foo\08bar" foo<0>8bar
21 "foo\09bar" foo<0>9bar
21 "foo\09bar" foo<0>9bar
22 "foo\70bar" foo8bar
22 "foo\70bar" SyntaxError
23 "foo\71bar" foo9bar
23 "foo\71bar" SyntaxError
24 "foo\72bar" foo:bar
24 "foo\72bar" SyntaxError
25 "foo\73bar" foo;bar
25 "foo\73bar" SyntaxError
26 "foo\74bar" foo<bar
26 "foo\74bar" SyntaxError
27 "foo\75bar" foo=bar
27 "foo\75bar" SyntaxError
28 "foo\76bar" foo>bar
28 "foo\76bar" SyntaxError
29 "foo\77bar" foo?bar
29 "foo\77bar" SyntaxError
30 "foo\78bar" foo<7>8bar
30 "foo\78bar" SyntaxError
31 "foo\79bar" foo<7>9bar
31 "foo\79bar" SyntaxError
32 "foo\070bar" foo8bar
32 "foo\070bar" SyntaxError
33 "foo\071bar" foo9bar
33 "foo\071bar" SyntaxError
34 "foo\072bar" foo:bar
34 "foo\072bar" SyntaxError
35 "foo\073bar" foo;bar
35 "foo\073bar" SyntaxError
36 "foo\074bar" foo<bar
36 "foo\074bar" SyntaxError
37 "foo\075bar" foo=bar
37 "foo\075bar" SyntaxError
38 "foo\076bar" foo>bar
38 "foo\076bar" SyntaxError
39 "foo\077bar" foo?bar
39 "foo\077bar" SyntaxError
40 "foo\078bar" foo<7>8bar
40 "foo\078bar" SyntaxError
41 "foo\079bar" foo<7>9bar
41 "foo\079bar" SyntaxError
42 "foo\370bar" foo<f8>bar
42 "foo\370bar" SyntaxError
43 "foo\371bar" foo<f9>bar
43 "foo\371bar" SyntaxError
44 "foo\372bar" foo<fa>bar
44 "foo\372bar" SyntaxError
45 "foo\373bar" foo<fb>bar
45 "foo\373bar" SyntaxError
46 "foo\374bar" foo<fc>bar
46 "foo\374bar" SyntaxError
47 "foo\375bar" foo<fd>bar
47 "foo\375bar" SyntaxError
48 "foo\376bar" foo<fe>bar
48 "foo\376bar" SyntaxError
49 "foo\377bar" foo<ff>bar
49 "foo\377bar" SyntaxError
50 "foo\378bar" foo<1f>8bar
50 "foo\378bar" SyntaxError
51 "foo\379bar" foo<1f>9bar
51 "foo\379bar" SyntaxError
52 "foo\400bar" foo 0bar
52 "foo\400bar" SyntaxError
53 "foo\3771bar" foo<ff>1bar
53 "foo\3771bar" SyntaxError
===*/

// Escape length is 1 to 3 digits.  Fourth digit is literal.
// For 3-digit form the range is \000 to \377, first digit
// cannot be 4.
[
    '"foo\\0bar"',
    '"foo\\1bar"',
    '"foo\\2bar"',
    '"foo\\3bar"',
    '"foo\\4bar"',
    '"foo\\5bar"',
    '"foo\\6bar"',
    '"foo\\7bar"',
    '"foo\\8bar"',
    '"foo\\80bar"',
    '"foo\\9bar"',
    '"foo\\90bar"',

    '"foo\\00bar"',
    '"foo\\01bar"',
    '"foo\\02bar"',
    '"foo\\03bar"',
    '"foo\\04bar"',
    '"foo\\05bar"',
    '"foo\\06bar"',
    '"foo\\07bar"',
    '"foo\\08bar"',
    '"foo\\09bar"',
    '"foo\\70bar"',
    '"foo\\71bar"',
    '"foo\\72bar"',
    '"foo\\73bar"',
    '"foo\\74bar"',
    '"foo\\75bar"',
    '"foo\\76bar"',
    '"foo\\77bar"',
    '"foo\\78bar"',
    '"foo\\79bar"',

    '"foo\\070bar"',
    '"foo\\071bar"',
    '"foo\\072bar"',
    '"foo\\073bar"',
    '"foo\\074bar"',
    '"foo\\075bar"',
    '"foo\\076bar"',
    '"foo\\077bar"',
    '"foo\\078bar"',
    '"foo\\079bar"',
    '"foo\\370bar"',
    '"foo\\371bar"',
    '"foo\\372bar"',
    '"foo\\373bar"',
    '"foo\\374bar"',
    '"foo\\375bar"',
    '"foo\\376bar"',
    '"foo\\377bar"',
    '"foo\\378bar"',
    '"foo\\379bar"',

    '"foo\\400bar"',

    '"foo\\3771bar"',
].forEach(function (v, i) {
    try {
        print(i, v, safeString(eval(v)));
    } catch (e) {
        print(i, v, e.name);
    }
    try {
        print(i, v, safeString(eval('"use strict"; ' + v)));
    } catch (e) {
        print(i, v, e.name);
    }
});

/*===
SyntaxError
foo<0>bar
===*/

// Not allowed in strict mode.  However, '\0' is allowed in strict mode.
try {
    safePrint(eval('(function () { "use strict"; return "foo\\101bar"; })()'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('(function () { "use strict"; return "foo\0bar"; })()'));
} catch (e) {
    print(e.name);
}

/*===
8
8
9
9
===*/

// Behavior for \8 and \9 is interesting.  Reading ES2015 and ES2016, they
// should never be allowed: NonEscapeCharacter excludes EscapeCharacter, which
// includes all digits (\0 to \9); \0 has a specific upper level rule to
// allow it.  However, both Spidermonkey and V8 accept \8 and \9 as literal
// '8' and '9', even in strict mode.  Test for that behavior.

try {
    safePrint(eval('(function () { return "\\8"; })()'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('(function () { "use strict"; return "\\8"; })()'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('(function () { return "\\9"; })()'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('(function () { "use strict"; return "\\9"; })()'));
} catch (e) {
    print(e.name);
}

/*===
foo<0>bar
foo<0>bar
foo<0>bar
foo<0>0bar
foo<0>bar
SyntaxError
SyntaxError
SyntaxError
===*/

// If two or three zero digits are used in non-strict mode, it is interpreted
// as a single octal escape.  Four zero digits are treated as a single \000
// followed by a literal 0.
try {
    safePrint(eval('"foo\\0bar"'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('"foo\\00bar"'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('"foo\\000bar"'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('"foo\\0000bar"'));
} catch (e) {
    print(e.name);
}

// Strict mode rejects all of these except for a single \0.
try {
    safePrint(eval('(function () { "use strict"; return "foo\\0bar"; })()'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('(function () { "use strict"; return "foo\\00bar"; })()'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('(function () { "use strict"; return "foo\\000bar"; })()'));
} catch (e) {
    print(e.name);
}
try {
    safePrint(eval('(function () { "use strict"; return "foo\\0000bar"; })()'));
} catch (e) {
    print(e.name);
}

/*===
foo<0>8bar
===*/

// Another octal test.
try {
    safePrint(eval('"foo\\008bar"'));
} catch (e) {
    print(e.name);
}
