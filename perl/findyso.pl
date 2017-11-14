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
   
   $Id: = $
   $Locker: $
   $Log: $


=cut
# This finction is used to return program usage.
sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <RUNID>
 
 INPUT:
   -r|--ra     Center R.A. of query 
   -d|--dec    Declination
 OPTIONS:
   -x|--dra     RA range for query
   -y|--ddec    DEC range for query
   -c|--cat     Selection c2d region
   -s|--ds9     Output DS9 regions
   -p|--ds9type Selecting ds9 region type
   -l|--color   Selecting region color
   -y|--class   Selecting YSO class
   -t|--source  Selecting source name
 
 EXAMPLES:
  $SCRIPT_NAME --ra={RA in degree} --dec={Dec in degree} [options]

EOF
}


use warnings;
use strict;
use Cwd;
use Getopt::Long;
use DBI;

our $SCRIPT_NAME = 'findyso';

our %config=(
      SOURCE         => '',
      CRA            => -999.0,
      CDEC           => -999.0,
      DRA            => 0.0,
      DDEC           => 0.0,
      RAD            => 0.5,
      CAT            => '',
      COLOR          => 'red',
      CLASS          => 'NONE',
      DS9            => 0,
      DS9TYPE        => 'circle'
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
my($c2d_id,$ra,$dec,$j_flux_c,$h_flux_c,$k_flux_c,$i1_flux_c,$recno,
      $i2_flux_c,$i3_flux_c,$i4_flux_c,$m1_flux_c,$m2_flux_c,$alpha);
my($alpha_up, $alpha_low);

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
   "w|ddec=f"     => \$config{DDEC},
   "a|rad=f"      => \$config{RAD},
   "c|cat=s"      => \$config{CAT},
   "s|ds9"        => \$config{DS9},
   "p|ds9type=s"  => \$config{DS9TYPE},
   "t|source=s"   => \$config{SOURCE},
   "l|color=s"    => \$config{COLOR},
   "y|class=s"    => \$config{CLASS},
   "h|help"       => \$opt_help
   );

if($opt_help){
   usage();
   exit 0;
}   

# if ($config{CRA} == -999.0 or $config{CDEC} == -999.0){
#    usage();
#    exit 0;
# }
# Check the source name format
if ($config{SOURCE} ne ''){
    unless ($config{SOURCE} =~ /^J/ ){
      print "error: the format of source name is not correct.\n";
      exit 0;  
    }
}

# Define the class using SED
if ($config{CLASS} eq 'I') {
   $alpha_up=999.0;
   $alpha_low=0.3;

} elsif ($config{CLASS} eq 'F'){
   $alpha_up=0.3;
   $alpha_low=-0.3;
} elsif ($config{CLASS} eq 'II'){
   $alpha_up=-0.3;
   $alpha_low=-1.6;
} elsif ($config{CLASS} eq 'III'){
   $alpha_up=-1.6;
   $alpha_low=-999.0;
} else {
   $alpha_up=999.0;
   $alpha_low=-999.0;
}

# Checking the table for query
if ($config{CAT} eq 'c2d' || $config{CAT} eq 'oph' || $config{CAT} eq 'per'  ||
    $config{CAT} eq 'ser' || $config{CAT} eq 'cha' || $config{CAT} eq 'lup1' ||
    $config{CAT} eq 'lup3' || $config{CAT} eq 'lup4' ){
} else {   
   print "error: catalog is not in database.\n";
   exit 0;
}

$dbh = DBI->connect('DBI:Pg:dbname=C2D', 'psqluser', '2rrxorlx');
if ($dbh) {

     if ($config{CAT} eq 'c2d'){
         $sth = $dbh->prepare("SELECT  c2d_id, recno, ra,dec, alpha
            from c2d_yso 
            WHERE RA < $config{CRA}+$config{RAD} and RA > $config{CRA}-$config{RAD} and 
               DEC > $config{CDEC}-$config{RAD} and DEC < $config{CDEC}+$config{RAD} and
               alpha >= $alpha_low and alpha < $alpha_up"); 
                    
        $sth->execute;
        
        while (@vetor = $sth->fetchrow) {
            ($c2d_id,$recno,$ra,$dec,$alpha)=@vetor;
             if ($config{DS9}){
               printf("fk5; %s(%11.7fd %11.7fd,0.005) # color = %s\n",$config{DS9TYPE},$ra,
		  $dec,$config{COLOR});
             } else {
               printf("%4s %4s SSTc2d%20s %f %f %8.2f\n",$i,$recno,
                  $c2d_id,$ra,$dec,$alpha);
            }
            $i++;
                  
        }
                    
     } elsif ($config{DRA} != 0 && $config{DDEC} != 0){
         my $catname=$config{CAT}.'_yso';
         $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C,  H_FLUX_C, KS_FLUX_C, 
            IR1_flux_c,IR2_flux_c, IR3_flux_c, IR4_flux_c, MP1_flux_c, MP2_flux_c, alpha
            from $catname 
            WHERE RA < $config{CRA}+$config{DRA} and RA > $config{CRA}-$config{DRA} and 
               DEC > $config{CDEC}-$config{DDEC} and DEC < $config{CDEC}+$config{DDEC}");      

         $sth->execute;
         
         while (@vetor = $sth->fetchrow) {
            ($c2d_id,$ra,$dec,$j_flux_c,$h_flux_c,$k_flux_c,$i1_flux_c,
               $i2_flux_c,$i3_flux_c,$i4_flux_c,$m1_flux_c,$m2_flux_c,$alpha)=@vetor;
            
            if ($config{DS9}){
               printf("fk5; %s(%11.7fd %11.7fd,0.005) # color = %s\n",$config{DS9TYPE},$ra,
		  $dec,$config{COLOR});
             } else {
               printf("%4s %23s %f %f %8.2f %8.2f %8.2f %8.2f\n",$i,
                  $c2d_id,$ra,$dec,$alpha, $j_flux_c, $h_flux_c, $k_flux_c);
            }
            $i++;
         }

      } elsif ($config{SOURCE} ne ''){
         my $source_name="SSTc2d ".$config{SOURCE};
         my $catname=$config{CAT}.'_yso';
         $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C,  H_FLUX_C, KS_FLUX_C, 
            IR1_flux_c,IR2_flux_c, IR3_flux_c, IR4_flux_c, MP1_flux_c, MP2_flux_c, alpha
            from $catname 
             WHERE trim(BOTH ' ' from c2d_id) = '$source_name'");
          $sth->execute;
         
         while (@vetor = $sth->fetchrow) {
            ($c2d_id,$ra,$dec,$j_flux_c,$h_flux_c,$k_flux_c,$i1_flux_c,
               $i2_flux_c,$i3_flux_c,$i4_flux_c,$m1_flux_c,$m2_flux_c,$alpha)=@vetor;
            
            if ($config{DS9}){
               printf("fk5; %s(%11.7fd %11.7fd,0.005) # color = %s\n",$config{DS9TYPE},$ra,
		  $dec,$config{COLOR});
             } else {
               printf("%4s %23s %f %f %8.2f %8.2f %8.2f %8.2f\n",$i,
                  $c2d_id,$ra,$dec,$alpha, $j_flux_c, $h_flux_c, $k_flux_c);
            }
            $i++;
         }
        

     } else {
         my $catname=$config{CAT}.'_yso';
         
         $sth = $dbh->prepare("SELECT  c2d_ID,RA, DEC, J_FLUX_C,  H_FLUX_C, KS_FLUX_C, 
            IR1_flux_c,IR2_flux_c, IR3_flux_c, IR4_flux_c, MP1_flux_c, MP2_flux_c, alpha
            from $catname 
            WHERE RA < $config{CRA}+$config{RAD} and RA > $config{CRA}-$config{RAD} and 
               DEC > $config{CDEC}-$config{RAD} and DEC < $config{CDEC}+$config{RAD}");      

         $sth->execute;
         
         while (@vetor = $sth->fetchrow) {
            ($c2d_id,$ra,$dec,$j_flux_c,$h_flux_c,$k_flux_c,$i1_flux_c,
               $i2_flux_c,$i3_flux_c,$i4_flux_c,$m1_flux_c,$m2_flux_c,$alpha)=@vetor;
            
            if ($config{DS9}){
               printf("fk5; %s(%11.7fd %11.7fd,0.005) # color = %s\n",$config{DS9TYPE},$ra,
		  $dec,$config{COLOR});
            } else {
               printf("%4s %23s %f %f %8.2f %8.2f %8.2f %8.2f\n",$i,
                  $c2d_id,$ra,$dec,$alpha, $j_flux_c, $h_flux_c, $k_flux_c);
            }
            $i++;
         }
     }

   $sth->finish;
   $dbh->disconnect();
} else {
   print "Cannot connect to Postgres server: $DBI::errstr\n";
   print " db connection failed\n";
}

