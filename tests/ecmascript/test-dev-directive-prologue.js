/*
 *  Various directive prologue tests.
 *
 *  How to detect whether strict mode is enabled?  This can be done
 *  through any strict mode feature.  One very simple one is this:
 *  strict functions don't get a 'this' binding when invoked, whereas
 *  'this' binding for non-strict functions is the global object.
 *
 *  Thus, the following prints true when executed inside a strict mode
 *  function:
 *
 *    print(!this);
 *
 */

/*===
false
true
false
true
false
true
===*/

/* Basic test for strict mode directive. */

function f_nostrict() {
    print(!this);
}

function f_strict() {
    'use strict';
    print(!this);
}

function f_nostrict_escape() {
    /* escaped characters are significant for directives: the expression
     * below is a directive, but not a valid 'use strict' directive.
     */
    'use\u0020strict';
    print(!this);
}

function f_strict_escape() {
    /* the first directive is ignored, the second is still processed (we're
     * still in the directive prologue).
     */
    'use\u0020strict';  // not a 'use strict' directive, but still a valid directive
    'use strict';       // but this is
    print(!this);
}

function f_nostrict_plain_string_only() {
    /* to be part of the directive prologue, the expression must be a plain
     * string with no parens etc.
     */
    ('use strict');  // not a valid 'use strict' directive, terminates prologue
    'use strict';    // no longer a 'use strict' directive, not inside prologue
    print(!this);
}

function f_strict_comments_ok() {
    /* comments and whitespace don't affect directive prologue parsing */
    'use strict' /* comment */;
    print(!this);
}

f_nostrict();
f_strict();
f_nostrict_escape();
f_strict_escape();
f_nostrict_plain_string_only();
f_strict_comments_ok();

/*===
false
===*/

/* This was broken at some point: unless the first statement parsing had
 * finished, the "in directive prologue" flag would be incorrectly set
 * for a nested statement.
 */

function f_nostrict_nested() {
    try {
        'use strict';  // should have no effect
    } catch (e) {
    }

    print(!this);
}

f_nostrict_nested();

/*===
use strict
use strict
===*/

/* Although directives are special, they are still valid expression statements
 * and must generate an implicit return value for e.g. eval().
 */

try {
    print(eval("'use strict';"));
    print(eval("'use strict'; var x = 10;"));  // var statement is empty, no effect
} catch (e) {
    print(e.name);
}
