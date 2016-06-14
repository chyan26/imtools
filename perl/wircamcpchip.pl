#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             wircamcpchip
#----------------------------------------------------------------------------
# Contents:   This program devide a wircam image into frames.
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
use Getopt::Long;
use File::Copy;
use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :shortnames );
use Astro::FITS::CFITSIO qw( :constants );
use LWP::UserAgent;

our $SCRIPT_NAME = 'wircamcpchip';
our $SCRIPT_VERSION = '1.0';
our $SCRIPT_AUTHOR = 'chyan';

our %config=(
      CONFPATH       => '/cfht/conf/',
      RAWPATH        => $ENV{'WCRAWPATH'},
      PROCESSEDPATH  => $ENV{'WCPCSPATH'},
      STACKPATH      => $ENV{'WCSTKPATH'},
      CALIBPATH      => $ENV{'WCCLBPATH'},
      MISSFITSCONF   => './wircamcp_wcs.missfits',
      SLICE          => 1,
      CHIP           => 0,
      BACKUPORIGIN   => 0,
      VERBOSE        => 0,
      OUTPUT         => '',
      );

# This finction is used to return program usage.
sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <filename.fits>
 
 OPTIONS:
   -h|--help       Get help message.
   -s|--slice      Slice the FITS cube.
   -c|--chip       Split the FITS cube into sigle frame FITS images.
   -b|--backup     Back original file.
   -v|--verbose    Check output on screen.
   -o|--output     The file name of output image. 
 EXAMPLES:
  $SCRIPT_NAME 899999p.fits 

EOF
}


my($opt_help,$file);
my($command,$slicename,$basename);
my(%mhd,%hd,$ra,$dec);
my($backfile,$odoname);
my(@filelist,@slicelist);


# check if the program id is passed.
if(@ARGV == 0){
   usage();
   exit 0;
}

# Get options from command line
GetOptions(
   "h|help"      => \$opt_help,
   "b|backup"    => \$config{BACKUPORIGIN},
   "s|slice"     => \$config{SLICE},
   "c|chip"      => \$config{CHIP},
   "o|output=s"  => \$config{OUTPUT},
   "v|verbose"   => \$config{VERBOSE},
   "k|keepfile"  => \$config{KEEPFILE}
   );

if($opt_help){
   usage();
   exit 0;
}   

if ($config{CHIP} == 1){
	$config{SLICE} = 0;
}

if ($config{CHIP} == 0 and $config{SLICE} == 0){
	die("Error: Please specify CHIP or SLICE.");
}

# Generating the MissFITS configuration file.
my $precommand = join(" ","missfits -dd >",$config{MISSFITSCONF});
if (system($precommand) != 0){
	die("Error: Can't produce MissFITS configuration file.");
}

@filelist=@ARGV;



foreach (@ARGV){   
   if (-e $_){
      $file = trim($_);
      $basename=getFitsBaseName($file);
      %mhd=getFitshd($file);
      
      #print("$mhd{NAXIS3}\n"); 
      # if there is slice >1
      if (defined($mhd{NAXIS3}) and $mhd{NAXIS3} ge 2){
         $command = join(" ","missfits -c $config{MISSFITSCONF}",
                        "-WRITE_XML N -OUTFILE_TYPE SLICE",
                        "-SLICE_SUFFIX -slice%02d.fits",
                        "-SLICE_KEYWORD FOTC,FOTCS,FOTCN,ABSOR",
                        "-SLICEKEY_FORMAT _%02d",
                        "-VERBOSE_TYPE QUIET -NTHREADS 1",
                        "$file");
         if ($config{VERBOSE} == 1){
         	system($command); 
         } else {
         	system("$command 1>>/dev/null");
         }
                       
               
      } else {
         # if slice = 1
         $slicename=$basename.'-slice01.fits';
         
         if(-l $file){
         	my $link=readlink($file);
         	$command = "cp $link $slicename";
         } else {
         	$command = "cp $file $slicename";
         }
          
         system($command);
         #copy($file,$slicename);
         @slicelist=(@slicelist,$slicename);
      }
      
    } else {
    	die("Error: one of the files in list doesn't exist.");
    }
    
   @slicelist=<$basename*slice??.fits>;
   
   if ($config{SLICE} == 0){ 
	   foreach(@slicelist){
	         # split each file into chips
	         
	         $command = join(" ","missfits -c $config{MISSFITSCONF}",
	            "-WRITE_XML N -OUTFILE_TYPE SPLIT",
	            "-SPLIT_SUFFIX -%02d.fits",
	            "-VERBOSE_TYPE QUIET -NTHREADS 1",
	            "$_");
	         if ($config{VERBOSE} == 1){
	         	system($command); 
	         } else {
	         	system("$command 1>>/dev/null");
	         }
	   }
   }

   if ($config{BACKUPORIGIN} == 0){system("rm -rf $file");}

}


system("rm -rf $config{MISSFITSCONF} *.back *slice??-00.fits");

