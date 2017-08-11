<?php
require("db-connect.php"); // connect to the database

$tablename = "sensordata2";

if ($_SERVER['REQUEST_METHOD'] === 'POST') :  // if data is POSTed, insert new values into database
  try {
    $stmt = $db->prepare("INSERT INTO `${tablename}` (time, field1, field2, field3, field4, field5, field6, field7, field8) VALUES (:t, :field1,:field2,:field3,:field4,:field5,:field6,:field7,:field8);");
	$fields = array();
	$fields[":t"] = $_SERVER['REQUEST_TIME'];
    for($i = 1; $i <= 8; $i++) {
      $fields[":field${i}"] = get($_POST["field${i}"], null);
    }
    $stmt->execute($fields);
  } catch(PDOException $ex) {
    http_response_code(500);
    echo "Can't insert data into database";
    error_log($ex->getMessage());
  }
elseif ($_SERVER['REQUEST_METHOD'] === 'GET') :  // when client sends a GET request, display the table as a webpage
  $tbody = "";
  $error = "";
  try {
    foreach($db->query("SELECT * FROM `${tablename}`") as $row) {  // for all rows (entries) in the database
      $tbody .=  "<tr><td>".$row['id']."</td><td>".date("Y-m-d\tH:i:s",$row['time'])."</td>";  // print the index and time
      for($i = 1; $i <= 8; $i++) {  // print the values of all 8 fields
        $tbody .= "<td>".$row["field${i}"]."</td>";
      }
      $tbody .= "</tr>\r\n";
    }
  } catch(PDOException $ex) {
    $error = "Unable to retrieve data from database";
    error_log($ex->getMessage());
  }
  ?>
<html>
	<head>
		<title>ESP8266 Sensor Data</title>
	</head>
	<body>
		<style>
		td, th {
			padding: 2px 6px;
			text-align: left;
		}		
		</style>
        <table>
			<tr><th>Index</th><th>Time</th><th>Value 1</th><th>Value 2</th><th>Value 3</th><th>Value 4</th><th>Value 5</th><th>Value 6</th><th>Value 7</th><th>Value 8</th></tr>
			<?php echo $tbody; ?>
		</table>
        <?php echo $error; ?>		
	</body>
</html>

<?php
endif;	// $_SERVER['REQUEST_METHOD'] === 'GET'
?>

<?php
function get(&$var, $default=NULL) {
    return isset($var) ? $var : $default;
}
?>
