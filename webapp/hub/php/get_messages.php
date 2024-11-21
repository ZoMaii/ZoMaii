<?php
require_once 'db/connection.php';

$query = $db->query("SELECT * FROM messages ORDER BY created_at DESC");
$messages = $query->fetchAll(PDO::FETCH_ASSOC);
header('Content-Type: application/json');
echo json_encode($messages);
?>
