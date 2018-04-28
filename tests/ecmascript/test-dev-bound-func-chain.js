/*
 *  Test bound function chains, related to changing the internal implementation
 *  to "collapse" bound function chains.
 */

/*===
F() bound foo
object this-F
string foo
undefined undefined
undefined undefined
undefined undefined
G() bound bound foo
object this-F
string foo
string bar
string quux
undefined undefined
H() bound bound bound foo
object this-F
string foo
string bar
string quux
string baz
I() bound bound bound foo
object this-F
string foo
string bar
string quux
number 123
F() bound max
-Infinity
G() bound bound max
3
H() bound bound bound max
4
I() bound bound bound bound max
5
bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound bound foo
object this-0
100
0 string arg-0
1 string arg-1
2 string arg-2
3 string arg-3
4 string arg-4
5 string arg-5
6 string arg-6
7 string arg-7
8 string arg-8
9 string arg-9
10 string arg-10
11 string arg-11
12 string arg-12
13 string arg-13
14 string arg-14
15 string arg-15
16 string arg-16
17 string arg-17
18 string arg-18
19 string arg-19
20 string arg-20
21 string arg-21
22 string arg-22
23 string arg-23
24 string arg-24
25 string arg-25
26 string arg-26
27 string arg-27
28 string arg-28
29 string arg-29
30 string arg-30
31 string arg-31
32 string arg-32
33 string arg-33
34 string arg-34
35 string arg-35
36 string arg-36
37 string arg-37
38 string arg-38
39 string arg-39
40 string arg-40
41 string arg-41
42 string arg-42
43 string arg-43
44 string arg-44
45 string arg-45
46 string arg-46
47 string arg-47
48 string arg-48
49 string arg-49
50 string arg-50
51 string arg-51
52 string arg-52
53 string arg-53
54 string arg-54
55 string arg-55
56 string arg-56
57 string arg-57
58 string arg-58
59 string arg-59
60 string arg-60
61 string arg-61
62 string arg-62
63 string arg-63
64 string arg-64
65 string arg-65
66 string arg-66
67 string arg-67
68 string arg-68
69 string arg-69
70 string arg-70
71 string arg-71
72 string arg-72
73 string arg-73
74 string arg-74
75 string arg-75
76 string arg-76
77 string arg-77
78 string arg-78
79 string arg-79
80 string arg-80
81 string arg-81
82 string arg-82
83 string arg-83
84 string arg-84
85 string arg-85
86 string arg-86
87 string arg-87
88 string arg-88
89 string arg-89
90 string arg-90
91 string arg-91
92 string arg-92
93 string arg-93
94 string arg-94
95 string arg-95
96 string arg-96
97 string arg-97
98 string arg-98
99 string arg-99
===*/

function test() {
    var func;
    var F, G, H, I;

    // Final function is an ECMAScript function.

    func = function foo(a, b, c, d) {
        print(typeof this, this);
        print(typeof a, a);
        print(typeof b, b);
        print(typeof c, c);
        print(typeof d, d);
    };
    F = func.bind('this-F', 'foo');
    G = F.bind('this-G', 'bar', 'quux');
    H = G.bind('this-H', 'baz', 'quuux');
    I = G.bind('this-I', 123, 234);  // both H and I bind via G

    print('F()', F.name);
    F();
    print('G()', G.name);
    G();
    print('H()', H.name);
    H();
    print('I()', I.name);
    I();

    // Final function is a native function.

    func = Math.max;
    F = func.bind(null);
    G = F.bind(null, 3);
    H = G.bind(null, 4);
    I = H.bind(null, 5);

    print('F()', F.name);
    print(F());
    print('G()', G.name);
    print(G());
    print('H()', H.name);
    print(H());
    print('I()', I.name);
    print(I());

    // Lightfunc final target needs testing too; it is covered by Math.max()
    // if DUK_USE_LIGHTFUNC_BUILTINS is enabled.

    // Long chain.

    func = function foo() {
        print(typeof this, this);
        print(arguments.length);
        for (var i = 0; i < arguments.length; i++) {
            print(i, typeof arguments[i], arguments[i]);
        }
    };

    for (var i = 0; i < 100; i++) {
        func = func.bind('this-' + i, 'arg-' + i);
    }
    print(func.name);
    func();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
