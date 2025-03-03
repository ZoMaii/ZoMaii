<?php
require_once 'db/connection.php';

// 获取 JSON 数据并解码
$data = json_decode(file_get_contents("php://input"), true);

if (isset($data['name']) && isset($data['content'])) {
    $name = htmlspecialchars($data['name'], ENT_QUOTES, 'UTF-8');
    $content = htmlspecialchars($data['content'], ENT_QUOTES, 'UTF-8');

    $query = $db->prepare("INSERT INTO messages (name, content, created_at) VALUES (:name, :content, :created_at)");
    $result = $query->execute([
        ':name' => $name,
        ':content' => $content,
        ':created_at' => date('Y-m-d H:i:s'),
    ]);

    header('Content-Type: application/json');
    if ($result) {
        echo json_encode(['success' => true, 'message' => 'Message added successfully']);
    } else {
        echo json_encode(['success' => false, 'message' => 'Failed to add message']);
    }
} else {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Invalid input']);
}
?>
