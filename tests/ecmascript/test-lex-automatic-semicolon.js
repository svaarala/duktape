/*
 *  Automatic semicolon insertion (E5 Section 7.9).
 *
 *  Note: depending on implementation a few tests will cover behavior,
 *  or insertion must be tested in all possible syntactic constructs.
 */

/*===
10
14
===*/

function f_basic1() {
    var x

    // parsed as "x = 1 + 2 + 3 + 4;"
    x = 1 + 2 +
        3 + 4
    print(x)

    // parsed as "x = 2 + 3 + 4 + 5;", because must parse longest
    // possible valid expression
    x = 2 + 3
        + 4 + 5
    print(x)
}

f_basic1();

/*===
123
===*/

// semicolon inserted at end of block, offending token is '}'
// rule 1 of E5 Section 7.9.1
function f_basic2() { return 123 }

print(f_basic2());

/*===
234
===*/

// semicolon inserted at end of input
// rule 2 of E5 Section 7.9.1
function f_basic3() { return eval("234"); }

print(f_basic3());

/*===
10 20
-> 10 21
-> 11 21
10 20
-> 10 19
-> 9 19
return from inner
return from inner
undefined
SyntaxError
===*/

// semicolon inserted in the middle of a restricted
// production (even if continuing was possible syntactically)
// rule 3 of E5 Section 7.9.1

var x = 10;
var y = 20;

function f_postinc() {
    print(x, y);

    // interpreted as "x; ++y;"
    x
    ++y

    print('->', x, y);

    // interpreted as "x++; y;"
    x++
    y

    print('->', x, y);
}

function f_postdec() {
    print(x, y);

    // interpreted as "x; --y;"
    x
    --y

    print('->', x, y);

    // interpreted as "x--; y;"
    x--
    y

    print('->', x, y);
}

function f_continue() {
	var done = false;

	outer:
	for (;;) {
		if (done) {
			print('return from outer');
			return;
		}

		for (;;) {
			if (done) {
				print('return from inner');
				return;
			}
			done = true;

			// interpreted as 'continue; outer;'
			// continues inner loop -> exits from inner
			// ('continue outer' would exit from outer)
			continue
			outer
		}
	}
}

function f_break() {
	outer:
	for (;;) {
		for (;;) {
			// interpreted as 'break: outer'
			break
			outer
		}

		print('return from inner');
		return;
	}

	print('return from outer');
}

function f_return() {
    /* interpreted as "return; 123;" */
    return
    123;
}

/* Note: there is no empty variant for 'throw' (unlike for return, break,
 * and continue), so a line break between throw and its value is a
 * SyntaxError.
 */
var src_throw = "function() {\n" +
                "    try {\n" +
                "        // interpreted as 'throw; 123' instead of 'throw 123'\n" +
                "        throw\n" +
                "        123\n" +
                "    } catch(e) {\n" +
                "        print(e);\n" +
                "    }\n" +
                "}";

try {
    x = 10; y = 20; f_postinc();
} catch (e) {
    print(e.name);
}

try {
    x = 10; y = 20; f_postdec();
} catch (e) {
    print(e.name);
}

try {
    f_continue();
} catch (e) {
    print(e.name);
}

try {
    f_break();
} catch (e) {
    print(e.name);
}

try {
    print(f_return());
} catch (e) {
    print(e.name);
}

try {
    var f_throw = eval(src_throw);
    f_throw();
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
SyntaxError
===*/

/* A specific post-increment/decrement case; e.g. "a+b\n++" must be
 * interpreted as "a+b;++;" which is a SyntaxError.
 */

try {
    eval("a+b\n++");
} catch (e) {
    print(e.name);
}

try {
    eval("a+b\n--");
} catch (e) {
    print(e.name);
}

/* XXX: check for completeness */
