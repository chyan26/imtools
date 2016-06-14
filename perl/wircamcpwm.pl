#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             wircamcpwm
#----------------------------------------------------------------------------
# Contents:   This program produce the weight image for each FITS file
#----------------------------------------------------------------------------
# Part of:     WIRCam C pipeline                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------
=pod
  
  History
   
   $Id: = $
   $Locker: $
   $Log: $


=cut

do "/cfht/bin/wircamlibperl";

use warnings;
use strict;
use Cwd;
use File::Copy;
use File::Basename;
use Getopt::Long;
use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :shortnames );
use Astro::FITS::CFITSIO qw( :constants );
use LWP::UserAgent;


our $SCRIPT_NAME = 'wircamcpwm';
our $SCRIPT_VERSION = '1.0';
our $SCRIPT_AUTHOR = 'chyan';

our %config=(
      CONFPATH       => '/cfht/conf/',
      CALIBPATH      => $ENV{'WCCLBPATH'},
      WWCONF         => './wircamcp_missfits.conf',
      BADPIXMAP		 => 'badpix16_20120727HST192309_v100.fits',
      BACKUPORIGIN   => 0,
      KEEPFILE       => 0,
      VERBOSE        => 0,
      OUTPUT         => '',
      );


# This finction is used to return program usage.
sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <filename.fits>
 
 OPTIONS:
   -h|--help       Get help message.
 
 EXAMPLES:
  $SCRIPT_NAME 899999p.fits 

EOF
}


my($opt_help,@filelist);
my($file,$basename,$badpixname,$maskname);
my(%mhd,@extname,$ext,@result);
my($ww,$command);
my $pwd=getcwd();
# check if the program id is passed.
if(@ARGV == 0){
   usage();
   exit 0;
}

# Get options from command line
GetOptions(
   "h|help"      => \$opt_help,
   "o|output=s"  => \$config{OUTPUT},
   "v|verbose"   => \$config{VERBOSE},
   "k|keepfile"  => \$config{KEEPFILE}
   );

if($opt_help){
   usage();
   exit 0;
}   

my $precommand = join(" ","ww -dd >",$config{WWCONF});
if (system("$precommand 1>> /dev/null") != 0){
	die("Error: Can't produce WeightWatcher configuration file.");
}

@filelist=@ARGV;

foreach (@ARGV){ 
   if (-e $_){
      $file = trim($_);
      $basename=getFitsBaseName($file);
      %mhd=getFitshd($file);
      
      
      open(EXTNAME, "fhextname $file 2>&1 |");
         @extname=<EXTNAME>;
      close(EXTNAME);
      $ext=@extname;
      
      
      #print "defined($mhd{NAXIS3})\n";
       if ($ext == 4){
         $ww=$basename.'.wm';
         $command = join(" ","ww -c $config{WWCONF}",
            "-WEIGHT_NAMES $config{CALIBPATH}/$config{BADPIXMAP},$file",
            "-FLAG_NAMES '' ",
            "-OUTWEIGHT_NAME $ww",
            "-WEIGHT_MIN 0.5,0.3",
            "-WEIGHT_MAX 2,65535",
            "-WEIGHT_OUTFLAGS 0,0");
         unless (-e $ww){ 
         	system("$command 1>>/dev/null 2>>/dev/null");
	        if ($config{VERBOSE}){print "Skipping mapping: $file\n"}; 
         } else {
         	if ($config{VERBOSE}){print "Weight mapping: $file\n"};
         }
         #system("$command");
       } else {
         $ww=$basename.'.wm';
         @result=split('-',$basename);
         $badpixname=basename($config{BADPIXMAP}, '.fits');
         $maskname=$badpixname.'-slice01-'.$result[2].'.fits';
         #my $maskname='badpix_chihung-slice01'.'-'.$result[2].'.fits';
         
         unless (-e $maskname){
            # copy bad pixel mask to here
            copy($config{CALIBPATH}.'/'.$config{BADPIXMAP},$pwd);         
            system("wircamcpchip $config{BADPIXMAP}");
            
         }
         $command = join(" ","ww -c $config{WWCONF}",
	            "-WEIGHT_NAMES $maskname,$file",
	            "-FLAG_NAMES '' ",
	            "-OUTWEIGHT_NAME $ww",
	            "-WEIGHT_MIN 0.5,0.3",
	            "-WEIGHT_MAX 2,65535",
	            "-WEIGHT_OUTFLAGS 0,0");
         
         unless (-e $ww){ 
         	if ($config{VERBOSE}){print "Weight mapping: $file\n"};
         	system("$command 1>>/dev/null 2>>/dev/null");
         } else {
	        if ($config{VERBOSE}){print "Skipping mapping: $file\n"}; 
         }
       }
   } else {
      print "Error: can't locate $_\n";
      exit 1;
   }
}

my $badpixfiles = $badpixname.'*.fits';

system("rm -rf $badpixfiles");
system("rm -rf $config{WWCONF} flag.fits ww.xml");
print("All done. \n");

exit 0;