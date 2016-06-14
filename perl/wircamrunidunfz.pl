#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             wircamrunidunfz
#----------------------------------------------------------------------------
# Contents:   This is the WIRCam image unpacker and is used to
#              unpack *.fits.fz file from CADC based on RUNID 
#----------------------------------------------------------------------------
# Part of:     WIRCam pipeline                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------
=pod
  
 

=cut

use warnings;
use strict;
use Cwd;
use Getopt::Long;
use LWP::UserAgent;

our $SCRIPT_NAME = 'wircamrunidunfz';
our %config=(
      PROCESSEDPATH  => '/arrays/spica/wircam/processed',
      RUNID          => '',
      VERBOSE        => 0,
      OVERRIDE       => 0,
      CAL            => 0,
      LIST           => 0
       );

sub trim {
   my $string=$_;
   for ($string) {
       s/^\s+//;
       s/\s+$//;
        }
   return $string;
}

sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <RUNID>
 
 INPUT:
   -r|--runid        Specify program runid.
 OPTIONS:
   -v|--verbose      Return message
   -o|--override     Return message
   -h|--help         Get help message

  $SCRIPT_NAME --runid=06AT99 [options]

EOF
}
# Declares of variables
my($opt_help,$ua,@words,$count,@file);
my($pre2008a_url,$url,$response,$odoname);
my($command);

# check if the program id is passed.
if(@ARGV == 0){
   usage();
   exit 0;
}

# Get options from command line
GetOptions(
   "r|runid=s"      => \$config{RUNID},
   "o|override"     => \$config{OVERRIDE},
   "v|verbose"      => \$config{VERBOSE},
   "h|help"         => \$opt_help
   );

# check if the program id is passed.
if(system("which funpack 1>>/dev/null 2>>/dev/null") != 0){
   print("Error: fpack and funpack are not installed or in correct path.\n");
   exit 0;
}
 

# Quit the program if directory is not there
if ($config{RUNID} eq ""){
   print("Error: RUNID is not specified.\n");
   usage();
   exit 0;
}

unless (-e $config{PROCESSEDPATH}.'/'.$config{RUNID}){  
    print("Error: RUNID is not existed.\n");
    exit 0;
}

@file=<$config{PROCESSEDPATH}/$config{RUNID}/*fits.fz>;

if($#file != 0){
    foreach(@file){
        my $packed=$_;
        s/.fz//;
        my $unpacked=$_;
        
        if (-e $unpacked && $config{OVERRIDE} == 1){
            if ($config{VERBOSE} == 1){print("Unpacking $packed\n");}
            system("rm -rf $unpacked");
            system("funpack $packed");
        } else {
            print("Skipping $packed\n");
            next;
        }
    }

} else {
  print("Error: RUNID is not specified.\n"); 
  exit 0;  
}

 
#chdir($config{PROCESSEDPATH}.'/'.$config{RUNID});
#my @list = <*.fits>;




