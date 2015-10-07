sub test {
    my $i;

    sub f { return; }

    $i = 0;
    for ($i = 0; $i < 1e7; $i++) {
        f();
        f();
        f();
        f();
        f();
        f();
        f();
        f();
        f();
        f();
    }
}

test();
