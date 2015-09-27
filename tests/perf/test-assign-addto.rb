def test()
    t = 0
    a = 10

    i = 0
    while i < 1e7 do
        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        t += a; t += a; t += a; t += a; t += a
        t += a; t += a; t += a; t += a; t += a

        i += 1
    end
end

test()
