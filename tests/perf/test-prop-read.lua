function test()
    local obj = { xxx1 = 1, xxx2 = 2, xxx3 = 3, xxx4 = 4, foo = 123 }
    local ign
    for i=1,1e7 do
        ign = obj.foo
        ign = obj.foo
        ign = obj.foo
        ign = obj.foo
        ign = obj.foo
        ign = obj.foo
        ign = obj.foo
        ign = obj.foo
        ign = obj.foo
        ign = obj.foo
    end
end

test()
