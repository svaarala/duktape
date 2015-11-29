local function test()
    local t1 = {}
    local t2 = {}

    for i=1,1024 do
        t1[i] = string.char(math.floor(math.random(255)))
    end
    t1 = table.concat(t1)
    for i=1,1024 do
        t2[i] = t1
    end
    t2 = table.concat(t2)
    print(#t2)

    local base64 = require('base64')
    for i=1,2000 do
        local res = base64.encode(t2)
    end
end
test()
