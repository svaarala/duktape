local function mandel()
    local w = 76
    local h = 28
    local iter = 100000
    local i, j, k, c
    local x0, y0, xx, yy, xx2, yy2
    local line

    for i=0,h-1 do
        y0 = (i / h) * 2.5 - 1.25
        line = {}
        for j=0,w-1 do
            x0 = (j / w) * 3.0 - 2.0
            xx = 0
            yy = 0
            c = '#'
            for k=0,iter-1 do
                xx2 = xx*xx; yy2 = yy*yy
                if xx2 + yy2 < 4.0 then
                    yy = 2*xx*yy + y0
                    xx = xx2 - yy2 + x0
                else
                    if k < 3 then
                        c = '.'
                    elseif k < 5 then
                        c = ','
                    elseif k < 10 then
                        c = '-'
                    else
                        c = '='
                    end
                    break
                end
            end
            table.insert(line, c)
        end
        print(table.concat(line, ''))
    end
end

mandel()
