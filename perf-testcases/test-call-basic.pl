sub test {
  my $i;

  sub f { return; }

  $i = 0;
  for ($i = 0; $i < 1e8; $i++) {
    f();
  }
}

test();
