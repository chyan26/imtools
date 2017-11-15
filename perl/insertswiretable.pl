#! /usr/bin/perl


#my $file='/data/disk/chyan/Projects/SWIRE/Catalog/SWIRE2_N1_cat_IRAC24_16jun05.tbl';
#my $file='/data/disk/chyan/Projects/SWIRE/Catalog/SWIRE2_N2_cat_IRAC24_16jun05.tbl';
# my $file='/data/disk/chyan/Projects/SWIRE/Catalog/SWIRE2_XMM_cat_IRAC24_16jun05.tbl';
# my $file='/data/disk/chyan/Projects/SWIRE/Catalog/SWIRE3_CDFS_cat_IRAC24_21Dec05.tbl';
my $file='/data/disk/chyan/Projects/SWIRE/Catalog/SWIRE2_Lockman_cat_IRAC24_10Nov05.tbl';

open(FILE, $file);
while(<FILE>){
   if (/^\|/){next;}
   if (/^\\/){next;}
   my @word=split(/\s+/,$_);
   my $total=@word;
   my $t=0;
   for($i=1; $i<=$#word; $i++){
      #print("$word[$i] ,");
#    #foreach(@word){
       if ($i == $total-1){
          print("$word[$i] ");
       } else {
          print("$word[$i] ,");
          $t++;
       }
   }
   print("\n");
}

close(FILE);
