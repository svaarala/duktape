/*
 *  Special identifier names.  Cover special (unusual) cases of
 *  E5 Section 7.6.  Also check for identifier names that must be
 *  rejected.
 */

/*===
1 number
2 number
2 number
===*/

var $ = 1;
print($, typeof $);

$ = { $: 2, _: print };
$._($.$, typeof $.$);
\u0024.\u005f(\u0024.\u0024, typeof \u0024.\u0024);

/*===
SyntaxError
SyntaxError
===*/

try {
    eval('var # = 1; print(#);');
} catch (e) {
    print(e.name);
}

try {
    /* U+0023 = '#', note that Rhino allows this while it fails the above test */
    eval('var \\u0023 = 1; print(\\u0023);');
} catch (e) {
    print(e.name);
}

/* XXX: add more tests */
