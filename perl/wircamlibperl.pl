#! /usr/bin/perl 
#----------------------------------------------------------------------------
#                             wircamlibperl                    
#----------------------------------------------------------------------------
# Contents:    
#----------------------------------------------------------------------------
# Part of:     WIRCam C pipeline                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------

# Subroutine to get the FITS header information.
# $_[0] -> file name.
sub getFitshd($){
	my(@extname);
	my %fitshd=();
	
	# There are many packages that can be use to list all extensions.
	#  In CFHT, the standard is fhextname and fhlist
	if (system("which listextname 1>>/dev/null 2>>/dev/null") == 0){
		open(EXTNAME, "listextname $_[0] 2>&1 |");
		@extname=<EXTNAME>;
		close(EXTNAME);
		
		if ($extname[0] =~ /^error/){
			open(FITSHD, "listhead $_[0]|");
		} else {
			open(FITSHD, "listhead $extname[0]|");
		}
		
	}
	
#	open(EXTNAME, "fhextname $_[0] 2>&1 |");
#	@extname=<EXTNAME>;
#	close(EXTNAME);
#	
#	if ($extname[0] =~ /^error/){
#		open(FITSHD, "fhlist $_[0]|");
#	} else {
#		open(FITSHD, "fhlist $extname[0]|");
#	}
	
#Read each line in FITS header to build a hash.
	while(<FITSHD>){
		# Skip all comment fields
		if (/^COMMENT/){next;}
		#Remove all comment field
		#	s/\/.*//; #/
		s/\'//g;
		s/=/ /;
		tr/[a-z]/[A-Z]/;
#print("$_\n");
		my @words=split(/\s+/,$_);
		$fitshd{"$words[0]"}=$words[1];
		if ($words[0] eq "CRUNID"){
			$_ = $words[1];
			tr/[A-Z]/[a-z]/;
			$fitshd{"$words[0]"}=$_;
		}	
		if ($words[0] eq "CMMTSEQ"){
			$words[5]=~s/000000/00/;
			$words[6]=~s/000000/00/;        
			$fitshd{"$words[0]"}=join(" ",$words[1],$words[5],$words[6]);
		} 
		if ($words[0] eq "OBJECT"){         
			if (defined $words[2]){ 
				$fitshd{"$words[0]"}=join("",$words[1],$words[2]);
			} else{
				$fitshd{"$words[0]"}=$words[1];
			}	
		}
		if ($words[0] eq "FILTER"){
			if ($words[1] eq "BRG"){
				$fitshd{"$words[0]"}='BrG'
			}
			if ($words[1] eq "KCONT"){
				$fitshd{"$words[0]"}='Kcont'
			}
		}  

		$fitshd{"NAXIS3"} = 1 unless defined $fitshd{"NAXIS3"};   
}
	close(FITSHD);
	
	return %fitshd;
}

sub trim {
   my $string=$_;
   for ($string) {
       s/^\s+//;
       s/\s+$//;
	}
   return $string;
}

sub getRunID($){
   my $string;
   $_ = $_[0];
   s#.*/##;
   tr/[A-Z]/[a-z]/;
	$string = $_;
   return $string;
}

sub getFitsBaseName($){
   my $string;
   s#.*/##;
   s/.fits//;
   $string=$_;
   return $string;
}

# This function takes a string in the format of 
#  yyyy-mm-ddThh:mm:ss to julian day.
sub date2Julian($){
   my $string=$_[0];
   my @words=split("T",$string);
   my($year,$month,$day);
   my($hh,$mm,$ss);
   my($mjd,$ut);
   
   ($year,$month,$day)=split("-",$words[0]);
   ($hh,$mm,$ss)=split(":",$words[1]);
   $ut=$hh+$mm/60+$ss/3600;
   $mjd=cal2mjd($day, $month, $year, $ut/24);
   return $mjd;
}


sub strpos($\@){
   my $string=$_[0];
   my @list=@{$_[1]};
   my $i=0;
   my $result=-1;
   foreach (@list){
      if ($_ =~ $string){
         $result=$i;
      }
      $i++;
   }
   return $result;
}

