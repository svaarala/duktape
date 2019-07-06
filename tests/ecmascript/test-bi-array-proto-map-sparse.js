/*===
10000001
10000001
25
call count: 1
done
===*/

try {
    var callCount = 0;
    var a = []; a[1e7] = 5;
    print(a.length);
    var b = a.map(function (v) { callCount++; return v*v; });
    print(b.length);
    print(b[1e7]);
    print('call count: ' + callCount);
} catch (e) {
    print(e.stack || e);
}

print('done');
