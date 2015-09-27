sub test {
    my %obj = ( 'xxx1' => 1, 'xxx2' => 2, 'xxx3' => 3, 'xxx4' => 4, 'foo' => 123 );
    my $i;

    for ($i = 0; $i < 1e7; $i++) {
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
        $obj{'foo'} = 234;
    }
}

test();
