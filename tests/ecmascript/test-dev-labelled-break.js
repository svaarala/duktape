/*===
0
1
2
3
4
5
while done
0
1
2
3
4
5
while done
0
1
2
3
4
5
while done
===*/

function test1() {
    var i;
    labelled:
    do {
        for (i = 0; i < 100; i++) {
            print(i);
            if (i == 5) { break labelled; }
        }
        throw new Error('never here');
    } while(true);
    print('while done');
}

function test2() {
    var i = 0;
    labelled:
    do {
        print(i);
        if (i++ == 5) { break labelled; }
    } while(true);
    print('while done');
}

/* Here the special thing is that the labelled statement is a block,
 * not an iterator statement directly.
 */
function test3() {
    var i = 0;
    labelled:
    {
        do {
            print(i);
            if (i++ == 5) { break labelled; }
        } while(true);
        throw new Error('never here');
    }
    print('while done');
}

try {
    test1();
} catch (e) {
    print(e);
}

try {
    test2();
} catch (e) {
    print(e);
}

try {
    test3();
} catch (e) {
    print(e);
}
