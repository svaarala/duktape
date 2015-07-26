/*
 *  For statement (E5 Section 12.6.3).
 */

/*===
basic
part A
part B
body, i: 0 count_a: 1 count_b: 1 count_c: 0
part C
part B
body, i: 1 count_a: 1 count_b: 2 count_c: 1
part C
part B
body, i: 2 count_a: 1 count_b: 3 count_c: 2
part C
part B
body, i: 3 count_a: 1 count_b: 4 count_c: 3
part C
part B
body, i: 4 count_a: 1 count_b: 5 count_c: 4
part C
part B
body, i: 5 count_a: 1 count_b: 6 count_c: 5
part C
part B
body, i: 6 count_a: 1 count_b: 7 count_c: 6
part C
part B
body, i: 7 count_a: 1 count_b: 8 count_c: 7
part C
part B
body, i: 8 count_a: 1 count_b: 9 count_c: 8
part C
part B
body, i: 9 count_a: 1 count_b: 10 count_c: 9
part C
part B
===*/

function basicTest() {
    var i;
    var count_a = 0, count_b = 0, count_c = 0;

    for (print('part A'), count_a++, i = 0;
         print('part B'), count_b++, i < 10;
         print('part C'), count_c++, i++) {
        print('body, i:', i, 'count_a:', count_a, 'count_b:', count_b, 'count_c:', count_c);
    }
}

print('basic');

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
break and continue
part A
part B
body, i: 0 count_a: 1 count_b: 1 count_c: 0
end body
part C
part B
body, i: 1 count_a: 1 count_b: 2 count_c: 1
end body
part C
part B
body, i: 2 count_a: 1 count_b: 3 count_c: 2
end body
part C
part B
body, i: 3 count_a: 1 count_b: 4 count_c: 3
end body
part C
part B
body, i: 4 count_a: 1 count_b: 5 count_c: 4
continue at 4 and 6
part C
part B
body, i: 5 count_a: 1 count_b: 6 count_c: 5
end body
part C
part B
body, i: 6 count_a: 1 count_b: 7 count_c: 6
continue at 4 and 6
part C
part B
body, i: 7 count_a: 1 count_b: 8 count_c: 7
end body
part C
part B
body, i: 8 count_a: 1 count_b: 9 count_c: 8
break at 8
===*/

/* Break and continue */

function breakContinueTest() {
    var i;
    var count_a = 0, count_b = 0, count_c = 0;

    for (print('part A'), count_a++, i = 0;
         print('part B'), count_b++, i < 10;
         print('part C'), count_c++, i++) {
        print('body, i:', i, 'count_a:', count_a, 'count_b:', count_b, 'count_c:', count_c);
        if (i == 4 || i == 6) {
            print('continue at 4 and 6');
            continue;
        }
        if (i == 8) {
            print('break at 8');
            break;
        }
        print('end body');
    }
}

print('break and continue');

try {
    breakContinueTest();
} catch (e) {
    print(e);
}


/*===
syntax errors
SyntaxError
OK
===*/

/* Some specific SyntaxError tests. */

print('syntax errors');

try {
    /* SyntaxError, because the first part of the three-part for
     * must not contain a top level 'in'.
     */
    eval("x={}; for ('foo' in x; false; false) {};");
    print("never here");
} catch (e) {
    print(e.name);
}

try {
    /* No SyntaxError because an 'in' may appear inside parenthesis */
    eval("x={}; for (('foo' in x); false; false) {};");
    print("OK");
} catch (e) {
    print(e.name);
}
