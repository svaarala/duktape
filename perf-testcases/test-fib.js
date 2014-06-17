/*
 *  Fibonacci test, exercises call handling and recursion
 */

function fib(n) {
    return n <= 1 ? n : fib(n - 2) + fib(n - 1);
}

try {
    print(fib(35));
} catch (e) {
    print(e.stack || e);
}
