<?php
header('Content-Type: application/json');
$conn = new mysqli("localhost", "root", "", "greenhouse");

if ($conn->connect_error) {
    echo json_encode(["error" => "Connection failed"]);
    exit();
}

$plant = isset($_GET['plant']) ? $conn->real_escape_string($_GET['plant']) : '';

$sql = "SELECT * FROM plants_data WHERE plant_name='$plant' LIMIT 1";
$result = $conn->query($sql);

if ($result && $result->num_rows > 0) {
    echo json_encode($result->fetch_assoc());
} else {
    // Return zeros if no data found to prevent the website from being stuck on "Loading..."
    echo json_encode([
        "temperature" => 0,
        "humidity" => 0,
        "soil_moisture" => 0,
        "light" => 0
    ]);
}
?>