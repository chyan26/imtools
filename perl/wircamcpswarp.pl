#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             wircamcpscamp
#----------------------------------------------------------------------------
# Contents:     
#----------------------------------------------------------------------------
# Part of:     WIRCam C pipeline                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------
=pod
  
  History
   
   $Id: $
   $Locker:  $
   $Log:$

=cut

do "/cfht/bin/wircamlibperl";

use warnings;
use strict;
use Cwd;
use Getopt::Long;
use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :shortnames );
use Astro::FITS::CFITSIO qw( :constants );

our $SCRIPT_NAME = 'wircamcpswarp';
our $SCRIPT_VERSION = '1.0';
our $SCRIPT_AUTHOR = 'chyan';

our %config=(
      CONFPATH       => '/cfht/conf/',
      CALIBPATH      => $ENV{'WCCLBPATH'},
      SWARPCONF      => './wircamcp_swarp.conf',
      VERBOSE        => 0,
      OUTPUT         => '',
      );

# This finction is used to return program usage.
sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <filename.fits>
 
 OPTIONS:
   -h|--help       Get help message.
   -v|--verbose    Check output on screen.
   -o|--output     The file name of output image. 
 EXAMPLES:
  $SCRIPT_NAME 899999p.fits 899998p.fits

EOF
}

my($opt_help,@filelist,$command,$basename);

# Get options from command line
GetOptions(
   "h|help"      => \$opt_help,
   "o|output=s"  => \$config{OUTPUT},
   "v|verbose"   => \$config{VERBOSE},
   );


# check if the program id is passed.
if(@ARGV == 0){
   usage();
   exit 0;
}

@filelist=@ARGV;

my $precommand = join(" ","swarp -dd >",$config{SWARPCONF});
if (system("$precommand 1>> /dev/null") != 0){
	die("Error: Can't produce SWARP configuration file.");
}


$command = join(" ","swarp -c $config{SWARPCONF}",
   "-WEIGHT_TYPE MAP_WEIGHT",
   "-WEIGHT_IMAGE '' -WEIGHT_SUFFIX .wm",
   "-WEIGHT_THRESH  0 -BLANK_BADPIXELS Y",
   "-RESAMPLING_TYPE LANCZOS3",
   "-COMBINE_TYPE MEDIAN @filelist");
  
if ($config{VERBOSE} == 1){
	system("$command");   
} else {
	system("$command 1>>/dev/null 2>>/dev/null");   
}
   
if ($config{OUTPUT} ne ''){
   $_=$config{OUTPUT};
   s/.fits//;
   s#.*/##;
   $basename=$_;
   system("mv -f coadd.fits $basename.fits");
   system("mv -f coadd.weight.fits $basename.weight.fits");
}

system("rm -rf $config{SWARPCONF}");









