/*
 *  Many temporaries at the same time.
 *
 *  Intent is to ensure temporaries are exhausted from the easily accessible
 *  range, and spilling is required to handle the expression correctly.
 *  Parentheses are required to ensure temporaries are truly needed.
 */

/* NOTE: Rhino croaks, but Smjs computes correctly. */

/*===
603
===*/

var x = 1;
var y = 2;

/* result = (1 + 20*10) * (x + y) = 201 * 3 = 603 */
var z = x +

    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +

    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +
    (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x + (y + (x +

    y

    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))

    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))
    )) )) )) )) )) )) )) )) )) ))

;

print(z);
