#! /usr/bin/perl
#----------------------------------------------------------------------------
#                             wircamcpstack
#----------------------------------------------------------------------------
# Contents:    
#----------------------------------------------------------------------------
# Part of:     WIRCam C pipeline                               
# From:        ASIAA                    
# Author:      Chi-hung Yan(chyan@asiaa.sinica.edu.tw)                       
#----------------------------------------------------------------------------
=pod
  
  History
   
   $Id: wircamcpstack.pl,v 1.20 2009/08/11 11:49:53 chyan Locked $
   $Locker: chyan $
   $Log:	wircamcpstack.pl,v $
# Revision 1.20  2009/08/11  11:49:53  chyan
# Adding the support of old database (pre2008a) and skiping exist WM files.
# 


=cut


do "/cfht/bin/wircamlibperl";

use warnings;
use strict;
use Cwd;
use Getopt::Long;
use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :shortnames );
use Astro::FITS::CFITSIO qw( :constants );
use LWP::UserAgent;


our $SCRIPT_NAME = 'wircamcpstack';
our %config=(
      ONEPT          => 0,
      RUNID          => '',
      DOLOG          => 0,
      RAWWCS         => 0,
      LOGFILE        => '',
      STATUSLOG      => '',
      RAWPATH        => $ENV{'WCRAWPATH'},
      PROCESSEDPATH  => $ENV{'WCPCSPATH'},
      STACKPATH      => $ENV{'WCSTKPATH'},
      CALIBPATH      => $ENV{'WCCLBPATH'},
      BADPIXMAP      => 'badpix16_20120727HST192309_v100.fits',
      SEXCONF        => './wircamcp_sex.conf',
      SCAMPCONF      => './wircamcp_scamp.conf',
      MISSFITSCONF   => './wircamcp_missfits.conf',
      WWCONF         => './wircamcp_ww.conf',
      SWARPCONF      => './wircamcp_swarp.conf',
      PIXELSCALETYPE => 'MEDIAN',
      PIXELSCALE     => 0.0,
      GROUPFILE      => 'group.txt',
      FILTER         => '',
      DSTTHRES       => 1.0,
      RESAMPLE       => 'LANCZOS2',
      MOSAICTYPE     => 'SAME_CRVAL',
      COMBINETYPE    => 'MEDIAN',
      MAXOMEGA       => 10.0,
      OMEGATOOBIG    => 0,
      NOLINK         => 0,
      ALLDATA        => 0,
      NOSPLIT        => 0,
      NOSEX          => 0,
      NOWW           => 0,
      NOSCAMP        => 0,
      NOSWARP        => 0,
      NOCLEAN        => 0,
      JAVA           => 0,
      );

sub dumpSettings(){
   $SIG{INT} = \&userStop;
   open(LOG,">> $config{LOGFILE}");
   printf(LOG "------------------");
   printf(LOG "Start of program settings");
   printf(LOG "------------------\n");
   foreach (keys(%config)){
      printf(LOG "%s => %s\n",$_,$config{$_});
   }
   printf(LOG "------------------");
   printf(LOG "End of program settings");
   printf(LOG "------------------\n");
   close(LOG);
}

# This function checks if the odometer is validated or not. 
sub checkValidated($){
   my $ua = LWP::UserAgent->new;
   $ua->timeout(10);
   $ua->env_proxy;
   my($pre2008a_url,$url,$response);
   
   if ($_[0] >= 968148){
      $url='http://www.cfht.hawaii.edu/~chyan/expinfo.php?obsid='.$_[0];
      $response = $ua->get($url);
   } else {
      $pre2008a_url='http://www.cfht.hawaii.edu/~chyan/aexpinfo.php?obsid='.$_[0];
      $response = $ua->get($pre2008a_url);
   }
      
   #Check if user set to ALLDATA
   if ($config{ALLDATA} == 0){
      #Check if the content is all empty.
      if ($response->content=~/\S/){
         my @words=split(" ",$response->content);
         if ($words[0] eq "VALIDATED" and $words[1] eq "OBJECT"){
            return 1;
         } else {
            return 0;
         }
      } else {
         return 1;   # set file to be validated if no entry in database
      }
   } else {
      return 1;
   }
}


# This function is used to update the status log.
sub updateStatus($){
   my $string = $_[0];
   open(STATUS,">$config{STATUSLOG}");
   printf(STATUS "%s",$string);
   close(STATUS);
}

# This function is used to terminate the program with program encountered error message. 
sub quitStack($){
   $SIG{INT} = \&userStop;
   my $message=$_[0];
   # Reomve status log
   if (-e $config{STATUSLOG}){
      system("rm -rf $config{STATUSLOG}");
   }
   die("$message\n");
}

# This function dump message to file
#  The proto-type"
#   dumpToFile($file,$string)
sub dumpToFile(\$\$){
   $SIG{INT} = \&userStop;
   my $file=${$_[0]};
   my $string=${$_[1]};
   my $date;
   # get date from system
   
   open(DATE,"date +%Y-%m-%dCST%T |");
   while(<DATE>){
      $date=trim($_);
   }
   close(DATE);
   open(LOGFILE,">> $file");
   printf(LOGFILE "%s: %s \n",$date,$string);
   close(LOGFILE);
}

sub buildDB($){
   $SIG{INT} = \&userStop;
   my %datadb;
   my @list = <$_[0]/*p.fits>;
   foreach(@list){
      my $file = $_;
      s/p.fits//;
      s#.*/##;
      my $odoname = $_;
      my %fitshd = getFitshd($file);
      $datadb{$odoname}={%fitshd};
      
   }
   return %datadb;
}

# This function check the validality of each processed file then make link
#  to stacking directory
sub mkLink(\$\@){
   my $filter=${$_[0]};
   my @file=@{$_[1]};
   my($file,$string);
   foreach(@file){   
      s#.*/##;
      s/p.fits//;
      my $redfile = $_;
       if (checkValidated($redfile) == 1){
#          printf("%s Validated.\n",$redfile);
         $file = $redfile.'p.fits';
         system("ln -fs $config{PROCESSEDPATH}/$config{RUNID}/$filter/$file $config{STACKPATH}/$config{RUNID}/$filter/\n");
          $string='Validated: '.$file;
         dumpToFile($config{LOGFILE},$string);
      } else {
         $file = $redfile.'p.fits';
         $string='Rejected: '.$file;
         dumpToFile($config{LOGFILE},$string);
      }
   }
}

# This function replace the current WCS to the WCS in Raw image.
sub useRawWCS(\@){
   $SIG{INT} = \&userStop;
   my @file=@{$_[0]};
   my $rawpath=$config{RAWPATH}.'/'.$config{RUNID};
   my $redfile;
   my $rawfile;
   my $status=0;
   my($newtr,$rawtr);
   foreach (@file){
#print "$_\n";
      $redfile=$_;
      s/p-[0-9][0-9].fits/o.fits/;
      
#print "$_\n";
      $rawfile=$_;
      print "Use $rawfile to replace WCS in $redfile.\n";
      $rawfile=$rawpath.'/'.$rawfile;
      if (!-e $rawfile){
         die "Error: can not find raw image to replace WCS.\n";
      }
      
      for (my $i=1;$i<=4;$i++){
          my $new_ext = $redfile.'['.$i.']';
         my $raw_ext = $rawfile.'['.$i.']';
      #    print "$extension\n";
          $newtr = Astro::FITS::CFITSIO::open_file($new_ext,READWRITE,$status);
          $rawtr = Astro::FITS::CFITSIO::open_file($raw_ext,READONLY,$status);
         my $ehd = $rawtr->read_header;
         my $nhd = $newtr->read_header;
                  
#if (@$nhd{WCS_ORIG} eq "'IMWCS-linear'"){
            if (defined(@$nhd{CROTA1})){$newtr->delete_key("CROTA1",$status);}
            if (defined(@$nhd{CROTA2})){$newtr->delete_key("CROTA2",$status);}
            if (defined(@$nhd{SECPIX1})){$newtr->delete_key("SECPIX1",$status);}
            if (defined(@$nhd{SECPIX2})){$newtr->delete_key("SECPIX2",$status);}
            if (defined(@$nhd{CDELT1})){$newtr->delete_key("CDELT1",$status);}
            if (defined(@$nhd{CDELT2})){$newtr->delete_key("CDELT2",$status);}
            if (defined(@$nhd{WCSTOL})){$newtr->delete_key("WCSTOL",$status);}
#         }

         
         $newtr->update_key_flt("CRVAL1",@$ehd{CRVAL1},10,"",$status);
         $newtr->update_key_flt("CRVAL2",@$ehd{CRVAL2},10,"",$status);
         $newtr->update_key_flt("CRPIX1",@$ehd{CRPIX1},10,"",$status);
         $newtr->update_key_flt("CRPIX2",@$ehd{CRPIX2},10,"",$status);
         $newtr->update_key_flt("CD1_1",@$ehd{CD1_1},10,"",$status);
         $newtr->update_key_flt("CD1_2",@$ehd{CD1_2},10,"",$status);
         $newtr->update_key_flt("CD2_1",@$ehd{CD2_1},10,"",$status);
         $newtr->update_key_flt("CD2_2",@$ehd{CD2_2},10,"",$status);
         
         $nhd = $newtr->read_header;
       }
      print "Done\n";
   }
}

# This function is used to constructe the list of exposure groups
#  The proto-type:
#     pickgroup(%datadb,@filelist,$distance,$groupfile)
sub pickgroup(\%\@\$\$){
    $SIG{INT} = \&userStop;
    my %datadb=%{$_[0]};
    my @file=@{$_[1]};
    my $dst=${$_[2]};
    my $tempfile=${$_[3]};
    my($cra,$cdec,$sep,$fgroup);
    my(@gra,@gdec)=();
    my($ra,$dec);
    my(@ungroup,@group)=();
    my $pt;
    my($list_obj,$object,@groupname);
   
    foreach (@file){
        s#.*/##;
        s/p.fits//;
        my $odoname = $_;
        @ungroup=(@ungroup,$odoname);
    }
    open(TMPFILE,">$tempfile");
    close(TMPFILE);
    my $ungroup=@ungroup;
    @groupname=();
    $pt=0;
    while ($ungroup != 0){
       
        my @temp = @ungroup;
        my $i=0;
        @group=();
        @ungroup=();
        
        
        foreach(@temp){
            my $odoname = $_;  
            if ($i eq 0){ 
                $cra=$datadb{$odoname}{CRVAL1};
                $cdec=$datadb{$odoname}{CRVAL2};             
            }
            
            $ra=$datadb{$odoname}{CRVAL1};
            $dec=$datadb{$odoname}{CRVAL2};
            $sep=sqrt((($ra-$cra)**2)+(($dec-$cdec)**2)); 
            if ($sep <= $dst){
                @group=(@group,$odoname);
                $object=$datadb{$odoname}{OBJECT};
            } else {
              @ungroup=(@ungroup,$odoname);
            }
            $i++;
        }
          
         # First check if the object name is defined.
          if (defined $list_obj){
            # if the object name exist, check all group name
            foreach(@groupname){
               my $temp=trim($_);
               # Once the matched name found in name list(@groupnames), then do pt++ and
               #   do the next loop.
               if ($object eq $temp){
                  $pt++;
                  $list_obj=$object.'_'.$pt;
                  last;
               }
               if ($object ne $temp){
                  $list_obj=$object;
               } 
            }
             @groupname=(@groupname,$list_obj);
          } else {
             $list_obj=$object;
            @groupname=(@groupname,$list_obj);
          }
          open(TMPFILE,">>$tempfile");
          foreach (@group){
             printf (TMPFILE "%s ",$_);
          }
          printf (TMPFILE "%s ",$list_obj);
          printf (TMPFILE "\n");
          close(TMPFILE);
          my $group=@group;
          $ungroup=$ungroup-$group;
       }
}


# This function is used to execute missfits.
#  Proto-type:
#    runMissfits(@filelist)
sub runMissfits(\@){
   $SIG{INT} = \&userStop;
   my @file=@{$_[0]}; 
   
	# Generate a MissFITS configuration file
	my $precommand = join(" ","missfits -dd >",$config{MISSFITSCONF});
	dumpToFile($config{LOGFILE},$precommand);
	if (system("$precommand 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
		quitStack("Error: Can't produce MissFITS configuration file for RUNID=$config{RUNID}.");
	}
   
   
   foreach (@file){
      my $file=trim($_);
      
      if ($config{DOLOG} == 0) {
         my $command=join(" ","missfits -c $config{MISSFITSCONF}",
                           "-SLICE_KEYWORD FOTC,FOTCS,FOTCN,ABSOR",
                           "-SLICEKEY_FORMAT _%02d",
                           "-OUTFILE_TYPE SLICE",
                           "-SLICE_SUFFIX -%02d.fits",
                           "-VERBOSE_TYPE NORMAL -NTHREADS 1",
                           "-SAVE_TYPE REPLACE",
                           "-WRITE_XML Y -XML_NAME missfits.xml",
                           "-XSL_URL missfits.xsl",
                           $file);
         if (system("$command") != 0){
            quitStack("Error: can not finish missfits.");
         }
      } else {
         my $command=join(" ","missfits -c $config{MISSFITSCONF}",
                           "-SLICE_KEYWORD FOTC,FOTCS,FOTCN,ABSOR",
                           "-OUTFILE_TYPE SLICE",
                           "-SLICEKEY_FORMAT _%02d",
                           "-SLICE_SUFFIX -%02d.fits",
                           "-VERBOSE_TYPE NORMAL -NTHREADS 1",
                           "-SAVE_TYPE REPLACE",
                           "-WRITE_XML Y -XML_NAME missfits.xml",
                           "-XSL_URL missfits.xsl",
                           $file);
         dumpToFile($config{LOGFILE},$command);
         if (system("$command 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
            quitStack("Error: MissFITS stoped RUNID=$config{RUNID}.");
         }
      }
   }
   system("rm -rf $config{MISSFITSCONF}");
   
}


# This function is used to execute sex.
#  Proto-type:
#    runSextractor(@filelist)
sub runSextractor(\@){
	$SIG{INT} = \&userStop;
	my @file=@{$_[0]};
    
   
	# Generate a SExtractor configuration file
	my $precommand = join(" ","sex -dd >",$config{SEXCONF});
	dumpToFile($config{LOGFILE},$precommand);
	if (system("$precommand 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
		quitStack("Error: Can't produce SExtractor configuration file for RUNID=$config{RUNID}.");
	}
   
   foreach (@file){
      my $file=trim($_);
      s/.fits/.cat/;
      my $cat=trim($_);
      
      # if the sex catalog is there then skip the sex
      if (-e $cat){
         my $command = join(" ","File $cat exists, skip.");
         dumpToFile($config{LOGFILE},$command);
      } else {   
        
         if ($config{DOLOG} == 1) {
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
	            "-CATALOG_NAME $cat $file");
                 
         	dumpToFile($config{LOGFILE},$command);
            if (system("$command 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
            	quitStack("Error: SExtractor stoped RUNID=$config{RUNID}.");
            }
         } else {
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
	            "-CATALOG_NAME $cat $file");
         	
            if (system("$command") != 0){
                quitStack("Error: SExtractor stoped RUNID=$config{RUNID}.");
            }
         }
         
      }
   }
   
   
   system("rm -rf $config{SEXCONF}");
   
}

# This function is used to execute scamp.
#  Proto-type:
#    runScamp(@odometerlist,$object,$filter)
sub runScamp(\@\$\$){
   $SIG{INT} = \&userStop;
   my @word=@{$_[0]};
   my $object=${$_[1]};
   my $filter=${$_[2]};
   my($command,$band);

	# Generate a SCAMP configuration file
	my $precommand = join(" ","scamp -dd >",$config{SCAMPCONF});
	dumpToFile($config{LOGFILE},$precommand);
	if (system("$precommand 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
		quitStack("Error: Can't produce SCAMP configuration file for RUNID=$config{RUNID}.");
	}

   my $string=join("*.cat ",@word);
   $string=$string."*.cat";
   $command=join(" ","scamp",
                  "-c $config{SCAMPCONF}",
                  "-ASTREF_CATALOG 2MASS",
                  "-MOSAIC_TYPE FIX_FOCALPLANE",
                  "-MAGZERO_OUT 25",
                  "-SOLVE_PHOTOM Y",
                  "-CHECKPLOT_DEV PNG",
                  "-MAGZERO_KEY FOTC",
                  "-AHEADER_GLOBAL /cfht/conf/wircam.ahead",
                  "-CROSSID_RADIUS 3.0 -VERBOSE_TYPE NORMAL",
                  "-XSL_URL scamp.xsl",
                  "-NTHREADS 4",
                  $string);
    
    
    if ($config{DOLOG} == 1) {
      dumpToFile($config{LOGFILE},$command);               
      if (system("$command 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
         quitStack("Error: Scamp stoped RUNID=$config{RUNID} Filter=$filter.");
      }
   } else {   
      if (system("$command 1>>/dev/null 2>>/dev/null") != 0){
         quitStack("Error: Scamp stoped RUNID=$config{RUNID} Filter=$filter.");
      }
   }
    my @image=<*.png>;
    $object=$object."_".$filter;
   # Make directory to store files produced by SCAMP.
    if (!-e $object){
      system("mkdir $object");
   }
   if (system("mv -f @image $object/") != 0){
      print "Warning: Can not copy check plots!\n";
   }
   
   if (system("cp -f scamp.xml /cfht/conf/scamp.xsl $object/") != 0){
      print "Warning: Can not copy XML files!\n";
   }

   if (system("rm -rf $config{SCAMPCONF}") != 0){
      print "Cannot delete SCAMP configuration file!\n";
   }

}

sub buildLogFile(\$){
   $SIG{INT} = \&userStop;
   my $RUNID=${$_[0]};
   my $runid=$RUNID;
   my $time;
   my($gpid,$ppid,@words);
   $_ = $runid;
   tr/[A-Z]/[a-z]/;
   $runid = $_;
   open(TIME,"date +%Y%m%d |");
   while(<TIME>){
      $time=trim($_);
   }
   close(TIME);

   my $filename=join("",'wircamstack_log_',$runid,'_',$time,'.txt');
   $config{LOGFILE}=$config{STACKPATH}."/".$RUNID."/".$filename;
   
   if (-e $config{LOGFILE}){
      system("rm -rf $config{LOGFILE}");
      system("touch $config{LOGFILE}");
   }
   $gpid=getpgrp(0);
   open(PS,"ps ax |grep wircamcpstack |grep perl |grep runid|");
      while(<PS>){
         my $re=trim($_);
         @words=split(/\s+/,$re);
         $ppid=$words[0];
         last;
      }
   close(PS);
   if ($config{JAVA} == 1){
      $config{STATUSLOG} = $config{STACKPATH}."/".$RUNID."/".$ppid.'.status';      
   } else {
      $config{STATUSLOG} = $config{STACKPATH}."/".$RUNID."/".$gpid.'.status';
   }
   if (-e $config{STATUSLOG}){
      system("rm -rf  $config{STACKPATH}/$RUNID/*.status");
      system("touch  $config{STATUSLOG}");
   } else {
      system("rm -rf  $config{STACKPATH}/$RUNID/*.status");
      system("touch  $config{STATUSLOG}");
   }
}

# This function is used to execute ww.
#  Proto-type:
#    runWW(@odometerlist)
sub runWW(\@){
   $SIG{INT} = \&userStop;
   my @file=@{$_[0]};
   my $number=@file;
   
	# Generate a MissFITS configuration file
	my $precommand = join(" ","ww -dd >",$config{WWCONF});
	dumpToFile($config{LOGFILE},$precommand);
	if (system("$precommand 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
		quitStack("Error: Can't produce MissFITS configuration file for RUNID=$config{RUNID}.");
	}
   
   foreach (@file){
      my $file=trim($_);
      s/.fits/.wm/;
      my $ww=trim($_);
      
      if (-e $ww){
        my $command = join(" ","File $ww exists, skip.");
        dumpToFile($config{LOGFILE},$command);
      } else {
         my $command = join(" ","ww -c $config{WWCONF}",
                           "-WEIGHT_NAMES $config{CALIBPATH}/$config{BADPIXMAP},$file",
                           "-FLAG_NAMES '' ",
                           "-OUTWEIGHT_NAME $ww",
                           "-WEIGHT_MIN 0.5,0.3",
                           "-WEIGHT_MAX 2,65535",
                           "-WEIGHT_OUTFLAGS 0,0");
         
         if ($config{DOLOG} == 1) {
            dumpToFile($config{LOGFILE},$command);
            if (system("$command 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
               dumpToFile($config{LOGFILE},$command);
               quitStack("Error: WW stoped RUNID=$config{RUNID}");
            }
         } else {
            if (system($command) != 0){
               quitStack("Error: WW stoped RUNID=$config{RUNID}");
            }
         }
      }
   }
   if (system("rm -rf $config{WWCONF}") != 0 ){
   		print "Cannot delete WeightWatcher configuration file!\n";
   }
}

sub userStop(){
   $SIG{INT} = \&userStop;
   quitStack("Error: users terminates the program.\n");
}

# This function is used to execute swarp.
#  Proto-type:
#    runSwarp(@odometerlist,$object,$filter)
sub runSwarp(\@\$\$){
   $SIG{INT} = \&userStop;
   my @word=@{$_[0]};
   my $object=${$_[1]};
   my $filter=${$_[2]};
   my $command;
   my $string=join("*p-*.fits ",@word);
   $string=$string."*p-*.fits";
	
	# Generate a SWARP configuration file
	my $precommand = join(" ","swarp -dd >",$config{SWARPCONF});
	dumpToFile($config{LOGFILE},$precommand);
	if (system("$precommand 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
		quitStack("Error: Can't produce MissFITS configuration file for RUNID=$config{RUNID}.");
	}
    
   if ($config{DOLOG} == 1) {
   
      $command=join(" ","swarp","-c $config{SWARPCONF}",
      				 "-WEIGHT_TYPE MAP_WEIGHT",
                     "-WEIGHT_SUFFIX .wm",
                     "-WEIGHT_IMAGE  '' ",
                     "-WEIGHT_THRESH  0",
                     "-BLANK_BADPIXELS Y",
                     "-RESAMPLING_TYPE $config{RESAMPLE}",
                     "-COMBINE_TYPE $config{COMBINETYPE}",
                     "-VERBOSE_TYPE NORMAL",
                     "-XSL_URL swarp.xsl",
                     "-NTHREADS 4",
                     $string);
      dumpToFile($config{LOGFILE},$command);               
      if (system("$command 1>>$config{LOGFILE} 2>>$config{LOGFILE}") != 0){
         quitStack("Error: SWARP stoped RUNID=$config{RUNID} Filter=$filter.");
      }
   } else {
      $command=join(" ","swarp","-c $config{SWARPCONF}",
                     "-WEIGHT_TYPE MAP_WEIGHT",
                     "-WEIGHT_SUFFIX .wm",
                     "-WEIGHT_IMAGE  '' ",
                     "-WEIGHT_THRESH  0",
                     "-BLANK_BADPIXELS Y",
                     "-RESAMPLING_TYPE $config{RESAMPLE}",
                     "-COMBINE_TYPE $config{COMBINETYPE}",
                     "-VERBOSE_TYPE NORMAL",
                     "-XSL_URL swarp.xsl",
                     "-NTHREADS 4",
                     $string);
      if (system("$command") != 0 ){
         quitStack("Error: SWARP stoped RUNID=$config{RUNID} Filter=$filter.");
      }
   }
  
   my $coadd=$object."_".$filter."_coadd.fits";
   my $wemap=$object."_".$filter."_coadd.weight.fits";
   

    if (system("mv -f coadd.fits $coadd") != 0){
       print("Warning: Can not rename stacked file.\n");
   }
    if (system("mv -f coadd.weight.fits $wemap") != 0){
       print("Warning: Can not rename weight image.\n");
   }
   $object=$object."_".$filter;
   if (system("cp -f swarp.xml /cfht/conf/swarp.xsl $object/") != 0){
      print "Warning: Can not copy SWARP XML files!\n";
   }
   if (system("rm -rf $config{SWARPCONF}") != 0){
      print "Warning: Can not remove SWARP configuration files!\n";
   }
   
   
}

sub buildList(){
   my @list=();
   
   open(REDDIR,"ls $config{PROCESSEDPATH}/$config{RUNID}|");
   while(<REDDIR>){
   my $temp=trim($_);
      @list=(@list,$temp);
}
   close(REDDIR);

   return @list;
}



# This finction is used to return program usage.
sub usage(){

print<<EOF

 USAGE: $SCRIPT_NAME <RUNID>
 
 INPUT:
   -r|--runid      Runid
 
 OPTIONS:
   -h|--help       Get help message
   -o|--onept      Force to stack all images using one pointing
   -d|--dstth      Minimum distance between different pointings
   -c|--rawwcs     Use WCS in Raw data.
   -t|--smptype    Resampling Type [NEAREST,BILINEAR,LANCZO2,LANCZO3,LANCZO4]
   -x|--mostype    Mosaic Type 
                     [UNCHANGED,SAME_CRVAL,SHARE_PROJAXIS,FIX_FOCALPLANE,LOOSE]
   -z|--comtype    Flux scaling type [MEDIAN,AVERAGE,MIN,MAX,WEIGHTED,CHI2,SUM]
   -O|--rawpath    Path of directory that store the raw images
   -P|--redpath    Path of directory that store the processed images
   -A|--stackpath  Path of directory that stacked image will be strored
   -f|--filter     Specified filter for image stacking
   -D|--alldata    Use all image data for stacking (skip database checking!!)
   -l|--nolink     Skipping file linking
   -m|--nosplit    Skipping FITS spliting
   -s|--nosex      Skipping SExtractor procedure
   -w|--noww	   Skipping weight map builing
   -a|--noscamp    Skipping SCMAP 
   -p|--noswarp    Do not building final stack
   -C|--noclean    Keeping all files produced in stacking         
   -j|--java       A flag notification that this script is launced from JAVA.         
 EXAMPLES:
  $SCRIPT_NAME --runid=06AT99 [options]

EOF
}

$SIG{INT} = \&userStop;

my($opt_help,$runid,$log);
my($stackpath,$filterpath,$filter);
my $pwd=getcwd();
my(@list,@file,@odometer)=();
my %datadb=();
my $object;
#my $resample='LANCZOS2';
#my $mostype='SAME_CRVAL';
#my $comtype='MEDIAN';
my $pi = 3.14159265358979;

# check if the program id is passed.
if(@ARGV == 0){
   usage();
   exit 0;
}

# Get options from command line
GetOptions("onept"  => \$config{ONEPT},
   "r|runid=s"      => \$config{RUNID},
   "f|filter=s"     => \$config{FILTER},
   "d|dstth=f"      => \$config{DSTTHRES},
   "t|smptype=s"    => \$config{RESAMPLE},
   "x|mostype=s"    => \$config{MOSAICTYPE},
   "z|comtype=s"    => \$config{COMBINETYPE},
   "e|pixscale=f"   => \$config{PIXELSCALE},   
   "i|pixtype=s"    => \$config{PIXELSCALETYPE},   
   "c|rawwcs"       => \$config{RAWWCS},
   "D|alldata"      => \$config{ALLDATA},
   "l|nolink"       => \$config{NOLINK},
   "m|nosplit"      => \$config{NOSPLIT},
   "s|nosex"        => \$config{NOSEX},
   "w|noww"         => \$config{NOWW},
   "a|noscamp"      => \$config{NOSCAMP},
   "p|noswarp"      => \$config{NOSWARP},
   "O|rawpath=s"    => \$config{RAWPATH},
   "P|redpath=s"    => \$config{PROCESSEDPATH},
   "A|stackpath=s"  => \$config{STACKPATH},
   "C|noclean"      => \$config{NOCLEAN},
   "j|java"         => \$config{JAVA},
   "h|help"         => \$opt_help
   );

if($opt_help){
   usage();
   exit 0;
}   



if ($config{RUNID} eq ""){
   print("Error: no RUNID specified.\n");
   usage();
   exit 0;
}

# Make a direcroty to put stacked image.
if (-e $config{PROCESSEDPATH}.'/'.$config{RUNID}){
   $stackpath=join('/',$config{STACKPATH},$config{RUNID});
   if (!-e $stackpath){
      system("mkdir $stackpath");
   }
   buildLogFile($config{RUNID});
} else {
   print "error: This RUNID does not exist.\n";
   exit 0;
}

if ($config{LOGFILE} ne ""){
   $_ = $config{LOGFILE};
   if (!/^\//){
      $config{LOGFILE}=$pwd."/".$config{LOGFILE};
}
   system("touch $config{LOGFILE}");
   $config{DOLOG} = 1;
}

# Check resample method
if ($config{RESAMPLE} ne "NEAREST" && $config{RESAMPLE} ne "BILINEAR" &&
$config{RESAMPLE} ne "LANCZOS2" && $config{RESAMPLE} ne "LANCZOS3" && 
$config{RESAMPLE} ne "LANCZOS4"){

   die "Error: incorrect resample type. \n";

} 

# Check mosaic method
if ($config{MOSAICTYPE} ne "UNCHANGED" && $config{MOSAICTYPE} ne "SAME_CRVAL" &&
$config{MOSAICTYPE} ne "SHARE_PROJAXIS" && $config{MOSAICTYPE} ne "FIX_FOCALPLANE" && 
$config{MOSAICTYPE} ne "LOOSE"){
   die "Error: incorrect mosaic type. \n";
} 


# Check combine method
if ($config{COMBINETYPE} ne "MEDIAN" && $config{COMBINETYPE} ne "AVERAGE" &&
$config{COMBINETYPE} ne "MIN" && $config{COMBINETYPE} ne "MAX" && 
$config{COMBINETYPE} ne "WEIGHTED" && $config{COMBINETYPE} ne "CHI2" && 
$config{COMBINETYPE} ne "SUM"){
   die "Error: incorrect image combine type. \n";
} 

# Check the pixel scale and pixel scale type
if ($config{PIXELSCALE} > 0){
	$config{PIXELSCALETYPE} = "MANUAL"
}

if ($config{PIXELSCALETYPE} ne "MANUAL" && $config{PIXELSCALETYPE} ne "FIT" &&
$config{PIXELSCALETYPE} ne "MIN" && $config{PIXELSCALETYPE} ne "MAX" && 
$config{PIXELSCALETYPE} ne "MEDIAN" 
){
   die "Error: incorrect image pixel scale type. \n";
} 



dumpSettings();

# Get the list of filters
open(DIR,"ls $config{PROCESSEDPATH}/$config{RUNID}|");
while(<DIR>){
   my $temp=trim($_);
   @list=(@list,$temp);
   $filterpath=join('/',$stackpath,$temp);
   # clean the unused file in each directory
}
close(DIR);


foreach (@list){
   my $dir=join('/',$stackpath,$_);
   
   s#.*/##;
   $filter=$_;
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}
   
   # Establish sub-directories for each filter
   if (!-e $dir){
      system("mkdir $dir");
   } else {
      system("rm -rf $dir/*resamp*.fits");
   }
}


# Make a symbolic link to processed files.
foreach (@list){
   s#.*/##;
   $filter=$_;
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}
   
   updateStatus("LINKING");
   @file=<$config{PROCESSEDPATH}/$config{RUNID}/$_/*p.fits>;
   
   if ($config{NOLINK} == 0){
       mkLink($filter,@file); 	
   }  
}


# Now go to each directory and working in these directory
foreach (@list){
   updateStatus("GROUPING");
   my $dir=join('/',$stackpath,$_);
    
   chdir $dir;
   %datadb=buildDB($dir);
   @file=<*p.fits>;
   
   s#.*/##;
   $filter=$_;
   
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}
   
   if ($config{ONEPT} == 0) {
        pickgroup(%datadb,@file,$config{DSTTHRES},$config{GROUPFILE});
   }
}


# Checking the total sky coverage of the stacking
@list=buildList();
foreach (@list){
   updateStatus("CHECKING COVERAGE");
   my $dir=join('/',$stackpath,$_);
   my($min_ra,$max_ra,$min_dec,$max_dec);
   my($ddec,$cdec,$dra,$omega);
   my $ra_flag=0;
   my $dec_flag=0;
   
   s#.*/##;
   $filter=$_;
   
   chdir $dir;
   %datadb=buildDB($dir);
   
   if ($config{ONEPT} == 0){
        
   } else {
        @file=<*p.fits>;
        foreach (@file){
            s#.*/##;
            s/p.fits//;
            if ($ra_flag == 0){
                $min_ra=$datadb{$_}{RA_DEG};
                $max_ra=$datadb{$_}{RA_DEG};
                $ra_flag=1;
            } else {
                if ($datadb{$_}{RA_DEG} < $min_ra){$min_ra=$datadb{$_}{RA_DEG};}
                if ($datadb{$_}{RA_DEG} > $max_ra){$max_ra=$datadb{$_}{RA_DEG};}
            }
            if ($dec_flag == 0){
                $min_dec=$datadb{$_}{DEC_DEG};
                $max_dec=$datadb{$_}{DEC_DEG};
                $dec_flag=1;
            } else {
                if ($datadb{$_}{DEC_DEG} < $min_ra){$min_dec=$datadb{$_}{DEC_DEG};}
                if ($datadb{$_}{DEC_DEG} > $max_ra){$max_dec=$datadb{$_}{DEC_DEG};}
            }
        }
        #print("$max_dec $max_dec\n");
                
        $ddec=abs($max_dec-$min_dec);
        $cdec=$min_dec+($ddec/2.0);
        $dra=($max_ra-$min_ra)*cos((($cdec/180.0)*$pi));  
        $omega=$ddec*$dra; 
        
        my $line=sprintf("The sky coverage is %5.3f square degree for %s filter.",$omega,$filter);
        dumpToFile($config{LOGFILE},$line);
        if ($omega > $config{MAXOMEGA}){
            $config{OMEGATOOBIG}=$config{OMEGATOOBIG}+1;
        }
   } 
} 

if ($config{OMEGATOOBIG} != 0){
	quitStack("Error: the angular area of one of final stack images is larger than default value.");
} 



@list=buildList();
foreach (@list){
   updateStatus("SPLIT");
    my $dir=join('/',$stackpath,$_);
    
   chdir $dir;
   %datadb=buildDB($dir);
   @file=<*p.fits>;
   
   s#.*/##;
   $filter=$_;
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}
   
# Using Missfits to split all images
   @file=<[0-9]*p.fits>;
   if ($config{NOSPLIT} == 0){runMissfits(@file);}
#   if ($config{NOSPLIT} == 0){splitCube(@file);}
}

@list=buildList();
foreach (@list){
   updateStatus("RAWWCS");
    my $dir=join('/',$stackpath,$_);
    
   chdir $dir;
   @file=<*p.fits>;
   
   s#.*/##;
   $filter=$_;
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}
   
   @file=<[0-9]*p-*.fits>;
   if ($config{RAWWCS} == 1){useRawWCS(@file);}
}

@list=buildList();

foreach (@list){
   updateStatus("SEX");
    my $dir=join('/',$stackpath,$_);
    
   chdir $dir;
   @file=<*p.fits>;
   
   s#.*/##;
   $filter=$_;
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}

   # Run source extractor
   @file=<[0-9]*p-*.fits>;
   if ($config{NOSEX} == 0){runSextractor(@file);}
}

@list=buildList();

foreach (@list){
   updateStatus("WW");
    my $dir=join('/',$stackpath,$_);
    
   chdir $dir;
   @file=<*p.fits>;
   
   s#.*/##;
   $filter=$_;
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}
   
   # Run weight watcher
   @file=<[0-9]*p-*.fits>;
   if ($config{NOWW} == 0){runWW(@file);}

}
@list=buildList();
foreach (@list){
   updateStatus("SCAMP");
   my $dir=join('/',$stackpath,$_);

   chdir $dir;
   @file=<*p-*.fits>;
   
   s#.*/##;
   $filter=$_;
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}

   if ($config{ONEPT} == 0) {
      open(FILE,"$config{GROUPFILE}");
      while(<FILE>){
   
         @odometer=split(" ",$_);
         $object=$odometer[$#odometer];
         pop(@odometer);
         if ($config{NOSCAMP} == 0){runScamp(@odometer,$object,$filter);}
      }
      close(FILE);
   } else {
      @odometer=();
      
      @file=<[0-9]*p.fits>;
      $object=$config{RUNID};
      foreach(@file){
         s#.*/##;
         s/p.fits//;
         @odometer=(@odometer,$_);
      }   
      if ($config{NOSCAMP} == 0){runScamp(@odometer,$object,$filter);}
   }

}



@list=buildList();

foreach (@list){
   updateStatus("SWARP");
    my $dir=join('/',$stackpath,$_);
    
   chdir $dir;
   @file=<*p.fits>;
   
   s#.*/##;
   $filter=$_;
   if ($config{FILTER} ne "" && $config{FILTER} ne $filter){next;}

   if ($config{ONEPT} == 0) {
      open(FILE,"$config{GROUPFILE}");
      while(<FILE>){
   
         @odometer=split(" ",$_);
         $object=$odometer[$#odometer];
         pop(@odometer);
         if ($config{NOSWARP} == 0){runSwarp(@odometer,$object,$filter);}
   }
      close(FILE);
   } else {
      @odometer=();
      
      @file=<[0-9]*p.fits>;
      $object=$config{RUNID};
      foreach(@file){
         s#.*/##;
         s/p.fits//;
         @odometer=(@odometer,$_);
   }      
      if ($config{NOSWARP} == 0){runSwarp(@odometer,$object,$filter);}
   }
}

if ($config{NOCLEAN} == 0){
   @list=buildList();
   # Remove all sybmolic link in directory
   foreach (@list){
      updateStatus("CLEAN");
      my $dir=join('/',$stackpath,$_);
      
      chdir $dir;
      %datadb=buildDB($dir);
      
      @file=<*p.fits>;
      system("rm -f @file");
      
      @file=<*p-*.fits>;
      system("rm -f @file");
      
      @file=<*.fits.back>;
      system("rm -f @file");
      
      @file=<*.sex>;
      system("rm -f @file");
      
      @file=<*p-*.head>;
      system("rm -f @file");
      
      @file=<*p-*.wm>;
      system("rm -f @file");
      
      system("rm -rf flag.fits *.cat")
   }
}



#if (-e $config{STATUSLOG}){
#system("rm -rf $config{STATUSLOG}");
#}
my $command="COMPLETE";
dumpToFile($config{LOGFILE},$command);

