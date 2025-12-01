#!/usr/bin/perl -w
#
# Convert a bmp file into a c data statement.
#
require 5.006;
use strict;
use English;
use FileHandle;
use POSIX;

################################################################################
# Get the file name
if( @ARGV != 1 )
{
   print STDERR "Usage: binary2text.pl  binaryfile.bin\n";
   exit(1);
}
my $filename = $ARGV[0];

################################################################################
# Make the output file name
$_ = $filename;
/\./;
my $root = $`;
my $output = $root . ".data";
print STDERR "converting $filename to $output\n";

if( $filename eq $output )
{
   print STDERR "Error converting $filename to $output - Output will overwrite the input!\n";
   exit(1);
}

###############################################################
# Returns the ascii hex character for the nibble
sub tohex ($)
{
   my $val = shift;
   my $chars = "0123456789ABCDEF";
   return substr $chars, $val, 1;
}

################################################################################
# open the bmp file for reading
sysopen(INPUT, $filename, O_RDONLY | O_BINARY) || die "Can't open file $filename to load : $!\n";
binmode INPUT;
open(OUTPUT, "> $output") || die "Can't open file $output to write : $!\n";

print OUTPUT "byte $root" . "_data[] = {";

my $needComma = 0;
my $lr = 0;
my $byte = 0;
my $done = 0;
while ( !$done )
{
   if( $needComma == 1 )
   {
      print OUTPUT ",";
   }
   else
   {
      $needComma = 1;
   }

   if( $lr++ > 40 )
   {
      print OUTPUT "\n";
      $lr = 0;
   }

   if ( sysread INPUT, $byte, 1 )
   {
      # I wasted 3 hours trying to get perl to print the byte in hex.
      # I gave up and did it myself.
      my $left = int(ord($byte) / 16);
      my $right = ord($byte) - ($left * 16);
      print OUTPUT "0x", tohex($left), tohex($right);
   }
   else
   {
      $done = 1;
   }
}

# Append a 0 byte on the end
print OUTPUT "0x00";

print OUTPUT "};\n\n";
exit(0);
