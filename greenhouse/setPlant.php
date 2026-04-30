<?php
$conn = mysqli_connect("localhost", "root", "", "greenhouse");

if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $plant_name = mysqli_real_escape_string($conn, $_POST['plant_name']);
    
    // امسحي النبتة القديمة وحط الجديدة
    mysqli_query($conn, "DELETE FROM current_plant");
    mysqli_query($conn, "INSERT INTO current_plant (plant_name) VALUES ('$plant_name')");
    
    echo "OK";
}
?>