/*
 *  Regexp vs. division parsing bug repro.
 */

/*===
67
5
5
5.05
4.95
0.000004166666666666667
0.000004166666666666667
0.000004163077043181538
0.0000041700190039138786
0.000004163232000266447
0.5
SyntaxError
/foo/
done
===*/

try {
    // Original issue found in the wild.
    print(eval('z = 0; [67,69,71][0|z++/20]'));

    // Coverage for similar issues.
    print(eval('z = 100; z++/20'));
    print(eval('z = 100; z--/20'));
    print(eval('z = 100; ++z/20'));
    print(eval('z = 100; --z/20'));

    // Other related tests.
    print(eval('x = 100; y = 200; z = 300; w = 400; x++/y++/z++/w++;'));
    print(eval('x = 100; y = 200; z = 300; w = 400; x--/y--/z--/w--;'));
    print(eval('x = 100; y = 200; z = 300; w = 400; ++x/++y/++z/++w;'));
    print(eval('x = 100; y = 200; z = 300; w = 400; --x/--y/--z/--w;'));
    print(eval('x = 100; y = 200; z = 300; w = 400; x++/y--/++z/--w;'));
    print(eval('x = 100; y = 200; x++/y;'));
    try {
        print(String(eval('x = 100; y = 200; x++\n/foo/')));
    } catch (e) {
        print(e.name);
    }
    try {
        print(String(eval('x = 100; y = 200; x++;\n/foo/')));
    } catch (e) {
        print(e.name);
    }
} catch (e) {
    print(e.stack || e);
}
print('done');
