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

our $SCRIPT_NAME = 'wircamcpscamp';
our $SCRIPT_VERSION = '1.0';
our $SCRIPT_AUTHOR = 'chyan';

our %config=(
      CONFPATH       => '/cfht/conf/',
      CALIBPATH      => $ENV{'WCCLBPATH'},
      SCAMPCONF      => './wircamcp_scamp.conf',
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
  $SCRIPT_NAME 899999p.cat 899998p.cat

EOF
}

my($opt_help,@filelist,$command);

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

# Generating the MissFITS configuration file.
my $precommand = join(" ","scamp -dd >",$config{SCAMPCONF});
if (system($precommand) != 0){
	die("Error: Can't produce SCAMP configuration file.");
}


# $command = join(" ","scamp -c /cfht/conf/wircamcp_wcs.scamp",      
#    "-MAGZERO_OUT 25 -SOLVE_PHOTOM Y",          
#    "-CHECKPLOT_DEV PNG -MAGZERO_KEY FOTC",     
#    "-AHEADER_GLOBAL /cfht/conf/wircam.ahead",  
#    "-CROSSID_RADIUS 3.0 -VERBOSE_TYPE NORMAL", 
#    "-XSL_URL scamp.xsl -NTHREADS 4 @filelist");
$command = join(" ","scamp -c $config{SCAMPCONF}",
     "-ASTREF_CATALOG 2MASS",
     "-MOSAIC_TYPE FIX_FOCALPLANE",
     "-MAGZERO_OUT 25",
     "-SOLVE_PHOTOM Y",
     "-CHECKPLOT_DEV PNG",
     "-MAGZERO_KEY FOTC",
     "-AHEADER_GLOBAL /cfht/conf/wircam.ahead",
     "-CROSSID_RADIUS 3.0 -VERBOSE_TYPE NORMAL",
	 "-XSL_URL scamp.xsl -NTHREADS 4 @filelist");
   
   
system("$command");   
   
   
   
   
   
   
   
   
   
   