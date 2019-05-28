/*===
t1
t2
done
===*/

print('t1');
var t1 = Duktape.dec('base64', '');  // create zero-size dynamic plain buffer
print('t2');
var t2 = Duktape.dec('base64', t1);  // segfault
print('done');
