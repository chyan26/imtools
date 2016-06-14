#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             wircamrunidget
#----------------------------------------------------------------------------
# Contents:   This is the WIRCam image downloader and this program is used to
#              download the WIRCam image from CADC based on RUNID or ODOMETER
#              number. 
#----------------------------------------------------------------------------
# Part of:     WIRCam pipeline                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------
=pod
  
 

=cut

use sigtrap 'handler' => \&userStop, 'INT';
use warnings;
use strict;
use Cwd;
use Getopt::Long;
use LWP::UserAgent;

our $SCRIPT_NAME = 'wircamrunidget';
our %config=(
      FILEMAX        => 0,
      RUNID          => '',
      OBSID          => '',
      RAW            => 0,
      NOSKY          => 0,
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

sub userStop{
   print("Error: users terminates the program.\n");
   kill 9, $$;
}



sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <RUNID>
 
 INPUT:
   -r|--runid        Specify program runid.
   -f|--odometer     Specify single odometer file. 
   -o|--raw          Downloading raw image *o.fits.
   -s|--detrend      Downloading detrended *s.fits file
   -p|--processed    Downloading processed *p.fits file
   -l|--list         List the file without downloading
 OPTIONS:
   -h|--help         Get help message

  $SCRIPT_NAME --runid=06AT99 [options]

EOF
}


#------------------------------------------------------
# Beginning of the Main program
#------------------------------------------------------


# Declares of variables
my($opt_help,$ua,@words,$count,@file);
my($pre2008a_url,$url,$response,$odoname);
my($command,$filecount,$i);
 
# check if the program id is passed.
if(@ARGV == 0){
   usage();
   exit 0;
}


# Get options from command line
GetOptions("f|filemax=s"  => \$config{FILEMAX},
   "r|runid=s"      => \$config{RUNID},
   "f|odometer=s"   => \$config{OBSID},
   "o|raw"          => \$config{RAW},
   "s|detrend"      => \$config{NOSKY},
   "p|processed"    => \$config{CAL},
   "l|list"         => \$config{LIST}, 
   "h|help"         => \$opt_help
   );

if($opt_help){
   usage();
   exit 0;
}   
# Quit the program if neigher of RUNID and ODOMETER are specified.
if ($config{RUNID} eq "" && $config{OBSID} eq ""){
   print("Error: no RUNID or ODOMETER specified.\n");
   usage();
   exit 0;
}

if ($config{RUNID} ne "" && $config{OBSID} ne ""){
   print("Error: both RUNID and ODOMETER are specified, please choose one.\n");
   exit 0;
}


# If there is no prefer data type, download p.fit
if ($config{RAW} == 0 && $config{CAL} == 0 and $config{NOSKY} == 0){
	$config{CAL} = 1;
}

if ($config{RUNID} eq ""){
	#print "$config{OBSID}\n";
	#print "Downloading single file.\n";
		
	if ($config{CAL} == 1){
		$odoname=$config{OBSID}.'p';
		$command="'http://www.cadc-ccda.hia-iha.nrc-cnrc.gc.ca/getData/anon?archive=CFHT&file_id=$odoname&dua=true'";
		print "Downloading $odoname.fits.gz\n";
		if ($config{LIST} eq 0){
			if (system("wget $command 2>>/dev/null 1>>/dev/null") != 0){
				print "Error: failed to download file: $odoname\n";
			}
		}
	}
	if ($config{NOSKY} == 1){
		$odoname=$config{OBSID}.'s';
		$command="'http://www.cadc-ccda.hia-iha.nrc-cnrc.gc.ca/getData/anon?archive=CFHT&file_id=$odoname&dua=true'";
		print "Downloading $odoname\n";
		if ($config{LIST} eq 0){
			if (system("wget $command 2>>/dev/null 1>>/dev/null") != 0){
				print "Error: failed to download file: $odoname\n";
			}
		}
	}
	if ($config{RAW} == 1){
		$odoname=$config{OBSID}.'o';
		$command="'http://www.cadc-ccda.hia-iha.nrc-cnrc.gc.ca/getData/anon?archive=CFHT&file_id=$odoname&dua=true'";
		print "Downloading $odoname\n";
		if ($config{LIST} eq 0){
			if (system("wget $command 2>>/dev/null 1>>/dev/null") != 0){
				print "Error: failed to download file: $odoname\n";
			}
		}
	}


} else {
	
	# Convert all the string to uppercase
	$config{RUNID}=uc($config{RUNID});
	
	# Starting to query the CFHT data base
	$ua = LWP::UserAgent->new;
	$ua->timeout(10);
	$ua->env_proxy;
	  
	
	if ($config{RUNID} =~ /^0[5467]/){
		$url='http://www.cfht.hawaii.edu/~chyan/aodolist.php?runid='.$config{RUNID};	
		$response = $ua->get($url); 
	} elsif ($config{RUNID} =~ /^0[89]/) {
		$url='http://www.cfht.hawaii.edu/~chyan/bodolist.php?runid='.$config{RUNID};	
		$response = $ua->get($url); 	
	} else {	
		$url='http://www.cfht.hawaii.edu/~chyan/odolist.php?runid='.$config{RUNID};
		$response = $ua->get($url);
	}	
	
	@words=split("<br>",$response->content);
	
	if (scalar(@words) == 1){
		print("Error: This RUNID does not exist. Please double check.\n");
	   exit 0;
	}
	
	@file=();
	$count=0;
	foreach (@words){
		if ($config{FILEMAX} != 0){
			if ($count >= $config{FILEMAX}){last;}
			@file=(@file,$_);
			$count++;
		} else {
			if ($_ =~ /^\s/){next;}
			@file=(@file,$_);
		} 
	}
	
	$filecount=$#file+1;	
	$i=0;
	foreach (@file){
		if ($config{CAL} == 1){
			$odoname=trim($_).'p';
			$command="'http://www.cadc-ccda.hia-iha.nrc-cnrc.gc.ca/getData/anon?archive=CFHT&file_id=$odoname&dua=true'";
			print "Downloading $odoname.fits.gz...... $i/$filecount \n";
			if ($config{LIST} eq 0){
				if (system("wget $command 2>>/dev/null 1>>/dev/null") != 0){
					print "Error: failed to download file: $odoname\n";
				}
			}
		}
		if ($config{NOSKY} == 1){
			$odoname=trim($_).'s';
			$command="'http://www.cadc-ccda.hia-iha.nrc-cnrc.gc.ca/getData/anon?archive=CFHT&file_id=$odoname&dua=true'";
			print "Downloading $odoname ...... $i/$filecount \n";
			if ($config{LIST} eq 0){
				if (system("wget $command 2>>/dev/null 1>>/dev/null") != 0){
					print "Error: failed to download file: $odoname\n";
				}
			}
		}
		if ($config{RAW} == 1){
			$odoname=trim($_).'o';
			$command="'http://www.cadc-ccda.hia-iha.nrc-cnrc.gc.ca/getData/anon?archive=CFHT&file_id=$odoname&dua=true'";
			print "Downloading $odoname...... $i/$filecount \n";
			if ($config{LIST} eq 0){
				if (system("wget $command 2>>/dev/null 1>>/dev/null") != 0){
					print "Error: failed to download file: $odoname\n";
				}
			}
		}
		
		$i=$i+1;
	
	}   

}










