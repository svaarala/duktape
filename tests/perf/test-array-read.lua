function test()
    local arr = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }
    local ign
    for i=1,1e7 do
        ign = arr[8]
        ign = arr[8]
        ign = arr[8]
        ign = arr[8]
        ign = arr[8]
        ign = arr[8]
        ign = arr[8]
        ign = arr[8]
        ign = arr[8]
        ign = arr[8]
    end
end

test()
