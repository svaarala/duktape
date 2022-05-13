/*---
{
    "custom": true
}
---*/

/*===
test 1000
1000 true
1000 false
test 10000
10000 true
10000 RangeError
test 10001
10001 RangeError
test 10002
10002 RangeError
done
===*/

function mkObj(n) {
    var res = { foo: 123 };
    while (--n > 0) {
        res = Object.create(res);
    }
    return res;
}

function test(n) {
    print('test', n);
    try {
        var obj = mkObj(n);
        print(n, 'foo' in obj);
        print(n, 'bar' in obj);
    } catch (e) {
        print(n, e.name);
    }
}

test(1000);
test(10000);
test(10001);
test(10002);
//test(1000000);
print('done');
