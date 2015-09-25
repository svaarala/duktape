/*===
50
===*/

function test() {
    var x = 10;
    var y = 20;

    // In Duktape 1.3.0 and prior this incorrect compiles to:
    //
    //     LDREG      r0, r1      ; x <- y  -- here 'y' is still 20
    //     LDINT      r4, 30      ; temp <- 30
    //     LDREG      r1, r4      ; y <- temp
    //     ADD        r4, r1, r4  ; temp <- y (=30) + temp (=30) --> 60
    //
    // Correct alternative is, for example:
    //
    //     LDREG      r4, r1      ; temp <- y
    //     LDREG      r0, r4      ; x <- temp
    //     LDCONST    r1, c1      ; y <- 30
    //     ADD        r4, r4, c1  ; temp <- temp (=20) + 30 --> 50
    //
    // For this to happen in Duktape 1.3.0 and prior: (1) the RHS value must
    // be a register bound variable, (2) it must appear as part of the same
    // expression, and (3) be mutated during the expression.
    //
    // Conceptually the correct behavior is for the expression result to be a
    // fresh temporary (which is never changed).  A constant value is also OK
    // as an expression value because it never changes.  A register bound
    // variable is OK as the result expression value as long as the variable
    // value is not mutated during the rest of the expression; this is the case
    // e.g. for a "top level" assignment like "x = 123;".

    print( (x = y) + (y = 30) );
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
