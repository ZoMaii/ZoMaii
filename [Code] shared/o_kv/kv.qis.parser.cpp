#include "kv_parser.h"
#include <iostream>
#include <algorithm>
#include <cctype>

KVParser::KVParser() {
    // 初始化收集器
    comment_collection_.clear();
    illegal_collection_.clear();
    error_collection_.clear();
}

ParseResult KVParser::parseFile(const std::string& file_path) {
    ParseResult result;
    result.clear();
    comment_collection_.clear();
    illegal_collection_.clear();
    error_collection_.clear();
    
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            result.errors.push_back("无法打开文件: " + file_path);
            return result;
        }
        
        // 读取文件内容
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        file.close();
        return parseContent(content);
        
    } catch (const std::exception& e) {
        result.errors.push_back("文件读取错误: " + std::string(e.what()));
        return result;
    }
}

ParseResult KVParser::parseContent(const std::string& content) {
    ParseResult result;
    result.clear();
    
    // 统一换行符
    std::string normalized_content = normalizeLineEndings(content);
    
    // 分割行
    std::vector<std::string> lines = KVUtils::split(normalized_content, '\n');
    
    for (size_t i = 0; i < lines.size(); ++i) {
        int line_num = static_cast<int>(i + 1);
        auto [record_type, record_data] = parseLine(lines[i], line_num);
        
        switch (record_type) {
            case RecordType::KV: {
                const auto& [key, value] = record_data;
                if (result.records.find(key) != result.records.end()) {
                    error_collection_.push_back(
                        "第" + std::to_string(line_num) + "行: 重复的键 '" + key + "'，将使用第一个出现的值"
                    );
                } else {
                    result.records[key] = value;
                }
                break;
            }
            case RecordType::COMMENT: {
                result.comments.push_back(record_data.first);
                comment_collection_.push_back(record_data.first);
                break;
            }
            case RecordType::INVALID: {
                result.invalid_lines.emplace_back(line_num, record_data.first);
                illegal_collection_.emplace_back(line_num, record_data.first);
                break;
            }
        }
    }
    
    result.errors = error_collection_;
    return result;
}

std::pair<RecordType, std::pair<std::string, std::string>> 
KVParser::parseLine(const std::string& line, int line_num) {
    std::string trimmed = KVUtils::trim(line);
    
    // 空行视为注释
    if (trimmed.empty()) {
        return {RecordType::COMMENT, {"", ""}};
    }
    
    // 检查是否为注释行
    if (trimmed[0] == comment_char_) {
        // 验证是否严格以#开头（没有前导空格）
        if (!line.empty() && line[0] == comment_char_) {
            return {RecordType::COMMENT, {trimmed, ""}};
        } else {
            error_collection_.push_back(
                "第" + std::to_string(line_num) + "行: 非法注释格式（前导空格）"
            );
            return {RecordType::INVALID, {line, ""}};
        }
    }
    
    // 尝试解析为KV记录
    return parseKVLine(line, line_num);
}

std::pair<RecordType, std::pair<std::string, std::string>> 
KVParser::parseKVLine(const std::string& line, int line_num) {
    std::vector<char> variable_layer;
    bool key_mode = true;
    std::string key;
    size_t value_start_index = 0;
    
    size_t i = 0;
    while (i < line.length()) {
        char c = line[i];
        
        if (key_mode) {
            if (c == escape_char_ && i + 1 < line.length()) {
                // 转义字符：忽略当前\，压入下一个字符
                char next_char = line[i + 1];
                variable_layer.push_back(next_char);
                i += 2; // 跳过两个字符
                continue;
            } else if (c == kv_separator_) {
                // 找到分隔符，进入值模式
                key = std::string(variable_layer.begin(), variable_layer.end());
                key_mode = false;
                value_start_index = i + 1;
                break;
            } else {
                variable_layer.push_back(c);
                i++;
            }
        } else {
            break;
        }
    }
    
    if (key_mode || key.empty()) {
        error_collection_.push_back(
            "第" + std::to_string(line_num) + "行: 未找到有效的键值分隔符"
        );
        return {RecordType::INVALID, {line, ""}};
    }
    
    // 提取值
    std::string value;
    if (value_start_index < line.length()) {
        value = line.substr(value_start_index);
    }
    
    // 验证值
    if (isEmptyOrWhitespace(value)) {
        error_collection_.push_back(
            "第" + std::to_string(line_num) + "行: 值不能为空或仅为空格"
        );
        return {RecordType::INVALID, {line, ""}};
    }
    
    return {RecordType::KV, {key, value}};
}

std::string KVParser::normalizeLineEndings(const std::string& content) {
    std::string result;
    result.reserve(content.length());
    
    for (size_t i = 0; i < content.length(); ++i) {
        if (content[i] == '\r') {
            if (i + 1 < content.length() && content[i + 1] == '\n') {
                // CRLF -> LF
                result += '\n';
                i++; // 跳过下一个字符
            } else {
                // CR -> LF
                result += '\n';
            }
        } else {
            result += content[i];
        }
    }
    
    return result;
}

bool KVParser::isEmptyOrWhitespace(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char c) {
        return std::isspace(c);
    });
}

bool KVParser::isReservedChar(char c) const {
    return c == '#' || c == ':' || c == '\\';
}

void KVParser::printResults(const ParseResult& result) const {
    std::cout << "==================================================" << std::endl;
    std::cout << "解析结果" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    std::cout << "\n有效的KV记录 (" << result.records.size() << " 个):" << std::endl;
    for (const auto& [key, value] : result.records) {
        std::cout << "  " << key << ": " << value << std::endl;
    }
    
    std::cout << "\n注释记录 (" << result.comments.size() << " 个):" << std::endl;
    for (const auto& comment : result.comments) {
        std::cout << "  " << comment << std::endl;
    }
    
    std::cout << "\n非法行 (" << result.invalid_lines.size() << " 个):" << std::endl;
    for (const auto& [line_num, content] : result.invalid_lines) {
        std::cout << "  第" << line_num << "行: " << content << std::endl;
    }
    
    std::cout << "\n错误信息 (" << result.errors.size() << " 个):" << std::endl;
    for (const auto& error : result.errors) {
        std::cout << "  " << error << std::endl;
    }
    
    std::cout << "\n注释收集 (" << comment_collection_.size() << " 个):" << std::endl;
    for (const auto& comment : comment_collection_) {
        std::cout << "  " << comment << std::endl;
    }
    
    std::cout << "\n非法收集 (" << illegal_collection_.size() << " 个):" << std::endl;
    for (const auto& [line_num, content] : illegal_collection_) {
        std::cout << "  第" << line_num << "行: " << content << std::endl;
    }
}

// KVUtils 实现
void KVUtils::createTestFile(const std::string& filename) {
    std::string test_content = 
        "# 正常注释\n"
        "a:name\n"
        "A:name\n"
        "# 带空格的注释（将被视为非法）\n"
        " # 这个注释前面有空格\n"
        "路径:/home/test\n"
        "adr:C:\\\\User\\\\admin\n"
        "\n"
        "# 转义测试\n"
        "key\\:with:colon:value1\n"
        "key\\\\with:backslash:value2\n"
        "key\\#with:hash:value3\n"
        "\n"
        "# 重复键测试\n"
        "database:host1\n"
        "database:host2\n"
        "\n"
        "# 空值测试（将被视为非法）\n"
        "empty_value:\n"
        "whitespace_value:   \n"
        "\n"
        "# 视觉欺骗\n"
        "server\\:port:8080\n"
        "server:port:9090\n"
        "\n"
        "# 混合测试\n"
        "normal:value\n"
        "#normal:commented_value\n"
        " # spaced:comment:illegal\n";
    
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file << test_content;
        file.close();
        std::cout << "测试文件 '" << filename << "' 已创建" << std::endl;
    } else {
        std::cerr << "无法创建测试文件" << std::endl;
    }
}

std::vector<std::string> KVUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string KVUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

bool KVUtils::startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}