<?php
putenv ("SYBASE=/apps/sybase");
putenv ("SYBPLATFORM=linux");
putenv ("LD_LIBRARY_PATH=/apps/sybase/lib");
putenv ("LC_ALL=default");
putenv ('PATH=\"/apps/sybase/bin:$PATH\"');
putenv ("DSQUERY=DBS12");


$obsid = $_GET['obsid'];

$db_connect = sybase_connect( "DBS12", "opuser", "opuser") or die ("cannot connect");

$query = '';

$query = "select a.status, a._obstype, b.iq, a.eval from xexp a, xexpe b where a._obsid=b.obsid and a._obsid=" . $obsid;

$db_query = sybase_query($query);
$row=sybase_fetch_row($db_query);
$status = $row[0];
$obstype = $row[1];
$iq = $row[2];
$eval = $row[3];
sybase_close($db_connect) or die ("cannot disconnect");

print $status;
print " ";
print $obstype;
print " ";
printf("%.3f", $iq);
print " ";
print $eval;
?>


