def test()
    def f() return end

    i = 0
    while i < 1e6 do
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        i += 1
    end
end

test()
