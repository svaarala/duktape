def test()
    def f() return end

    i = 0
    while i < 1e8 do
        f()
        i += 1
    end
end

test()
