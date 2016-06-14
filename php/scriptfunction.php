<?php
FUNCTION parseMyDate2JD($string){
	$month=substr($string,0,3);
	$dd=substr($string,4,2)*1.0;
	$yy=substr($string,7,4)*1.0;
	
	$hour=substr($string,12,2)*1.0;
	$min=substr($string,15,2)*1.0;
	$am=substr($string,18,2)*1.0;
	switch ($month) {
	    case "Jan":
	        $mm=1;
	        break;
		case "Feb":
		    $mm=2;
		    break;				
		case "Mar":
		    $mm=3;
		    break;	
		case "Apr":
	        $mm=4;
	        break;
	    case "May":
	        $mm=5;
	        break;
	    case "Jun":
	        $mm=6;
	        break;
		case "Jul":
		    $mm=7;
		    break;				
		case "Aug":
		    $mm=8;
		    break;	
		case "Sep":
	        $mm=9;
	        break;
	    case "Oct":
	        $mm=10;
	        break;
		case "Nov":
	        $mm=11;
	        break;
	    case "Dec":
	        $mm=12;
	        break;
	}
	if (strcmp($am,"PM") == 0){
		$hh=$hh+12;
	}
	return gregoriantojd($mm,$dd,$yy)+(($hour-12)/24)+($min/1440);
}

FUNCTION parseFlatName2JD($string){
	$array=explode("_", $string);
	//printf("%s <br>",$array[2]);

	$yy=substr($array[2],0,4)*1.0;
	$mm=substr($array[2],4,2)*1.0;
	$dd=substr($array[2],6,2)*1.0;
	
	$hour=substr($array[2],11,2)*1.0+10.0;
	$min=substr($array[2],13,2)*1.0;
	
	if ($hour > 24){
		$dd++;
		$hour=$hour-24;
	}
	
	return gregoriantojd($mm,$dd,$yy)+(($hour-12)/24)+($min/1440);
}

FUNCTION parseDarkName2etime($string){
	$array=explode("_", $string);
	$etime=substr($array[1],0,3)*1.0;
	return $etime;
}


FUNCTION getFlatImage($crunid,$raw_jd,$filter,$calibdata,$type){
	$flatname='NONE';
	switch ($type) {
	    case "PROCESS_FLAT":		
			$diff=-9999.0;
			for ($j=0;$j<sizeof($calibdata[$crunid]);$j++){

				// Looking for calibration files with the same filter
				if (strcmp($filter,$calibdata[$crunid][$j]["filter"]) == 0){

					// Now, looking for flat image
					if (strcmp($calibdata[$crunid][$j]["type"],$type) == 0){
						$flat_jd=parseFlatName2JD($calibdata[$crunid][$j]["filename"]);
						if ($diff == -9999.0){
							$flatname=$calibdata[$crunid][$j]["filename"];
							$diff=abs($flat_jd-$raw_jd);
						} else {
							if (abs($flat_jd-$raw_jd) < $diff){
								$flatname=$calibdata[$crunid][$j]["filename"];
								$diff=abs($flat_jd-$raw_jd);						
							}
						}	
					}
				}
			}
			break;
		case "MADOMEFLAT":
			for ($j=0;$j<sizeof($calibdata[$crunid]);$j++){
				if (strcmp($filter,$calibdata[$crunid][$j]["filter"]) == 0){
					if (strcmp($calibdata[$crunid][$j]["type"],$type) == 0){
						$flatname=$calibdata[$crunid][$j]["filename"];
					}
				}	
			}
			break;
	}			
	// Have to add a check before return value
	
	return $flatname;	
}

FUNCTION RADEG($string){
	$array=explode(":", $string);
	//printf("%s %s %s <br>",$array[0],$array[1],$array[2]);
	
	 $h=$array[0]*1.0;
	 $m=$array[1]*1.0;
	 $s=$array[2]*1.0;
	 return 15.0*($h+($m/60.0)+($s/3600.0));
}

FUNCTION DECDEG($string){
	$array=explode(":", $string);
	//printf("%s %s %s <br>",$array[0],$array[1],$array[2]);
	
	 $h=$array[0]*1.0;
	 $m=$array[1]*1.0;
	 $s=$array[2]*1.0;
	 return ($h+($m/60.0)+($s/3600.0));
}

FUNCTION getDarkImage($crunid,$raw_jd,$etime,$calibdata){
	$darkname="NONE";
	$diff=-9999.0;
	for ($j=0;$j<sizeof($calibdata[$crunid]);$j++){
		if (strcmp($calibdata[$crunid][$j]["type"],"MASTERDARK") == 0){
			$dark_etime=parseDarkName2etime($calibdata[$crunid][$j]["filename"]);
			if ($diff == -9999.0){
				$darkname=$calibdata[$crunid][$j]["filename"];
				$diff=abs($dark_etime-$etime);
			} else {
				if (abs($dark_etime-$etime) < $diff){
					$darkname=$calibdata[$crunid][$j]["filename"];
					$diff=abs($dark_etime-$etime);				
				}
			}	
		}
	}	
	return $darkname;	
}


/**
 * 
 * This function is used to return the sky frame used for building sky background.
 *  Note: this is for DP patterns
 * @param unknown_type $target
 * @param unknown_type $odometer
 * @param unknown_type $date
 * @param unknown_type $ra
 * @param unknown_type $dec
 * @param unknown_type $filter
 * @return Ambigous <string, unknown>
 */
FUNCTION getSkyFramesDP($target,$odometer,$date,$ra,$dec,$filter){
	$rejcurr=1;
	
	$k=0;
	$list="";
	
	for ($i=0; $i < sizeof($odometer); $i++){
		
		/* Skip target frame itself. */
		if  (($rejcurr == 1) && (strcmp($target["odometer"],$odometer[$i]) == 0)){
			continue;
		}
		
		$diff=abs(parseMyDate2JD($target["date"])-parseMyDate2JD($date[$i]))*24.0*60.0;
		
		/**
		*    Pick up all frames within maximum time requirement 
		*/
		if ($diff < 20 && strcmp(strtoupper($target["filter"]),strtoupper($filter[$i])) == 0){
			
			$dec1=decdeg($target["dec"]);
			$dec2=decdeg($dec[$i]);
			$ra1=radeg($target["ra"]);
			$ra2=radeg($ra[$i]);
			
			/** Calculate angular separation */
			$sep=60.0*rad2deg(acos(cos(deg2rad(90-$dec1))*cos(deg2rad(90-$dec2))+
				sin(deg2rad(90-$dec1))*sin(deg2rad(90-$dec2))*cos(deg2rad(abs($ra1-$ra2)))));
			
			/**
			 *  Pick up frames with separation less than 15 arcmin
			 */
			if ($sep < 15){
				$list[$k]=$odometer[$i];
				$k++;
			}
		
		} 
	}
	return $list;
}


?>