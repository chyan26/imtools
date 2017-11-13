#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             swirecat
#----------------------------------------------------------------------------
# Contents:   
#----------------------------------------------------------------------------
# Part of:     SWIRE catalog tool                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------
=pod
  
  History
   
   $Id: = $
   $Locker: $
   $Log: $


=cut
# This finction is used to return program usage.
sub usage(){
   print<<EOF

 USAGE: $SCRIPT_NAME <RUNID>
 
 INPUT:
   -r|--ra      Runid
   -d|--dec     Declination
 OPTIONS:
 
 
 EXAMPLES:
  $SCRIPT_NAME --ra={RA in degree} --dec={Dec in degree} [options]

EOF
}

sub log10 {
  my $n = shift;
  return log($n)/log(10);
}


use warnings;
use strict;
use Cwd;
use Getopt::Long;
use DBI;
our $SCRIPT_NAME = 'findswire';

our %config=(
      CRA            => 0,
      CDEC           => 0,
      FLUX           => 1,
      MAG            => 0,
      RAD            => 0.5,
      QDET           => 'Q',
      DS9            => 0
      );

my $dbh;
my $sth;
my @vetor;
my $field;


my $rad=0.5;
my $i=1;

my($opt_help);

my($object,$ra,$dec,$mj,$mje, $mh,$mhe,$mk,$mke,$flux_kr_36,$uncf_kr_36
      ,$flux_kr_45,$uncf_kr_45,$flux_kr_58,$uncf_kr_58,$flux_kr_80,$uncf_kr_80
      ,$flux_kr_24,$uncf_kr_24);

my($flux_j_mj,$flux_h_mj,$flux_k_mj,$uncf_j_mj,$uncf_h_mj,$uncf_k_mj);

my($mag_36,$mag_45,$mag_58,$mag_80,$mag_24,$magerr_36,
   $magerr_45,$magerr_58,$magerr_80,$magerr_24);



# Get options from command line
GetOptions(
   "f|flux"       => \$config{FLUX},
   "m|mag"        => \$config{MAG},
   "a|rad=f"      => \$config{RAD},
   "s|ds9"        => \$config{DS9},
   "h|help"       => \$opt_help
   );

if($opt_help){
   usage();
   exit 0;
} 


if($config{MAG}){
	$config{FLUX} = 0;
}

my @table=('swire_n1','swire_n2','swire_xmm','swire_cdfs','swire_lockman','swire_s1');

$dbh = DBI->connect('DBI:Pg:dbname=SWIRE', 'psqluser', '2rrxorlx');


if ($dbh) {
   foreach(@table){
      $sth = $dbh->prepare("SELECT  object,ra,dec,flux_kr_36,uncf_kr_36
         ,flux_kr_45,uncf_kr_45,flux_kr_58,uncf_kr_58,flux_kr_80,uncf_kr_80
         ,flux_kr_24,uncf_kr_24 from ".$_." where (flux_kr_45 > 0 and 
         flux_kr_58 > 0 and flux_kr_80 > 0) ");
      
      $sth->execute;
   
      while (@vetor = $sth->fetchrow) {
         ($object,$ra,$dec,$flux_kr_36,$uncf_kr_36
         ,$flux_kr_45,$uncf_kr_45,$flux_kr_58,$uncf_kr_58,$flux_kr_80,$uncf_kr_80
         ,$flux_kr_24,$uncf_kr_24)=@vetor;
         
         unless (defined($flux_kr_36)){$flux_kr_36 = -99.0;}
         unless (defined($flux_kr_45)){$flux_kr_45 = -99.0;}
         unless (defined($flux_kr_58)){$flux_kr_58 = -99.0;}
         unless (defined($flux_kr_80)){$flux_kr_80 = -99.0;}
         unless (defined($flux_kr_24)){$flux_kr_24 = -99.0;}
         
         unless (defined($uncf_kr_36)){$uncf_kr_36 = -99.0;}
         unless (defined($uncf_kr_45)){$uncf_kr_45 = -99.0;}
         unless (defined($uncf_kr_58)){$uncf_kr_58 = -99.0;}
         unless (defined($uncf_kr_80)){$uncf_kr_80 = -99.0;}
         unless (defined($uncf_kr_24)){$uncf_kr_24 = -99.0;}
         
         
         if ($config{MAG}){
   
            if ($flux_kr_36 ge 0){
	            $mag_36=2.5*log10(280900.0/($flux_kr_36*0.001));
	            $magerr_36=log10(($uncf_kr_36/$flux_kr_36)+1)/log10(2.512);
            } else {
	            $mag_36=-99.0;
               $magerr_36=-99.0;     
            }
            
            if ($flux_kr_45 ge 0){
               $mag_45=2.5*log10(179700.0/($flux_kr_45*0.001));
               $magerr_45=log10(($uncf_kr_45/$flux_kr_45)+1)/log10(2.512);
            } else {
               $mag_45=-99.0;
               $magerr_45=-99.0;
            }
            
            if ($flux_kr_58 ge 0){
               $mag_58=2.5*log10(115000.0/($flux_kr_58*0.001));
               $magerr_58=log10(($uncf_kr_58/$flux_kr_58)+1)/log10(2.512);
            } else {
               $mag_58=-99.0;
               $magerr_58=-99.0;
            }
            
            if ($flux_kr_80 ge 0){
               $mag_80=2.5*log10(64130.0/($flux_kr_80*0.001));
               $magerr_80=log10(($uncf_kr_80/$flux_kr_80)+1)/log10(2.512);
            } else {
               $mag_80=-99.0;
               $magerr_80=-99.0;
            }
            
            if ($flux_kr_24 ge 0){
               $mag_24=2.5*log10(7170.0/($flux_kr_24*0.001));
               $magerr_24=log10(($uncf_kr_24/$flux_kr_24)+1)/log10(2.512);            
            } else {
               $mag_24=-99.0;
               $magerr_24=-99.0;
            }
            
   #          printf("%4s %26s %f %f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f 
   #          %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f \n",$i,$object
   #          ,$ra,$dec,$mj,$mje, $mh,$mhe,$mk,$mke,$flux_kr_36,$uncf_kr_36
   #          ,$flux_kr_45,$uncf_kr_45,$flux_kr_58,$uncf_kr_58,$flux_kr_80,$uncf_kr_80
   #          ,$flux_kr_24,$uncf_kr_24);
   #          
            printf("%4s %26s %8.6f %f %7.3f %7.3f %7.3f %7.3f %7.3f %7.3f %7.3f %7.3f %7.3f %7.3f \n",$i,$object
            ,$ra,$dec,$mag_36,$magerr_36
            ,$mag_45,$magerr_45,$mag_58,$magerr_58,$mag_80,$magerr_80
            ,$mag_24,$magerr_24);
         
         } else {	
            $flux_j_mj=10**3*10**(-(($mj-8.00)/2.5));
            $flux_h_mj=10**3*10**(-(($mh-7.52)/2.5));
            $flux_k_mj=10**3*10**(-(($mk-7.06)/2.5));
         
            $uncf_j_mj=$flux_j_mj*((2.512**$mje)-1);
            $uncf_h_mj=$flux_h_mj*((2.512**$mhe)-1);
            $uncf_k_mj=$flux_k_mj*((2.512**$mke)-1);
            
            printf("%4s %26s %f %f %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e %9.3e \n",$i,$object
            ,$ra,$dec,$flux_j_mj,$uncf_j_mj, $flux_h_mj,$uncf_h_mj,$flux_k_mj,$uncf_k_mj
            ,$flux_kr_36*0.001,$uncf_kr_36*0.001
            ,$flux_kr_45*0.001,$uncf_kr_45*0.001,$flux_kr_58*0.001,$uncf_kr_58*0.001
            ,$flux_kr_80*0.001,$uncf_kr_80*0.001,$flux_kr_24*0.001,$uncf_kr_24*0.001);
         
         }
         $i++;
         
      }
      $sth->finish;
   }
   
   
   $dbh->disconnect();
} else {
   print "Cannot connect to Postgres server: $DBI::errstr\n";
   print " db connection failed\n";
}

