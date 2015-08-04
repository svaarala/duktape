sub mk {
    my $n = shift;
    my @res = ();
    my $i;
    my $tmp;

    for ($i = 0; $i < $n; $i++) {
        $res[$i] = 'x';
    }

    $tmp = join('', @res);
    print(length $tmp);
    print("\n");
    return $tmp;
}

sub test {
    my $a, $b, $c, $d, $e, $f, $g;
    my $i, $ign;

    $a = mk(0);
    $b = mk(1);
    $c = mk(16);
    $d = mk(256);
    $e = mk(4096);
    $f = mk(65536);
    $g = mk(1048576);

    for ($i = 0; $i < 1e7; $i++) {
        $ign = ($a == $a);
        $ign = ($a == $b);
        $ign = ($a == $c);
        $ign = ($a == $d);
        $ign = ($a == $e);
        $ign = ($a == $f);
        $ign = ($a == $g);

        $ign = ($b == $b);
        $ign = ($b == $c);
        $ign = ($b == $d);
        $ign = ($b == $e);
        $ign = ($b == $f);
        $ign = ($b == $g);

        $ign = ($c == $c);
        $ign = ($c == $d);
        $ign = ($c == $e);
        $ign = ($c == $f);
        $ign = ($c == $g);

        $ign = ($d == $d);
        $ign = ($d == $e);
        $ign = ($d == $f);
        $ign = ($d == $g);

        $ign = ($e == $e);
        $ign = ($e == $f);
        $ign = ($e == $g);

        $ign = ($f == $f);
        $ign = ($f == $g);

        $ign = ($g == $g);
    }
}

test();
