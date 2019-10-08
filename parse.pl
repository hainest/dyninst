use strict;
use warnings;
use Capture::Tiny qw(capture);

sub execute {
	my $cmd = shift;

	my ($stdout,$stderr,$exit) = capture { system($cmd); };
	$exit = (( $exit >> 8 ) != 0 || $exit == -1 || ( $exit & 127 ) != 0);
	die "Error executing '$cmd'\n$stderr\n" if $exit;
	return $stdout;
}
#
# Input should be output of `git log -SgetAllVariables`
#
die "Usage: $0 infile" unless @ARGV == 1;

open my $fdIn, '<', $ARGV[0] or die "Unable to open '$ARGV[0]': $!\n";

while(<$fdIn>) {
  chomp;
  my ($commit, $date) = split(' ');
  my $changes = execute("git grep getAllVariables $commit");
  print "Changes for $commit ($date):\n";
  for my $line (split("\n", $changes)) {
    my (undef, $rest) = split(':', $line, 2); # get rid of leading commit ID
    print "\t$rest\n";
  }
  print "\n";
}
