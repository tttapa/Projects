<?php
$database = "IoT";  // name of the database
$username = "root";  // MySQL username
$password = "password123";  // MySQL password

try {  // connect to database
  $db = new PDO("mysql:host=localhost;dbname=${database};charset=utf8mb4", 
    $username, $password, 
    array(PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION));
} catch(PDOException $ex) {
  http_response_code(500);
  error_log($ex->getMessage());
  die("Unable to connect to the database");
}?>
