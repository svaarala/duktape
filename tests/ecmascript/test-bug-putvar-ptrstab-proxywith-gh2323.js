/*===
done
===*/

function main() {
    var v3 = [2,2,2,2,2,2,2,2,2];
    var v4 = v3;
    var v5 = {constructor:String,call:String,apply:String,get:String,construct:String,defineProperty:String,has:String,preventExtensions:String};
    v3.length = 1343;
    var v7 = new Proxy(v4,v5);
    with (v7) {
        var v9 = 0;
    }
}
try {
    main();
} catch (e) {
    print(e.stack || e);
}
print('done');
