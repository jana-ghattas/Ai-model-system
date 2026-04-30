<?php
$conn = mysqli_connect("localhost", "root", "", "greenhouse");

$result = mysqli_query($conn, "SELECT * FROM current_plant ORDER BY id DESC LIMIT 1");
$row = mysqli_fetch_assoc($result);

echo json_encode($row);
?>