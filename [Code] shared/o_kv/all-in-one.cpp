#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

enum class RecordType {
    KV,
    COMMENT,
    INVALID
};

struct ParseResult {
    std::unordered_map<std::string, std::string> records;
    std::vector<std::string> comments;
    std::vector<std::pair<int, std::string>> invalid_lines;
    std::vector<std::string> errors;
    
    void clear() {
        records.clear();
        comments.clear();
        invalid_lines.clear();
        errors.clear();
    }
};

class KVUtils {
public:
    // 创建测试文件
    static void createTestFile(const std::string& filename);
    
    // 字符串分割
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    // 去除字符串首尾空白
    static std::string trim(const std::string& str);
    
    // 检查字符串是否以指定前缀开头
    static bool startsWith(const std::string& str, const std::string& prefix);
};

class KVParser {
public:
    KVParser();
    ~KVParser() = default;
    
    // 解析文件
    ParseResult parseFile(const std::string& file_path);
    
    // 解析文本内容
    ParseResult parseContent(const std::string& content);
    
    // 打印解析结果
    void printResults(const ParseResult& result) const;
    
    // 获取收集的日志
    const std::vector<std::string>& getCommentCollection() const { return comment_collection_; }
    const std::vector<std::pair<int, std::string>>& getIllegalCollection() const { return illegal_collection_; }
    const std::vector<std::string>& getErrorCollection() const { return error_collection_; }

private:
    // 解析单行
    std::pair<RecordType, std::pair<std::string, std::string>> parseLine(const std::string& line, int line_num);
    
    // 处理KV行
    std::pair<RecordType, std::pair<std::string, std::string>> parseKVLine(const std::string& line, int line_num);
    
    // 统一换行符
    std::string normalizeLineEndings(const std::string& content);
    
    // 检查字符串是否为空或仅空白
    bool isEmptyOrWhitespace(const std::string& str);
    
    // 字符检查
    bool isReservedChar(char c) const;
    
    // 日志收集
    std::vector<std::string> comment_collection_;
    std::vector<std::pair<int, std::string>> illegal_collection_;
    std::vector<std::string> error_collection_;
    
    // 配置常量
    const char escape_char_ = '\\';
    const char comment_char_ = '#';
    const char kv_separator_ = ':';
    const std::string encoding_ = "utf-8";
};

// KVParser 实现
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
        auto parse_result = parseLine(lines[i], line_num);
        RecordType record_type = parse_result.first;
        auto record_data = parse_result.second;
        
        switch (record_type) {
            case RecordType::KV: {
                const auto& key = record_data.first;
                const auto& value = record_data.second;
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
        return std::make_pair(RecordType::COMMENT, std::make_pair("", ""));
    }
    
    // 检查是否为注释行
    if (trimmed[0] == comment_char_) {
        // 验证是否严格以#开头（没有前导空格）
        if (!line.empty() && line[0] == comment_char_) {
            return std::make_pair(RecordType::COMMENT, std::make_pair(trimmed, ""));
        } else {
            error_collection_.push_back(
                "第" + std::to_string(line_num) + "行: 非法注释格式（前导空格）"
            );
            return std::make_pair(RecordType::INVALID, std::make_pair(line, ""));
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
        return std::make_pair(RecordType::INVALID, std::make_pair(line, ""));
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
        return std::make_pair(RecordType::INVALID, std::make_pair(line, ""));
    }
    
    return std::make_pair(RecordType::KV, std::make_pair(key, value));
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
    for (const auto& kv : result.records) {
        std::cout << "  " << kv.first << ": " << kv.second << std::endl;
    }
    
    std::cout << "\n注释记录 (" << result.comments.size() << " 个):" << std::endl;
    for (const auto& comment : result.comments) {
        std::cout << "  " << comment << std::endl;
    }
    
    std::cout << "\n非法行 (" << result.invalid_lines.size() << " 个):" << std::endl;
    for (const auto& invalid_line : result.invalid_lines) {
        std::cout << "  第" << invalid_line.first << "行: " << invalid_line.second << std::endl;
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
    for (const auto& illegal : illegal_collection_) {
        std::cout << "  第" << illegal.first << "行: " << illegal.second << std::endl;
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

void testParser() {
    std::cout << "创建测试文件..." << std::endl;
    KVUtils::createTestFile("test_config.kv");
    
    std::cout << "\n开始解析测试文件..." << std::endl;
    KVParser parser;
    ParseResult result = parser.parseFile("test_config.kv");
    
    parser.printResults(result);
    
    // 功能验证
    std::cout << "\n==================================================" << std::endl;
    std::cout << "功能验证" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    // 验证重复键处理
    if (result.records.find("database") != result.records.end()) {
        std::cout << "重复键验证: database = " << result.records.at("database") 
                  << " (应使用第一个值 'host1')" << std::endl;
    }
    
    // 验证转义处理
    if (result.records.find("key:with:colon") != result.records.end()) {
        std::cout << "转义验证1: key:with:colon = " 
                  << result.records.at("key:with:colon") << std::endl;
    }
    
    if (result.records.find("key\\with:backslash") != result.records.end()) {
        std::cout << "转义验证2: key\\with:backslash = " 
                  << result.records.at("key\\with:backslash") << std::endl;
    }
    
    // 验证注释处理
    int comment_count = 0;
    for (const auto& comment : result.comments) {
        if (KVUtils::startsWith(comment, "# 正常注释")) {
            comment_count++;
        }
    }
    std::cout << "注释验证: 找到 " << comment_count << " 个正常注释" << std::endl;
}

void interactiveTest() {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "交互式测试" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    KVParser parser;
    std::string line;
    
    while (true) {
        std::cout << "\n输入要解析的行 (输入 'quit' 退出): ";
        std::getline(std::cin, line);
        
        if (line == "quit") {
            break;
        }
        
        ParseResult result = parser.parseContent(line);
        
        if (!result.records.empty()) {
            for (const auto& kv : result.records) {
                std::cout << "解析成功: 键='" << kv.first << "', 值='" << kv.second << "'" << std::endl;
            }
        } else if (!result.comments.empty()) {
            std::cout << "解析为注释: " << result.comments[0] << std::endl;
        } else if (!result.invalid_lines.empty()) {
            std::cout << "解析为非法行: " << result.invalid_lines[0].second << std::endl;
        } else {
            std::cout << "无法解析该行" << std::endl;
        }
    }
}

int main() {
    try {
        testParser();
        interactiveTest();
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}