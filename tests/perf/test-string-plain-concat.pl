sub test {
    my $i, $j, $t;

    for ($i = 0; $i < 1; $i++) {
        $t = '';
        for ($j = 0; $j < 1e5; $j++) {
            $t = $t . 'x';
            #print(length $t);
            #print("\n");
        }
    }
}

test()
