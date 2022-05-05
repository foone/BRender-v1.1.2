#
#
while (<>) {
	s/(\s|,|:)\+0x/${1}0x/g;
	s/(push|pop)ad/$1al/;
	if(/repn?e/) {
		print "$`$&\n$'\n";
	} elsif(/(\w*:?)\s+\.(\w*)\s+(\d+)\s+DUP\((.*)\)/) {
#		print ">> id=<$1> type=<$2> count=<$3> value=<$4>\n";
		print "$1\t.$2 $4\n";
		foreach $count (2..$3) {
			print "\t.$2 $4\n";
		}
	} else {
		print ;
	}
}
