/*
 *  Fibonacci test, exercises call handling and recursion
 *
 *  Artificial variant where slow path function name lookup is avoided
 *  to better measure the actual call handling part of the overhead.
 */

if (typeof print !== 'function') { print = console.log; }

function fib(f, n) {
    return n <= 1 ? n : f(f, n - 2) + f(f, n - 1);
}

try {
    print(fib(fib, 35));
} catch (e) {
    print(e.stack || e);
    throw e;
}
