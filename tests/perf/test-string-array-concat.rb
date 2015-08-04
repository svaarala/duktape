def test()
    i = 0
    while i < 5e3 do
        t = []
        j = 0
        while j < 1e4 do
            t[j] = 'x'
            j += 1
        end
        t = t.join()
        i += 1
    end
end

test()
