#! /usr/bin/perl 
#----------------------------------------------------------------------------
#                             wircamcpsex                     
#----------------------------------------------------------------------------
# Contents: This program is used to excute SExtractor to for C-Pipeline.  The
#            default path of produced catalog will be put in    
#----------------------------------------------------------------------------
# Part of:     WIRcam C pipeline                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------

#use warnings;
use strict;
use POSIX;
use Getopt::Long;


# Define global variable
our $SCRIPT_NAME = 'wircamcpsex';
our $SCRIPT_VERSION = '1.0';
our $SCRIPT_AUTHOR = 'chyan';

our %config=(
      CONFPATH       => '/cfht/conf/',
      CALIBPATH      => $ENV{'WCCLBPATH'},
      BADPIXMAP		 => 'badpix16_20120727HST192309_v100.fits',
      SEXCONF        => './wircamcp_sex.conf',
      VERBOSE        => 0,
      OUTPUT         => '',
      );

sub nearest {
 my $halfhex = unpack('H*', pack('d', 0.5));
 my $half = unpack('d',pack('H*', $halfhex));
 my ($targ, @inputs) = @_;
 my @res = ();
 my $x=0;

 $targ = abs($targ) if $targ < 0;
 foreach $x (@inputs) {
   if ($x >= 0) {
      push @res, $targ * int(($x + $half * $targ) / $targ);
   } else {
      push @res, $targ * POSIX::ceil(($x - $half * $targ) / $targ);
   }
 }
 return (wantarray) ? @res : $res[0];
}


# Subroutine to get the FITS header information.
# $_[0] -> file name.
sub getFitshd($){
   my(@extname);
   my %fitshd=();
   
   open(EXTNAME, "fhextname $_[0] 2>&1 |");
   @extname=<EXTNAME>;
   close(EXTNAME);
   
   if ($extname[0] =~ /^error/){
      open(FITSHD, "fhlist $_[0]|");
   } else {
      open(FITSHD, "fhlist $extname[0]|");
   }
   #Read each line in FITS header to build a hash.
   while(<FITSHD>){
      # Skip all comment fields
      if (/^COMMENT/){next;}
      #Remove all comment field
      #s/\/.*//; #/
      s/\'//g;
      s/=/ /;
      tr/[a-z]/[A-Z]/;
      #print("$_\n");
      my @words=split(/\s+/,$_);
      $fitshd{"$words[0]"}=$words[1];
      if ($words[0] eq "CMMTSEQ"){
      #   print("$_\n");
         $words[5]=~s/000000/00/;
         $words[6]=~s/000000/00/;        
         $fitshd{"$words[0]"}=join(" ",$words[1],$words[5],$words[6]);
      } 
      if ($words[0] eq "OBJECT"){         
         if (defined $words[2]){ 
            $fitshd{"$words[0]"}=join(" ",$words[1],$words[2]);
         } else{
            $fitshd{"$words[0]"}=$words[1];
         }
      }

      $fitshd{"NAXIS3"} = 1 unless defined $fitshd{"NAXIS3"};   
   }
   close(FITSHD);
   
   return %fitshd;
}

sub getCatName($){
   my $string;
   $_ = $_[0];
   s#.*/##;
   s/fits/cat/;  
   $string = $_;
   return $string;
}

sub getRunID($){
   my $string;
   $_ = $_[0];
   
	s#.*/##;
   $string = $_;
   return $string;
}

sub usage(){

print<<EOF
 USAGE: $SCRIPT_NAME <OPTIONS> FITS_file

 OPTIONS:
   -v|--verbose      Turn on verbose mode.
   -h|--help         Get usage.

 EXAMPLES:
  $SCRIPT_NAME 987473p.fits
EOF
}


#--------------------------------
# Here begins the main program
#--------------------------------

# define local variables
local $_;

# Define private variables
my($catname,$opt_help);
#my $confdir = "/cfht/conf";
#my $calib="/arrays/menka/wircam/calib";
#my $sexconf = "$confdir/wircamcp_wcs.sex";
#my $runid = &getcwd();
#my $catpath = "$runid/catalog";
my $sexname;

 
# Get options from command line
GetOptions(
   "catalog:s"  =>  \$catname,
   "h|help"     =>  \$opt_help,
   "v|verbose"  =>  \$config{VERBOSE}
   );

# Show help message if -h is specified ot no argument input
if($opt_help || @ARGV == 0){
   usage();
   exit 0;
}   

my $precommand = join(" ","sex -dd >",$config{SEXCONF});
if (system("$precommand 1>> /dev/null") != 0){
	die("Error: Can't produce SExtractor configuration file.");
}

# Check if files exist 
foreach (@ARGV){
   if (-e $_){
      my $file = $_;
      my %hd=getFitshd($file);  
      my $zp = nearest(.01, $hd{ZEROPT});   
      
      my $cat = getCatName($file);
      my $command = join(" ","sex -c $config{SEXCONF}",
                     "-CATALOG_TYPE FITS_LDAC",
                     "-DETECT_MINAREA 4 -DETECT_THRESH 10",
                     "-ANALYSIS_THRESH 10",
                     "-PARAMETERS_NAME /cfht/conf/wircamcp_wcs.param",
                     "-FILTER_NAME /cfht/conf/wircamcp_default.conv",
                     "-STARNNW_NAME /cfht/conf/wircamcp_default.nnw",
                     "-VERBOSE_TYPE NORMAL",
                     "-WEIGHT_TYPE MAP_WEIGHT",
                     "-WEIGHT_IMAGE $config{CALIBPATH}/$config{BADPIXMAP}",
                     "-NTHREADS 4 -WRITE_XML Y",
                     "-XSL_URL sextractor.xsl",
                     "-MAG_ZEROPOINT $zp",
                     "-CATALOG_NAME $cat $file");
	  if ($config{VERBOSE} == 1){
	  	system("$command");
	  } else {
	  	system("$command 1>>/dev/null 2>>/dev/null");
	  }
	  
      
   }
   else{
      usage();
      exit 1;  
   }   
}

system("rm -rf $config{SEXCONF}");
print("All done. \n");
exit 0;

