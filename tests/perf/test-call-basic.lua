function test()
    local function f()
        return
    end

    for i=1,1e8 do
        f()
    end
end

test()
