#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             findmir
#----------------------------------------------------------------------------
# Contents:   
#----------------------------------------------------------------------------
# Part of:     C2D catalog tool                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------
=pod
  
  History
   
   $Id: findmir.pl,v 1.2 2011/03/15 10:22:25 chyan Locked $
   $Locker: chyan $
   $Log:	findmir.pl,v $
# Revision 1.2  2011/03/15  10:22:25  chyan
# Preparing for adding output error in code.
# 


=cut
# This finction is used to return program usage.
sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <RUNID>
 
 INPUT:
   -r|--ra     Center R.A. of query 
   -d|--dec    Declination
 OPTIONS:
   -m|--mips   Selecting stars based on MIPS detections.
   -f|--full   Selectiing sources from FULL catalog instead of High Relibility  
 
 EXAMPLES:
  $SCRIPT_NAME --ra={RA in degree} --dec={Dec in degree} [options]

EOF
}


use warnings;
use strict;
use Cwd;
use Getopt::Long;
use DBI;

our $SCRIPT_NAME = 'findmir';

our %config=(
      SOURCE         => '',
      CRA            => -999.0,
      CDEC           => -999.0,
      DRA            => 0,
      DDEC           => 0,
      RAD            => 0.5,
      QDET           => 'Q',
      CAT            => '',
      DS9            => 0,
      FULL           => 0,
      MIPS           => 0
      );

my $dbh;
my $sth;
my @vetor;
my $field;

my $cra = 247.97944;
my $cdec= -24.52919;
my $rad=0.5;
my $MP1_Q_det='A';
my $i=1;

my($opt_help);
my($c2d_id,$ra,$dec,$j_flux_c,$j_d_flux_c,$h_flux_c,$h_d_flux_c,$k_flux_c,$k_d_flux_c,
      $i1_flux_c,$i1_d_flux_c,$i2_flux_c,$i2_d_flux_c,$i3_flux_c,$i3_d_flux_c,$i4_flux_c,
      $i4_d_flux_c,$m1_flux_c,$m1_d_flux_c,$m2_flux_c,$m2_d_flux_c);
my($cattype);

# check if the program id is passed.
if(@ARGV == 0){
   usage();
   exit 0;
}

# Get options from command line
GetOptions(
   "r|ra=f"       => \$config{CRA},
   "d|dec=f"      => \$config{CDEC},
   "x|dra=f"      => \$config{DRA},
   "y|ddec=f"     => \$config{DDEC},
   "a|rad=f"      => \$config{RAD},
   "q|qdet=s"     => \$config{QDET},
   "c|cat=s"      => \$config{CAT},
   "s|ds9"        => \$config{DS9},
   "t|source=s"     => \$config{SOURCE},
   "f|full"       => \$config{FULL},
   "m|mips"       => \$config{MIPS},
   "h|help"       => \$opt_help
   );

if($opt_help){
   usage();
   exit 0;
}   
if ($config{CRA} ==  -999.0 or $config{CDEC} ==  -999.0){
   if ($config{SOURCE} eq ''){
      usage();
      exit 0;
   }
}


# Define catalog type
if ($config{FULL} == 1){
   $cattype='oph_full';
} else {
   $cattype='oph_hrel';
}

# Check the condition of detection quality
unless ($config{QDET} =~ /[ABCDEQ]/ ){
   print "error: the condition of detection quality is not allowed.\n";
   exit 0;  
}

# Check the source name format
if ($config{SOURCE} ne ''){
    unless ($config{SOURCE} =~ /^J/ ){
      print "error: the format of source name is not correct.\n";
      exit 0;  
    }
}

$dbh = DBI->connect('DBI:Pg:dbname=C2D', 'psqluser', '2rrxorlx');
if ($dbh) {
  
   if ($config{QDET} ne 'Q'){
      $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C,  H_FLUX_C, KS_FLUX_C, 
         IR1_flux_c,IR2_flux_c, IR3_flux_c, IR4_flux_c, MP1_flux_c, MP2_flux_c
         from $cattype 
         WHERE RA < $config{CRA}+$config{RAD} and RA > $config{CRA}-$config{RAD} and 
            DEC > $config{CDEC}-$config{RAD} and DEC < $config{CDEC}+$config{RAD} and 
            Q_pos != 'Q' and MP1_flux_c > 0 and 
            MP1_Q_det_c = '$config{QDET}'");      
   }
   
   # Choose sources with detection larger than 3-sigma
   if ($config{QDET} eq 'Q'){
      
      #
      if ($config{MIPS} == 1){
         $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C,  H_FLUX_C, KS_FLUX_C, 
            IR1_flux_c,IR2_flux_c, IR3_flux_c, IR4_flux_c, MP1_flux_c, MP2_flux_c
            from $cattype 
            WHERE RA < $config{CRA}+$config{RAD} and RA > $config{CRA}-$config{RAD} and 
            DEC > $config{CDEC}-$config{RAD} and DEC < $config{CDEC}+$config{RAD} and 
            Q_pos != 'Q' and MP1_flux_c > 0 and 
            (MP1_Q_det_c = 'A' or MP1_Q_det_c = 'B' or MP1_Q_det_c = 'K')");

      } elsif ($config{DRA} != 0 && $config{DDEC} != 0){
         $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C, J_D_flux_c,  H_FLUX_C, H_D_FLUX_C, 
            KS_FLUX_C, KS_D_FLUX_C, IR1_flux_c, IR1_D_flux_c, IR2_flux_c, IR2_D_flux_c, IR3_flux_c, 
            IR3_D_flux_c, IR4_flux_c, IR4_D_flux_c, MP1_flux_c, MP1_D_flux_c, MP2_flux_c, MP2_D_flux_c
            from $cattype 
             WHERE RA < $config{CRA}+$config{DRA} and RA > $config{CRA}-$config{DRA} and 
               DEC > $config{CDEC}-$config{DDEC} and DEC < $config{CDEC}+$config{DDEC} and 
               Q_pos != 'Q' and 
               (IR1_Q_det_c = 'A' or IR1_Q_det_c = 'B' or IR1_Q_det_c = 'C' or IR1_Q_det_c = 'K') and 
               (IR2_Q_det_c = 'A' or IR2_Q_det_c = 'B' or IR2_Q_det_c = 'C' or IR2_Q_det_c = 'K') and
               (IR3_Q_det_c = 'A' or IR3_Q_det_c = 'B' or IR3_Q_det_c = 'C' or IR3_Q_det_c = 'K') and
               (IR4_Q_det_c = 'A' or IR4_Q_det_c = 'B' or IR4_Q_det_c = 'C' or IR4_Q_det_c = 'K')");
      
      } elsif ($config{SOURCE} ne ''){
	 my $source_name="SSTc2d ".$config{SOURCE};
         $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C, J_D_flux_c,  H_FLUX_C, H_D_FLUX_C, 
            KS_FLUX_C, KS_D_FLUX_C, IR1_flux_c, IR1_D_flux_c, IR2_flux_c, IR2_D_flux_c, IR3_flux_c, 
            IR3_D_flux_c, IR4_flux_c, IR4_D_flux_c, MP1_flux_c, MP1_D_flux_c, MP2_flux_c, MP2_D_flux_c
            from $cattype 
             WHERE trim(BOTH ' ' from c2d_id) = '$source_name'");
         
    
      } else {
      
         $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C, J_D_flux_c,  H_FLUX_C, H_D_FLUX_C, 
            KS_FLUX_C, KS_D_FLUX_C, IR1_flux_c, IR1_D_flux_c, IR2_flux_c, IR2_D_flux_c, IR3_flux_c, 
            IR3_D_flux_c, IR4_flux_c, IR4_D_flux_c, MP1_flux_c, MP1_D_flux_c, MP2_flux_c, MP2_D_flux_c
            from $cattype 
            WHERE RA < $config{CRA}+$config{RAD} and RA > $config{CRA}-$config{RAD} and 
               DEC > $config{CDEC}-$config{RAD} and DEC < $config{CDEC}+$config{RAD} and 
               Q_pos != 'Q' and 
               (IR1_Q_det_c = 'A' or IR1_Q_det_c = 'B' or IR1_Q_det_c = 'C' or IR1_Q_det_c = 'K') and 
               (IR2_Q_det_c = 'A' or IR2_Q_det_c = 'B' or IR2_Q_det_c = 'C' or IR2_Q_det_c = 'K') and
               (IR3_Q_det_c = 'A' or IR3_Q_det_c = 'B' or IR3_Q_det_c = 'C' or IR3_Q_det_c = 'K') and
               (IR4_Q_det_c = 'A' or IR4_Q_det_c = 'B' or IR4_Q_det_c = 'C' or IR4_Q_det_c = 'K')");
      }
   
   }
    
   if (lc($config{CAT}) eq 'yso'){
       
       $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C, J_D_flux_c,  H_FLUX_C, H_D_FLUX_C, 
            KS_FLUX_C, KS_D_FLUX_C, IR1_flux_c, IR1_D_flux_c, IR2_flux_c, IR2_D_flux_c, IR3_flux_c, 
            IR3_D_flux_c, IR4_flux_c, IR4_D_flux_c, MP1_flux_c, MP1_D_flux_c, MP2_flux_c, MP2_D_flux_c
         from oph_yso 
         WHERE RA < $config{CRA}+$config{RAD} and RA > $config{CRA}-$config{RAD} and 
            DEC > $config{CDEC}-$config{RAD} and DEC < $config{CDEC}+$config{RAD} and 
            OBJECT_TYPE != 'Galc' ");
     
   } 
   $sth->execute;

   while (@vetor = $sth->fetchrow) {
      
      ($c2d_id,$ra,$dec,$j_flux_c,$j_d_flux_c,$h_flux_c,$h_d_flux_c,$k_flux_c,$k_d_flux_c,
      $i1_flux_c,$i1_d_flux_c,$i2_flux_c,$i2_d_flux_c,$i3_flux_c,$i3_d_flux_c,$i4_flux_c,
      $i4_d_flux_c,$m1_flux_c,$m1_d_flux_c,$m2_flux_c,$m2_d_flux_c)=@vetor;
      
      if ($config{DS9}){
         printf("fk5; circle(%11.7fd %11.7fd,0.005)\n",$ra,$dec);
      } else {
         printf("%4s %23s %f %f %11.4f %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F %11.4F\n",$i,
            $c2d_id,$ra,$dec,$j_flux_c,$j_d_flux_c,$h_flux_c,$h_d_flux_c,$k_flux_c,$k_d_flux_c,
            $i1_flux_c,$i1_d_flux_c,$i2_flux_c,$i2_d_flux_c,$i3_flux_c,$i3_d_flux_c,$i4_flux_c,
            $i4_d_flux_c,$m1_flux_c,$m1_d_flux_c,$m2_flux_c,$m2_d_flux_c);
      }
      $i++;
   }


   $sth->finish;
   $dbh->disconnect();
} else {
   print "Cannot connect to Postgres server: $DBI::errstr\n";
   print " db connection failed\n";
}

