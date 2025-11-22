#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import logging
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass
from enum import Enum

class RecordType(Enum):
    KV = "kv"
    COMMENT = "comment"
    INVALID = "invalid"

@dataclass
class ParseResult:
    records: Dict[str, str]  # 有效的KV记录
    comments: List[str]      # 注释记录
    invalid_lines: List[Tuple[int, str]]  # 非法行 (行号, 内容)
    errors: List[str]        # 错误信息

class KVParser:
    """
    KV配置文件解析器
    按照特定语法规则解析键值对配置文件
    """
    
    def __init__(self):
        self.reserved_chars = {'#', ':', '\\'}
        self.escape_char = '\\'
        self.comment_char = '#'
        self.kv_separator = ':'
        
        # 日志收集
        self.comment_collection = []
        self.illegal_collection = []
        self.error_collection = []
        
        # 配置
        self.encoding = 'utf-8'
        self.line_separator = '\n'
    
    def parse_file(self, file_path: str) -> ParseResult:
        """
        解析文件
        """
        try:
            with open(file_path, 'r', encoding=self.encoding) as f:
                content = f.read()
            return self.parse_content(content)
        except UnicodeDecodeError:
            # 尝试其他编码
            try:
                with open(file_path, 'r', encoding='gbk') as f:
                    content = f.read()
                return self.parse_content(content)
            except Exception as e:
                return ParseResult({}, [], [], [f"文件编码错误: {e}"])
        except Exception as e:
            return ParseResult({}, [], [], [f"文件读取错误: {e}"])
    
    def parse_content(self, content: str) -> ParseResult:
        """
        解析文本内容
        """
        # 重置收集器
        self.comment_collection = []
        self.illegal_collection = []
        self.error_collection = []
        
        # 统一换行符
        content = content.replace('\r\n', '\n').replace('\r', '\n')
        lines = content.split('\n')
        
        kv_records = {}
        comments = []
        invalid_lines = []
        
        for line_num, line in enumerate(lines, 1):
            record_type, result = self._parse_line(line, line_num)
            
            if record_type == RecordType.KV:
                key, value = result
                if key in kv_records:
                    self.error_collection.append(f"第{line_num}行: 重复的键 '{key}'，将使用第一个出现的值")
                else:
                    kv_records[key] = value
                    
            elif record_type == RecordType.COMMENT:
                comments.append(result)
                self.comment_collection.append(result)
                
            elif record_type == RecordType.INVALID:
                invalid_lines.append((line_num, result))
                self.illegal_collection.append((line_num, result))
        
        return ParseResult(
            records=kv_records,
            comments=comments,
            invalid_lines=invalid_lines,
            errors=self.error_collection
        )
    
    def _parse_line(self, line: str, line_num: int) -> Tuple[RecordType, any]:
        """
        解析单行
        """
        stripped_line = line.strip()
        
        # 空行
        if not stripped_line:
            return RecordType.COMMENT, ""
        
        # 检查是否为注释行（第一个非空字符是 #）
        first_char = stripped_line[0]
        if first_char == self.comment_char:
            # 检查是否严格以 # 开头（没有前导空格）
            if line.lstrip()[0] == self.comment_char:
                return RecordType.COMMENT, stripped_line
            else:
                return RecordType.INVALID, line
        
        # 尝试解析为KV记录
        return self._parse_kv_line(line, line_num)
    
    def _parse_kv_line(self, line: str, line_num: int) -> Tuple[RecordType, any]:
        """
        解析KV行
        """
        # 在处理层逐个字符扫描
        processing_layer = []
        variable_layer = []
        key_mode = True
        key = None
        value_start_index = 0
        
        i = 0
        while i < len(line):
            char = line[i]
            
            if key_mode:
                # 键处理模式
                if char == self.escape_char and i + 1 < len(line):
                    # 转义字符：忽略当前\，压入下一个字符
                    next_char = line[i + 1]
                    variable_layer.append(next_char)
                    i += 2  # 跳过两个字符
                    continue
                elif char == self.kv_separator:
                    # 找到分隔符，进入值模式
                    key = ''.join(variable_layer)
                    key_mode = False
                    value_start_index = i + 1
                    break
                else:
                    variable_layer.append(char)
                    i += 1
            else:
                # 不应该在键模式外到达这里
                break
        
        if key_mode or key is None:
            # 没有找到有效的分隔符
            self.error_collection.append(f"第{line_num}行: 未找到有效的键值分隔符")
            return RecordType.INVALID, line
        
        # 提取值
        value = line[value_start_index:]
        
        # 验证值
        if not value.strip():
            self.error_collection.append(f"第{line_num}行: 值不能为空或仅为空格")
            return RecordType.INVALID, line
        
        return RecordType.KV, (key, value)
    
    def print_results(self, result: ParseResult):
        """
        打印解析结果
        """
        print("=" * 50)
        print("解析结果")
        print("=" * 50)
        
        print(f"\n有效的KV记录 ({len(result.records)} 个):")
        for key, value in result.records.items():
            print(f"  {key}: {value}")
        
        print(f"\n注释记录 ({len(result.comments)} 个):")
        for comment in result.comments:
            print(f"  {comment}")
        
        print(f"\n非法行 ({len(result.invalid_lines)} 个):")
        for line_num, content in result.invalid_lines:
            print(f"  第{line_num}行: {content}")
        
        print(f"\n错误信息 ({len(result.errors)} 个):")
        for error in result.errors:
            print(f"  {error}")
        
        # 打印收集的日志
        print(f"\n注释收集 ({len(self.comment_collection)} 个):")
        for comment in self.comment_collection:
            print(f"  {comment}")
        
        print(f"\n非法收集 ({len(self.illegal_collection)} 个):")
        for line_num, content in self.illegal_collection:
            print(f"  第{line_num}行: {content}")

def create_test_file():
    """
    创建测试文件，包含各种边界情况
    """
    test_content = """# 正常注释
a:name
A:name
# 带空格的注释（将被视为非法）
 # 这个注释前面有空格
路径:/home/test
adr:C:\\User\\admin

# 转义测试
key\\:with:colon:value1
key\\\\with:backslash:value2
key\\#with:hash:value3

# 重复键测试
database:host1
database:host2

# 空值测试（将被视为非法）
empty_value:
whitespace_value:   

# 视觉欺骗
server\\:port:8080
server:port:9090

# 特殊字符
special\u003Akey:normal_value
unicode_key:\u8def\u5f84

# 混合测试
normal:value
#normal:commented_value
 # spaced:comment:illegal
"""
    
    with open('test_config.kv', 'w', encoding='utf-8') as f:
        f.write(test_content)
    
    print("测试文件 'test_config.kv' 已创建")


if __name__ == "__main__":
    
    # 交互式测试
    print("\n" + "=" * 50)
    print("交互式测试")
    print("=" * 50)
    
    parser = KVParser()
    # result = parser.parse_file('./discuss.txt')
    # create_test_file()
    # result = parser.parse_file('test_config.kv')
    # parser.print_results(result)

    # # 获取结果
    # # print(result.records)      # KV字典
    # print(result.comments)     # 注释列表
    # print(result.errors)       # 错误信息
    while True:
        try:
            line = input("\n输入要解析的行 (输入 'quit' 退出): ")
            if line.lower() == 'quit':
                break
            
            result = parser.parse_content(line)
            if result.records:
                for key, value in result.records.items():
                    print(f"解析成功: 键='{key}', 值='{value}'")
            elif result.comments:
                print(f"解析为注释: {result.comments[0]}")
            elif result.invalid_lines:
                print(f"解析为非法行: {result.invalid_lines[0][1]}")
            else:
                print("无法解析该行")
                
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"解析错误: {e}")