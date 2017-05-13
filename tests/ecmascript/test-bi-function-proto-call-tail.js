/*===
0
100000
200000
300000
400000
500000
600000
700000
800000
900000
1000000
done
===*/

function test(count) {
    if ((count % 1e5) == 0) {
        print(count);
    }
    if (count < 1e6) {
        return test.call(null, count + 1);
    }
    return 'done';
}

try {
    print(test(0));
} catch (e) {
    print(e.stack || e);
}
