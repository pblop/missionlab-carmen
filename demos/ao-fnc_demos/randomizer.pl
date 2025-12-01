#!/usr/bin/perl


$numargs = $#ARGV + 1;
if ($numargs != 4) {
    print "Usage: ./randomizer.pl land_file input_feature_file x_size y_size\n";
    die;
}

# load up random land squares
open(LANDFILE, $ARGV[0]);
my @landarray = ();
while (<LANDFILE>) {
    s/#.*//;            # ignore comments
    next if /^(\s)*$/;  # skip blank lines
    unshift(@landarray, $_);
}

close(LANDFILE);

$landsize = @landarray;
$landsquares = $landsize / 4;

print "Land squares: $landsquares \n";

open (INFILE, $ARGV[1]);
open (OUTFILE, ">test.feature.data");

$robotcnt = 0;
while(<INFILE>) {
    print OUTFILE $_;
    if (/RuntimeCNPBidderTask/) {
	$posflag = 0;
	while ($posflag == 0) {
	    $rx = int(rand($ARGV[2]));
	    $ry = int(rand($ARGV[3]));
	    
	    $posflag = 1;
	    for ($cnt = 0; $cnt < $landsquares; $cnt++) {
		$cx1 = $landarray[$cnt * 4];
		$cy1 = $landarray[$cnt * 4 + 1];
		$cx2 = $landarray[$cnt * 4 + 2];
		$cy2 = $landarray[$cnt * 4 + 3];	
	
		if (($rx >= $cx1) && ($rx <= $cx2)) {
		    if (($ry >= $cy1) && ($ry <= $cy2)) {
			$posflag = 0;
		    }
		}
	    }
	}
	$_ = <INFILE>;
	print OUTFILE "coordinate $rx.00 $ry.00\n";
	print "Robot $robotcnt: coordinate $rx.00 $ry.00\n";
	$robotcnt++
    }
}

close(OUTFILE);
close(INFILE);



 
