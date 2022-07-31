/*
 *  Fibonacci test, exercises call handling and recursion
 */

function fib(n) {
    return n <= 1 ? n : fib(n - 2) + fib(n - 1);
}

print(fib(35));
