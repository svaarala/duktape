/*===
A
B
done
===*/

print('A');
Object.defineProperty(Array.prototype, 0, { set: function () {} });
print('B');
eval("function\u0009\u2029w(\u000c)\u00a0{\u000d};");
print('done');
