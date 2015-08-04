-- Translated from tests/ecmascript/test-dev-mandel2-func.js.

function mandel()
    local w = 80
    local h = 40
    local iter = 100
    local i, j, k
    local x0, y0, xx, yy, c, xx2, yy2
    local res

    for i=0,h-1 do
        y0 = (i / h) * 4.0 - 2.0
        res = {}

        for j=0,w-1 do
            x0 = (j / w) * 4.0 - 2.0
            xx = 0
            yy = 0
            c = '#'

            for k=0,iter-1 do
                xx2 = xx*xx
                yy2 = yy*yy

                if xx2 + yy2 < 4.0 then
                    yy = 2*xx*yy + y0
                    xx = xx2 - yy2 + x0
                else
                    c = '.';
                    break
                end
            end

            table.insert(res, c)
        end

        print(table.concat(res))
    end
end

mandel()
