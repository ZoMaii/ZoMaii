<?php
require_once 'db/connection.php';

// 获取 JSON 数据并解码
$data = json_decode(file_get_contents("php://input"), true);

if (isset($data['name']) && isset($data['url']) && isset($data['description'])) {
    $name = htmlspecialchars($data['name'], ENT_QUOTES, 'UTF-8');
    $url = htmlspecialchars($data['url'], ENT_QUOTES, 'UTF-8');
    $description = htmlspecialchars($data['description'], ENT_QUOTES, 'UTF-8');

    $query = $db->prepare("INSERT INTO links (name, url, description) VALUES (:name, :url, :description)");
    $result = $query->execute([
        ':name' => $name,
        ':url' => $url,
        ':description' => $description,
    ]);

    header('Content-Type: application/json');
    if ($result) {
        echo json_encode(['success' => true, 'message' => 'Link added successfully']);
    } else {
        echo json_encode(['success' => false, 'message' => 'Failed to add link']);
    }
} else {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Invalid input']);
}
?>
