print "Content-type: text/plain\r\n\r\n";
  print "Server's environment\n";
  foreach ( keys %ENV ) {
      print "$_\t$ENV{$_}\n";
  }
