def test():
    i = 0
    while i < 1e5:
        arr = []
        j = 0
        while j < 10:
            arr.append('foo'); arr.append('bar')
            arr.append('foo'); arr.append('bar')
            arr.append('foo'); arr.append('bar')
            arr.append('foo'); arr.append('bar')
            arr.append('foo'); arr.append('bar')
            j += 1
        i += 1

test()
