function test()
    local t

    for i=1,5e3 do
        t = {}
        for j=1,1e4 do
            t[j] = 'x'
        end
        t = table.concat(t)
    end
end

test()
