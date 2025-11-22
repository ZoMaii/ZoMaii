#include "kv_parser.h"
#include <iostream>
#include <string>

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
            for (const auto& [key, value] : result.records) {
                std::cout << "解析成功: 键='" << key << "', 值='" << value << "'" << std::endl;
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