#ifndef KV_PARSER_H
#define KV_PARSER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>

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

// 工具函数
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

#endif // KV_PARSER_H