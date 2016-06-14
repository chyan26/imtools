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
$cablibpath = $_GET['calibpath'];
$dppattern = $_GET['dppatern'];


/** Inintialize the script page. */
printf("#! /bin/bash  <br>");
printf("<br>");

/** Declare path variabe  */
if (empty($rawpath)) {
	$rawpath='$RAWPATH';
	printf("export RAWPATH= REMEMBER to put your path name HERE!!  <br>");
	
}
if (empty($redpath)){
	printf("export REDPATH=REMEMBER to put your path name HERE!!  <br>");
	$redpath='$REDPATH';
	
}

if (empty($calibpath)) {
	printf("export CALIBPATH=REMEMBER to put your path name HERE!!  <br>");
	$calibpath='$CALIBPATH';
}


/**
 *  Preset DP pattern to DP if it is not given
 */
if (empty($dppattern)) {
	$dppattern="DP";
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



// Query for the image validation
$db_connect = sybase_connect( "DBS12", "opuser", "opuser") or die ("cannot connect");
$query = "select status,eval from xexp where  _runid='$runid' and _obstype='OBJECT'";
$db_query = sybase_query($query);
$totalrows = sybase_num_rows($db_query);
for($ri = 0; $ri < $totalrows; $ri++) {
    $row=sybase_fetch_array($db_query);
    $validate[$ri] = $row["status"];
    $eval[$ri] = $row["eval"];
}
sybase_close($db_connect) or die ("cannot disconnect");

for ($i=0;$i<sizeof($filename);$i++){
	
	$target=array("odometer"=>$odometer[$i],
		"date"=>$date[$i],"ra"=>$ra[$i],"dec"=>$dec[$i],"filter"=>$filter[$i]);

	if (strcmp($dppattern,"DP") == 0){
		$skylist=getSkyFramesDP($target,$odometer,$date,$ra,$dec,$filter);
	}
	
	$skyframes="";
	$maskframes="";
	$flag=0;
	
	for ($f=0;$f<sizeof($skylist);$f++){
		$file=$redpath.'/'.$skylist[$f].'s.fits';
		$mask=$redpath.'/'.$skylist[$f].'m.fits';
		if ($flag==0){
			$skyframes=$file;
			$maskframes=$mask;
			$flag=1;
		} else {
			$skyframes=$skyframes.',\<br>'.$file;
			$maskframes=$maskframes.',\<br>'.$mask;
		}
	
	}
	$maskimage="badpix16_20110413HST185556_v100.fits";
	
	printf("# File %s <br>",$odometer[$i]);
	printf("<br>");
	printf("imsky --filelist=%s \ <br>", $skyframes);
	printf("&nbsp&nbsp&nbsp--mask=%s \<br>",$maskframes);
	printf("&nbsp&nbsp&nbsp--output=%s/%s \<br>",$redpath,$target["odometer"].'y.fits');
	
	printf("<br>");
	printf("<br>");
	
	printf("imskysub --detrend=%s/%s \ <br>",$redpath,$target["odometer"].'s.fits');
	printf("&nbsp&nbsp&nbsp--sky=%s/%s \<br>",$redpath,$target["odometer"].'y.fits');
	printf("&nbsp&nbsp&nbsp--mask=%s/%s \<br>",$calibpath,$maskimage);
	printf("&nbsp&nbsp&nbsp--output=%s/%s \<br>",$redpath,$target["odometer"].'p-uncalib.fits');
	printf("<br>");
	printf("<br>");
	
	
}




?>


