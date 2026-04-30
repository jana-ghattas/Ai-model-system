<?php
$conn = mysqli_connect("localhost", "root", "", "greenhouse");

if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $temp     = $_POST['temperature'];
    $humidity = $_POST['humidity'];
    $soil     = $_POST['soil_moisture'];
    $light    = $_POST['light'];
    
    // Update current status
    mysqli_query($conn, "UPDATE current_plant SET 
        temperature='$temp', 
        humidity='$humidity', 
        soil_moisture='$soil', 
        light='$light',
        timestamp=NOW()
        ORDER BY id DESC LIMIT 1");

    // Also log to history table (plants_data)
    $plant_name_result = mysqli_query($conn, "SELECT plant_name FROM current_plant ORDER BY id DESC LIMIT 1");
    $plant_row = mysqli_fetch_assoc($plant_name_result);
    $current_name = $plant_row['plant_name'];

    mysqli_query($conn, "INSERT INTO plants_data (plant_name, temperature, humidity, soil_moisture, light) 
        VALUES ('$current_name', '$temp', '$humidity', '$soil', '$light')");
    
    echo "OK";
}
?>