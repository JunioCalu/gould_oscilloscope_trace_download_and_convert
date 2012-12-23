#!/usr/bin/perl
#
#  tracks2csv.pl
#
#  Copyright (C) 2012 samplemaker
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public License
#  as published by the Free Software Foundation; either version 2.1
#  of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301 USA

use warnings;
use LWP::Simple;
use File::Basename;

# match pattern for "scintific expressions"
# [-+]?\d+\.\d+[eE][-+]?\d+  if we copy this into a variable we need some extra \
$sci_num = "[-+]?\\d+\\.\\d+[eE][-+]?\\d+";

print "Usage: tracks2csv.pl trackfile.dat > trackfile.csv\n";

my $in_file = "./";
$num_args = $#ARGV + 1;
if ($num_args != 1) {
  $in_file .= "dump.asci";
}
else
{
  $in_file .= $ARGV[0];
}
open (INFILE, "<$in_file") ||
    die "cannot open $in_file";
while (<INFILE>) {
  $t .= $_;
}

# /s means ignore newlines (may occur in the data section)

$t =~ /\|CF,1;\|CT,1;\|KS;\|CE,1;(.*)/s;

$t_CD_CR = $1;


$t_CD_CR =~ /\|CD,1,($sci_num),1,($sci_num).*;\|CR.*($sci_num),($sci_num)(.*)/s;

$samplerate = sprintf("%.10g", $1);
# samplerate [seconds/sample]. Trecord = Ts*number of samples
print "#Samplerate:        \t", $samplerate, "\n";
$trig_delay = sprintf("%.10g", $2);
#trigger point in seconds with respect to start of data
print "#Trigger delay:     \t", $trig_delay, "\n";
$mesial_voltage = sprintf("%.10g", $3);
#0xff correlates with y-max position on the scope screen, regardless offset
#0x00 correlates with y-min position on the scope screen, regardless offset
#umrechnung: phys = 2*(hex/0xff)*mesial_voltage-mesial_voltage-offset
print "#Mesial voltage:    \t", $mesial_voltage, "\n";
$offset = sprintf("%.10g", $4);
#offset (channel position) in physical units
print "#Offset in volts:  \t", $offset, "\n";
$t_CS = $5;


$t_CS =~ /\|CS,1,(\d+),(.*);\|CA,1,0000000000;/s;

$num_data = int($1);
print "#Number of Samples: \t", $num_data, "\n";
$sample_data = $2;


my $smp = 0;
my @data = map{ord($_)}( split('', $sample_data) );
foreach my $sample (@data){
  $tb = sprintf("%1.7e", ($samplerate*$smp-$trig_delay));
  $phys = sprintf("%1.7e", 2*$mesial_voltage*$sample/256-$mesial_voltage-$offset);
  print "",$tb,"\t",$phys, "\n";
  $smp++;
  #print "",$sample, "\n";
}

#my @liste = $sample_data;
#foreach(@liste)
#{
#  my @liste1 = split(//, $_);
#  foreach(@liste1)
#  {
#    $sample = sprintf("%12.5e", 2*$mesial_voltage*ord($_)/256-$mesial_voltage-$offset);
#    print "",$sample, "\n";
#  }
#}

close(INFILE) ||
    die "can't close $in_file";

exit 0;
