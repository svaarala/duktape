// Test the Duktape (polyfill) specific unhandled rejection events in detail.

/*---
{
    "custom": true,
    "skip": true
}
---*/

/*===
create P
unhandledRejection: rawReject undefined 123
create Q
unhandledRejection: rawReject undefined 234
create R
unhandledRejection: rawReject undefined 345
P.catch
unhandledRejection: rawHandle P 123
tick 1a
unhandledRejection: reject Q 234
unhandledRejection: reject R 345
tick 1b
Q.catch
unhandledRejection: rawHandle Q 234
tick 2a
unhandledRejection: handle Q 234
tick 2b
done
===*/

if (typeof Duktape === 'object' && typeof Promise === 'function' && Promise.isPolyfill) {
    Promise.unhandledRejection = function (args) {
        console.log('unhandledRejection:', args.event, args.promise.name, args.reason);
        throw new Error('aiee');  // ignored
    };
} else {
    throw new TypeError('test is Duktape specific');
}

// rawReject .name is undefined because it happens inline and we don't yet
// have a chance to give the Promise a .name.

print('create P');
var P = Promise.reject(123); P.name = 'P';
print('create Q');
var Q = Promise.reject(234); Q.name = 'Q';
print('create R');
var R = Promise.reject(345); R.name = 'R';

print('P.catch');
P.catch(Math.cos);

print('tick 1a');
Promise.runQueue();
print('tick 1b');

print('Q.catch');
Q.catch(Math.cos);

print('tick 2a');
Promise.runQueue();
print('tick 2b');

print('done');
