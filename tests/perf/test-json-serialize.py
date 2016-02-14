import json

def build():
    obj = {}
    obj['key1'] = 'foo'
    obj['key2'] = 'bar'
    obj['key3'] = 'quux'
    obj['key4'] = 'baz'
    obj['key5'] = 'quuux'
    obj['key6'] = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ]
    obj['key7'] = [ None, None, True, 123.456, 1e200, {}, {}, {} ]

    return obj

def test():
    obj = build()
    i = 0
    while i < 1e5:
        ignore = json.dumps(obj)
        i += 1

test()
