/*
 *  Some old cases from doc/compiler.txt.
 *
 *  Note: the case for 'f(3)' is duplicated on purpose.
 */

function f(x) { print("f() called with x=" + x); return x; }

/*===
f() called with x=1
f() called with x=2
f() called with x=3
f() called with x=102
f() called with x=103
f() called with x=104
104
===*/

/* Basic case without default clauses, match in the middle */

print(eval("switch (3) {\n" +
           "  case f(1): f(100);\n" +
           "  case f(2): f(101);\n" +
           "  case f(3): f(102);\n" +
           "  case f(4): f(103);\n" +
           "  case f(3): f(104);\n" +
           "}"));

/*===
f() called with x=1
f() called with x=2
f() called with x=3
f() called with x=102
f() called with x=103
f() called with x=104
104
===*/

/* Default clause with a match after the default clause */

print(eval("switch (3) {\n" +
           "  case f(1): f(100);\n" +
           "  case f(2): f(101);\n" +
           "  default:\n" +
           "  case f(3): f(102);\n" +
           "  case f(4): f(103);\n" +
           "  case f(3): f(104);\n" +
           "}"));

/*===
f() called with x=1
f() called with x=2
f() called with x=101
f() called with x=102
f() called with x=103
f() called with x=104
104
===*/

/* Default clause, with match before default clause */

print(eval("switch (2) {\n" +
           "  case f(1): f(100);\n" +
           "  case f(2): f(101);\n" +
           "  default:\n" +
           "  case f(3): f(102);\n" +
           "  case f(4): f(103);\n" +
           "  case f(3): f(104);\n" +
           "}"));

/*===
f() called with x=1
f() called with x=2
f() called with x=3
f() called with x=4
f() called with x=3
f() called with x=102
f() called with x=103
f() called with x=104
104
===*/

/* Default clause without a match:: */

print(eval("switch (5) {\n" +
           "  case f(1): f(100);\n" +
           "  case f(2): f(101);\n" +
           "  default: \n" +
           "  case f(3): f(102);\n" +
           "  case f(4): f(103);\n" +
           "  case f(3): f(104);\n" +
           "}"));
