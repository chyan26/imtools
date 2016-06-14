#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             classifyfile
#----------------------------------------------------------------------------
# Contents:    
#----------------------------------------------------------------------------
# Part of:     WIRCam C pipeline                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------
=pod
   History
   $Id: classifyfile.pl,v 1.1 2009/07/29 17:38:17 chyan Locked $
   $Locker: chyan $
   $Log:	classifyfile.pl,v $
# Revision 1.1  2009/07/29  17:38:17  chyan
# Initial revision
# 

=cut

do "/cfht/bin/wircamlibperl";

use warnings;
use strict;
use Cwd;
use Getopt::Long;

our $SCRIPT_NAME = 'wircamfiltersort';
our %config=(
		RUNID				=> '',
        PROCESSEDPATH  => $ENV{'WCPCSPATH'},
		);


# This finction is used to return program usage.
sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <RUNID>
 
 INPUT:
   -r|--runid      Runid
   -P|--redpath    Path
 EXAMPLES:
  $SCRIPT_NAME --runid=06AT99 [options]

EOF
}

my($opt_help,%fitshd,$command);

# check if the program id is passed.
if(@ARGV == 0){
   usage();
   exit 0;
}

# Get options from command line
GetOptions("r|runid=s" =>  \$config{RUNID},
	"P|redpath=s"       =>  \$config{PROCESSEDPATH}
   );

if($opt_help){
   usage();
   exit 0;
}   


chdir($config{PROCESSEDPATH}.'/'.$config{RUNID});
my @list = <*.fits>;


foreach(@list){
	my $file = $_;
  	if (-e $file){  
	   s/[ps].fits//;
	   s#.*/##;
	   my $odoname = $_;
	   %fitshd = getFitshd($file);
	   my $path=$fitshd{FILTER};
	   if (-l $file){
	   	  my $link=readlink($file);
	   	  #print "ln -sf ../$link $path/$odoname"."p\.fits\n";
	   	  
	   	  $command='ln -sf ../'.$link.' '.$path.'/'.$odoname.'p.fits';
	   	  unlink($file);
	   	  #print "$cline\n";
	   	  #$command=$cline;
	   } else {
	      $command='mv -f '.$odoname.'*[ps].fits  '.$path;		
	   }	 
	   
	   if (-e $path){
         print "$command\n";
         system($command);
      } else {
         mkdir($path);
         print "make dir\n"; 
         print "$command\n";	
         system($command);
   
      }
   } 
	#print "$odoname $fitshd{FILTER}\n";
}



