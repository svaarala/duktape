def fib(n)
    if n <= 1 then
        return n
    else
        return fib(n - 2) + fib(n - 1)
    end
end

print(fib(35).to_s + "\n")
