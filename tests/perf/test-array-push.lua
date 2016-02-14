local function test()
    for i=1,1e5 do
        local arr = {}
        for j=1,10 do
            table.insert(arr, 'foo'); table.insert(arr, 'bar')
            table.insert(arr, 'foo'); table.insert(arr, 'bar')
            table.insert(arr, 'foo'); table.insert(arr, 'bar')
            table.insert(arr, 'foo'); table.insert(arr, 'bar')
            table.insert(arr, 'foo'); table.insert(arr, 'bar')
        end
    end
end

test()
