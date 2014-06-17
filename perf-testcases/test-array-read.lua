function test()
    local arr = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }
    for i=1,1e8 do
        local ign = arr[8]
    end
end

test()
