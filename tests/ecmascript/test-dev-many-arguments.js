/*
 *  Test functions with many arguments.  In Duktape 1.0, 100 arguments are
 *  expected to work, 1000 arguments are not (this is a custom limit).  The
 *  actual limit is somewhere in the middle.
 */

/* Marked custom because of a custom internal limit */
/*---
{
    "custom": true
}
---*/

/*===
arg0 my-arg-0
success for: 1
arg0 my-arg-0
arg1 my-arg-1
arg2 my-arg-2
arg3 my-arg-3
arg4 my-arg-4
arg5 my-arg-5
arg6 my-arg-6
arg7 my-arg-7
arg8 my-arg-8
arg9 my-arg-9
success for: 10
arg0 my-arg-0
arg1 my-arg-1
arg2 my-arg-2
arg3 my-arg-3
arg4 my-arg-4
arg5 my-arg-5
arg6 my-arg-6
arg7 my-arg-7
arg8 my-arg-8
arg9 my-arg-9
arg10 my-arg-10
arg11 my-arg-11
arg12 my-arg-12
arg13 my-arg-13
arg14 my-arg-14
arg15 my-arg-15
arg16 my-arg-16
arg17 my-arg-17
arg18 my-arg-18
arg19 my-arg-19
arg20 my-arg-20
arg21 my-arg-21
arg22 my-arg-22
arg23 my-arg-23
arg24 my-arg-24
arg25 my-arg-25
arg26 my-arg-26
arg27 my-arg-27
arg28 my-arg-28
arg29 my-arg-29
arg30 my-arg-30
arg31 my-arg-31
arg32 my-arg-32
arg33 my-arg-33
arg34 my-arg-34
arg35 my-arg-35
arg36 my-arg-36
arg37 my-arg-37
arg38 my-arg-38
arg39 my-arg-39
arg40 my-arg-40
arg41 my-arg-41
arg42 my-arg-42
arg43 my-arg-43
arg44 my-arg-44
arg45 my-arg-45
arg46 my-arg-46
arg47 my-arg-47
arg48 my-arg-48
arg49 my-arg-49
arg50 my-arg-50
arg51 my-arg-51
arg52 my-arg-52
arg53 my-arg-53
arg54 my-arg-54
arg55 my-arg-55
arg56 my-arg-56
arg57 my-arg-57
arg58 my-arg-58
arg59 my-arg-59
arg60 my-arg-60
arg61 my-arg-61
arg62 my-arg-62
arg63 my-arg-63
arg64 my-arg-64
arg65 my-arg-65
arg66 my-arg-66
arg67 my-arg-67
arg68 my-arg-68
arg69 my-arg-69
arg70 my-arg-70
arg71 my-arg-71
arg72 my-arg-72
arg73 my-arg-73
arg74 my-arg-74
arg75 my-arg-75
arg76 my-arg-76
arg77 my-arg-77
arg78 my-arg-78
arg79 my-arg-79
arg80 my-arg-80
arg81 my-arg-81
arg82 my-arg-82
arg83 my-arg-83
arg84 my-arg-84
arg85 my-arg-85
arg86 my-arg-86
arg87 my-arg-87
arg88 my-arg-88
arg89 my-arg-89
arg90 my-arg-90
arg91 my-arg-91
arg92 my-arg-92
arg93 my-arg-93
arg94 my-arg-94
arg95 my-arg-95
arg96 my-arg-96
arg97 my-arg-97
arg98 my-arg-98
arg99 my-arg-99
success for: 100
failure for: 1000 -> RangeError
===*/

function createSource(n) {
    var res = [];
    var i;

    res.push('(function manyargs(');
    for (i = 0; i < n; i++) {
        if (i > 0) { res.push(','); }
        res.push('arg' + i);
    }
    res.push(') {');
    for (i = 0; i < n; i++) {
        res.push('print("arg' + i + '", arg' + i + ');');
    }
    res.push('})');
    res.push('(');
    for (i = 0; i < n; i++) {
        if (i > 0) { res.push(','); }
        res.push('"my-arg-' + i + '"');
    }
    res.push(')');
    return res.join('');
}

function wrappedTest(n) {
    var src = createSource(n);
    try {
        eval(src);
        print('success for:', n);
    } catch (e) {
        print('failure for:', n, '->', e.name);
    }
}

function test() {
    wrappedTest(1);
    wrappedTest(10);
    wrappedTest(100);
    wrappedTest(1000);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
