/*===
done
===*/

try {
    var v3 = "c".repeat(536870912);
    var v4 = "base64".endsWith(v3);
} catch (e) {
    print(e.stack || e);
}
print('done');
