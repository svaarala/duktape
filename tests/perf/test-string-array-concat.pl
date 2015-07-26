sub test {
    my $i, $j, @t, $ign;

    for ($i = 0; $i < 5e3; $i++) {
        @t = ();
        for ($j = 0; $j < 1e4; $j++) {
            $t[$j] = 'x';
        }
        $ign = join('', @t);
    }
}

test();
