function test()
    local obj = { xxx1 = 1, xxx2 = 2, xxx3 = 3, xxx4 = 4, foo = 123 }
    for i=1,1e8 do
        local ign = obj.foo
    end
end

test()
