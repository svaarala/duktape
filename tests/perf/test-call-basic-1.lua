function test()
    local function f()
        return
    end

    for i=1,1e6 do
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f()
    end
end

test()
