function test() {
    var i;
    var re = /[a-z]foo.bar.quux.baz{1,3}[a-zA-Z0-9]+$/;
    var t;
    var txt = 'gfoo!bar!quux!bazzzABBA';
    for (i = 0; i < 1e4; i++) {
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
        t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt); t = re.exec(txt);
    }
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
