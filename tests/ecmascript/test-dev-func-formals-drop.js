/*
 *  Some internal test coverage for https://github.com/svaarala/duktape/pull/1141.
 *
 *  This should be executed with debugger support disabled: otherwise _Formals
 *  is always kept.
 */

/*---
{
    "custom": true
}
---*/

/*===
3
foo bar quux
240
arg239: aiee
4
object
5
inner
3
aiee
===*/

function test() {
    var func;

    // Base case: _Formals is not needed, and nargs matches _Formals.length.
    // Closure .length is taken from nargs.

    func = function foo(a,b,c) { print(a,b,c); };
    print(func.length);
    func('foo', 'bar', 'quux');

    // Special case: enough many formal arguments -> _Formals is longer than
    // nargs.  This could happen in theory -- but currently there's no such
    // limit in the compiler.  So just test for a large argument count.

    func = function foo(
        arg000, arg001, arg002, arg003, arg004, arg005, arg006, arg007,
        arg008, arg009, arg010, arg011, arg012, arg013, arg014, arg015,
        arg016, arg017, arg018, arg019, arg020, arg021, arg022, arg023,
        arg024, arg025, arg026, arg027, arg028, arg029, arg030, arg031,
        arg032, arg033, arg034, arg035, arg036, arg037, arg038, arg039,
        arg040, arg041, arg042, arg043, arg044, arg045, arg046, arg047,
        arg048, arg049, arg050, arg051, arg052, arg053, arg054, arg055,
        arg056, arg057, arg058, arg059, arg060, arg061, arg062, arg063,
        arg064, arg065, arg066, arg067, arg068, arg069, arg070, arg071,
        arg072, arg073, arg074, arg075, arg076, arg077, arg078, arg079,
        arg080, arg081, arg082, arg083, arg084, arg085, arg086, arg087,
        arg088, arg089, arg090, arg091, arg092, arg093, arg094, arg095,
        arg096, arg097, arg098, arg099, arg100, arg101, arg102, arg103,
        arg104, arg105, arg106, arg107, arg108, arg109, arg110, arg111,
        arg112, arg113, arg114, arg115, arg116, arg117, arg118, arg119,
        arg120, arg121, arg122, arg123, arg124, arg125, arg126, arg127,
        arg128, arg129, arg130, arg131, arg132, arg133, arg134, arg135,
        arg136, arg137, arg138, arg139, arg140, arg141, arg142, arg143,
        arg144, arg145, arg146, arg147, arg148, arg149, arg150, arg151,
        arg152, arg153, arg154, arg155, arg156, arg157, arg158, arg159,
        arg160, arg161, arg162, arg163, arg164, arg165, arg166, arg167,
        arg168, arg169, arg170, arg171, arg172, arg173, arg174, arg175,
        arg176, arg177, arg178, arg179, arg180, arg181, arg182, arg183,
        arg184, arg185, arg186, arg187, arg188, arg189, arg190, arg191,
        arg192, arg193, arg194, arg195, arg196, arg197, arg198, arg199,
        arg200, arg201, arg202, arg203, arg204, arg205, arg206, arg207,
        arg208, arg209, arg210, arg211, arg212, arg213, arg214, arg215,
        arg216, arg217, arg218, arg219, arg220, arg221, arg222, arg223,
        arg224, arg225, arg226, arg227, arg228, arg229, arg230, arg231,
        arg232, arg233, arg234, arg235, arg236, arg237, arg238, arg239
    ) {
        print('arg239:', arg239);
    };
    print(func.length);
    // A direct call attempt hits a register limit, so use indirect call.
    func.apply(null, [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 'aiee'
    ]);

    // Function accessing arguments -> causes _Formals to be kept.
    func = function (a,b,c,d) { print(typeof arguments); };
    print(func.length);
    func();

    // Function with an inner function -> causes _Formals to be kept.
    func = function (a,b,c,d,e) { function inner() { print('inner') }; inner(); };
    print(func.length);
    func();

    // Function may do a direct eval -> causes _Formals to be kept
    // (because the eval may access 'arguments').
    func = function (a,b,c) { print(eval('"aiee"')); };
    print(func.length);
    func();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
