function test()
    local function f()
        return
    end

    for i=1,1e7 do
        f()
        f()
        f()
        f()
        f()
        f()
        f()
        f()
        f()
        f()
    end
end

test()
