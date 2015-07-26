function p(x) { print(Duktape.enc('jx', x)); }

p(_.assign({ 'a': 1 }, { 'b': 2 }, { 'c': 3 }));
p(_.map([1, 2, 3], function(n) { return n * 3; }));
