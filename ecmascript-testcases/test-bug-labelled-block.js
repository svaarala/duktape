/*
 *  Bug testcase for a labelled block statement bug found in test262 test
 *  case:
 *
 *    ch12/12.6/12.6.1/S12.6.1_A4_T5
 *    ch12/12.6/12.6.2/S12.6.2_A4_T5
 */

/*===
loop 1 part A
loop 1 part B
loop 1 body
loop 2 part A
loop 2 part B
loop 2 body
after loop 2
===*/

function labelledBlockTest() {
    label1:
    for (print("loop 1 part A"); print("loop 1 part B"), true; print("loop 1 part C")) {
        print("loop 1 body");
        break label1;
    }

    /* This should work the same as 'label1', but Duktape 0.9.0 fails with
     * an INVALID opcde after printing 'loop 2 body' when handling 'break label2'.
     */
    label2:
    {
        for (print("loop 2 part A"); print("loop 2 part B"), true; print("loop 2 part C")) {
            print("loop 2 body");
            break label2;
        }
    }

    print("after loop 2");
}

try {
    labelledBlockTest();
} catch (e) {
    print(e);
}
