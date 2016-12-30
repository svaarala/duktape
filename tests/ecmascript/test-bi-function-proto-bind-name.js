/*
 *  ES2015 19.2.3.2 specifies how a bound function .name is set.
 */

/*===
"bound myFunc"
"bound "
"bound bound bound cos"
===*/

function test() {
    var f = function myFunc() {};
    var g = (1, 2, function () {});  // anonymous

    // Name becomes 'bound ' + name property.
    var bound_f = f.bind();
    print(JSON.stringify(bound_f.name));

    // If name property is missing or not a string, empty string is
    // used and the result is 'bound '.  A non-string name is covered
    // by an API testcase which can force the name using duk_def_prop().
    var bound_g = g.bind();
    print(JSON.stringify(bound_g.name));

    // The bound prefix multiplies.
    print(JSON.stringify(Math.cos.bind().bind().bind().name));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
