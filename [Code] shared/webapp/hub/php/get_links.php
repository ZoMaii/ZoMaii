<?php
require_once 'db/connection.php';

$query = $db->query("SELECT * FROM links");
$links = $query->fetchAll(PDO::FETCH_ASSOC);
header('Content-Type: application/json');
echo json_encode($links);
?>
