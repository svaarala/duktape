/* FIXME: case clauses after default */
/* FIXME: multiple fall through cases */

/*===
third case matches
default matches
===*/

/* Case matching goes through a default if it is in the middle. */

switch (3) {
    case 1:
        print("first case matches");
        break;
    default:
        print("default matches");
        break;
    case 2:
        print("second case matches");
        break;
    case 3:
        print("third case matches");
        break;
}

/* Case matching goes through a default if it is in the middle,
 * then ends up in the default if nothing matches.
 */

switch (4) {
    case 1:
        print("first case matches");
        break;
    default:
        print("default matches");
        break;
    case 2:
        print("second case matches");
        break;
    case 3:
        print("third case matches");
        break;
}

/*===
first case matches
default matches
second case matches
third case matches
===*/

/* Case fall-throughs will execute default clause statements if it is
 * in the middle, just as normal case clauses would get executed.
 */

switch (1) {
    case 1:
        print("first case matches");
    default:
        print("default matches");
    case 2:
        print("second case matches");
    case 3:
        print("third case matches");
}

/*===
default matches
second case matches
third case matches
===*/

/* If default matches, it will fall through any case clauses following it. */

switch (4) {
    case 1:
        print("first case matches");
    default:
        print("default matches");
    case 2:
        print("second case matches");
    case 3:
        print("third case matches");
}

/*===
default matches
===*/

/* Implementation corner case: only default clause exists. */

switch (1) {
    default:
        print("default matches");
        break;
}

/*===
out of switch
===*/

/* Implementation corner case: no case clauses and no default clause. */

switch (1) {
}
print("out of switch");

/*===
third case matches
===*/

/* Switch case values may be arbitrary expressions.  Here the
 * 'foo' case would throw a ReferenceError, but is not reached.
 */

switch (3) {
   case 1:
       print("first case matches");
       break;
   case 2:
       print("second case matches");
       break;
   case 1+2:
       print("third case matches");
       break;
   case foo:  // ReferenceError
       print("fourth case matches");
       break;
}

/*===
ReferenceError
===*/

/* Here, the matching case is never found because the ReferenceError occurs
 * before the matching case.
 */

try {
    /* Here 'foo' throws a ReferenceError before the matching case is found. */
    eval("switch (3) {" +
         "case 1: print('first case matches'); break;" +
         "case 2: print('second case matches'); break;" +
         "case foo: " +  // ReferenceError
         "case 3: print('fourth case matches'); break;" +
         "default: print('default case matches'); break;" +
         "}");
} catch (e) {
    print(e.name);
}

/*===
first case matches
===*/

/* Multiple cases with the same value.  Only the first matches. */

switch (1) {
    case 1:
        print("first case matches");
        break;
    case 1:
        print("second case matches");
        break;
    default:
        print("default case matches");
        break;
}

/*===
SyntaxError
===*/

/* Multiple 'default' cases are a SyntaxError. */

try {
    eval("switch (1) {" +
         "case 1: print('first case matches'); break;" +
         "default: print('first default case matches'); break;" +
         "default: print('second default case matches'); break;" +
         "}");
} catch (e) {
    print(e.name);
}
