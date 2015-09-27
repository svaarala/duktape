def test()
    arr = Array[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]
    i = 0
    while i < 1e7 do
        arr[7] = 234
        arr[7] = 234
        arr[7] = 234
        arr[7] = 234
        arr[7] = 234
        arr[7] = 234
        arr[7] = 234
        arr[7] = 234
        arr[7] = 234
        arr[7] = 234
        i += 1
    end
end

test()
