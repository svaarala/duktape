/*===
call
NaN
revoke
revoked
call
TypeError
revoke again
revoked
call
TypeError
done
===*/

var P = Proxy.revocable(Math.cos, {});
var F = P.proxy;

try {
    print('call');
    print(F());
} catch (e) {
    print(e.name);
}

print('revoke');
P.revoke();
print('revoked');

try {
    print('call');
    print(F());
} catch (e) {
    print(e.name);
    //print(e.stack);
}

print('revoke again');  // nop
P.revoke();
print('revoked');

try {
    print('call');
    print(F());
} catch (e) {
    print(e.name);
    //print(e.stack);
}

print('done');
