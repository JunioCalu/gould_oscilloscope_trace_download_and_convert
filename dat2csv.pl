#!/usr/bin/perl
#
#  tracks2csv.pl
#
#  Copyright (C) 2012/2017 samplemaker
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
use strict;

my $datFile = "./";
my $num_args = $#ARGV + 1;
if ($num_args != 1) {
  die "Usage: ./dat2csv.pl trace.dat > trace.csv";
}
else
{
  $datFile .= $ARGV[0];
}

open (INFILE, "<$datFile") ||
    die "cannot open $datFile [Usage: ./dat2csv.pl trace.dat > trace.csv]";
my $stream;
while (<INFILE>) {
  $stream .= $_;
}
close(INFILE) ||
    die "can't close $datFile";

$stream =~ /\|CD,1,([^,]+),1,([^,]+),([^,]+),[^,]+,[^,]+;/;  
print "#X-Axis:\n";
my $sampleRate = sprintf("%1.7e", $1);
# sampleRate [seconds/sample]. Trecord = Ts * (number of samples)
print "#Samplerate:        \t", $sampleRate, "\n";
my $triggerDelay = sprintf("%1.7e", $2);
# trigger point in seconds with respect to start of data
print "#Trigger delay:     \t", $triggerDelay, "\n";
my $timeBaseSource = $3;
if ($timeBaseSource == 0){
  print "#Time base:       \tExt clock\n";
} elsif ($timeBaseSource == 1){
  print "#Time base:        \tDSO timebase\n";
} else {
  print "#Error: unknown timebase field\n";
};

$stream =~ /\|CR,1,1,0,1,[^,]+,[^,]+,[^,]+,[^,]+,([^,]+),([^,]+),([^,]+),[^,]+,[^,]+;(.*)/;
print "#Y-Axis:\n";
my $mesialVoltage = sprintf("%1.7e", $1);
# +-maximum possible voltage on scope screen depending on volts per division, regardless offset
print "#Mesial voltage:    \t", $mesialVoltage, "\n";
my $offsetVoltage = sprintf("%1.7e", $2);
# offsetVoltage (channel position) in physical units
print "#Offset in volts:   \t", $offsetVoltage, "\n";
my $variableVoltsDiv = $3;
if ($variableVoltsDiv == 0){
  print "#Variable volts/div on\n";
} elsif ($variableVoltsDiv == 1){
  print "#Variable volts/div off\n";
} else {
  print "#Error: variable volts/div\n";
};

$stream =~ /\|NT,1,[^,]+,([^,]+),[^,]+,([^,]+);/;
print "#Date an year:      \t", $1, "\n";
print "#Time:              \t", $2, "\n";
$stream =~ /\|NL,1,([^,]+);/;
print "#DSO type:          \t", $1, "\n";
$stream =~ /\|CS,1,(\d+),(.*);\|CA,1,0000000000;/s;
my $num_data = int($1);
print "#Number of Samples: \t", $num_data, "\n";
my $traceStream = $2;


my $timeBase = 0;
# calculate ascii code and map to new array
my @sampleData = map{ord($_)}( split('', $traceStream) );
foreach my $sample (@sampleData){
  my $curTime = sprintf("%1.7e", ($sampleRate*$timeBase-$triggerDelay) );
  my $physVoltage = sprintf("%1.7e", $mesialVoltage*($sample-128.0)/128.0-$offsetVoltage );
  # could be also
  # my $physVoltage = sprintf("%1.7e", $mesialVoltage*($sample-128.0)/127.0-$offsetVoltage );
  # range in data file 0x00 .. 0xff
  # 0xff correlates with y-max display position on the scope screen, regardless offset and magnification
  # 0x01 correlates with y-min display position on the scope screen, regardless offset and magnification
  # 0x00 can occur in file but is not visible on the display ("out of range")
  # 0x80 is the "zero line" on the graticule
  # max graticule border is approx at 0xF8
  # min graticule border is approx at 0x08
  print "",$curTime,"\t",$physVoltage, "\n";
  $timeBase++;
  # print "",$sample, "\n";
}

exit 0;

