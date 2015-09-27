function test()
    local arr = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }
    for i=1,1e7 do
        arr[8] = 234
        arr[8] = 234
        arr[8] = 234
        arr[8] = 234
        arr[8] = 234
        arr[8] = 234
        arr[8] = 234
        arr[8] = 234
        arr[8] = 234
        arr[8] = 234
    end
end

test()
