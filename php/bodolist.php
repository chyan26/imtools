<?php
putenv ("SYBASE=/apps/sybase");
putenv ("SYBPLATFORM=linux");
putenv ("LD_LIBRARY_PATH=/apps/sybase/lib");
putenv ("LC_ALL=default");
putenv ('PATH=\"/apps/sybase/bin:$PATH\"');
putenv ("DSQUERY=DBS12");


$runid = $_GET['runid'];

$db_connect = sybase_connect( "DBW12", "nr_login_b", "nr_login_b") or die ("cannot connect");

$query = '';

$query = "select _obsid from op_b.dbo.xexp where _runid='$runid' and _obstype='OBJECT'";

$db_query = sybase_query($query);
$totalrows = sybase_num_rows($db_query);

for($ri = 0; $ri < $totalrows; $ri++) {
  $row=sybase_fetch_array($db_query);
  $odometer = $row["_obsid"];
  printf("%s <br>",$odometer);
}

sybase_close($db_connect) or die ("cannot disconnect");

?>


