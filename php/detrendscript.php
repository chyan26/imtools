<?php
putenv ("SYBASE=/apps/sybase");
putenv ("SYBPLATFORM=linux");
putenv ("LD_LIBRARY_PATH=/apps/sybase/lib");
putenv ("LC_ALL=default");
putenv ('PATH=\"/apps/sybase/bin:$PATH\"');
putenv ("DSQUERY=DBS12");

REQUIRE "scriptfunction.php";

$runid = $_GET['runid'];
$rawpath = $_GET['rawpath'];
$redpath = $_GET['redpath'];
$calibpath = $_GET['calibpath'];


/** Inintialize the script page. */
printf("#! /bin/bash  <br>");
printf("<br>");

/** Declare path variabe  */
if (empty($rawpath)) {
	$rawpath='$RAWPATH';
	printf("export RAWPATH = REMEMBER to put your path name HERE!!  <br>");
	
}
if (empty($redpath)){
	printf("export REDPATH = REMEMBER to put your path name HERE!!  <br>");
	$redpath='$REDPATH';
	
}
if (empty($calibpath)) {
	printf("export CALIBPATH = REMEMBER to put your path name HERE!!  <br>");
	$calibpath='$CALIBPATH';
}


printf("<br>");
printf("<br>");


/* Declare variables that will be used for query */
$query = '';
$odometer;
$filename;
$filter;
$ra;
$dec;
$etime;
$date;

$ucrunid;


$calibinfo;
$calibfile;
$calibdata;

// Query 1, Establblishing the list 
$db_connect = sybase_connect( "envdb", "iiwi", "iiwiiiwi") or die ("cannot connect");
$query = "select filename,crunid,filter,ra,dec,etime,odometer,mydatetime from exposure where runid='$runid' and etype= 'OBJECT'";
$db_query = sybase_query($query);
$totalrows = sybase_num_rows($db_query);
if ($totalrows == 0){
	die("This RUNID may not exist.  Please double check!!");
}

for($ri = 0; $ri < $totalrows; $ri++) {
    $row=sybase_fetch_array($db_query);
    $filename[$ri] = $row["filename"];
    $odometer[$ri] = $row["odometer"];
	$filter[$ri]=$row["filter"];
    $crunid[$ri] = $row["crunid"];
    $ra[$ri] = $row["ra"];
    $dec[$ri] = $row["dec"];
    $etime[$ri] = $row["etime"];
    $date[$ri] = $row["mydatetime"];
    //printf("%s %s %s %s %s<br>",$filename[$ri],$crunid[$ri],$filter[$ri],$ra[$ri],$dec[$ri]);
}

sybase_close($db_connect) or die ("cannot disconnect");


$db_query = sybase_query($query);
$row=sybase_fetch_row($db_query);
$status = $row[0];
$obstype = $row[1];
$iq = $row[2];
$eval = $row[3];
sybase_close($db_connect) or die ("cannot disconnect");


// Find out the unique CRUNID 
$ii=0;
for ($i=0; $i < sizeof($crunid); $i++){
	if ($i == 0){
		$ucrunid[$ii]=$crunid[$i];
		$ii++;
	} else {
		if ($ucrunid[$ii-1] == $crunid[$i]){
			continue;
		} else {
		 	$ucrunid[$ii]=$crunid[$i];
			$i++;
		}
	} 	
}

// Query the calibration information
for ($i=0; $i < sizeof($ucrunid); $i++){
	$db_connect = sybase_connect( "envdb", "iiwi", "iiwiiiwi") or die ("cannot connect");
	$query = "select filename,filter,type,etime from calibration where crunid='$ucrunid[$i]'";
	$db_query = sybase_query($query);
	$totalrows = sybase_num_rows($db_query);
	for($ri = 0; $ri < $totalrows; $ri++) {
    	$row=sybase_fetch_array($db_query);
		$calibinfo[$ri]=array("filename"=>$row["filename"],
			"filter"=>$row["filter"],"type"=>$row["type"],"etime"=>$row["etime"]);
	}
	sybase_close($db_connect) or die ("cannot disconnect");

	$calibdata[$ucrunid[$i]]=$calibinfo;
}

for ($i=0;$i<sizeof($filename);$i++){
	$raw_jd=parseMyDate2JD($date[$i]);

	$flatimage=getFlatImage($crunid[$i],$raw_jd,$filter[$i],$calibdata,"PROCESS_FLAT");
	$darkimage=getDarkImage($crunid[$i],$raw_jd,$etime[$i],$calibdata);
	$maskimage="badpix16_20110413HST185556_v100.fits";
	//printf("%s %s %s <br>",$filename[$i],$crunid[$i],$filter[$i]);
	printf("<br>");
	printf("imdetrend --raw=%s/%s \ <br>",$rawpath,$filename[$i]);
	printf("&nbsp&nbsp&nbsp--out=%s/%ss.fits \ <br>",$redpath,$odometer[$i]);
	printf("&nbsp&nbsp&nbsp--flat=%s/%s \ <br>",$calibpath,$flatimage);
	printf("&nbsp&nbsp&nbsp--dark=%s/%s \ <br>",$calibpath,$darkimage);
	printf("&nbsp&nbsp&nbsp--badpix=%s/%s \ <br>",$calibpath,$maskimage);
	printf("&nbsp&nbsp&nbsp--correctrefpix  <br>");

	printf("<br>");
	printf("immask --image=%s/%ss.fits \ <br>",$redpath,$odometer[$i]);
	printf("&nbsp&nbsp&nbsp--weight=%s/%s \ <br>",$calibpath,$maskimage);
	printf("&nbsp&nbsp&nbsp--out=%s/%sm.fits \ <br>",$redpath,$odometer[$i]);
	printf("&nbsp&nbsp&nbsp--sigma=4.5  <br>");
	
}



?>


