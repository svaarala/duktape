function test()
    local t

    for i=1,1 do
        t = ''
        for j=1,1e5 do
            t = t .. 'x'
            --print(#t)
        end
    end
end

test()
