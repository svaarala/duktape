sub test {
    my @arr = ( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 );
    my $i;
    my $ign;

    for ($i = 0; $i < 1e7; $i++) {
        $ign = $arr[7];
        $ign = $arr[7];
        $ign = $arr[7];
        $ign = $arr[7];
        $ign = $arr[7];
        $ign = $arr[7];
        $ign = $arr[7];
        $ign = $arr[7];
        $ign = $arr[7];
        $ign = $arr[7];
    }
}

test();
