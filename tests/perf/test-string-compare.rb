def test()
    def mk(n)
        res = []
        i = 0
        while i < n do
            res[i] = 'x'
            i += 1
        end
        res = res.join()
        print(res.length)
        print("\n")
        return res
    end

    a = mk(0)
    b = mk(1)
    c = mk(16)
    d = mk(256)
    e = mk(4096)
    f = mk(65536)
    g = mk(1048576)

    i = 0
    while i < 1e7 do
        ign = (a == a)
        ign = (a == b)
        ign = (a == c)
        ign = (a == d)
        ign = (a == e)
        ign = (a == f)
        ign = (a == g)

        ign = (b == b)
        ign = (b == c)
        ign = (b == d)
        ign = (b == e)
        ign = (b == f)
        ign = (b == g)

        ign = (c == c)
        ign = (c == d)
        ign = (c == e)
        ign = (c == f)
        ign = (c == g)

        ign = (d == d)
        ign = (d == e)
        ign = (d == f)
        ign = (d == g)

        ign = (e == e)
        ign = (e == f)
        ign = (e == g)

        ign = (f == f)
        ign = (f == g)

        ign = (g == g)

        i += 1
    end
end

test()
